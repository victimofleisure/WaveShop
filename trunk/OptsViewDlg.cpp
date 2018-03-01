// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version
		01		12feb13	add view palette
		02		12mar13	add show channel names
		03		28apr13	remove persistence

        view options dialog
 
*/

// OptsViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "OptsViewDlg.h"
#include <afxpriv.h>	// for WM_KICKIDLE
#include "ViewColorsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsViewDlg dialog

COptsViewDlg::COptsViewDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsViewDlg)
	//}}AFX_DATA_INIT
}

void COptsViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsViewDlg)
	//}}AFX_DATA_MAP
	enum {
		MIN_ZOOM_STEP = 101,
		MAX_ZOOM_STEP = 1000,
		MIN_DENSITY = 100,
		MAX_DENSITY = 10000,
	};
	DDX_Text(pDX, IDC_OPT_VIEW_ZOOM_STEP_HORZ, m_oi.m_ZoomStepHorz);
	DDV_MinMaxDouble(pDX, m_oi.m_ZoomStepHorz, MIN_ZOOM_STEP, MAX_ZOOM_STEP);
	DDX_Text(pDX, IDC_OPT_VIEW_ZOOM_STEP_VERT, m_oi.m_ZoomStepVert);
	DDV_MinMaxDouble(pDX, m_oi.m_ZoomStepVert, MIN_ZOOM_STEP, MAX_ZOOM_STEP);
	DDX_Check(pDX, IDC_OPT_VIEW_TIME_IN_FRAMES, m_oi.m_TimeInFrames);
	DDX_Check(pDX, IDC_OPT_VIEW_VERT_SYNC_CHANS, m_oi.m_VertSyncChans);
	DDX_Check(pDX, IDC_OPT_VIEW_VERT_ZOOM_CURSOR, m_oi.m_VertZoomCursor);
	DDX_Check(pDX, IDC_OPT_VIEW_SHOW_INTERPOLAT, m_oi.m_ShowInterpolation);
	DDX_Check(pDX, IDC_OPT_VIEW_CHECK_FOR_UPDATES, m_oi.m_CheckForUpdates);
	DDX_Check(pDX, IDC_OPT_VIEW_CHANNEL_NAMES, m_oi.m_ShowChannelNames);
	DDX_Text(pDX, IDC_OPT_VIEW_MAX_DENSITY, m_oi.m_MaxDensity);
	DDV_MinMaxDouble(pDX, m_oi.m_MaxDensity, MIN_DENSITY, MAX_DENSITY);
}

BEGIN_MESSAGE_MAP(COptsViewDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsViewDlg)
	ON_BN_CLICKED(IDC_OPT_VIEW_COLORS, OnColors)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsViewDlg message handlers

BOOL COptsViewDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptsViewDlg::OnOK() 
{
	CPropertyPage::OnOK();
}

LRESULT COptsViewDlg::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
    UpdateDialogControls(this, TRUE);
    return FALSE;
}

void COptsViewDlg::OnColors() 
{
	CViewColorsDlg	dlg(m_oi);
	dlg.DoModal();
}
