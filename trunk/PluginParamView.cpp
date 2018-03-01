// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25jun13	initial version

        plugin parameters view
 
*/

// PluginParamView.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PluginParamView.h"
#include "Plugin.h"
#include "ladspa.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPluginParamView

IMPLEMENT_DYNCREATE(CPluginParamView, CScrollView)

CPluginParamView::CPluginParamView()
{
	m_Desc = NULL;
}

CPluginParamView::~CPluginParamView()
{
}

BOOL CPluginParamView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	CWinApp	*pApp = AfxGetApp();
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		pApp->LoadStandardCursor(IDC_ARROW),	// standard cursor
		HBRUSH(COLOR_3DFACE + 1),			// background brush
		pApp->LoadIcon(IDR_MAINFRAME));		// app's icon
	cs.dwExStyle |= WS_EX_CONTROLPARENT;	// enable tabbing for child controls
	return CScrollView::PreCreateWindow(cs);
}

void CPluginParamView::OnDraw(CDC* pDC)
{
	// required because base class is abstract
}

bool CPluginParamView::InitRows(LPCTSTR Path, UINT SampleRate)
{
	ASSERT(m_Desc == NULL);	// reuse not allowed
	m_Desc = CPlugin::Load(m_Dll, Path);	// load plugin
	if (m_Desc == NULL)	// if load failed
		return(FALSE);
	// init row array and calculate maximum widths
	CClientDC	dc(this);
	dc.SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	int	MaxNameWidth = 0;
	CIntRange	MaxBoundWidth(0, 0);
	CPlugin::CPortStats	PortStats;
	CPlugin::GetPortStats(m_Desc, PortStats);	// get plugin's port statistics
	int	rows = INT64TO32(PortStats.m_ControlIn.GetSize());	// get number of rows
	m_Row.SetSize(rows);	// allocate row array
	int	iRow = 0;
	for (iRow = 0; iRow < rows; iRow++) {	// for each parameter row
		CPluginParamRow&	row = m_Row[iRow];
		UINT	iPort = PortStats.m_ControlIn[iRow];
		row.Init(m_Desc, iPort, SampleRate);	// init row data
		// update maximum widths
		CSize	sz = dc.GetTextExtent(row.GetName());
		MaxNameWidth = max(MaxNameWidth, sz.cx);
		CString	s;
		s = CPluginParamRow::ValToStr(row.GetBounds().Start);
		sz = dc.GetTextExtent(s);
		MaxBoundWidth.Start = max(MaxBoundWidth.Start, sz.cx);
		s = CPluginParamRow::ValToStr(row.GetBounds().End);
		sz = dc.GetTextExtent(s);
		MaxBoundWidth.End = max(MaxBoundWidth.End, sz.cx);
	}
	m_Resize.SetParentWnd(this);	// init control resizer
	for (iRow = 0; iRow < rows; iRow++) {	// for each parameter row
		CPluginParamRow&	row = m_Row[iRow];
		if (!row.Create(this, iRow, MaxNameWidth, MaxBoundWidth))	// create row
			AfxThrowResourceException();
		// add row controls to resizing array; only slider and upper bound move
		int	BaseCtrlID = CPluginParamRow::GetBaseCtrlID(iRow);
		m_Resize.AddControl(BaseCtrlID + CPluginParamRow::COL_SLIDER, BIND_LEFT | BIND_RIGHT);
		m_Resize.AddControl(BaseCtrlID + CPluginParamRow::COL_UPPER, BIND_RIGHT);
	}
	m_Resize.FixControls();	// finish resizer init
	HGDIOBJ	font = GetStockObject(DEFAULT_GUI_FONT);
	SendMessageToDescendants(WM_SETFONT, (WPARAM)font);	// set font for child controls
	int	TotalHeight = CPluginParamRow::GetTotalHeight(rows);
	SetScrollSizes(MM_TEXT, CSize(0, TotalHeight));	// set scrollable height
	return(TRUE);
}

void CPluginParamView::GetParams(CPlugin::CParamArray& Param) const
{
	int	rows = GetRowCount();
	Param.SetSize(rows);	// size caller's parameter array
	for (int iRow = 0; iRow < rows; iRow++)	// for each row
		Param[iRow] = float(m_Row[iRow].GetVal());	// retrieve parameter
}

bool CPluginParamView::SetParams(const CPlugin::CParamArray& Param)
{
	if (Param.GetSize() != GetRowCount())	// if parameter/row count mismatch
		return(FALSE);
	int	rows = GetRowCount();
	for (int iRow = 0; iRow < rows; iRow++)	// for each row
		m_Row[iRow].SetVal(Param[iRow]);	// update row
	return(TRUE);
}

void CPluginParamView::RestoreDefaults()
{
	int	rows = GetRowCount();
	for (int iRow = 0; iRow < rows; iRow++)	// for each row
		m_Row[iRow].RestoreDefault();
}

BEGIN_MESSAGE_MAP(CPluginParamView, CScrollView)
	//{{AFX_MSG_MAP(CPluginParamView)
	ON_WM_SIZE()
	ON_WM_MOUSEACTIVATE()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(EN_KILLFOCUS, 0, SHRT_MAX, OnEditKillFocus)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPluginParamView diagnostics

#ifdef _DEBUG
void CPluginParamView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CPluginParamView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPluginParamView message handlers

void CPluginParamView::OnSize(UINT nType, int cx, int cy) 
{
	// disable horizontal scrolling; make scroll width same as window width
	SetScrollSizes(MM_TEXT, CSize(cx, m_totalLog.cy));
	// compensate control resizing for scroll position
	m_Resize.SetOriginShift(CPoint(0, 0) - GetScrollPosition());
	m_Resize.OnSize();	// resize controls to fit horizontally with window
	CScrollView::OnSize(nType, cx, cy);	// do this last
}

int CPluginParamView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return MA_ACTIVATE;	// don't call base class, prevents assertion
}

void CPluginParamView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSliderCtrl	*pSlider = DYNAMIC_DOWNCAST(CSliderCtrl, pScrollBar);
	if (pSlider != NULL) {
		int	nID = pSlider->GetDlgCtrlID();
		int	iRow = CPluginParamRow::GetRowIdx(nID);
		m_Row[iRow].OnSliderChange();
	}
	CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPluginParamView::OnEditKillFocus(UINT nID)
{
	int	iRow = CPluginParamRow::GetRowIdx(nID);
	m_Row[iRow].OnEditChange();
}
