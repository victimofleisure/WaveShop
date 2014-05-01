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
        02      25feb13	add Resample
        03      02mar13	move speaker names to base class
		04		04mar13	in FindClipping, add clipping level
		05		20mar13	add GetSpectrum
		06		06jun13	add channel selection to peak stats and normalize
		07		10jul13	add ClampNormToSample

		wave processing
 
*/

#ifndef CWAVEPROCESS_INCLUDED
#define CWAVEPROCESS_INCLUDED

#include "WaveEdit.h"

class CAsyncJob;

class CWaveProcess : public CWaveEdit {
public:
// Construction
	virtual	~CWaveProcess();

// Constants
	enum {	// normalization flags
		NORM_NORMALIZE		= 0x01,	// normalize
		NORM_UNBIAS			= 0x02,	// correct DC bias
		NORM_INDEPENDENT	= 0x04,	// normalize channels independently
	};
	enum {	// RMS statistics flags
		RMS_ACCOUNT_FOR_DC	= 0x01,	// account for DC offset
		RMS_0_DB_FS_SQUARE	= 0x02,	// 0 dB = full scale square wave, else sine
	};
	enum {
		MIN_LEVEL = -100,			// minimum level in decibels
	};

// Types
	class CConvert : public WObject {
	public:
		enum {	// sample units
			VALUE,
			PERCENT,
			DECIBELS,
			UNITS
		};
		CConvert();
		CConvert(const CWave& Wave);
		void	Create(const CWave& Wave);
		SAMPLE	m_NegRail;			// negative rail
		SAMPLE	m_PosRail;			// positive rail
		double	SampleToNorm(double Sample) const;
		double	NormToSample(double Norm) const;
		double	ClampNormToSample(double Norm) const;
		double	UnitToSample(double Val, int Unit) const;
		double	SampleToUnit(double Sample, int Unit) const;
	};
	class CPeakStats : public CConvert {
	public:
		struct CHAN_STATS {	// per-channel statistics
			SAMPLE	Min;		// minimum sample
			SAMPLE	Max;		// maximum sample
			double	Sum;		// sum of samples
		};
		typedef CArrayEx<CHAN_STATS, CHAN_STATS&> CChanStatsArray;
		CPeakStats();
		CChanStatsArray	m_Chan;		// array of per-channel statistics
		CStringArray	m_ChanName;	// array of channel names
		CHAN_STATS	m_Total;		// statistics for all channels combined
		W64INT	m_Frames;			// number of frames
		double	GetNormMin(int ChanIdx) const;
		double	GetNormMax(int ChanIdx) const;
		double	GetPeakDecibels(int ChanIdx) const;
		double	GetBias(int ChanIdx) const;
		double	GetNormBias(int ChanIdx) const;
		double	GetNormMin() const;
		double	GetNormMax() const;
		double	GetPeakDecibels() const;
	};
	typedef CArrayEx<double, double> CDblArray;
	class CRMSStats : public WObject {
	public:
		class CChanInfo : public WObject {	// per-channel information
		public:
			double	m_DCBias;		// DC offset correction if any
			double	m_Min;			// minimum of window RMS values
			double	m_Max;			// maximum of window RMS values
			double	m_Avg;			// average of window RMS values
			double	m_Total;		// total RMS value for all samples
			double	m_AvgSum;		// sum of window RMS values to be averaged
			int		m_AvgCount;		// count of window RMS values to be averaged
			double	m_TotalSum;		// sum of all squared samples, for total RMS
			double	m_WindowSum;	// sum of squared samples for current window
			CDblArray	m_HistoBin;	// histogram bins
		};
		typedef CArrayEx<CChanInfo, CChanInfo&>	CChanInfoArray;
		CChanInfoArray	m_Chan;		// array of per-channel information
		CStringArray	m_ChanName;	// array of channel names
		CIntRange	m_HistoRange;	// range of non-empty histogram bins
	};
	struct RMS_STATS_PARMS {	// user-specified parameters for RMS statistics
		double	WindowSize;			// sliding window size in seconds
		int		PanesPerWindow;		// number of panes per window; 1 = no overlap
		int		HistogramRange;		// range of histogram in decibels
		int		HistogramSubdiv;	// histogram subdivisions per decibel
		UINT	Flags;				// see RMS statistics flags enum above
	};
	struct CLIP_THRESHOLD {
		UINT	Start;	// number of consecutive clipped samples to start clipping
		UINT	Stop;	// number of consecutive unclipped samples to stop clipping
		double	Level;	// clipping level in decibels; zero gives standard behavior
	};
	struct CLIP_SPAN {
		W64INT	Start;			// frame at which clipping began
		UINT	Length;			// duration of clipping, in frames
		UINT	Channel;		// index of channel that clipped
	};
	typedef CArrayEx<CLIP_SPAN, CLIP_SPAN&>	CClipSpanArray;
	struct FIND_SAMPLE_INFO {
		enum {
			WRAP	= 0x01,		// search should wrap around
			INVERT	= 0x02,		// find first non-matching sample
			REVERSE	= 0x04,		// search backward instead of forward
			ABS_VAL	= 0x08,		// take absolute value of samples before comparison
		};
		int		ChannelIdx;		// index of channel to search, or -1 for all channels
		SAMPLE	TargetStart;	// starting sample value of target range
		SAMPLE	TargetEnd;		// ending sample value of target range
		W64INT	StartFrame;		// frame to start searching from
		UINT	Flags;			// flags; see enum above
		int		MatchChannel;	// index of matching channel, or -1 if not found
		W64INT	MatchFrame;		// index of matching frame, or -1 if not found
	};
	typedef CRange<SAMPLE> CSampleRange;
	class CSpectrum : public WObject {
	public:
		CSpectrum();
		int		m_WindowFunction;	// window function index
		int		m_WindowSize;		// window size, in frames
		bool	m_SeparateChannels;	// if true, analyze channel each separately
		CDblArray	m_Bin;			// array of frequency bins for each channel
	};

// Operations
	static	double	SafeLinearToDecibels(double Linear);
	bool	GetPeakStats(const CW64IntRange& Sel, CPeakStats& Stats, const BYTE *ChanSel = NULL) const;
	bool	Normalize(const CW64IntRange& Sel, UINT Flags = NORM_NORMALIZE, double PeakLevel = 1, const BYTE *ChanSel = NULL);
	bool	Reverse(const CW64IntRange& Sel);
	bool	Invert(const CW64IntRange& Sel);
	bool	GetRMSStats(const CW64IntRange& Sel, CRMSStats& RMSStats, const RMS_STATS_PARMS& Parms, CPeakStats *PeakStats = NULL) const;
	bool	SwapChannels(const CW64IntRange& Sel, const CUIntRange& Pair);
	bool	Amplify(const CW64IntRange& Sel, double Gain);
	bool	Fade(const CW64IntRange& Sel, const CDblRange& Range, bool IsLog);
	bool	CopyChannel(UINT ChannelIdx, CWaveEdit& Wave) const;
	bool	InsertChannel(UINT ChannelIdx, const CWaveEdit& Wave);
	bool	DeleteChannel(UINT ChannelIdx);
	bool	ExtractChannels(LPCTSTR Path) const;
	bool	FindClipping(const CW64IntRange& Sel, const CLIP_THRESHOLD& Threshold, CClipSpanArray& ClipSpan) const;
	bool	ChangeFormat(UINT Channels, UINT SampleRate, UINT SampleBits);
	bool	FindSample(FIND_SAMPLE_INFO& FindInfo) const;
	bool	Resample(UINT NewSampleRate, int Quality);
	bool	GetSpectrum(const CW64IntRange& Sel, CSpectrum& Spectrum);

protected:
// Types
	class CWaveParms : public WObject {
	public:
		CWaveParms(CWaveProcess& Wave);
		CWaveProcess&	m_Wave;	// reference to wave to be processed
	};
	class CConstWaveParms : public WObject {
	public:
		CConstWaveParms(const CWaveProcess& Wave);
		const CWaveProcess&	m_Wave;	// const reference to wave to be processed
	};
	class CWaveSelParms : public CWaveParms {
	public:
		CWaveSelParms(CWaveProcess& Wave, const CW64IntRange& Sel);
		CW64IntRange	m_Selection;	// selection to process
	};
	class CConstWaveSelParms : public CConstWaveParms {
	public:
		CConstWaveSelParms(const CWaveProcess& Wave, const CW64IntRange& Sel);
		CW64IntRange	m_Selection;	// selection to process
	};
	class CPeakStatsParms : public CConstWaveSelParms {
	public:
		CPeakStatsParms(const CWaveProcess& Wave, const CW64IntRange& Sel, CPeakStats& Stats, const BYTE *ChanSel);
		CPeakStats&	m_Stats;		// reference to destination peak statistics
		const BYTE *m_ChanSel;	// pointer to optional channel selection
	};
	class CNormalizeParms : public CWaveSelParms {
	public:
		CNormalizeParms(CWaveProcess& Wave, const CW64IntRange& Sel, const CPeakStats& PeakStats, UINT Flags, double PeakLevel, const BYTE *ChanSel);
		const CPeakStats&	m_PeakStats;	// reference to peak statistics
		UINT	m_Flags;		// normalization flags; see enum
		double	m_PeakLevel;	// peak level to normalize to, in dB
		const BYTE *m_ChanSel;	// pointer to optional channel selection
	};
	struct NORMALIZE_INFO {
		double	MaxMag;			// maximum sample magnitude
		double	Scale;			// normalization scaling 
		double	Bias;			// DC bias correction
	};
	typedef CArrayEx<NORMALIZE_INFO, NORMALIZE_INFO&> CNormalizeInfoArray;
	class CRMSStatsParms : public CConstWaveSelParms  {
	public:
		CRMSStatsParms(const CWaveProcess& Wave, const CW64IntRange& Sel, const CPeakStats& PeakStats, CRMSStats& RMSStats, const RMS_STATS_PARMS& UserParms);
		const CPeakStats&	m_PeakStats;	// reference to peak statistics
		CRMSStats&	m_RMSStats;	// reference to destination RMS statistics
		const RMS_STATS_PARMS&	m_UserParms;	// reference to user-specified parameters
	};
	class CAmplifyParms : public CWaveSelParms {
	public:
		CAmplifyParms(CWaveProcess& Wave, const CW64IntRange& Sel, double Gain);
		double	m_Gain;			// gain in dB; positive boosts, negative attenuates
	};
	class CSwapChannelsParms : public CWaveSelParms {
	public:
		CSwapChannelsParms(CWaveProcess& Wave, const CW64IntRange& Sel, const CUIntRange& Pair);
		CUIntRange	m_Pair;		// pair of channels to swap, as channel indices
	};
	class CFadeParms : public CWaveSelParms {
	public:
		CFadeParms(CWaveProcess& Wave, const CW64IntRange& Sel, const CDblRange& Range, bool IsLog);
		CDblRange	m_Range;	// initial and final attenuation, in dB
		bool	m_IsLog;		// if true, logarithmic fade, else linear
	};
	class CCopyChannelParms : public CConstWaveParms {
	public:
		CCopyChannelParms(const CWaveProcess& Wave, UINT ChannelIdx, CWaveEdit& DestinationWave);
		UINT	m_ChannelIdx;	// index of channel to copy
		CWaveEdit&	m_DestinationWave;	// reference to destination wave
	};
	class CInsertChannelParms : public CWaveParms {
	public:
		CInsertChannelParms(CWaveProcess& Wave, UINT ChannelIdx, const CWaveEdit& InsertWave);
		UINT	m_ChannelIdx;	// insert position, as a channel index
		const CWaveEdit&	m_InsertWave;	// reference to wave being inserted
	};
	class CDeleteChannelParms : public CWaveParms {
	public:
		CDeleteChannelParms(CWaveProcess& Wave, UINT ChannelIdx);
		UINT	m_ChannelIdx;	// index of channel to delete
	};
	struct CLIP_STATE {
		UINT	nClipped;		// count of consecutive clipped samples
		UINT	nUnclipped;		// count of consecutive unclipped samples
	};
	class CFindClippingParms : public CConstWaveSelParms {
	public:
		CFindClippingParms(const CWaveProcess& Wave, const CW64IntRange& Sel, const CLIP_THRESHOLD& Threshold, CClipSpanArray& ClipSpan);
		CLIP_THRESHOLD	m_Threshold;	// clipping start and stop thresholds
		CClipSpanArray&	m_ClipSpan;	// reference to array of clipping spans
	};
	class CChangeFormatParms : public CWaveParms {
	public:
		CChangeFormatParms(CWaveProcess& Wave, UINT Channels, UINT SampleRate, UINT SampleBits);
		UINT	m_Channels;		// number of channels
		UINT	m_SampleRate;	// sampling frequency in Hz
		UINT	m_SampleBits;	// bits per sample
	};
	class CFindSampleParms : public CConstWaveParms {
	public:
		CFindSampleParms(const CWaveProcess& Wave, FIND_SAMPLE_INFO& FindInfo);
		FIND_SAMPLE_INFO&	m_FindInfo;
	};
	class CSpectrumParms : public CConstWaveSelParms {
	public:
		CSpectrumParms(const CWaveProcess& Wave, const CW64IntRange& Sel, CSpectrum& Spectrum);
		CSpectrum&	m_Spectrum;	// reference to spectrum data
	};

// Operations
	static	bool	GetPeakStats(CAsyncJob& Job, CPeakStatsParms *Parms);
	static	bool	Normalize(CAsyncJob& Job, CNormalizeParms *Parms);
	static	bool	Reverse(CAsyncJob& Job, CWaveSelParms *Parms);
	static	bool	Invert(CAsyncJob& Job, CWaveSelParms *Parms);
	static	bool	GetRMSStats(CAsyncJob& Job, CRMSStatsParms *Parms);
	static	bool	SwapChannels(CAsyncJob& Job, CSwapChannelsParms *Parms);
	static	bool	Amplify(CAsyncJob& Job, CAmplifyParms *Parms);
	static	bool	Fade(CAsyncJob& Job, CFadeParms *Parms);
	static	bool	CopyChannel(CAsyncJob& Job, CCopyChannelParms *Parms);
	static	bool	InsertChannel(CAsyncJob& Job, CInsertChannelParms *Parms);
	static	bool	DeleteChannel(CAsyncJob& Job, CDeleteChannelParms *Parms);
	static	bool	FindClipping(CAsyncJob& Job, CFindClippingParms *Parms);
	static	bool	ChangeFormat(CAsyncJob& Job, CChangeFormatParms *Parms);
	static	bool	FindSample(CAsyncJob& Job, CFindSampleParms *Parms);
	static	bool	GetSpectrum(CAsyncJob& Job, CSpectrumParms *Parms);

// Helpers
	static	CString	MakeChannelPath(const CString& BasePath, const CStringArray& ChannelName, int ChanIdx);
};

inline CWaveProcess::CConvert::CConvert(const CWave& Wave)
{
	Create(Wave);
}

inline double CWaveProcess::CConvert::ClampNormToSample(double Norm) const
{
	return(NormToSample(CLAMP(Norm, -1, 1)));
}

#endif
