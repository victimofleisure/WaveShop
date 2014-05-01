// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24oct12	initial version

		zoom ruler control

*/

// ZoomRulerCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ZoomRulerCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZoomRulerCtrl

IMPLEMENT_DYNAMIC(CZoomRulerCtrl, CDragRulerCtrl)

CZoomRulerCtrl::CZoomRulerCtrl()
{
	m_Target = NULL;
	ZeroMemory(m_Marker, sizeof(m_Marker));
}

CZoomRulerCtrl::~CZoomRulerCtrl()
{
}

void CZoomRulerCtrl::OnDragBegin(UINT Flags, CPoint Point) 
{
	m_Target->ModifyStyle(0, WS_CLIPSIBLINGS);
	CRect	rMarker;
	GetMarkerRect(m_DragOrigin, rMarker);
	CreateMarker(rMarker, m_Marker[MARKER_START]);
	GetMarkerRect(Point, rMarker);
	CreateMarker(rMarker, m_Marker[MARKER_END]);
	UpdateCursor(!(Flags & MK_SHIFT));
}

void CZoomRulerCtrl::EndDrag()
{
	CDragRulerCtrl::EndDrag();	// must call base class
	for (int iMark = 0; iMark < MARKERS; iMark++) {	// for each marker
		if (m_Marker[iMark] != NULL) {
			m_Marker[iMark]->DestroyWindow();
			m_Marker[iMark].SetEmpty();	// release marker, thereby deleting it
		}
	}
	m_Target->SetFocus();	// focus target window
}

void CZoomRulerCtrl::UpdateCursor(bool ZoomIn)
{
	int	CursorID = ZoomIn ? IDC_TOOL_ZOOM_IN : IDC_TOOL_ZOOM_OUT;
	HCURSOR	hCursor = AfxGetApp()->LoadCursor(CursorID);
	SetCursor(hCursor);
}

bool CZoomRulerCtrl::CreateMarker(const CRect& Rect, CMarkerPtr& Marker)
{
	Marker.CreateObj();	// allocate marker window instance
	LPCTSTR	ClassName = AfxRegisterWndClass(0);
	DWORD	style = WS_CHILD | WS_VISIBLE;
	if (IsVertical())
		style |= MS_VERTICAL;
	if (!Marker->CreateEx(0, ClassName, NULL, style, Rect, GetParentFrame(), 0))
		return(FALSE);
	Marker->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);	// top of Z-order
	return(TRUE);
}

void CZoomRulerCtrl::GetTargetRect(CRect& Rect) const
{
	m_Target->GetClientRect(Rect);
}

void CZoomRulerCtrl::GetMarkerRect(CPoint Point, CRect& Rect)
{
	CRect	rTarget, rMarker;
	GetTargetRect(rTarget);
	MapWindowPoints(m_Target, &Point, 1);
	if (IsVertical()) {
		int	y = CLAMP(Point.y, rTarget.top, rTarget.bottom);	// clamp to target window
		rMarker = CRect(CPoint(0, y), CSize(rTarget.Size().cx, 1));
	} else {
		int	x = CLAMP(Point.x, rTarget.left, rTarget.right);	// clamp to target window
		rMarker = CRect(CPoint(x, 0), CSize(1, rTarget.Size().cy));
	}
	m_Target->MapWindowPoints(GetParentFrame(), rMarker);
	Rect = rMarker;
}

BOOL CZoomRulerCtrl::CMarker::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message == WM_PAINT) {
		CPaintDC	dc(this);
		CPen	pen(PS_DOT, 1, GetSysColor(COLOR_HIGHLIGHT));
		HGDIOBJ	PrevPen = dc.SelectObject(pen);	// select dotted pen
		CRect	rc;
		GetClientRect(rc);
		DWORD	style = GetStyle();
		CPoint	start, end;
		if (style & MS_VERTICAL) {	// if vertical marker
			start = CPoint(rc.left, rc.top);
			end = CPoint(rc.right, rc.top);
		} else {	// horizontal marker
			start = CPoint(rc.left, rc.top);
			end = CPoint(rc.left, rc.bottom);
		}
		dc.MoveTo(start);	// draw marker
		dc.LineTo(end);
		dc.SelectObject(PrevPen);	// select previous pen
		return TRUE;
	}
	return CWnd::OnWndMsg(message, wParam, lParam, pResult);
}

/////////////////////////////////////////////////////////////////////////////
// CZoomRulerCtrl message map

BEGIN_MESSAGE_MAP(CZoomRulerCtrl, CDragRulerCtrl)
	//{{AFX_MSG_MAP(CZoomRulerCtrl)
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZoomRulerCtrl message handlers

void CZoomRulerCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_DragState == DRAG_ACTIVE) {
		CRect	rMarker;
		GetMarkerRect(point, rMarker);
		ASSERT(m_Marker[MARKER_END] != NULL);
		m_Marker[MARKER_END]->MoveWindow(rMarker);
	}
	CDragRulerCtrl::OnMouseMove(nFlags, point);
}

BOOL CZoomRulerCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) {
		switch (pMsg->wParam) {
		case VK_SHIFT:
			UpdateCursor(pMsg->message == WM_KEYUP);
			break;
		}
	}
	return CDragRulerCtrl::PreTranslateMessage(pMsg);
}
