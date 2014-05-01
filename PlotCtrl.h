// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08dec12	initial version
        01      19mar13	add GetSeries
        02      24mar13	add baseline attribute to series
        03      25mar13	add tick arrays
        04      26mar13	add log scale
        05      28mar13	add data tip
        06      31mar13	add safe log
		07		07apr13	add UpdateAndInvalidatePlot

		plot control

*/

#if !defined(AFX_PLOTCTRL_H__8DA580FF_35B7_464A_BC56_4AF82D60B1FE__INCLUDED_)
#define AFX_PLOTCTRL_H__8DA580FF_35B7_464A_BC56_4AF82D60B1FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlotCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlotCtrl window

#include "RulerCtrl.h"
#include "ArrayEx.h"
#include "DPoint.h"
#include "DoubleBufDC.h"
#include "DataTipCtrl.h"

class CPlotCtrl : public CWnd
{
	DECLARE_DYNAMIC(CPlotCtrl)
// Construction
public:
	CPlotCtrl();
	BOOL	Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Constants
	enum {	// styles
		HIDE_CLIPPED_VALS	= 0x0001,	// hide values that would be clipped
		PLOT_BORDER			= 0x0002,	// show border around plot area
		HORZ_MAJOR_GRID		= 0x0004,	// show major grid lines for horizontal axis
		HORZ_MINOR_GRID		= 0x0008,	// show minor grid lines for horizontal axis
		VERT_MAJOR_GRID		= 0x0010,	// show major grid lines for vertical axis
		VERT_MINOR_GRID		= 0x0020,	// show minor grid lines for vertical axis
		DATA_TIPS			= 0x0040,	// show tool tip when cursor near data point
		MAJOR_GRID = HORZ_MAJOR_GRID | VERT_MAJOR_GRID,
		MINOR_GRID = HORZ_MINOR_GRID | VERT_MINOR_GRID,
		ALL_GRID = MAJOR_GRID | MINOR_GRID,
		DEFAULT_STYLE = WS_CHILD | WS_VISIBLE | PLOT_BORDER | MAJOR_GRID | DATA_TIPS,
	};
	enum {	// ruler indices
		RULER_LEFT,
		RULER_TOP,
		RULER_RIGHT,
		RULER_BOTTOM,
		RULERS
	};
	enum {	// ruler mask bits
		RM_LEFT		= 0x0001,
		RM_TOP		= 0x0002,
		RM_RIGHT	= 0x0004,
		RM_BOTTOM	= 0x0008,
		RM_HORZ		= RM_TOP | RM_BOTTOM,
		RM_VERT		= RM_LEFT | RM_RIGHT,
		RM_ALL		= RM_HORZ | RM_VERT,
	};
	enum {	// axes
		AXIS_HORZ,
		AXIS_VERT,
		AXES
	};
	enum {	// color indices
		CLR_BKGD,		// background color
		CLR_PLOT_BKGD,	// plot background color
		CLR_GRID,		// grid color
		CLR_TEXT,		// text color
		CLR_BORDER,		// border color
		COLORS,
	};
	enum {	// series flags
		SER_LINE			= 0x0001,	// outline series points
		SER_FILL			= 0x0002,	// fill series points
		SER_MARKER_LINE		= 0x0004,	// outline markers at series points
		SER_MARKER_FILL		= 0x0008,	// fill markers at series points
		SER_TRANSPARENT		= 0x0010,	// use transparent background for 
										// line patterns and hatched fill
		SER_FILL_BASELINE	= 0x0020,	// when filling series, add horizontal
										// baseline to series data
	};
	enum {	// marker styles
		MARK_SQUARE,
		MARK_CIRCLE,
		MARK_DIAMOND,
		MARK_TRIANGLE,
		MARKER_STYLES
	};
	enum {	// default clearances
		HORZ_MIN_MAJOR_TICK_GAP = 40,
		VERT_MIN_MAJOR_TICK_GAP = 30,
		LOG_MIN_MAJOR_TICK_GAP = 10,
		MIN_MINOR_TICK_GAP = 12,
	};

// Types
	typedef CArrayEx<DPoint, DPoint&> DPointArray;
	class CSeries : public WObject {
	public:
		CSeries();
		CSeries& operator=(const CSeries& Series);
		// note new members must be added to ctor and operator=
		// pen attributes apply only if series flags specify outline
		// brush attributes apply only if series flags specify fill
		CString	m_Name;				// series name
		DPointArray	m_Point;		// array of series data points
		double	m_Baseline;			// fill baseline, in data coords
		UINT	m_Flags;			// series flags; see enum above
		int		m_PenWidth;			// pen width in pixels
		int		m_PenStyle;			// pen style, for line patterns
		COLORREF	m_PenColor;		// pen color
		int		m_BrushHatch;		// brush hatch style, or -1 for none
		COLORREF	m_BrushColor;	// brush color
		int		m_BkColor;			// background color for line patterns and hatch
		int		m_MarkerStyle;		// marker style; see enum above
		int		m_MarkerSize;		// marker size in pixels
	};
	typedef CArrayEx<CSeries, CSeries&> CSeriesArray;

// Attributes
public:
	bool	IsValidRuler(int RulerIdx) const;
	CRulerCtrl&	GetRuler(int RulerIdx);
	const CRulerCtrl&	GetRuler(int RulerIdx) const;
	bool	IsRulerVisible(int RulerIdx) const;
	void	ShowRuler(int RulerIdx, bool Enable);
	static	UINT	GetRulerMask(int RulerIdx);
	UINT	GetVisibleRulers() const;
	void	SetVisibleRulers(UINT RulerMask);
	int		GetAxisRuler(int AxisIdx) const;
	void	GetRange(int RulerIdx, CDblRange& Range) const;
	void	SetRange(int RulerIdx, const CDblRange& Range);
	void	GetPlotRect(CRect& Rect) const;
	void	GetMargins(CRect& Margins) const;
	void	SetMargins(const CRect& Margins);
	COLORREF	GetColor(int ColorIdx) const;
	void	SetColor(int ColorIdx, COLORREF Color);
	void	SetSeriesCount(int Count);
	int		GetSeriesCount() const;
	const CSeries&	GetSeries(int SeriesIdx) const;
	CSeries&	GetSeries(int SeriesIdx);
	CPoint	GetLogScale() const;
	UINT	GetFitToData() const;
	void	SetFitToData(UINT RulerMask);
	int		GetDataTipPrecision() const;
	void	SetDataTipPrecision(int Precision);
	static	COLORREF	GetDefaultPenColor(int SeriesIdx);
	static	COLORREF	GetDefaultBrushColor(int SeriesIdx);

// Operations
public:
	void	UpdateRulers();
	void	UpdatePlot();
	void	Update();
	void	UpdateAndInvalidatePlot();
	void	InsertSeries(int SerIdx, CSeries& Series);
	void	RemoveSeries(int SerIdx);
	void	RemoveAllSeries();
	bool	FindPoint(CPoint Target, CPoint& DataPos) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlotCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPlotCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlotCtrl)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	afx_msg BOOL OnNeedText(UINT id, NMHDR* pTTTStruct, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		DEFAULT_PALETTE_SIZE = 32,
		TOOLTIP_ID = 1791,
	};
	static const int m_AxisRuler[2][2];	// ruler indices for each axis
	static const COLORREF m_DefaultPalette[DEFAULT_PALETTE_SIZE];

// Types
	class CRulerInfo : public WObject {
	public:
		CRulerInfo();
		CDblRange	m_Range;		// starting and ending value
	};
	class CMyDataTipCtrl : public CDataTipCtrl {
	public:
		CMyDataTipCtrl();
		virtual	bool	FindPoint(CPoint Target, CPoint& DataPos) const;
		CPlotCtrl	*m_PlotCtrl;
	};

// Member data
	CDoubleBufDC	m_DC;			// double-buffering device context
	CRulerCtrl	m_Ruler[RULERS];	// array of ruler controls
	UINT	m_VisibleRulers;		// bitmask of visible rulers
	UINT	m_FitToData;			// bitmask of rulers that should fit to data
	HFONT	m_Font;					// font handle, or NULL for system font
	CSize	m_WndSize;				// size of window, in client coords
	CRect	m_PlotRect;				// client rectangle excluding visible rulers
	CRulerInfo	m_RulerInfo[RULERS];	// array of ruler info
	COLORREF	m_Color[COLORS];	// array of colors
	CRect	m_Margins;				// plot margins, in pixels
	CSeriesArray	m_Series;		// array of series
	CRulerCtrl::CTickArray	m_TickArray[AXES];	// tick array for each axis
	CMyDataTipCtrl	m_Tip;			// data tip control
	int		m_TipPrecision;			// data tip precision, or -1 for default

// Helpers
	bool	FitToData();
	static	void	DrawMarker(CDC& dc, POINT Point, int Style, int Size);
	static	void	PolyMarker(CDC& dc, const POINT *Point, int Points, int Style, int Size);
	static	double	Wrap(double Val, double Limit);
	static	void	Twiddle(UINT& Data, UINT BitMask, bool SetBits);
	static	double	SafeLog10(double x);
	static	void	ApplyLogScale(CPoint IsLogScale, DPoint& Pt);
	static	void	SetTickGaps(CRulerCtrl& Ruler);
};

inline bool CPlotCtrl::IsValidRuler(int RulerIdx) const
{
	return(RulerIdx >= 0 && RulerIdx < RULERS);
}

inline CRulerCtrl&	CPlotCtrl::GetRuler(int RulerIdx)
{
	ASSERT(IsValidRuler(RulerIdx));
	return(m_Ruler[RulerIdx]);
}

inline const CRulerCtrl& CPlotCtrl::GetRuler(int RulerIdx) const
{
	ASSERT(IsValidRuler(RulerIdx));
	return(m_Ruler[RulerIdx]);
}

inline UINT CPlotCtrl::GetRulerMask(int RulerIdx)
{
	return(RM_LEFT << RulerIdx);
}

inline UINT CPlotCtrl::GetFitToData() const
{
	return(m_FitToData);
}

inline void CPlotCtrl::SetFitToData(UINT RulerMask)
{
	m_FitToData = RulerMask;
}

inline UINT CPlotCtrl::GetVisibleRulers() const
{
	return(m_VisibleRulers);
}

inline void CPlotCtrl::GetRange(int RulerIdx, CDblRange& Range) const
{
	ASSERT(IsValidRuler(RulerIdx));
	Range = m_RulerInfo[RulerIdx].m_Range;
}

inline void CPlotCtrl::GetPlotRect(CRect& Rect) const
{
	Rect = m_PlotRect;
}

inline void CPlotCtrl::GetMargins(CRect& Margins) const
{
	Margins = m_Margins;
}

inline void CPlotCtrl::SetMargins(const CRect& Margins)
{
	m_Margins = Margins;
}

inline COLORREF CPlotCtrl::GetColor(int ColorIdx) const
{
	ASSERT(ColorIdx >= 0 && ColorIdx < COLORS);
	return(m_Color[ColorIdx]);
}

inline void CPlotCtrl::SetColor(int ColorIdx, COLORREF Color)
{
	ASSERT(ColorIdx >= 0 && ColorIdx < COLORS);
	m_Color[ColorIdx] = Color;
}

inline int CPlotCtrl::GetSeriesCount() const
{
	return(m_Series.GetSize());
}

inline void CPlotCtrl::SetSeriesCount(int Count)
{
	m_Series.SetSize(Count);
}

inline const CPlotCtrl::CSeries& CPlotCtrl::GetSeries(int SeriesIdx) const
{
	return(m_Series[SeriesIdx]);
}

inline CPlotCtrl::CSeries& CPlotCtrl::GetSeries(int SeriesIdx)
{
	return(m_Series[SeriesIdx]);
}

inline int CPlotCtrl::GetDataTipPrecision() const
{
	return(m_TipPrecision);
}

inline void CPlotCtrl::SetDataTipPrecision(int Precision)
{
	m_TipPrecision = Precision;
}

inline void CPlotCtrl::UpdateAndInvalidatePlot()
{
	UpdatePlot();
	InvalidateRect(m_PlotRect);	// invalidate plot only, else rulers flicker
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLOTCTRL_H__8DA580FF_35B7_464A_BC56_4AF82D60B1FE__INCLUDED_)
