// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24oct12	initial version
        01      30jan13	left-click now zooms in instead of setting now

		time ruler control

*/

// TimeRulerCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "TimeRulerCtrl.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTimeRulerCtrl

IMPLEMENT_DYNAMIC(CTimeRulerCtrl, CZoomRulerCtrl)

CTimeRulerCtrl::CTimeRulerCtrl()
{
}

CTimeRulerCtrl::~CTimeRulerCtrl()
{
}

void CTimeRulerCtrl::GetTargetRect(CRect& Rect) const
{
	Rect = CRect(CPoint(0, 0), GetView()->GetPageSize());
}

void CTimeRulerCtrl::OnDrop(UINT Flags, CPoint Point) 
{
	CWaveShopView	*View = GetView();
	int	offset = View->GetTimeRulerOffset();
	Point.x += offset;	// correct for ruler's offset relative to view
	m_DragOrigin.x += offset;
	Point.x = CLAMP(Point.x, 0, View->GetWndSize().cx);	// clamp point to target
	int	dx = Point.x - m_DragOrigin.x;
	double	DeltaZoom = double(abs(dx)) / View->GetWndSize().cx;
	double	zoom = View->GetZoom();
	if (Flags & MK_SHIFT)	// if shift key pressed
		zoom /= DeltaZoom;	// zoom out
	else	// no modifier key
		zoom *= DeltaZoom;	// zoom in
	int	x = min(Point.x, m_DragOrigin.x);
	View->SetZoom(x, zoom);
	View->ScrollToPosition(View->GetScrollPosition() + x);
}

void CTimeRulerCtrl::StepZoom(CPoint Point, bool In)
{
	CRect	rc;
	GetClientRect(rc);
	if (rc.PtInRect(Point)) {	// if cursor within ruler
		CWaveShopView	*View = GetView();
		if (!View->GetWave().IsEmpty()) {	// if non-empty wave
			int	x = Point.x + View->GetTimeRulerOffset();
			View->StepZoom(x, In);	// step zoom
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTimeRulerCtrl message map

BEGIN_MESSAGE_MAP(CTimeRulerCtrl, CZoomRulerCtrl)
	//{{AFX_MSG_MAP(CTimeRulerCtrl)
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimeRulerCtrl message handlers

void CTimeRulerCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_DragState == DRAG_TRACK)	// if tracking cursor for potential drag
		StepZoom(point, TRUE);	// drag threshold wasn't reached; step zoom in instead
	CZoomRulerCtrl::OnLButtonUp(nFlags, point);
}

void CTimeRulerCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{
	StepZoom(point, FALSE);	// step zoom out
	CZoomRulerCtrl::OnRButtonUp(nFlags, point);
}
