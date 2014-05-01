// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      19mar13	initial version

        spectrum analyzer

*/

#ifndef CSPECTRUMANAL_INCLUDED
#define CSPECTRUMANAL_INCLUDED

#include "WaveProcess.h"
#include "KissFFT.h"

class CSpectrumAnal : public WObject {
public:
// Construction
	CSpectrumAnal();

// Constants
	#define WINDOWFUNCTIONDEF(name) WF_##name,
	enum {	// window functions
		#include "WindowFuncData.h"
		WINDOW_FUNCTIONS
	};

// Types
	typedef CWaveProcess::CDblArray CDblArray;

// Attributes
	const CWave	*GetWave() const;
	void	SetJob(CAsyncJob *Job);
	void	SetParms(int Windows, int WindowSize, int WindowFunction, bool SeparateChannels);
	void	SetSelection(const CW64IntRange& Selection);
	void	SetRange(const CW64IntRange& Range);
	void	SetRepeat(bool Repeat);
	void	GetOutput(CDblArray& Bin) const;
	const double	*GetOutput() const;
	bool	IsValidBinPtr(const double *Ptr) const;
	static	bool	IsValidBinPtr(const CDblArray& Bin, const double *Ptr);

// Operations
	void	ResetOutput();
	void	Setup(const CWave *Wave);
	bool	Analyze(W64INT Frame);
	static	void	GetWindowFunction(int FuncIdx, double *Buf, int Elems);
	static	CString	GetWindowFuncName(int FuncIdx);
	static	bool	IsPowerOfTwo(int x);

protected:
// Types
	class CComplex : public kiss_fft_cpx {
	public:
		double	Magnitude() const;
	};
	typedef CArrayEx<CComplex, CComplex&> CComplexArray;

// Constants
	static const int m_WindowFuncName[];	// resource IDs of function names

// Member data
	CKissFFT	m_FFT;			// FFT instance
	CDblArray	m_TimeIn;		// FFT input: array of scalar time data
	CComplexArray	m_FreqOut;	// FFT output: array of complex frequency data
	CDblArray m_WinFuncVal;		// array of precomputed window function values
	CWaveProcess::CConvert	m_Cvt;	// converter for normalizing samples
	const CWave	*m_Wave;		// pointer to audio container
	CAsyncJob	*m_Job;			// pointer to asynchronous job state
	int		m_Windows;			// number of windows to process
	int		m_WindowSize;		// size of window, in frames
	int		m_WindowFunction;	// index of window function
	bool	m_SeparateChannels;	// if true, separate channels, else combine
	bool	m_Repeat;			// if true, loop within selection
	CW64IntRange	m_Selection;	// selection to analyze, as byte offsets
	CDblArray	m_Bin;			// array of output magnitude bins, one for each
								// frequency band; if separating channels, array 
								// is 2-D (rows are channels, columns are bands)
	double	m_Scale;			// output scaling factor
};

inline double CSpectrumAnal::CComplex::Magnitude() const
{
	return(r * r + i * i);
}

inline CString CSpectrumAnal::GetWindowFuncName(int FuncIdx)
{
	ASSERT(FuncIdx >= 0 && FuncIdx < WINDOW_FUNCTIONS);
	return(LDS(m_WindowFuncName[FuncIdx]));
}

inline bool CSpectrumAnal::IsPowerOfTwo(int x)
{
	return(x && !(x & (x - 1)));
}

inline const CWave *CSpectrumAnal::GetWave() const
{
	return(m_Wave);
}

inline void CSpectrumAnal::GetOutput(CDblArray& Bin) const
{
	Bin = m_Bin;
}

inline const double *CSpectrumAnal::GetOutput() const
{
	return(m_Bin.GetData());
}

inline void CSpectrumAnal::ResetOutput()
{
	ZeroMemory(m_Bin.GetData(), m_Bin.GetSize() * sizeof(double));
}

inline bool CSpectrumAnal::IsValidBinPtr(const double *Ptr) const
{
	return(IsValidBinPtr(m_Bin, Ptr));
}

inline bool CSpectrumAnal::IsValidBinPtr(const CDblArray& Bin, const double *Ptr)
{
	return(Ptr >= Bin.GetData() && Ptr < Bin.GetData() + Bin.GetSize());
}

inline void CSpectrumAnal::SetJob(CAsyncJob *Job)
{
	m_Job = Job;
}

inline void CSpectrumAnal::SetSelection(const CW64IntRange& Selection)
{
	m_Selection = Selection;
}

inline void CSpectrumAnal::SetRepeat(bool Repeat)
{
	m_Repeat = Repeat;
}

#endif
