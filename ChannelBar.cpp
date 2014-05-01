// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version
        01      23mar13	add minimum minor tick gap

		channel control bar

*/

// ChannelBar.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "ChannelBar.h"
#include "WaveShopDoc.h"
#include "WaveShopView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChannelBar

IMPLEMENT_DYNAMIC(CChannelBar, CControlBar);

CChannelBar::CChannelBar()
{
	m_Width = 0;
	m_ViewOffset = 0;
	m_MinMajorTickGap = 20;
	m_MinMinorTickGap = 10;
	m_WidthPending = FALSE;
}

CChannelBar::~CChannelBar()
{
}

BOOL CChannelBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	m_dwStyle = (dwStyle & CBRS_ALL);
	dwStyle &= ~CBRS_ALL;
	dwStyle |= WS_CLIPCHILDREN;
	HCURSOR	hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	HBRUSH	hBrush = HBRUSH(COLOR_3DFACE + 1);
	LPCTSTR	lpszClass = AfxRegisterWndClass(CS_DBLCLKS, hCursor, hBrush);
    ASSERT(lpszClass);
	CRect	r(0, 0, 0, 0);
	if (!CControlBar::Create(lpszClass, _T("ChannelBar"), dwStyle, r, pParentWnd, nID))
		return FALSE;
	return TRUE;
}

int CChannelBar::FindRuler(const CRulerCtrl *Ruler) const
{
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++) {
		if (Ruler == m_Ruler[iChan])
			return(iChan);
	}
	return(-1);
}

bool CChannelBar::SetChannelCount(int Channels)
{
	int	PrevChans = GetChannelCount();
	if (Channels == PrevChans)
		return(TRUE);
	if (Channels > PrevChans) {	// if adding channels
		m_Ruler.SetSize(Channels);
		CRect	InitRect(0, 0, 0, 0);
		UINT	style = WS_CHILD | WS_VISIBLE	// window styles
			| CBRS_ALIGN_LEFT | CRulerCtrl::HIDE_CLIPPED_VALS;	// ruler styles
		HGDIOBJ	hFont = GetStockObject(DEFAULT_GUI_FONT);
		for (int iChan = PrevChans; iChan < Channels; iChan++) {
			CChannelRulerCtrl	*pRuler = new CChannelRulerCtrl;
			pRuler->Create(style, InitRect, this, 0);
			pRuler->SendMessage(WM_SETFONT, WPARAM(hFont));
			pRuler->SetMinMajorTickGap(m_MinMajorTickGap);
			pRuler->SetMinMinorTickGap(m_MinMinorTickGap);
			pRuler->SetTarget(m_View);
			m_Ruler[iChan] = pRuler;
		}
	} else {	// removing channels
		for (int iChan = Channels; iChan < PrevChans; iChan++) {
			m_Ruler[iChan]->DestroyWindow();
			delete m_Ruler[iChan];
		}
		m_Ruler.SetSize(Channels);
	}
	return(TRUE);
}

void CChannelBar::SetChannelPos(int ChannelIdx, int y, int Height)
{
//printf("CChannelBar::SetChannelPos %d %d %d\n", ChannelIdx, y, Height);
	CChannelRulerCtrl	*pRuler = m_Ruler[ChannelIdx];
	pRuler->MoveWindow(0, y + m_ViewOffset, m_Width, Height);
	pRuler->UpdateZoom(Height);
	UpdateWidth();
}

void CChannelBar::SetAmplitude(int ChannelIdx, double Origin, double Amplitude)
{
//printf("CChannelBar::SetAmplitude %d %f %f\n", ChannelIdx, Origin, Amplitude);
	CChannelRulerCtrl	*pRuler = m_Ruler[ChannelIdx];
	pRuler->SetOrigin(Origin);
	pRuler->SetAmplitude(Amplitude);
	CRect	rc;
	pRuler->GetClientRect(rc);
	pRuler->UpdateZoom(rc.Height());
	UpdateWidth();
}

int CChannelBar::CalcTextWidth() const
{
	int	width = 0;
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++) {
		CRulerCtrl	*pRuler = m_Ruler[iChan];
		CSize	sz = pRuler->CalcTextExtent();
		width = max(sz.cx, width);
	}
	if (width)
		width += GetSystemMetrics(SM_CXEDGE);	// add a bit of left margin
	return(width);
}

void CChannelBar::OnChannelPosChange()
{
//printf("CChannelBar::OnChannelPosChange\n");
	CPoint	org(0, 0);
	m_View->MapWindowPoints(this, &org, 1);
	m_ViewOffset = org.y;
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++) {
		CRulerCtrl	*pRuler = m_Ruler[iChan];
		int	Height;
		int	y = m_View->GetChannelPos(iChan, Height);
		pRuler->MoveWindow(0, y + m_ViewOffset, m_Width, Height);
	}
}

void CChannelBar::UpdateWidth()
{
//printf("CChannelBar::UpdateWidth %d\n", m_WidthPending);
	if (!m_WidthPending) {	// if width update not already pending in queue
		m_WidthPending = TRUE;
		PostMessage(UWM_CHAN_BAR_UPDATE_WIDTH);	// use post to allow delay
	}
}

void CChannelBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	// pure virtual but not implemented by base class
}

BEGIN_MESSAGE_MAP(CChannelBar, CControlBar)
	//{{AFX_MSG_MAP(CChannelBar)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PARENTNOTIFY()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_CHAN_BAR_UPDATE_WIDTH, OnUpdateWidth)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChannelBar message handlers

void CChannelBar::OnDestroy() 
{
	CControlBar::OnDestroy();
	SetChannelCount(0);	
}

CSize CChannelBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
//printf("CChannelBar::CalcFixedLayout %d %d\n", bStretch, bHorz);
	ASSERT(!bHorz);
	return(CSize(m_Width, 32767));	// stretch vertically to fill frame
}

void CChannelBar::OnSize(UINT nType, int cx, int cy) 
{
	CControlBar::OnSize(nType, cx, cy);
	int	chans = GetChannelCount();
	if (chans) {
		CPoint	org(0, 0);
		m_View->MapWindowPoints(this, &org, 1);
		m_ViewOffset = org.y;
		for (int iChan = 0; iChan < chans; iChan++) {
			CRulerCtrl	*pRuler = m_Ruler[iChan];
			CRect	r;
			pRuler->GetWindowRect(r);
			ScreenToClient(r);
			r.left = 0;
			r.right = m_Width;
			pRuler->MoveWindow(r);
		}
	}
}

LRESULT	CChannelBar::OnUpdateWidth(WPARAM wParam, LPARAM lParam)
{
	int	width = CalcTextWidth();
//printf("CChannelBar::OnUpdateWidth %d %d\n", width, m_Width);
	if (width != m_Width) {
		int	PrevViewWidth = m_View->GetWndSize().cx;	// save view width
		m_Width = width;	// order matters: CalcFixedLayout uses m_Width
		GetParentFrame()->RecalcLayout();
		// Adjust zoom for view's new width so that visible portion of wave
		// remains unchanged; otherwise message feedback can occur if view
		// is near threshold of showing/hiding its horizontal scroll bar.
		// For example, view's reduced width makes it show a scroll bar, so
		// view reduces channel heights; channel rulers revert to narrower
		// numbers, so channel bar narrows itself again, and around we go.
		int	ViewWidth = m_View->GetWndSize().cx;
		double	zoom = m_View->GetZoom() * double(PrevViewWidth) / ViewWidth;
		m_View->SetZoom(ViewWidth / 2, zoom);	// compensate zoom
	}
	m_WidthPending = FALSE;	// width is no longer pending
	return(0);
}

void CChannelBar::OnParentNotify(UINT message, LPARAM lParam) 
{
	CControlBar::OnParentNotify(message, lParam);
	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		m_View->SetFocus();
		break;
	}
}

void CChannelBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// handles click in gutter between channel rulers
	CControlBar::OnLButtonDown(nFlags, point);
	m_View->SetFocus();
}
