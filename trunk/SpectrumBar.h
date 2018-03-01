// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		30mar13	initial version
		01		07apr13	add ResetView

        real-time spectrum analyzer bar
 
*/

#if !defined(AFX_SPECTRUMBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_SPECTRUMBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpectrumBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpectrumBar window

#include "SpectrumAnal.h"
#include "PlotCtrl.h"
#include "OptionsInfo.h"

class CSpectrumBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CSpectrumBar);
// Construction
public:
	CSpectrumBar();

// Constants
	enum {	// plot styles
		PS_LINE,			// lines
		PS_AREA,			// filled area
		PS_OUTLINED_AREA,	// filled area with outlining
		PLOT_STYLES
	};
	enum {	// frequency axis types
		FAT_LINEAR,			// linear scale
		FAT_LOG,			// logarithmic scale
		FREQ_AXIS_TYPES
	};
	enum {	// channel modes
		CM_COMBINE,			// combine channels pre-analysis
		CM_SEPARATE,		// analyze each channel separately
		CHANNEL_MODES
	};
	enum {	// averaging presets
		AVG_NONE,			// no averaging; single window
		AVG_ULTRALIGHT,		// add 1 preceding windows
		AVG_LIGHT,			// add 2 preceding windows
		AVG_MEDIUM,			// add 4 preceding windows
		AVG_HEAVY,			// add 8 preceding windows
		AVG_SMOOTHEST,		// add 16 preceding windows
		AVERAGING_PRESETS
	};
	enum {	// averaging range
		MIN_AVERAGING = 0,
		MAX_AVERAGING = 1 << (AVERAGING_PRESETS - 2),
	};
	enum {	// show peaks styles
		SPS_OFF,			// don't show peaks
		SPS_LINES,			// show peaks as lines
		SPS_DOTS,			// show peaks as dots
		SHOW_PEAKS_STYLES
	};

// Attributes
public:
	void	GetParms(RTSA_PARMS& Parms) const;
	void	SetParms(const RTSA_PARMS& Parms);

// Operations
public:
	void	UpdateView();
	void	TimerHook(W64INT Frame);
	void	ResetView();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpectrumBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSpectrumBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSpectrumBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateFreqLinear(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnUpdateAveraging(CCmdUI* pCmdUI);
	afx_msg void OnAveraging(UINT nID);
	afx_msg void OnUpdatePlotStyle(CCmdUI* pCmdUI);
	afx_msg void OnPlotStyle(UINT nID);
	afx_msg void OnUpdateFreqAxis(CCmdUI* pCmdUI);
	afx_msg void OnFreqAxis(UINT nID);
	afx_msg void OnUpdateChannelMode(CCmdUI* pCmdUI);
	afx_msg void OnChannelMode(UINT nID);
	afx_msg void OnUpdateShowPeaks(CCmdUI* pCmdUI);
	afx_msg void OnShowPeaks(UINT nID);
	afx_msg void OnOptions();
	afx_msg void OnReset();
	DECLARE_MESSAGE_MAP()

// Constants
	static const UINT	m_PlotStyleFlags[PLOT_STYLES];

// Member data
	RTSA_PARMS	m_Parms;		// real-time spectrum analyzer parameters
	CSpectrumAnal	m_Anal;		// spectrum analyzer
	CPlotCtrl	m_Plot;			// plot control
	int		m_OutputChannels;	// number of output channels
	int		m_InputDuration;	// duration of audio to be analyzed, in frames
	int		m_PeakHoldTicks;	// peak hold time, in timer ticks
	double	m_PeakDecay;		// peak decay rate, in decibels per tick
	CByteArray	m_PeakHold;		// 2-D array of peak hold counters; rows are
								// channels, columns are FFT frequency bands

// Helpers
	void	OnParmsChange();
	static	bool	IsValidArrayPtr(const CByteArray& Arr, const BYTE *Ptr);
};

inline void CSpectrumBar::GetParms(RTSA_PARMS& Parms) const
{
	Parms = m_Parms;
}

inline bool CSpectrumBar::IsValidArrayPtr(const CByteArray& Arr, const BYTE *Ptr)
{
	return(Ptr >= Arr.GetData() && Ptr < Arr.GetData() + Arr.GetSize());
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPECTRUMBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
