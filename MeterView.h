// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09mar13	initial version
        01      26mar13	move double buffering to its own class
		02		23apr13	override OnMouseActivate to prevent activation
		03		26apr13	add wave member
		04		09may13	add clip detection

        meter view
 
*/

#if !defined(AFX_METERVIEW_H__0E72E047_568A_45B9_B938_301A4B2BF566__INCLUDED_)
#define AFX_METERVIEW_H__0E72E047_568A_45B9_B938_301A4B2BF566__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MeterView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMeterView view

#include "ArrayEx.h"
#include "DoubleBufDC.h"

class CWave;
class CWaveEdit;

class CMeterView : public CScrollView
{
protected:
	CMeterView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMeterView)

// Attributes
public:
	int		GetChannelCount() const;
	UINT	GetTimerFrequency() const;
	void	SetTimerFrequency(UINT Freq);
	void	SetWave(const CWaveEdit *Wave);
	bool	IsClipped(int ChannelIdx) const;
	double	GetLevel(int ChannelIdx) const;
	double	GetPeakLevel(int ChannelIdx) const;
	double	GetMaxLevel() const;

// Operations
public:
	void	UpdateView();
	void	UpdateMeters();
	void	UpdateCaptions();
	void	TimerHook(W64INT Frame);
	void	TimerHook();
	void	ResetAllClipping();
	void	ResetClipping(int ChannelIdx);
	static	double	LevelToDecibels(double Level);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeterView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMeterView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CMeterView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	struct SEGMENT {	// meter segment
		COLORREF	UnlitColor;	// color when unlit
		COLORREF	LitColor;	// color when lit
		int		Height;			// height in decibels
	};
	struct METER {
		double	Level;			// level, in normalized decibels
		double	PeakLevel;		// peak level, in normalized decibels
		int		PeakHold;		// peak hold timer, in timer ticks
		int		OnRailsCount;	// number of consecutive samples on the rails,
								// or -1 if clipping threshold was reached
	};
	typedef CArrayEx<METER, METER&> CMeterArray;

// Constants
	enum {	// meter segments
		SEG_SOFT,
		SEG_LOUD,
		SEG_PEAK,
		SEGMENTS
	};
	enum {
		RANGE = 48,				// meter range, in decibels
		BAR_WIDTH = 21,			// bar width, in pixels
		GUTTER = 3,				// gutter between bars, in pixels
		MARGIN_X = 5,			// horizontal margin around bars, in pixels
		MARGIN_TOP = 2,			// vertical margin above bars, in pixels
		MARGIN_BOTTOM = 5,		// vertical margin below bars, in pixels
		FONT_HEIGHT = 10,		// meter font height, in pixels
		TEXT_LEADING = 5,		// meter text leading, in pixels
		CAPTION_Y = 2,			// caption text offset, in pixels
		CAPTION_BOTTOM_MARGIN = 2,	// caption bottom margin, in pixels
	};
	static const SEGMENT	m_Segment[SEGMENTS];	// array of segment data

// Member data
	CDoubleBufDC	m_DC;		// double-buffered device context
	HGDIOBJ	m_PrevFont;			// previously selected font
	CSize	m_ClientSize;		// size of client area
	CSize	m_ViewSize;			// size of view
	CFont	m_MeterFont;		// meter font
	int		m_CaptionHeight;	// caption height, in pixels, including margins
	int		m_Ticks;			// number of ticks
	UINT	m_Timer;			// count of timer hook calls, for smoothing
	UINT	m_TimerFrequency;	// timer frequency, in Hz
	const CWaveEdit	*m_Wave;	// wave to monitor instead of active view's
	CIntArrayEx	m_MaxSamp;		// array of maximum samples, one per channel
	CMeterArray	m_Meter;		// array of meter states, one per channel

// Helpers
	void	CreateBackBuffer(int Width, int Height);
	int		CalcTickCount(int Height) const;
	int		CalcBarHeight(int Height) const;
	void	UpdatePeaks(const CWave& Wave);
	const CWaveEdit	*GetWave() const;
	int		FindClipIndicator(CPoint point) const;
	void	ComputeLevels(const CWave& Wave, W64INT Offset, W64INT Frames, CW64IntRange Selection, bool Repeat);
};

inline int CMeterView::GetChannelCount() const
{
	return(m_Meter.GetSize());
}

inline UINT CMeterView::GetTimerFrequency() const
{
	return(m_TimerFrequency);
}

inline void CMeterView::SetTimerFrequency(UINT Freq)
{
	m_TimerFrequency = Freq;
}

inline void CMeterView::SetWave(const CWaveEdit *Wave)
{
	m_Wave = Wave;
}

inline bool CMeterView::IsClipped(int ChannelIdx) const
{
	return(m_Meter[ChannelIdx].OnRailsCount < 0);
}

inline double CMeterView::GetLevel(int ChannelIdx) const
{
	return(m_Meter[ChannelIdx].Level);
}

inline double CMeterView::GetPeakLevel(int ChannelIdx) const
{
	return(m_Meter[ChannelIdx].PeakLevel);
}

inline double CMeterView::LevelToDecibels(double Level)
{
	return((1 - Level) * -RANGE);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_METERVIEW_H__0E72E047_568A_45B9_B938_301A4B2BF566__INCLUDED_)
