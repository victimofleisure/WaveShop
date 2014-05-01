// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version
        01      23jan13	default disk threshold to zero
		02		12feb13	add view palette
		03		02mar13	add MP3 import quality
		04		04mar13	add VBR encoding quality
		05		12mar13	add show channel names
        06      01apr13	add real-time spectrum analyzer
        07      17apr13	add temporary files folder
        08      20apr13	add MP4 import
		09		28apr13	refactor to make options a reference

        options dialog
 
*/

// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "OptionsDlg.h"
#include <afxpriv.h>	// for WM_KICKIDLE

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog

IMPLEMENT_DYNAMIC(COptionsDlg, CPropertySheet)

#define OPTIONSPAGEDEF(name) offsetof(COptionsDlg, m_##name##Dlg),
const int COptionsDlg::m_PageOffset[OPTIONS_PAGES] = {
	#include "OptionsPages.h"	// define offsets of property pages within sheet
};

COptionsDlg::COptionsDlg(UINT nIDCaption, COptionsInfo& Options, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
	#define OPTIONSPAGEDEF(name) m_##name##Dlg(m_oi),
	#include "OptionsPages.h"		// construct property pages
	m_oi(Options)	// init reference to options data
{
	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	for (int iPage = 0; iPage < OPTIONS_PAGES; iPage++) {
		CObject	*pObj = reinterpret_cast<CObject *>(LPBYTE(this) + m_PageOffset[iPage]);
		AddPage(STATIC_DOWNCAST(CPropertyPage, pObj));	// add property pages
	}
	m_CurPage = iSelectPage;
}

void COptionsDlg::CreateResetAllButton()
{
	CRect	r, rt;
	GetDlgItem(IDOK)->GetWindowRect(r);
	GetTabControl()->GetWindowRect(rt);
	ScreenToClient(r);
	ScreenToClient(rt);
	int	w = r.Width();
	r.left = rt.left;
	r.right = rt.left + w;
	CString	Title(LPCTSTR(IDS_OPT_RESET_ALL));
	m_ResetAll.Create(Title, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		r, this, IDS_OPT_RESET_ALL);
	m_ResetAll.SetFont(GetFont());
	// adjust tab order so new button comes last
	CWnd	*pCancelBtn = GetDlgItem(IDCANCEL);	// assume cancel is now last
	if (pCancelBtn != NULL)
		m_ResetAll.SetWindowPos(pCancelBtn, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void COptionsDlg::RestoreDefaults()
{
	m_oi = COptionsInfo::m_Defaults;
}

BEGIN_MESSAGE_MAP(COptionsDlg, CPropertySheet)
	//{{AFX_MSG_MAP(COptionsDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDS_OPT_RESET_ALL, OnResetAll)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

BOOL COptionsDlg::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	CreateResetAllButton();	// create reset all button
	SetActivePage(m_CurPage);	// set current page
	return bResult;
}

void COptionsDlg::OnDestroy() 
{
	m_CurPage = GetActiveIndex();
	CPropertySheet::OnDestroy();
}

void COptionsDlg::OnResetAll() 
{
	if (AfxMessageBox(IDS_OPT_RESTORE_DEFAULTS, MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		EndDialog(IDOK);
		RestoreDefaults();
	}
}

LRESULT COptionsDlg::OnKickIdle(WPARAM, LPARAM)
{
	SendMessageToDescendants(WM_KICKIDLE, 0, 0, FALSE, FALSE);
	return 0;
}
