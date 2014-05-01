// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version

		time ruler control bar

*/

// TimeRulerBar.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "TimeRulerBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTimeRulerBar

IMPLEMENT_DYNAMIC(CTimeRulerBar, CRulerBar)

CTimeRulerBar::CTimeRulerBar()
{
}

CTimeRulerBar::~CTimeRulerBar()
{
}

bool CTimeRulerBar::CreateRuler() 
{
	ASSERT(m_Ruler == NULL);	// ruler can only be created once
	m_Ruler = new CTimeRulerCtrl;
	UINT	style = WS_CHILD | WS_VISIBLE | m_dwStyle;	// include bar style
	CRect	r(0, 0, 0, 0);
	return(GetRuler()->Create(style, r, this, 0) != 0);	// derived create
}

BEGIN_MESSAGE_MAP(CTimeRulerBar, CRulerBar)
	//{{AFX_MSG_MAP(CTimeRulerBar)
	ON_WM_PARENTNOTIFY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimeRulerBar message handlers

void CTimeRulerBar::OnParentNotify(UINT message, LPARAM lParam) 
{
	CRulerBar::OnParentNotify(message, lParam);
	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		GetRuler()->GetTarget()->SetFocus();
		break;
	}
}
