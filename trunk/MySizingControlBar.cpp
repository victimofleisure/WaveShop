// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      31jan07	initial version
        01      18apr08	add show window handler
		02		14mar09	add style changed handler
		03		25may10	add size valid flag
		04		29mar13	remove FastIsVisible
		05      23apr13	handle command help

        wrapper for Cristi Posea's sizable control bar
 
*/

// MySizingControlBar.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MySizingControlBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMySizingControlBar dialog

IMPLEMENT_DYNAMIC(CMySizingControlBar, CSizingControlBarG);

CMySizingControlBar::CMySizingControlBar()
{
	//{{AFX_DATA_INIT(CMySizingControlBar)
	//}}AFX_DATA_INIT
	m_IsSizeValid = FALSE;
}

BEGIN_MESSAGE_MAP(CMySizingControlBar, CSizingControlBarG)
	//{{AFX_MSG_MAP(CMySizingControlBar)
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMySizingControlBar message handlers

void CMySizingControlBar::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	if (!(lpwndpos->flags & SWP_NOSIZE))	// if size is being changed
		m_IsSizeValid = TRUE;	// OnSize arguments are valid from now on
	CSizingControlBarG::OnWindowPosChanged(lpwndpos);
}

LRESULT CMySizingControlBar::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
	AfxGetApp()->WinHelp(GetDlgCtrlID());
	return TRUE;
}
