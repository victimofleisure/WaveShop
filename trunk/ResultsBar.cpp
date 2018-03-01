// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		12jan13	initial version
		01		04mar13	add owner data
		02		29mar13	in OnSize, test for valid size

        results bar
 
*/

// ResultsBar.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "ResultsBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResultsBar

IMPLEMENT_DYNAMIC(CResultsBar, CMySizingControlBar);

CResultsBar::CResultsBar()
{
	m_szVert = CSize(265, 0);	// default width when vertically docked
	m_View = NULL;
}

CResultsBar::~CResultsBar()
{
}

void CResultsBar::OnDestroyView(CWaveShopView *View)
{
	if (View == m_View) {	// if our owner view was destroyed
		m_Report.SetEmpty();	// delete report
		m_View = NULL;
	}
}

bool CResultsBar::ReportClipping(CWaveProcess::CClipSpanArray& ClipSpan, CWaveShopView *View)
{
	if (m_Report.IsEmpty())
		m_Report.CreateObj();	// create new report control instance
	m_View = View;	// set our owner view
	if (m_Report->m_hWnd == NULL) {	// if control doesn't exist, create it
		CRect	rc;
		GetClientRect(rc);
		UINT	style = WS_CHILD | WS_VISIBLE | LVS_OWNERDATA	// virtual list
			| LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS;
		if (!m_Report->Create(style, rc, this, 0))	// create report control
			return(FALSE);
	}
	m_Report->Update(ClipSpan, View);	// transfer clip span array to control
	return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CResultsBar message map

BEGIN_MESSAGE_MAP(CResultsBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CResultsBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResultsBar message handlers

int CResultsBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CResultsBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_IsSizeValid) {	// if size is valid
		if (!m_Report.IsEmpty())	// if report control exists
			m_Report->MoveWindow(0, 0, cx, cy);
	}
}
