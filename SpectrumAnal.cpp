// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      19mar13	initial version
        01      08apr13	in Setup, remove unused var

        spectrum analyzer

*/

#include "stdafx.h"
#include "Resource.h"
#include "SpectrumAnal.h"
#include "AsyncJob.h"
#include <math.h>

#define WINDOWFUNCTIONDEF(name) IDS_WNDFN_##name,
const int CSpectrumAnal::m_WindowFuncName[WINDOW_FUNCTIONS] = {
	#include "WindowFuncData.h"
};

CSpectrumAnal::CSpectrumAnal()
{
	m_Wave = NULL;
	m_Job = NULL;
	m_Windows = 0;
	m_WindowSize = 0;
	m_WindowFunction = 0;
	m_SeparateChannels = FALSE;
	m_Repeat = FALSE;
	m_Selection.SetEmpty();
	m_Scale = 0;
}

void CSpectrumAnal::SetParms(int Windows, int WindowSize, int WindowFunction, bool SeparateChannels)
{
	m_Windows = Windows;
	m_WindowSize = WindowSize;
	m_WindowFunction = WindowFunction;
	m_SeparateChannels = SeparateChannels;
}

void CSpectrumAnal::Setup(const CWave *Wave)
{
	ASSERT(Wave != NULL);
	ASSERT(IsPowerOfTwo(m_WindowSize));
	m_Wave = Wave;
	m_Selection = CW64IntRange(0, Wave->GetDataSize());	// default to entire audio
	m_Cvt.Create(*m_Wave);	// init normalizer
	UINT	chans;
	if (m_SeparateChannels)	// if analyzing channels separately
		chans = Wave->GetChannels();
	else
		chans = 1;
	int	WinSize = m_WindowSize;
	int	bins = WinSize / 2;
	// resize arrays for FFT
	m_TimeIn.SetSize(WinSize);	// allocate time domain input array
	// Kiss FFT expects one extra output element to be allocated for Nyquist point
	m_FreqOut.SetSize(WinSize / 2 + 1);	// allocate frequency domain output array
	int	TotalBins = bins;
	if (m_SeparateChannels)	// if analyzing channels separately
		TotalBins *= chans;	// bin array is 2-D: each channel gets its own bin array
	m_Bin.SetSize(TotalBins);	// allocate bin array(s)
	m_WinFuncVal.SetSize(WinSize);	// allocate window function value array
	GetWindowFunction(m_WindowFunction, m_WinFuncVal.GetData(), WinSize);
	m_FFT.Create(WinSize);	// allocate FFT buffers
	// windowing attenuates amplitudes by about half; average of window
	// function values is a workable approximation of correction factor
	double	WinFuncAmpCorr = 0;
	for (int iFrame = 0; iFrame < WinSize; iFrame++)	// for each frame
		WinFuncAmpCorr += m_WinFuncVal[iFrame];	// add window function value to sum
	WinFuncAmpCorr /= WinSize;	// average of window function values
	double	scale = 1.0 / bins;	// standard FFT amplitude scaling
	scale /= WinFuncAmpCorr;	// apply window function's amplitude correction
	scale *= scale;	// square scale, to account for magnitude squaring
	m_Scale = scale;
}

bool CSpectrumAnal::Analyze(W64INT Frame)
{
	ASSERT(m_Wave != NULL);
	ASSERT(IsPowerOfTwo(m_WindowSize));
	const CWave&	wave = *m_Wave;
	if (!wave.IsValid())	// if invalid wave
		return(FALSE);	// nothing to do
	if (Frame >= wave.GetFrameCount())	// if frame out of range
		return(FALSE);	// nothing to do
	// dereference values to reduce overhead
	UINT	SampleSize = wave.GetSampleSize();
	UINT	FrameSize = wave.GetFrameSize();
	UINT	chans = wave.GetChannels();
	W64INT	WinOffset = wave.GetByteOffset(0, Frame);
	W64INT	SelLen = m_Selection.Length();
	int	WinSize = m_WindowSize;
	int	bins = WinSize / 2;
	int	HalfWinSize = bins * FrameSize;
	int	windows = m_Windows;
	if (m_Job != NULL)
		m_Job->SetRange(0, windows);
	if (m_SeparateChannels) {	// if analyzing channels separately
		for (int iWin = 0; iWin < windows; iWin++) {	// for each window
			if (m_Job != NULL && m_Job->SetPosEx(iWin))	// if aborted
				return(FALSE);
			double	*pBin = m_Bin.GetData();	// point to first channel's bin array
			for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
				W64INT	offset = WinOffset + iChan * SampleSize;
				for (int iFrame = 0; iFrame < WinSize; iFrame++) {	// for each frame
					if (offset >= m_Selection.End) {	// if offset beyond end of selection
						if (!m_Repeat) {	// if not looping playback
							for (; iFrame < WinSize; iFrame++)	// for remaining frames
								m_TimeIn[iFrame] = 0;	// zero time domain input
							break;	// no more samples; skip to FFT
						}
						offset -= SelLen;	// wrap offset to start of selection
					}
					ASSERT(m_Selection.InRange(offset));	// verify offset within selection
					CWave::SAMPLE	samp = wave.GetSampleAt(offset);
					m_TimeIn[iFrame] = m_Cvt.SampleToNorm(samp) * m_WinFuncVal[iFrame];
					offset += FrameSize;
				}
				m_FFT.RealFFT(m_TimeIn.GetData(), m_FreqOut.GetData());	// do FFT
				for (int iBin = 1; iBin < bins; iBin++)	// for each bin except zero
					pBin[iBin] += m_FreqOut[iBin].Magnitude();	// add magnitude to bin
				pBin += bins;	// point to next channel's bin array
			}
			WinOffset += HalfWinSize;
		}
	} else {	// combining channels
		for (int iWin = 0; iWin < windows; iWin++) {	// for each window
			if (m_Job != NULL && m_Job->SetPosEx(iWin))	// if aborted
				return(FALSE);
			W64INT	offset = WinOffset;
			for (int iFrame = 0; iFrame < WinSize; iFrame++) {	// for each frame
				if (offset >= m_Selection.End) {	// if offset beyond end of selection
					if (!m_Repeat) {	// if not looping playback
						for (; iFrame < WinSize; iFrame++)	// for remaining frames
							m_TimeIn[iFrame] = 0;	// zero time domain input
						break;	// no more samples; skip to FFT
					}
					offset -= SelLen;	// wrap offset to start of selection
				}
				double	sum = 0;
				for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
					ASSERT(m_Selection.InRange(offset));	// verify offset within selection
					CWave::SAMPLE	samp = wave.GetSampleAt(offset);
					sum += m_Cvt.SampleToNorm(samp);	// add normalized sample to sum
					offset += SampleSize;
				}
				m_TimeIn[iFrame] = sum * m_WinFuncVal[iFrame];	// apply window function
			}
			m_FFT.RealFFT(m_TimeIn.GetData(), m_FreqOut.GetData());	// do FFT
			for (int iBin = 1; iBin < bins; iBin++)	// for each bin except zero
				m_Bin[iBin] += m_FreqOut[iBin].Magnitude();	// add magnitude to bin
			WinOffset += HalfWinSize;
		}
	}
	int	TotalBins = m_Bin.GetSize();
	for (int iBin = 1; iBin < TotalBins; iBin++) {	// for each bin except zero
		double	amp = m_Bin[iBin] * m_Scale / windows;	// scale amplitude
		if (amp) {	// if non-zero amplitude
			double	level = 10 * log10(amp);	// convert to decibels
			m_Bin[iBin] = max(level, CWaveProcess::MIN_LEVEL);	// clip low-level noise
		} else	// zero amplitude
			m_Bin[iBin] = CWaveProcess::MIN_LEVEL;	// substitute nominal silence
	}
	return(TRUE);
}

void CSpectrumAnal::GetWindowFunction(int FuncIdx, double *Buf, int Elems)
{
	int	Nm1 = Elems - 1;	// N - 1
	switch (FuncIdx) {
	case WF_RECTANGULAR:
		{
			for (int i = 0; i < Elems; i++) {
				Buf[i] = 1;
			}
		}
		break;
	case WF_BARTLETT:
		{
			int	half = Elems / 2;
			for (int i = 0; i < half; i++) {
				double	r = 2 * double(i) / Nm1;
				Buf[i] = r;
				Buf[i + half] = 1 - r;
			}
		}
		break;
	case WF_WELCH:
		{
			int	half = Elems / 2;
			for (int i = 0; i < half; i++) {
				double	r = 2 * double(i) / Nm1;
				double	q = 1 - r * r;
				Buf[half - 1 - i] = q;
				Buf[i + half] = q;
			}
		}
		break;
	case WF_HANN:	// AKA Hanning
		{
			for (int i = 0; i < Elems; i++) {
				Buf[i] = 0.50 * (1 - cos(2 * PI * i / Nm1));
			}
		}
		break;
	case WF_HAMMING:
		{
			for (int i = 0; i < Elems; i++) {
				Buf[i] = 0.54 - 0.46 * cos(2 * PI * i / Nm1);
			}
		}
		break;
	case WF_BLACKMAN:
		{
			for (int i = 0; i < Elems; i++) {
				Buf[i] = 0.42659 - 0.49656 * cos(2 * PI * i / Nm1) + 0.076849 * cos(4 * PI * i / Nm1);
			}
		}
		break;
	case WF_BLACKMAN_HARRIS:
		{
			for (int i = 0; i < Elems; i++) {
				Buf[i] = 0.35875 - 0.48829 * cos(2 * PI * i / Nm1) + 0.14128 * cos(4 * PI * i / Nm1) - 0.01168 * cos(6 * PI * i / Nm1);
			}
		}
		break;
	default:
		NODEFAULTCASE;
	}
}
