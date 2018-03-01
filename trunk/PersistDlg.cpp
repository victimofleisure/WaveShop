// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22apr05	initial version
		01		07jul05	keep dialog within screen
		02		23nov07	support Unicode
		03		16dec08	in OnShowWindow, move LoadWnd within bShow block
		04		24mar09	add special handling for main accelerators
		05		21dec12	in OnShowWindow, don't clamp to work area if maximized

        dialog that saves and restores its position
 
*/

// PersistDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PersistDlg.h"
#include "Persist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPersistDlg dialog

IMPLEMENT_DYNAMIC(CPersistDlg, CDialog);

CPersistDlg::CPersistDlg(UINT nIDTemplate, UINT nIDAccel, LPCTSTR RegKey, CWnd *pParent)
	: CDialog(nIDTemplate, pParent), m_RegKey(RegKey)
{
	//{{AFX_DATA_INIT(CPersistDlg)
	//}}AFX_DATA_INIT
	m_WasShown = FALSE;
	m_IDAccel = nIDAccel; 
	m_Accel = nIDAccel != NULL && nIDAccel != IDR_MAINFRAME ? 
		LoadAccelerators(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(nIDAccel)) : NULL;
}

void CPersistDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPersistDlg)
	//}}AFX_DATA_MAP
}

void CPersistDlg::UpdateMenu(CWnd *pWnd, CMenu *pMenu)
{
	CCmdUI	cui;
	cui.m_pMenu = pMenu;
	cui.m_nIndexMax = pMenu->GetMenuItemCount();
	for (UINT i = 0; i < cui.m_nIndexMax; i++) {
		cui.m_nID = pMenu->GetMenuItemID(i);
		if (!cui.m_nID)	// separator
			continue;
		if (cui.m_nID == -1) {	// popup submenu
			CMenu	*pSubMenu = pMenu->GetSubMenu(i);
			if (pSubMenu != NULL)
				UpdateMenu(pWnd, pSubMenu);	// recursive call
		}
		cui.m_nIndex = i;
		cui.m_pMenu = pMenu;
		cui.DoUpdate(pWnd, FALSE);
	}
}

BEGIN_MESSAGE_MAP(CPersistDlg, CDialog)
	//{{AFX_MSG_MAP(CPersistDlg)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPersistDlg message handlers

void CPersistDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	if (m_WasShown)
		CPersist::SaveWnd(REG_SETTINGS, this, m_RegKey);
}

void CPersistDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	if (bShow) {
		if (!m_WasShown && !IsWindowVisible()) {
			m_WasShown = TRUE;
			int	Flags = (GetStyle() & WS_THICKFRAME) ? 0 : CPersist::NO_RESIZE;
			CPersist::LoadWnd(REG_SETTINGS, this, m_RegKey, Flags);
		}
		if (!IsZoomed()) {	// unless we're maximized, clamp to work area
			// in case LoadWnd's SetWindowPlacement places us off-screen
			CRect	r, wr;
			GetWindowRect(r);
			if (SystemParametersInfo(SPI_GETWORKAREA, 0, wr, 0)) {
				CRect	br = wr;
				br.right -= GetSystemMetrics(SM_CXSMICON);
				br.bottom -= GetSystemMetrics(SM_CYCAPTION);
				CPoint	pt = r.TopLeft();
				if (!br.PtInRect(pt)) {	// if dialog is off-screen
					pt.x = CLAMP(pt.x, wr.left, wr.right - r.Width());
					pt.y = CLAMP(pt.y, wr.top, wr.bottom - r.Height());
					r = CRect(pt, CSize(r.Width(), r.Height()));
					MoveWindow(r);
				}
			}
		}
	}
}

BOOL CPersistDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
		if (m_Accel != NULL) {
			if (TranslateAccelerator(m_hWnd, m_Accel, pMsg))
				return(TRUE);
		} else {	// no local accelerator table
			// if non-system key down and main accelerators, give main a try
			if (pMsg->message == WM_KEYDOWN && m_IDAccel == IDR_MAINFRAME
			&& AfxGetMainWnd()->SendMessage(UWM_HANDLEDLGKEY, (WPARAM)pMsg))
				return(TRUE);
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
