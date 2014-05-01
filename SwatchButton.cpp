// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      15feb13	initial version
		01		02apr13	add EditColor

		swatch button

*/

// SwatchButton.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "SwatchButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSwatchButton

IMPLEMENT_DYNAMIC(CSwatchButton, CButton);

CSwatchButton::CSwatchButton()
{
	m_Color = 0;
}

CSwatchButton::~CSwatchButton()
{
}

void CSwatchButton::SetColor(COLORREF Color)
{
	m_Color = Color;
	m_Swatch.Invalidate();
}

void CSwatchButton::GetSwatchRect(CRect& Rect) const
{
	GetClientRect(Rect);
	Rect.DeflateRect(SWATCH_MARGIN, SWATCH_MARGIN);
}

bool CSwatchButton::EditColor(COLORREF *CustomColors)
{
	ASSERT(CustomColors != NULL);	// caller must allocate custom colors
	CColorDialog	dlg;
	dlg.m_cc.Flags |= CC_RGBINIT;
	dlg.m_cc.lpCustColors = CustomColors;
	dlg.m_cc.rgbResult = m_Color;
	if (dlg.DoModal() != IDOK)
		return(FALSE);
	SetColor(dlg.m_cc.rgbResult);
	return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CSwatchButton message map

BEGIN_MESSAGE_MAP(CSwatchButton, CButton)
	//{{AFX_MSG_MAP(CSwatchButton)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(BM_SETSTATE, OnSetState)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSwatchButton message handlers

void CSwatchButton::PreSubclassWindow() 
{
	CRect	rSwatch;
	GetSwatchRect(rSwatch);
	m_Swatch.Create(NULL, WS_CHILD | WS_VISIBLE, rSwatch, this);
	ModifyStyle(0, WS_CLIPCHILDREN);	// clip button around swatch
	CButton::PreSubclassWindow();
}

HBRUSH CSwatchButton::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	SetDCBrushColor(pDC->m_hDC, m_Color);
	return (HBRUSH)GetStockObject(DC_BRUSH);
}

LRESULT CSwatchButton::OnSetState(WPARAM wParam, LPARAM lParam)
{
	CRect	rSwatch;
	GetSwatchRect(rSwatch);
	if (wParam)	// if pushed
		rSwatch += CSize(PUSHED_OFFSET, PUSHED_OFFSET);	// offset swatch
	m_Swatch.MoveWindow(rSwatch);	// move swatch to updated position
	return Default();
}
