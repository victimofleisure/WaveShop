// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version

		ruler control bar

*/

// RulerBar.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "RulerBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRulerBar

IMPLEMENT_DYNAMIC(CRulerBar, CControlBar)

CRulerBar::CRulerBar()
{
	m_Ruler = NULL;
	m_Height = 16;	// reasonable default for horizontal bar
}

CRulerBar::~CRulerBar()
{
	delete m_Ruler;
}

BOOL CRulerBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	m_dwStyle = (dwStyle & CBRS_ALL);
	dwStyle &= ~CBRS_ALL;
	HCURSOR	hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	LPCTSTR	lpszClass = AfxRegisterWndClass(CS_DBLCLKS, hCursor);	// no background brush
    ASSERT(lpszClass);
	CRect	r(0, 0, 0, 0);
	if (!CControlBar::Create(lpszClass, _T("RulerBar"), dwStyle, r, pParentWnd, nID))
		return FALSE;
	if (!CreateRuler())
		return FALSE;
	return TRUE;
}

bool CRulerBar::CreateRuler()
{
	ASSERT(m_Ruler == NULL);	// ruler can only be created once
	m_Ruler = new CRulerCtrl;
	UINT	style = WS_CHILD | WS_VISIBLE | m_dwStyle;	// include bar style
	CRect	r(0, 0, 0, 0);
	return(m_Ruler->Create(style, r, this, 0) != 0);
}

void CRulerBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	// pure virtual but not implemented by base class
}

BEGIN_MESSAGE_MAP(CRulerBar, CControlBar)
	//{{AFX_MSG_MAP(CRulerBar)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRulerBar message handlers

CSize CRulerBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize	sz;
	if (bHorz)	// if horizontal
		sz = CSize(32767, m_Height);	// stretch horizontally to fill frame
	else	// vertical
		sz = CSize(m_Height, 32767);	// stretch vertically to fill frame
	return(sz);
}

void CRulerBar::OnSize(UINT nType, int cx, int cy) 
{
	CControlBar::OnSize(nType, cx, cy);
	if (m_Ruler != NULL)
		m_Ruler->MoveWindow(0, 0, cx, cy);
}
