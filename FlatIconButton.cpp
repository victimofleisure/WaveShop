// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      20nov12	initial version

		toolbar-style flat icon button

*/

// FlatIconButton.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "FlatIconButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFlatIconButton

CFlatIconButton::CFlatIconButton()
{
	m_Hovering = FALSE;
	m_Checked = FALSE;
	m_AutoCheck = FALSE;
	m_IconUp = NULL;
	m_IconDown = NULL;
}

CFlatIconButton::~CFlatIconButton()
{
}

void CFlatIconButton::PreSubclassWindow() 
{
	SetButtonStyle(GetButtonStyle() | BS_OWNERDRAW);	// force owner-draw
	CButton::PreSubclassWindow();
}

void CFlatIconButton::SetIcons(int ResUp, int ResDown)
{
	m_IconUp = SimpleLoadIcon(ResUp);
	m_IconDown = SimpleLoadIcon(ResDown);
}

HICON CFlatIconButton::SimpleLoadIcon(int ResID)
{
	return((HICON)LoadImage(AfxGetApp()->m_hInstance, 
		MAKEINTRESOURCE(ResID), IMAGE_ICON, 0, 0, 0));
}

bool CFlatIconButton::HitTest(CPoint Point)
{
	CRect	rc;
	GetClientRect(rc);
	return(rc.PtInRect(Point) != 0);
}

BEGIN_MESSAGE_MAP(CFlatIconButton, CButton)
	//{{AFX_MSG_MAP(CFlatIconButton)
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_MESSAGE(BM_GETCHECK, OnGetCheck)
	ON_MESSAGE(BM_SETCHECK, OnSetCheck)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFlatIconButton message handlers

LRESULT CFlatIconButton::OnSetCheck(WPARAM wParam, LPARAM lParam)
{
	bool	check = wParam & 1;
	if (check != m_Checked) {	// if checked state changed
		m_Checked = check;
		Invalidate();
	}
	return(0);
}

LRESULT CFlatIconButton::OnGetCheck(WPARAM wParam, LPARAM lParam)
{
	return(m_Checked);
}

void CFlatIconButton::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (m_AutoCheck) {
		if (HitTest(point))	// if cursor within button
			SetCheck(!m_Checked);	// toggle checked state
	}
	CButton::OnLButtonDblClk(nFlags, point);
}

void CFlatIconButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_AutoCheck) {
		// if button still pushed and cursor within button
		if ((GetState() & BST_PUSHED) && HitTest(point))
			SetCheck(!m_Checked);	// toggle checked state
	}
	CButton::OnLButtonUp(nFlags, point);
}

void CFlatIconButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	bool	IsHit = HitTest(point);
	if (m_Hovering) {	// if hovering
		if (!IsHit) {	// if cursor not on button anymore
			m_Hovering = FALSE;	// reset hovering
			Invalidate();
		}
	} else {	// not hovering
		if (IsHit) {	// if cursor on button
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = GetSafeHwnd();
			tme.dwHoverTime = HOVER_DEFAULT;
			_TrackMouseEvent(&tme);	// request leave notification
			m_Hovering = TRUE;	// enable hovering
			Invalidate();
		}
	}
	CButton::OnMouseMove(nFlags, point);
}

LRESULT CFlatIconButton::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_Hovering = FALSE;	// reset hovering
	Invalidate();
	return(0);
}

BOOL CFlatIconButton::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;	// no need to erase background
}

void CFlatIconButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	HDC	hDC = lpDrawItemStruct->hDC;
	CRect	rc(lpDrawItemStruct->rcItem);
	UINT	state = lpDrawItemStruct->itemState;
	CDC	dc;
	dc.Attach(hDC);	// attach device context object to handle
	HICON	hIcon;
	// owner-draw buttons don't support ODS_CHECKED; we must manage checked state
	if (m_Checked) {	// if button is checked
		dc.SetBkColor(GetSysColor(COLOR_WINDOW));
		dc.SetTextColor(GetSysColor(COLOR_3DFACE));
		dc.FillRect(rc, CDC::GetHalftoneBrush());	// halftoned background
		hIcon = m_IconDown;
	} else {	// button is unchecked
		dc.FillSolidRect(rc, GetSysColor(COLOR_3DFACE));	// normal background
		hIcon = m_IconUp;
	}
	if ((state & ODS_SELECTED) || m_Checked) {	// if selected or checked
		dc.Draw3dRect(rc, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));
	} else if (m_Hovering) {	// if cursor is over button
		dc.Draw3dRect(rc, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));
	}
	ICONINFO	info;
	if (GetIconInfo(hIcon, &info)) {	// get icon info
		BITMAP	bm;
		if (GetObject(info.hbmMask, sizeof(bm), &bm)) {	// get icon bitmap
			CSize	sz(bm.bmWidth, bm.bmHeight);	// center icon within button
			CPoint	pt((rc.Width() - sz.cx) / 2, (rc.Height() - sz.cy) / 2);
			if ((state & ODS_SELECTED) || m_Checked)	// if selected or checked
				pt += CSize(1, 1);	// offset icon slightly
			UINT	nFlags;
			if (state & ODS_DISABLED)	// if disabled
				nFlags = DSS_DISABLED;	// embossed icon
			else	// normal
				nFlags = DSS_NORMAL;	// unmodified icon
			HBRUSH	hBrush = NULL; 
			dc.DrawState(pt, sz, hIcon, nFlags, hBrush);	// draw icon
		}
	}
}
