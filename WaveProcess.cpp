// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04dec12	initial version
        01      25jan13	add FindSample
        02      07feb13	in FindClipping, compensate for selection start
        03      25feb13	add Resample
		04		01mar13	in ExtractChannels, add dual progress
		05		01mar13	in ExtractChannels, include channel index in path
        06      02mar13	move speaker names to base class
		07		04mar13	in FindClipping, add clipping level
		08		07mar13	in GetRMSStats, fix integer overflow on negative rail
		09		13mar13	copy channel mask
		10		20mar13	add GetSpectrum
		11		31mar13	use SetPosEx
		12		04apr13	in GetSpectrum, check Analyze return value
		13		06jun13	add channel selection to peak stats and normalize

		wave processing
 
*/

#include "stdafx.h"
#include "WaveShop.h"
#include "WaveProcess.h"
#include "AsyncJob.h"
#include <math.h>
#include "Benchmark.h"
#include "RingBuf.h"
#include "ProgressDlg.h"
#include "PathStr.h"
#include "SampleRateEx.h"	// for Resample
#include "KissFFT.h"
#include "SpectrumAnal.h"

CWaveProcess::~CWaveProcess()
{
}

CWaveProcess::CWaveParms::CWaveParms(CWaveProcess& Wave)
	: m_Wave(Wave)
{
	ASSERT(!Wave.IsFileOpen());	// processing not allowed on temp file; must be in memory
}

CWaveProcess::CConstWaveParms::CConstWaveParms(const CWaveProcess& Wave)
	: m_Wave(Wave)
{
	ASSERT(!Wave.IsFileOpen());	// processing not allowed on temp file; must be in memory
}

CWaveProcess::CWaveSelParms::CWaveSelParms(CWaveProcess& Wave, const CW64IntRange& Sel) :
	CWaveParms(Wave), m_Selection(Sel)
{
}

CWaveProcess::CConstWaveSelParms::CConstWaveSelParms(const CWaveProcess& Wave, const CW64IntRange& Sel) :
	CConstWaveParms(Wave), m_Selection(Sel)
{
}

double CWaveProcess::SafeLinearToDecibels(double Linear)
{
	if (Linear > 0)	// avoid infinity
		return(CDSPlayer::LinearToDecibels(Linear));
	return(MIN_LEVEL);
}

/////////////////////////////////////////////////////////////////////////////
// GetPeakStats

CWaveProcess::CPeakStatsParms::CPeakStatsParms(const CWaveProcess& Wave, const CW64IntRange& Sel, CPeakStats& Stats, const BYTE *ChanSel) 
	: CConstWaveSelParms(Wave, Sel), m_Stats(Stats)
{
	m_ChanSel = ChanSel;
}

CWaveProcess::CPeakStats::CPeakStats()
{
	ZeroMemory(&m_Total, sizeof(m_Total));
	m_Frames = 0;
	m_NegRail = 0;
	m_PosRail = 0;
}

bool CWaveProcess::GetPeakStats(CAsyncJob& Job, CPeakStatsParms *Parms)
{
	const CWaveProcess&	wave = Parms->m_Wave;
	CW64IntRange	sel = Parms->m_Selection;
	W64INT	frames = sel.Length();
	if (frames <= 0)
		ThrowError(IDS_WPRO_TOO_FEW_FRAMES);
	wave.GetChannelNames(Parms->m_Stats.m_ChanName);
	UINT	chans = wave.GetChannels();
	wave.GetSampleRails(Parms->m_Stats.m_NegRail, Parms->m_Stats.m_PosRail);
	Parms->m_Stats.m_Chan.RemoveAll();
	Parms->m_Stats.m_Chan.SetSize(chans);	// allocate channel stats array
	Parms->m_Stats.m_Frames = frames;
	UINT	iChan;
	for (iChan = 0; iChan < chans; iChan++) {	// for each channel
		CPeakStats::CHAN_STATS&	chst = Parms->m_Stats.m_Chan[iChan];
		chst.Min = INT_MAX;	// init accumulators to limits
		chst.Max = INT_MIN;
	}
	W64INT	Offset = wave.GetByteOffset(0, sel.Start);
	UINT	SampSize = wave.GetSampleSize();
	Job.SetRange(0, frames);
	for (W64INT iFrame = 0; iFrame < frames; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
			CPeakStats::CHAN_STATS&	chst = Parms->m_Stats.m_Chan[iChan];
			SAMPLE	samp = wave.GetSampleAt(Offset);
			if (samp < chst.Min)
				chst.Min = samp;
			if (samp > chst.Max)
				chst.Max = samp;
			chst.Sum += double(samp);
			Offset += SampSize;
		}
	}
	// compute total statistics for all channels
	CPeakStats::CHAN_STATS&	totst = Parms->m_Stats.m_Total;
	totst.Min = INT_MAX;
	totst.Max = INT_MIN;
	totst.Sum = 0;
	const BYTE	*ChanSel = Parms->m_ChanSel;
	for (iChan = 0; iChan < chans; iChan++) {	// for each channel
		if (ChanSel == NULL || ChanSel[iChan]) {	// if channel selected
			const CPeakStats::CHAN_STATS&	chst = Parms->m_Stats.m_Chan[iChan];
			if (chst.Min < totst.Min)
				totst.Min = chst.Min;
			if (chst.Max > totst.Max)
				totst.Max = chst.Max;
			totst.Sum += chst.Sum;
		}
	}
	return(TRUE);
}

bool CWaveProcess::GetPeakStats(const CW64IntRange& Sel, CPeakStats& Stats, const BYTE *ChanSel) const
{
	CTypedAsyncJob<CPeakStatsParms>	job;
	CPeakStatsParms	parms(*this, Sel, Stats, ChanSel);
	return(job.StartJob(GetPeakStats, &parms, LDS(IDS_WPRO_PEAK_STATS)));
}

CWaveProcess::CConvert::CConvert()
{
	m_NegRail = 0;
	m_PosRail = 0;
}

void CWaveProcess::CConvert::Create(const CWave& Wave)
{
	Wave.GetSampleRails(m_NegRail, m_PosRail);
}

double CWaveProcess::CConvert::SampleToNorm(double Sample) const
{
	double	rail = Sample < 0 ? -double(m_NegRail) : double(m_PosRail);
	ASSERT(rail != 0);	// otherwise divide by zero
	return(Sample / rail);
}

double CWaveProcess::CConvert::NormToSample(double Norm) const
{
	double	rail = Norm < 0 ? -double(m_NegRail) : double(m_PosRail);
	return(Norm * rail);
}

double CWaveProcess::CConvert::UnitToSample(double Val, int Unit) const
{
	double	samp;
	switch (Unit) {
	case VALUE:
		samp = Val;
		break;
	case PERCENT:
		samp = NormToSample(Val / 100);
		break;
	case DECIBELS:
		samp = NormToSample(CDSPlayer::DecibelsToLinear(Val));
		break;
	default:
		NODEFAULTCASE;
		samp = 0;
	}
	return(samp);
}

double CWaveProcess::CConvert::SampleToUnit(double Sample, int Unit) const
{
	double	val;
	switch (Unit) {
	case VALUE:
		val = Sample;
		break;
	case PERCENT:
		val = SampleToNorm(Sample) * 100;
		break;
	case DECIBELS:
		val = SafeLinearToDecibels(SampleToNorm(Sample));
		break;
	default:
		NODEFAULTCASE;
		val = 0;
	}
	return(val);
}

double CWaveProcess::CPeakStats::GetNormMin(int ChanIdx) const
{
	return(SampleToNorm(double(m_Chan[ChanIdx].Min)));
}

double CWaveProcess::CPeakStats::GetNormMax(int ChanIdx) const
{
	return(SampleToNorm(double(m_Chan[ChanIdx].Max)));
}

double CWaveProcess::CPeakStats::GetBias(int ChanIdx) const
{
	return(m_Chan[ChanIdx].Sum / m_Frames);
}

double CWaveProcess::CPeakStats::GetNormBias(int ChanIdx) const
{
	return(SampleToNorm(GetBias(ChanIdx)));
}

double CWaveProcess::CPeakStats::GetPeakDecibels(int ChanIdx) const
{
	double	peak = max(fabs(GetNormMin(ChanIdx)), fabs(GetNormMax(ChanIdx)));
	return(CDSPlayer::LinearToDecibels(max(peak, 1e-3)));
}

double CWaveProcess::CPeakStats::GetNormMin() const
{
	return(SampleToNorm(double(m_Total.Min)));
}

double CWaveProcess::CPeakStats::GetNormMax() const
{
	return(SampleToNorm(double(m_Total.Max)));
}

double CWaveProcess::CPeakStats::GetPeakDecibels() const
{
	double	peak = max(fabs(GetNormMin()), fabs(GetNormMax()));
	return(CDSPlayer::LinearToDecibels(max(peak, 1e-3)));
}

/////////////////////////////////////////////////////////////////////////////
// Normalize

CWaveProcess::CNormalizeParms::CNormalizeParms(CWaveProcess& Wave, const CW64IntRange& Sel, const CPeakStats& PeakStats, UINT Flags, double PeakLevel, const BYTE *ChanSel) 
	: CWaveSelParms(Wave, Sel), m_PeakStats(PeakStats)
{
	m_Flags = Flags;
	m_PeakLevel = PeakLevel;
	m_ChanSel = ChanSel;
}

bool CWaveProcess::Normalize(CAsyncJob& Job, CNormalizeParms *Parms)
{
	CWaveProcess&	wave = Parms->m_Wave;
	CW64IntRange	sel = Parms->m_Selection;
	W64INT	frames = sel.Length();
	if (frames <= 0)	// if not enough frames
		ThrowError(IDS_WPRO_TOO_FEW_FRAMES);
	UINT	chans = wave.GetChannels();
	CNormalizeInfoArray	NormInfo;
	NormInfo.SetSize(chans);	// allocate normalization info array
	UINT	iChan;
	for (iChan = 0; iChan < chans; iChan++) {	// for each channel
		const CPeakStats::CHAN_STATS& chst = Parms->m_PeakStats.m_Chan[iChan];
		NORMALIZE_INFO& info = NormInfo[iChan];
		double	Bias, MinVal, MaxVal;
		if (Parms->m_Flags & NORM_UNBIAS)	// if fixing DC bias
			Bias = chst.Sum / frames;	// bias is average of samples
		else
			Bias = 0;
		MinVal = fabs(double(chst.Min) - Bias);
		MaxVal = fabs(double(chst.Max) - Bias);
		info.MaxMag = max(MinVal, MaxVal);
		info.Bias = Bias;
	}
	// unless normalizing channels independently
	if (!(Parms->m_Flags & NORM_INDEPENDENT)) {
		// compute maximum magnitude for all channels
		double	GlobMaxMag = 0;
		const BYTE	*ChanSel = Parms->m_ChanSel;
		for (iChan = 0; iChan < chans; iChan++) {	// for each channel
			if (ChanSel == NULL || ChanSel[iChan]) {	// if channel selected
				NORMALIZE_INFO& info = NormInfo[iChan];
				if (info.MaxMag > GlobMaxMag)
					GlobMaxMag = info.MaxMag;
			}
		}
		for (iChan = 0; iChan < chans; iChan++)	// for each channel
			NormInfo[iChan].MaxMag = GlobMaxMag;	// set maximum magnitude
	}
	double	PeakLinear = CDSPlayer::DecibelsToLinear(Parms->m_PeakLevel);
	SAMPLE	NegRail, PosRail;
	wave.GetSampleRails(NegRail, PosRail);
	for (iChan = 0; iChan < chans; iChan++) {	// for each channel
		NORMALIZE_INFO& info = NormInfo[iChan];
		if (Parms->m_Flags & NORM_NORMALIZE)	// if normalizing
			info.Scale = double(PosRail) / info.MaxMag * PeakLinear;
		else	// not normalizing
			info.Scale = 1;
	}
	W64INT	Offset = wave.GetByteOffset(0, sel.Start);
	UINT	SampSize = wave.GetSampleSize();
	Job.SetRange(0, frames);
	for (W64INT iFrame = 0; iFrame < frames; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
			const NORMALIZE_INFO& info = NormInfo[iChan];
			SAMPLE	samp = wave.GetSampleAt(Offset);
			samp = round((double(samp) - info.Bias) * info.Scale);
			wave.SetSampleAt(Offset, samp);
			Offset += SampSize;
		}
	}
	return(TRUE);
}

bool CWaveProcess::Normalize(const CW64IntRange& Sel, UINT Flags, double PeakLevel, const BYTE *ChanSel)
{
	CPeakStats	Stats;
	if (!GetPeakStats(Sel, Stats, ChanSel))	// compute peak statistics first
		return(FALSE);
	CTypedAsyncJob<CNormalizeParms>	job;
	CNormalizeParms	parms(*this, Sel, Stats, Flags, PeakLevel, ChanSel);
	return(job.StartJob(Normalize, &parms, LDS(IDS_WPRO_NORMALIZE)));
}

/////////////////////////////////////////////////////////////////////////////
// Reverse

bool CWaveProcess::Reverse(CAsyncJob& Job, CWaveSelParms *Parms)
{
	CWaveProcess&	wave = Parms->m_Wave;
	CW64IntRange	sel = Parms->m_Selection;
	W64INT	frames = sel.Length();
	W64INT	mid = frames / 2;
	if (mid <= 0)	// if not enough frames
		ThrowError(IDS_WPRO_TOO_FEW_FRAMES);
	UINT	chans = wave.GetChannels();
	Job.SetRange(0, mid);
	for (W64INT iFrame = 0; iFrame < mid; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
			W64INT	iStart = sel.Start + iFrame;
			W64INT	iEnd = sel.End - 1 - iFrame;
			SAMPLE	a = wave.GetSample(iChan, iStart);
			SAMPLE	b = wave.GetSample(iChan, iEnd);
			wave.SetSample(iChan, iStart, b);
			wave.SetSample(iChan, iEnd, a);
		}
	}
	return(TRUE);
}

bool CWaveProcess::Reverse(const CW64IntRange& Sel)
{
	CTypedAsyncJob<CWaveSelParms>	job;
	CWaveSelParms	parms(*this, Sel);
	return(job.StartJob(Reverse, &parms, LDS(IDS_WPRO_REVERSE)));
}

/////////////////////////////////////////////////////////////////////////////
// Invert

bool CWaveProcess::Invert(CAsyncJob& Job, CWaveSelParms *Parms)
{
	CWaveProcess&	wave = Parms->m_Wave;
	CW64IntRange	sel = Parms->m_Selection;
	W64INT	frames = sel.Length();
	UINT	chans = wave.GetChannels();
	W64INT	offset = wave.GetByteOffset(0, sel.Start);
	UINT	SampleSize = wave.GetSampleSize();
	for (W64INT iFrame = 0; iFrame < frames; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
			SAMPLE	samp = wave.GetSampleAt(offset);
			samp = -samp;	// invert sample
			wave.SetSampleAt(offset, samp);
			offset += SampleSize;
		}
	}
	return(TRUE);
}

bool CWaveProcess::Invert(const CW64IntRange& Sel)
{
	CTypedAsyncJob<CWaveSelParms>	job;
	CWaveSelParms	parms(*this, Sel);
	return(job.StartJob(Invert, &parms, LDS(IDS_WPRO_INVERT)));
}

/////////////////////////////////////////////////////////////////////////////
// GetRMSStats

CWaveProcess::CRMSStatsParms::CRMSStatsParms(const CWaveProcess& Wave, const CW64IntRange& Sel, const CPeakStats& PeakStats, CRMSStats& RMSStats, const RMS_STATS_PARMS& UserParms)
	: CConstWaveSelParms(Wave, Sel), m_RMSStats(RMSStats), m_PeakStats(PeakStats), m_UserParms(UserParms)
{
	m_RMSStats.m_HistoRange.SetEmpty();
}

bool CWaveProcess::GetRMSStats(CAsyncJob& Job, CRMSStatsParms *Parms)
{
	const CWaveProcess&	wave = Parms->m_Wave;
	wave.GetChannelNames(Parms->m_RMSStats.m_ChanName);
	CW64IntRange	sel = Parms->m_Selection;
	const RMS_STATS_PARMS&	UserParms = Parms->m_UserParms;
	W64INT	WaveOffset = wave.GetByteOffset(0, sel.Start);
	UINT	SampSize = wave.GetSampleSize();
	W64INT frames = sel.Length();
	UINT	chans = wave.GetChannels();
	SAMPLE	NegRail, PosRail;
	wave.GetSampleRails(NegRail, PosRail);
	double	rail = -double(NegRail);	// convert to real first to avoid overflow
	int	PaneFrames = round(wave.GetSampleRate() 
		* UserParms.WindowSize / UserParms.PanesPerWindow);
	if (PaneFrames < 1)	// if invalid pane size
		wave.ThrowError(IDS_WPRO_RMS_BAD_PANE_SIZE);
	int	WindowFrames = PaneFrames * UserParms.PanesPerWindow;
	W64INT	panes = (frames - 1) / PaneFrames + 1;
	CRMSStats::CChanInfoArray&	ChanInfo = Parms->m_RMSStats.m_Chan;
	ChanInfo.RemoveAll();
	ChanInfo.SetSize(chans);	// allocate per-channel information array
	CDblArray	PaneSum;
	PaneSum.SetSize(chans);	// allocate pane sum array
	CRingBuf<CDblArray>	PaneRing(UserParms.PanesPerWindow);
	int	HistoBins = UserParms.HistogramRange * UserParms.HistogramSubdiv;
	UINT	iChan;
	for (iChan = 0; iChan < chans; iChan++) {	// for each channel
		CRMSStats::CChanInfo&	info = ChanInfo[iChan];
		info.m_Min = DBL_MAX;
		info.m_Max = -DBL_MAX;	// most negative value
		if (UserParms.Flags & RMS_ACCOUNT_FOR_DC)	// if accounting for DC offset
			info.m_DCBias = Parms->m_PeakStats.m_Chan[iChan].Sum / frames;
		info.m_HistoBin.SetSize(HistoBins + 1);	// allocate histogram bins
	}
	double	RMSOffset;
	if (Parms->m_UserParms.Flags & RMS_0_DB_FS_SQUARE)	// if full scale square
		RMSOffset = 0;	// no offset
	else	// full scale sine
		RMSOffset = 3.01;	// offset in dB
	CDblArray	OldPaneSum;
	CIntRange	HistoRange(INT_MAX, INT_MIN);
	W64INT	RemainFrames = frames;
	W64INT	ActualWindowFrames = 0;
	Job.SetRange(0, panes);
	for (int iPane = 0; iPane < panes; iPane++) {	// for each sliding window pane
		if (Job.SetPosEx(iPane))	// if aborted
			return(FALSE);
		for (iChan = 0; iChan < chans; iChan++)	// for each channel
			PaneSum[iChan] = 0;	// reset pane accumulator
		W64INT	ActualPaneFrames = min(PaneFrames, RemainFrames);
		for (W64INT iFrame = 0; iFrame < ActualPaneFrames; iFrame++) {	// for each frame
			for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
				SAMPLE	samp = wave.GetSampleAt(WaveOffset);
				double	rSamp = double(samp) - ChanInfo[iChan].m_DCBias;
				PaneSum[iChan] += rSamp * rSamp;
				WaveOffset += SampSize;
			}
		}
		RemainFrames -= ActualPaneFrames;
		if (PaneRing.GetCount() >= PaneRing.GetSize()) {	// if pane ring is full
			PaneRing.Pop(OldPaneSum);	// pop oldest pane sums
			// subtract oldest pane sums from window sums
			for (UINT iChan = 0; iChan < chans; iChan++)	// for each channel
				ChanInfo[iChan].m_WindowSum -= OldPaneSum[iChan];
			ActualWindowFrames -= PaneFrames;	// decrement window frame count
		}
		PaneRing.PushOver(PaneSum);
		ActualWindowFrames += ActualPaneFrames;	// increment window frame count
		for (iChan = 0; iChan < chans; iChan++) {	// for each channel
			CRMSStats::CChanInfo&	info = ChanInfo[iChan];
			double	sum = PaneSum[iChan];
			info.m_WindowSum += sum;	// add latest pane sum to window sum
			info.m_TotalSum += sum;	// add latest pane sum to total sum also
			if (ActualWindowFrames == WindowFrames) {	// if full window
				// compute window RMS from current window sum
				double	WindowRMS = sqrt(info.m_WindowSum 
					/ double(ActualWindowFrames)) / rail;
				if (WindowRMS < info.m_Min)	// update minimum
					info.m_Min = WindowRMS;
				if (WindowRMS > info.m_Max)	// update maximum
					info.m_Max = WindowRMS;
				info.m_AvgSum += WindowRMS;	// update average sum and count
				info.m_AvgCount++;
				// convert window RMS to decibels and compute real histogram bin index
				double	fBin = (SafeLinearToDecibels(WindowRMS) + RMSOffset)
					* UserParms.HistogramSubdiv + HistoBins;
				double	BinInt;	// separate real bin index into integer and fraction
				double	BinFrac = modf(fBin, &BinInt);
				int	iBin = round(BinInt);	// integer index of first adjacent bin
				if (iBin >= 0 && iBin < HistoBins) {	// if within array bounds
					// divide increment between pair of adjacent bins; each bin gets
					// a share proportional to its proximity to real bin index (fBin)
					info.m_HistoBin[iBin] += 1 - BinFrac;
					info.m_HistoBin[iBin + 1] += BinFrac;
					if (iBin < HistoRange.Start)	// update range of bins used
						HistoRange.Start = iBin;
					if (iBin + 1 > HistoRange.End)
						HistoRange.End = iBin + 1;
				}
			}
		}
	}
	// finalize statistics
	for (iChan = 0; iChan < chans; iChan++) {	// for each channel
		CRMSStats::CChanInfo&	info = ChanInfo[iChan];
		info.m_Min = SafeLinearToDecibels(info.m_Min) + RMSOffset;
		info.m_Max = SafeLinearToDecibels(info.m_Max) + RMSOffset;
		if (info.m_AvgCount)	// avoid potential divide by zero
			info.m_Avg = SafeLinearToDecibels(info.m_AvgSum / info.m_AvgCount) + RMSOffset;
		double	TotalSum = sqrt(info.m_TotalSum / frames) / rail;
		info.m_Total = SafeLinearToDecibels(TotalSum) + RMSOffset;
	}
	if (HistoRange.IsNormalized())	// if any bins were used
		Parms->m_RMSStats.m_HistoRange = HistoRange;	// store histogram range
	return(TRUE);
}

bool CWaveProcess::GetRMSStats(const CW64IntRange& Sel, CRMSStats& RMSStats, const RMS_STATS_PARMS& UserParms, CPeakStats *PeakStats) const
{
	CPeakStats	MyPeakStats;
	if (UserParms.Flags & RMS_ACCOUNT_FOR_DC) {	// if accounting for DC offset
		if (PeakStats == NULL) {	// if peak statistics not provided by caller
			PeakStats = &MyPeakStats;
			if (!GetPeakStats(Sel, MyPeakStats))	// compute peak statistics first
				return(FALSE);
		}
	}
	CTypedAsyncJob<CRMSStatsParms>	job;
	CRMSStatsParms	parms(*this, Sel, *PeakStats, RMSStats, UserParms);
	return(job.StartJob(GetRMSStats, &parms, LDS(IDS_WPRO_RMS_STATS)));
}

/////////////////////////////////////////////////////////////////////////////
// SwapChannels

CWaveProcess::CSwapChannelsParms::CSwapChannelsParms(CWaveProcess& Wave, const CW64IntRange& Sel, const CUIntRange& Pair)
	: CWaveSelParms(Wave, Sel), m_Pair(Pair)
{
}

bool CWaveProcess::SwapChannels(CAsyncJob& Job, CSwapChannelsParms *Parms)
{
	CWaveProcess&	wave = Parms->m_Wave;
	CW64IntRange	sel = Parms->m_Selection;
	CUIntRange	pair(Parms->m_Pair);
	if (wave.GetChannels() < 2)
		ThrowError(IDS_WPRO_TOO_FEW_CHANNELS);
	UINT	chans = wave.GetChannels();
	if (pair.Start >= chans || pair.End >= chans)	// if channel index out of range
		ThrowError(IDS_WPRO_BAD_CHANNEL);
	Job.SetRange(sel.Start, sel.End);
	pair.Normalize();	// make sure channel indices are in ascending order
	W64INT	OffsetA = wave.GetByteOffset(pair.Start, sel.Start);
	UINT	DeltaChan = (pair.End - pair.Start) * wave.GetSampleSize();
	UINT	FrameSize = wave.GetFrameSize();
	for (W64INT iFrame = sel.Start; iFrame < sel.End; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		W64INT	OffsetB = OffsetA + DeltaChan;
		SAMPLE	tmp = wave.GetSampleAt(OffsetA);
		wave.SetSampleAt(OffsetA, wave.GetSampleAt(OffsetB));
		wave.SetSampleAt(OffsetB, tmp);
		OffsetA += FrameSize;
	}
	return(TRUE);
}

bool CWaveProcess::SwapChannels(const CW64IntRange& Sel, const CUIntRange& Channels)
{
	CTypedAsyncJob<CSwapChannelsParms>	job;
	CSwapChannelsParms	parms(*this, Sel, Channels);
	return(job.StartJob(SwapChannels, &parms, LDS(IDS_WPRO_SWAP_CHANNELS)));
}

/////////////////////////////////////////////////////////////////////////////
// Amplify

CWaveProcess::CAmplifyParms::CAmplifyParms(CWaveProcess& Wave, const CW64IntRange& Sel, double Gain)
	: CWaveSelParms(Wave, Sel)
{
	m_Gain = Gain;
}

bool CWaveProcess::Amplify(CAsyncJob& Job, CAmplifyParms *Parms)
{
	CWaveProcess&	wave = Parms->m_Wave;
	CW64IntRange	sel = Parms->m_Selection;
	W64INT	frames = sel.Length();
	Job.SetRange(0, frames);
	UINT	chans = wave.GetChannels();
	UINT	SampSize = wave.GetSampleSize();
	SAMPLE	NegRail, PosRail;
	wave.GetSampleRails(NegRail, PosRail);
	W64INT	Offset = wave.GetByteOffset(0, sel.Start);
	double	LinearGain = CDSPlayer::DecibelsToLinear(Parms->m_Gain);
	for (W64INT iFrame = 0; iFrame < frames; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
			double	rSamp = double(wave.GetSampleAt(Offset));
			rSamp *= LinearGain;
			rSamp = CLAMP(rSamp, NegRail, PosRail);
			wave.SetSampleAt(Offset, round(rSamp));
			Offset += SampSize;
		}
	}
	return(TRUE);
}

bool CWaveProcess::Amplify(const CW64IntRange& Sel, double Gain)
{
	CTypedAsyncJob<CAmplifyParms>	job;
	CAmplifyParms	parms(*this, Sel, Gain);
	return(job.StartJob(Amplify, &parms, LDS(IDS_WPRO_AMPLIFY)));
}

/////////////////////////////////////////////////////////////////////////////
// Fade

CWaveProcess::CFadeParms::CFadeParms(CWaveProcess& Wave, const CW64IntRange& Sel, const CDblRange& Range, bool IsLog)
	: CWaveSelParms(Wave, Sel), m_Range(Range)
{
	m_IsLog = IsLog;
}

bool CWaveProcess::Fade(CAsyncJob& Job, CFadeParms *Parms)
{
	CWaveProcess&	wave = Parms->m_Wave;
	CW64IntRange	sel = Parms->m_Selection;
	W64INT	frames = sel.Length();
	Job.SetRange(0, frames);
	UINT	chans = wave.GetChannels();
	UINT	SampSize = wave.GetSampleSize();
	SAMPLE	NegRail, PosRail;
	wave.GetSampleRails(NegRail, PosRail);
	W64INT	Offset = wave.GetByteOffset(0, sel.Start);
	CDblRange	range(Parms->m_Range);
	bool	IsLog = Parms->m_IsLog;
	if (!IsLog) {	// if linear fade
		range.Start = CDSPlayer::DecibelsToLinear(range.Start);
		range.End = CDSPlayer::DecibelsToLinear(range.End);
	}
	double	delta = (range.End - range.Start) / frames;
	for (W64INT iFrame = 0; iFrame < frames; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
			double	rSamp = double(wave.GetSampleAt(Offset));
			double	atten = range.Start + delta * iFrame;
			if (IsLog)	// if logarithmic fade
				atten = CDSPlayer::DecibelsToLinear(atten);
			rSamp *= atten;
			rSamp = CLAMP(rSamp, NegRail, PosRail);
			wave.SetSampleAt(Offset, round(rSamp));
			Offset += SampSize;
		}
	}
	return(TRUE);
}

bool CWaveProcess::Fade(const CW64IntRange& Sel, const CDblRange& Range, bool IsLog)
{
	CTypedAsyncJob<CFadeParms>	job;
	CFadeParms	parms(*this, Sel, Range, IsLog);
	return(job.StartJob(Fade, &parms, LDS(IDS_WPRO_FADE)));
}

/////////////////////////////////////////////////////////////////////////////
// CopyChannel

CWaveProcess::CCopyChannelParms::CCopyChannelParms(const CWaveProcess& Wave, UINT ChannelIdx, CWaveEdit& DestinationWave)
	: CConstWaveParms(Wave), m_DestinationWave(DestinationWave)
{
	m_ChannelIdx = ChannelIdx;
}

bool CWaveProcess::CopyChannel(CAsyncJob& Job, CCopyChannelParms *Parms)
{
	const CWaveProcess&	wave = Parms->m_Wave;
	UINT	ChannelIdx = Parms->m_ChannelIdx;
	CWaveEdit&	DstWave = Parms->m_DestinationWave;
	UINT	chans = wave.GetChannels();
	if (ChannelIdx >= chans)	// if channel index out of range
		ThrowError(IDS_WPRO_BAD_CHANNEL);
	// create destination wave; same format as source but mono
	DstWave.SetFormat(1, wave.GetSampleRate(), wave.GetSampleBits());
	W64INT	frames = wave.GetFrameCount();
	DstWave.SetFrameCount(frames);	// allocate destination wave's data
	W64INT	SrcOfs = wave.GetByteOffset(ChannelIdx, 0);
	W64INT	DstOfs = 0;
	UINT	SrcSize = wave.GetFrameSize();
	UINT	DstSize = DstWave.GetFrameSize();
	Job.SetRange(0, frames);
	for (W64INT iFrame = 0; iFrame < frames; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		DstWave.SetSampleAt(DstOfs, wave.GetSampleAt(SrcOfs));	// copy sample
		SrcOfs += SrcSize;	// increment source offset
		DstOfs += DstSize;	// increment destination offset
	}
	return(TRUE);
}

bool CWaveProcess::CopyChannel(UINT ChannelIdx, CWaveEdit& Wave) const
{
	CTypedAsyncJob<CCopyChannelParms>	job;
	CCopyChannelParms	parms(*this, ChannelIdx, Wave);
	return(job.StartJob(CopyChannel, &parms, LDS(IDS_WPRO_COPY_CHANNEL)));
}

/////////////////////////////////////////////////////////////////////////////
// InsertChannel

CWaveProcess::CInsertChannelParms::CInsertChannelParms(CWaveProcess& Wave, UINT ChannelIdx, const CWaveEdit& InsertWave)
	: CWaveParms(Wave), m_InsertWave(InsertWave)
{
	m_ChannelIdx = ChannelIdx;
}

bool CWaveProcess::InsertChannel(CAsyncJob& Job, CInsertChannelParms *Parms)
{
	CWaveProcess&	wave = Parms->m_Wave;
	UINT	ChannelIdx = Parms->m_ChannelIdx;
	const CWaveEdit& InsWave = Parms->m_InsertWave;
	if (ChannelIdx > wave.GetChannels())	// if bad channel index; append is supported
		ThrowError(IDS_WPRO_BAD_CHANNEL);
	if (!InsWave.IsValid())	// if insert wave not valid
		ThrowError(IDS_WPRO_INVALID_WAVE);
	UINT	SampleRate, SampleBits;
	if (wave.IsValid()) {	// if we're valid
		if (InsWave.GetSampleSize() != wave.GetSampleSize())	// if sample size mismatch
			ThrowError(IDS_WEDT_INCOMPATIBLE_FORMATS);
		// destination has same format as source except more channels
		SampleRate = wave.GetSampleRate();
		SampleBits = wave.GetSampleBits();
	} else {	// we're invalid; get format from insert wave
		SampleRate = InsWave.GetSampleRate();
		SampleBits = InsWave.GetSampleBits();
	}
	UINT	DstChans = wave.GetChannels() + InsWave.GetChannels();	// multi-channel insert is supported
	// create destination wave
	CWave	DstWave(DstChans, SampleRate, SampleBits, wave.GetChannelMask());	// copy channel mask
	W64INT	SrcFrames = wave.GetFrameCount();
	W64INT	InsFrames = InsWave.GetFrameCount();
	W64INT	DstFrames = max(SrcFrames, InsFrames);	// insert wave can have more frames than source
	DstWave.SetFrameCount(DstFrames);	// allocate destination wave's data
	W64INT	SrcOfs = 0;
	W64INT	InsOfs = 0;
	W64INT	DstOfs = 0;
	UINT	SampSize = DstWave.GetSampleSize();
	CUIntRange	InsRange(ChannelIdx, ChannelIdx + InsWave.GetChannels());
	Job.SetRange(0, DstFrames);
	for (W64INT iFrame = 0; iFrame < DstFrames; iFrame++) {	// for each destination frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		for (UINT iChan = 0; iChan < DstChans; iChan++) {	// for each destination channel
			SAMPLE	samp;
			if (InsRange.InRange(iChan)) {	// if insert channel
				if (iFrame < InsFrames) {	// if insert frames remain
					samp = InsWave.GetSampleAt(InsOfs);	// load insert sample
					InsOfs += SampSize;	// increment insert offset
				} else	// not insert channel
					samp = 0;	// fill with silence
			} else {	// source channel
				if (iFrame < SrcFrames) {	// if source frames remain
					samp = wave.GetSampleAt(SrcOfs);	// load source sample
					SrcOfs += SampSize;	// increment source offset
				} else	// not source channel
					samp = 0;	// fill with silence
			}
			DstWave.SetSampleAt(DstOfs, samp);	// store sample to destination
			DstOfs += SampSize;	// increment destination offset
		}
	}
	wave.CWave::Copy(DstWave);	// replace source with destination
	return(TRUE);
}

bool CWaveProcess::InsertChannel(UINT ChannelIdx, const CWaveEdit& Wave)
{
	CTypedAsyncJob<CInsertChannelParms>	job;
	CInsertChannelParms	parms(*this, ChannelIdx, Wave);
	return(job.StartJob(InsertChannel, &parms, LDS(IDS_WPRO_INSERT_CHANNEL)));
}

/////////////////////////////////////////////////////////////////////////////
// DeleteChannel

CWaveProcess::CDeleteChannelParms::CDeleteChannelParms(CWaveProcess& Wave, UINT ChannelIdx)
	: CWaveParms(Wave)
{
	m_ChannelIdx = ChannelIdx;
}

bool CWaveProcess::DeleteChannel(CAsyncJob& Job, CDeleteChannelParms *Parms)
{
	CWaveProcess&	wave = Parms->m_Wave;
	UINT	ChannelIdx = Parms->m_ChannelIdx;
	UINT	SrcChans = wave.GetChannels();
	if (ChannelIdx >= SrcChans)	// if channel index out of range
		ThrowError(IDS_WPRO_BAD_CHANNEL);
	if (SrcChans == 1) {	// if we only have one channel
		wave.Init();	// reinitialize wave to default state
		return(TRUE);
	}
	// create destination wave; same format as source except one less channel
	CWave	DstWave(SrcChans - 1, wave.GetSampleRate(), wave.GetSampleBits(), 
		wave.GetChannelMask());	// copy channel mask
	W64INT	frames = wave.GetFrameCount();
	DstWave.SetFrameCount(frames);	// allocate destination wave's data
	W64INT	SrcOfs = 0;
	W64INT	DstOfs = 0;
	UINT	SampSize = wave.GetSampleSize();
	Job.SetRange(0, frames);
	for (W64INT iFrame = 0; iFrame < frames; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		for (UINT iChan = 0; iChan < SrcChans; iChan++) {	// for each source channel
			if (iChan != ChannelIdx) {	// if not deleted channel
				DstWave.SetSampleAt(DstOfs, wave.GetSampleAt(SrcOfs));	// copy sample
				DstOfs += SampSize;	// increment destination offset
			}
			SrcOfs += SampSize;	// increment source offset
		}
	}
	wave.CWave::Copy(DstWave);	// replace source with destination
	return(TRUE);
}

bool CWaveProcess::DeleteChannel(UINT ChannelIdx)
{
	CTypedAsyncJob<CDeleteChannelParms>	job;
	CDeleteChannelParms	parms(*this, ChannelIdx);
	return(job.StartJob(DeleteChannel, &parms, LDS(IDS_WPRO_DELETE_CHANNEL)));
}

/////////////////////////////////////////////////////////////////////////////
// ExtractChannels

CString	CWaveProcess::MakeChannelPath(const CString& BasePath, const CStringArray& ChannelName, int ChanIdx)
{
	CString	ChanIdxStr;
	ChanIdxStr.Format(_T(" %02d "), ChanIdx);	// convert channel index to string
	CString	ChanPath(BasePath + ChanIdxStr + ChannelName[ChanIdx] + WAV_EXT);
	return(ChanPath);
}

bool CWaveProcess::ExtractChannels(LPCTSTR Path) const
{
	CPathStr	BasePath(Path);
	BasePath.RemoveExtension();
	UINT	chans = GetChannels();
	CStringArray	ChannelName;
	GetChannelNames(ChannelName);
	CString	ChanPath = MakeChannelPath(BasePath, ChannelName, 0);
	if (PathFileExists(ChanPath)) {	// if first channel's path already exists
		CString	msg;
		AfxFormatString1(msg, IDS_CLOBBER_CHECK, ChanPath);
		if (AfxMessageBox(msg, MB_YESNO | MB_DEFBUTTON2) != IDYES)
			return(FALSE);	// user chickened out
	}
	CProgressDlg	ProgDlg(IDD_PROGRESS_DUAL);	// allow nested progress
	if (!ProgDlg.Create())	// create progress dialog
		AfxThrowResourceException();
	ProgDlg.SetWindowText(LDS(IDS_WPRO_EXTRACT_CHANNELS));
	ProgDlg.SetRange(0, chans);
	for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
		ProgDlg.SetPos(iChan);
		CWaveEdit	DstWave;
		if (!CopyChannel(iChan, DstWave))	// copy channel to destination wave
			return(FALSE);
		CString	ChanPath = MakeChannelPath(BasePath, ChannelName, iChan);
		if (!DstWave.ProgressWrite(ChanPath)) {	// if write failed or canceled
			DeleteFile(ChanPath);	// delete incomplete file
			return(FALSE);
		}
	}
	return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// FindClipping

CWaveProcess::CFindClippingParms::CFindClippingParms(const CWaveProcess& Wave, const CW64IntRange& Sel, const CLIP_THRESHOLD& Threshold, CClipSpanArray& ClipSpan)
	: CConstWaveSelParms(Wave, Sel), m_ClipSpan(ClipSpan)
{
	m_Threshold = Threshold;
}

bool CWaveProcess::FindClipping(CAsyncJob& Job, CFindClippingParms *Parms)
{
	const CWaveProcess&	wave = Parms->m_Wave;
	CW64IntRange	sel = Parms->m_Selection;
	CLIP_THRESHOLD	threshold = Parms->m_Threshold;
	CClipSpanArray&	ClipSpan = Parms->m_ClipSpan;
	ClipSpan.RemoveAll();
	if (!(threshold.Start && threshold.Stop))
		AfxThrowNotSupportedException();	// both thresholds must be non-zero
	W64INT	frames = sel.Length();
	Job.SetRange(0, frames);
	UINT	chans = wave.GetChannels();
	UINT	SampSize = wave.GetSampleSize();
	SAMPLE	NegRail, PosRail;
	wave.GetSampleRails(NegRail, PosRail);
	if (threshold.Level) {
		double	scale = CDSPlayer::DecibelsToLinear(threshold.Level);
		CConvert	cvt(wave);
		NegRail = round(double(NegRail) * scale);
		PosRail = round(double(PosRail) * scale);
	}
	W64INT	Offset = wave.GetByteOffset(0, sel.Start);
	CArrayEx<CLIP_STATE, CLIP_STATE&> ClipState;
	ClipState.SetSize(chans);	// one clipping state per channel
	for (W64INT iFrame = 0; iFrame < frames; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
			SAMPLE	samp = wave.GetSampleAt(Offset);
			bool	IsClipped;
			if (samp >= 0)	// if positive sample
				IsClipped = samp >= PosRail;	// clipped if at or above positive rail
			else	// negative sample
				IsClipped = samp <= NegRail;	// clipped if at or below negative rail
			CLIP_STATE&	cst = ClipState[iChan];	// dereference clipping state
			if (IsClipped) {	// if clipped sample
				// if clip span pending and unclipped count below stop threshold 
				if (cst.nClipped && cst.nUnclipped < threshold.Stop)
					cst.nClipped += cst.nUnclipped;	// count unclipped break as clipped
				cst.nClipped++;	// count clipped sample
				cst.nUnclipped = 0;	// reset unclipped count
			} else {	// unclipped sample
				cst.nUnclipped++;	// count unclipped sample
				// if unclipped count reaches stop threshold
				if (cst.nUnclipped >= threshold.Stop) {
					// if clipped count reaches start threshold
					if (cst.nClipped >= threshold.Start) {
						CLIP_SPAN	span;
						span.Start = sel.Start + iFrame	// offset from selection start
							- cst.nUnclipped - cst.nClipped + 1;
						span.Length = cst.nClipped;
						span.Channel = iChan;
						ClipSpan.Add(span);	// store clip span
					}
					cst.nClipped = 0;	// reset clipped count
				}
			}
			Offset += SampSize;
		}
	}
	// finalize any pending clips
	for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
		CLIP_STATE&	cst = ClipState[iChan];	// dereference clipping state
		if (cst.nClipped >= threshold.Start) {
			CLIP_SPAN	span;
			span.Start = sel.Start + frames - 1	// offset from selection start
				- cst.nUnclipped - cst.nClipped + 1;
			span.Length = cst.nClipped;
			span.Channel = iChan;
			ClipSpan.Add(span);	// store clip span
		}
	}
	return(TRUE);
}

bool CWaveProcess::FindClipping(const CW64IntRange& Sel, const CLIP_THRESHOLD& Threshold, CClipSpanArray& ClipSpan) const
{
	CTypedAsyncJob<CFindClippingParms>	job;
	CFindClippingParms	parms(*this, Sel, Threshold, ClipSpan);
	return(job.StartJob(FindClipping, &parms, LDS(IDS_WPRO_FIND_CLIPPING)));
}

/////////////////////////////////////////////////////////////////////////////
// ChangeFormat

CWaveProcess::CChangeFormatParms::CChangeFormatParms(CWaveProcess& Wave, UINT Channels, UINT SampleRate, UINT SampleBits)
	: CWaveParms(Wave)
{
	m_Channels = Channels;
	m_SampleRate = SampleRate;
	m_SampleBits = SampleBits;
}

bool CWaveProcess::ChangeFormat(CAsyncJob& Job, CChangeFormatParms *Parms)
{
	CWaveProcess&	wave = Parms->m_Wave;
	UINT	DstChannels = Parms->m_Channels;
	UINT	DstSampleRate = Parms->m_SampleRate;
	UINT	DstSampleBits = Parms->m_SampleBits;
	if (!(DstChannels && DstSampleRate && DstSampleBits))	// all must be non-zero
		ThrowError(IDS_WPRO_INVALID_WAVE);
	CWave	DstWave(DstChannels, DstSampleRate, DstSampleBits, 
		wave.GetChannelMask());	// copy channel mask
	int	SampleSizeChange = DstWave.GetSampleSize() - wave.GetSampleSize();
	// if channel count and sample size in bytes are unchanged
	if (DstChannels == wave.GetChannels() && !SampleSizeChange) {
		wave.m_SampleRate = DstSampleRate;
		wave.m_SampleBits = DstSampleBits;
		wave.OnFormatChange();	// won't reallocate data since size hasn't changed
		return(TRUE);
	}
	// if new sample size exceeds range of SAMPLE data type
	if (DstWave.GetSampleSize() > sizeof(SAMPLE))
		ThrowError(IDS_WPRO_INVALID_WAVE);
	bool	ShiftRight = SampleSizeChange < 0;	// if negative change, shift right
	int	SampleShift = abs(SampleSizeChange) << 3;	// convert bytes to shift in bits
	W64INT	frames = wave.GetFrameCount();
	DstWave.SetFrameCount(frames);
	Job.SetRange(0, frames);
	UINT	SrcChannels = wave.GetChannels();
	UINT	chans = max(SrcChannels, DstChannels);
	UINT	SrcSampSize = wave.GetSampleSize();
	UINT	DstSampSize = DstWave.GetSampleSize();
	W64INT	SrcOffset = 0;
	W64INT	DstOffset = 0;
	for (W64INT iFrame = 0; iFrame < frames; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
			SAMPLE	samp;
			if (iChan < SrcChannels) {	// if channel in source
				samp = wave.GetSampleAt(SrcOffset);	// load sample from source
				if (ShiftRight)
					samp >>= SampleShift;	// shift right
				else
					samp <<= SampleShift;	// shift left
				SrcOffset += SrcSampSize;	// increment source offset
			} else	// channel not in source
				samp = 0;	// fill with silence
			if (iChan < DstChannels) {	// if channel in destination
				DstWave.SetSampleAt(DstOffset, samp);	// store sample to destination
				DstOffset += DstSampSize;	// increment destination offset
			}
		}
	}
	wave.CWave::Copy(DstWave);	// replace source with destination
	return(TRUE);
}

bool CWaveProcess::ChangeFormat(UINT Channels, UINT SampleRate, UINT SampleBits)
{
	CTypedAsyncJob<CChangeFormatParms>	job;
	CChangeFormatParms	parms(*this, Channels, SampleRate, SampleBits);
	return(job.StartJob(ChangeFormat, &parms, LDS(IDS_WPRO_CHANGE_FORMAT)));
}

/////////////////////////////////////////////////////////////////////////////
// FindSample

CWaveProcess::CFindSampleParms::CFindSampleParms(const CWaveProcess& Wave, FIND_SAMPLE_INFO& FindInfo)
	: CConstWaveParms(Wave), m_FindInfo(FindInfo)
{
}

bool CWaveProcess::FindSample(CAsyncJob& Job, CFindSampleParms *Parms)
{
	const CWaveProcess&	wave = Parms->m_Wave;
	FIND_SAMPLE_INFO&	info = Parms->m_FindInfo;
	CSampleRange	target(info.TargetStart, info.TargetEnd);	// target range
	target.Normalize();	// enforce ascending order
	W64INT	frames = wave.GetFrameCount();
	Job.SetRange(0, frames);
	CIntRange	ChanRange;
	if (info.ChannelIdx < 0) {	// if searching all channels
		ChanRange.Start = 0;
		ChanRange.End = wave.GetChannels() - 1;
	} else {	// searching specified channel
		int	chans = wave.GetChannels();
		if (info.ChannelIdx >= chans)	// if channel index out of range
			ThrowError(IDS_WPRO_BAD_CHANNEL);
		ChanRange.Start = info.ChannelIdx;	// set range to specified channel
		ChanRange.End = info.ChannelIdx;
	}
	info.MatchChannel = -1;	// assume failure: init results to not found
	info.MatchFrame = -1;
	W64INT	CurFrame = info.StartFrame;	// init current frame index
	UINT	SampleSize = wave.GetSampleSize();
	for (int iFrame = 0; iFrame < frames; iFrame++) {	// for each frame
		if (Job.SetPosEx(iFrame))	// if aborted
			return(FALSE);
		if (CurFrame < 0) {	// if past start of data
			if (info.Flags & FIND_SAMPLE_INFO::WRAP)	// if wrapping
				CurFrame = frames - 1;	// wrap current frame index
			else	// not wrapping
				break;	// end search
		} else if (CurFrame >= frames) {	// if past end of data
			if (info.Flags & FIND_SAMPLE_INFO::WRAP)	// if wrapping
				CurFrame = 0;	// wrap current frame index
			else	// not wrapping
				break;	// end search
		}
		W64INT	offset = wave.GetByteOffset(ChanRange.Start, CurFrame);	// initial offset
		// for each channel in channel range
		for (int iChan = ChanRange.Start; iChan <= ChanRange.End; iChan++) {
			SAMPLE	samp = wave.GetSampleAt(offset);
			if (info.Flags & FIND_SAMPLE_INFO::ABS_VAL)
				samp = abs(samp);	// take absolute value of sample
			bool	match = samp >= target.Start && samp <= target.End;
			if (info.Flags & FIND_SAMPLE_INFO::INVERT)	// if inverted matching
				match = !match;	// match samples NOT in target range
			if (match) {	// if matching sample
				info.MatchChannel = iChan;	// store results
				info.MatchFrame = CurFrame;
				return(TRUE);	// early out
			}
			offset += SampleSize;	// increment offset past sample
		}
		if (info.Flags & FIND_SAMPLE_INFO::REVERSE)	// if reverse search
			CurFrame--;
		else	// forward search
			CurFrame++;
	}
	return(TRUE);	// target not found
}

bool CWaveProcess::FindSample(FIND_SAMPLE_INFO& FindInfo) const
{
	CTypedAsyncJob<CFindSampleParms>	job;
	CFindSampleParms	parms(*this, FindInfo);
	return(job.StartJob(FindSample, &parms, LDS(IDS_WPRO_FIND_SAMPLE)));
}

/////////////////////////////////////////////////////////////////////////////
// Resample

bool CWaveProcess::Resample(UINT NewSampleRate, int Quality)
{
	CSampleRateEx	sr;
	if (!sr.Create(Quality, GetChannels()))
		return(FALSE);
	CWave	dst;
	if (!sr.Resample(*this, dst, NewSampleRate))
		return(FALSE);
	CWave::Copy(dst);
	return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// GetSpectrum

CWaveProcess::CSpectrumParms::CSpectrumParms(const CWaveProcess& Wave, const CW64IntRange& Sel, CSpectrum& Spectrum)
	: CConstWaveSelParms(Wave, Sel), m_Spectrum(Spectrum)
{
}

CWaveProcess::CSpectrum::CSpectrum()
{
	m_WindowFunction = 0;
	m_WindowSize = 0;
	m_SeparateChannels = FALSE;
}

bool CWaveProcess::GetSpectrum(CAsyncJob& Job, CSpectrumParms *Parms)
{
	const CWave&	wave = Parms->m_Wave;
	CW64IntRange	sel(Parms->m_Selection);
	W64INT	frames = sel.Length();
	CSpectrum&	spectrum = Parms->m_Spectrum;
	int	WinSize = spectrum.m_WindowSize;
	int	windows = W64INT_CAST32(frames / WinSize);
	windows = windows * 2 - 1;	// overlap by half a window; exclude remnant
	if (windows <= 0)
		ThrowError(IDS_WPRO_TOO_FEW_FRAMES);
	CSpectrumAnal	anal;
	anal.SetParms(windows, WinSize, 
		spectrum.m_WindowFunction, spectrum.m_SeparateChannels);
	anal.SetJob(&Job);
	anal.Setup(&wave);
	if (!anal.Analyze(sel.Start))
		return(FALSE);
	anal.GetOutput(spectrum.m_Bin);
	return(TRUE);
}

bool CWaveProcess::GetSpectrum(const CW64IntRange& Sel, CSpectrum& Spectrum)
{
	CTypedAsyncJob<CSpectrumParms>	job;
	CSpectrumParms	parms(*this, Sel, Spectrum);
	return(job.StartJob(GetSpectrum, &parms, LDS(IDS_WPRO_GET_SPECTRUM)));
}
