// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26dec12	initial version

		drag ruler control

*/

// DragRulerCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "DragRulerCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDragRulerCtrl

IMPLEMENT_DYNAMIC(CDragRulerCtrl, CRulerCtrl)

CDragRulerCtrl::CDragRulerCtrl()
{
	m_DragState = DRAG_NONE;
	m_DragOrigin = CPoint(0, 0);
}

CDragRulerCtrl::~CDragRulerCtrl()
{
}

void CDragRulerCtrl::EndDrag()
{
	ReleaseCapture();	// release mouse input
	m_DragState = DRAG_NONE;	// reset drag state
}

void CDragRulerCtrl::OnDragBegin(UINT Flags, CPoint Point)
{
}

void CDragRulerCtrl::OnDrop(UINT Flags, CPoint Point)
{
}

/////////////////////////////////////////////////////////////////////////////
// CDragRulerCtrl message map

BEGIN_MESSAGE_MAP(CDragRulerCtrl, CRulerCtrl)
	//{{AFX_MSG_MAP(CDragRulerCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDragRulerCtrl message handlers

void CDragRulerCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_DragState = DRAG_TRACK;	// begin tracking possible drag
	m_DragOrigin = point;	// store drag origin
	SetCapture();	// capture mouse input
	CRulerCtrl::OnLButtonDown(nFlags, point);
}

void CDragRulerCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_DragState != DRAG_NONE) {
		if (m_DragState == DRAG_ACTIVE)
			OnDrop(nFlags, point);
		EndDrag();
	}
	CRulerCtrl::OnLButtonUp(nFlags, point);
}

void CDragRulerCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_DragState == DRAG_TRACK) {
		int	delta;
		if (IsVertical())
			delta = abs(point.y - m_DragOrigin.y);	// track vertical motion only
		else	// horizontal
			delta = abs(point.x - m_DragOrigin.x);	// track horizontal motion only
		int	DragThreshold = GetSystemMetrics(SM_CXDRAG);
		// if mouse motion relative to origin exceeds drag threshold
		if (delta >= DragThreshold) {
			m_DragState = DRAG_ACTIVE;	// set drag state to active
			OnDragBegin(nFlags, point);
			SetFocus();	// take focus so we receive keyboard input
			// recurse to update UI; safe due to changed drag state
			OnMouseMove(nFlags, point);
		}
	}
	CRulerCtrl::OnMouseMove(nFlags, point);
}

void CDragRulerCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CRulerCtrl::OnKillFocus(pNewWnd);
	if (m_DragState != DRAG_NONE)
		EndDrag();
}

BOOL CDragRulerCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
		if (m_DragState != DRAG_NONE)
			EndDrag();
		return TRUE;	// no further dispatching
	}
	return CRulerCtrl::PreTranslateMessage(pMsg);
}
