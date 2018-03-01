// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17may13	initial version
        01      18jun13	in DoDataExchange, validate channel count and sample rate

        record options dialog
 
*/

// OptsRecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "OptsRecordDlg.h"
#include <afxpriv.h>	// for WM_KICKIDLE
#include "RecordDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsRecordDlg dialog


COptsRecordDlg::COptsRecordDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsRecordDlg)
	//}}AFX_DATA_INIT
	m_SelHotKeyFunc = 0;
}

int COptsRecordDlg::GetActivationType()
{
	int	nID = GetCheckedRadioButton(IDC_OPT_REC_ACTIVATION, IDC_OPT_REC_ACTIVATION3);
	int	ActType = nID -= IDC_OPT_REC_ACTIVATION;
	ASSERT(ActType >= 0 && ActType < CRecordDlg::ACTIVATION_TYPES);
	return(ActType);
}

DWORD COptsRecordDlg::GetSysHotKey(const CHotKeyCtrl& HotKeyCtrl)
{
	WORD	VKeyCode, HKModFlags;
	HotKeyCtrl.GetHotKey(VKeyCode, HKModFlags);
	WORD	SysModFlags = 0;
	if (HKModFlags & HOTKEYF_ALT)
		SysModFlags |= MOD_ALT;
	if (HKModFlags & HOTKEYF_CONTROL)
		SysModFlags |= MOD_CONTROL;
	if (HKModFlags & HOTKEYF_SHIFT)
		SysModFlags |= MOD_SHIFT;
	return(MAKELONG(VKeyCode, SysModFlags));
}

void COptsRecordDlg::SetSysHotKey(CHotKeyCtrl& HotKeyCtrl, DWORD HotKeyDef)
{
	WORD	SysModFlags = HIWORD(HotKeyDef);
	ASSERT(!(SysModFlags & MOD_WIN));	// hot key control doesn't support Windows key
	WORD	HKModFlags = 0;
	if (SysModFlags & MOD_ALT)
		HKModFlags |= HOTKEYF_ALT;
	if (SysModFlags & MOD_CONTROL)
		HKModFlags |= HOTKEYF_CONTROL;
	if (SysModFlags & MOD_SHIFT)
		HKModFlags |= HOTKEYF_SHIFT;
	HotKeyCtrl.SetHotKey(LOWORD(HotKeyDef), HKModFlags);
}

void COptsRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsRecordDlg)
	DDX_Control(pDX, IDC_OPT_REC_HOT_KEY_FUNCTION, m_HotKeyFunction);
	DDX_Control(pDX, IDC_OPT_REC_HOT_KEY, m_HotKey);
	DDX_Control(pDX, IDC_OPT_REC_SAMPLE_SIZE, m_SampleSizeCombo);
	DDX_Control(pDX, IDC_OPT_REC_SAMPLE_RATE, m_SampleRateCombo);
	//}}AFX_DATA_MAP
	DDX_Radio(pDX, IDC_OPT_REC_ACTIVATION, m_oi.m_Record.ActivationType);
	DDX_Text(pDX, IDC_OPT_REC_CHANNELS, m_oi.m_Record.Channels);
	DDV_MinMaxUInt(pDX, m_oi.m_Record.Channels, 1, 100);
	DDX_Text(pDX, IDC_OPT_REC_START_DURATION, m_oi.m_Record.StartDuration);
	DDV_MinMaxFloat(pDX, m_oi.m_Record.StartDuration, 0.f, 3600.f);
	DDX_Text(pDX, IDC_OPT_REC_START_LEVEL, m_oi.m_Record.StartLevel);
	DDV_MinMaxFloat(pDX, m_oi.m_Record.StartLevel, -100.f, 0.f);
	DDX_Text(pDX, IDC_OPT_REC_STOP_DURATION, m_oi.m_Record.StopDuration);
	DDV_MinMaxFloat(pDX, m_oi.m_Record.StopDuration, 0.f, 3600.f);
	DDX_Text(pDX, IDC_OPT_REC_STOP_LEVEL, m_oi.m_Record.StopLevel);
	DDV_MinMaxFloat(pDX, m_oi.m_Record.StopLevel, -100.f, 0.f);
	DDX_Text(pDX, IDC_OPT_REC_FOLDER_EDIT, m_oi.m_RecordFolderPath);
	if (m_oi.m_Record.ActivationType != CRecordDlg::ACT_PROMPT)
		COptsEditDlg::ValidateFolder(pDX, IDC_OPT_REC_FOLDER_EDIT, m_oi.m_RecordFolderPath);
	CRecordDlg::ValidateSampleRate(pDX, IDC_OPT_REC_SAMPLE_RATE, 
		m_SampleRateCombo, m_oi.m_Record.SampleRate);
	if (pDX->m_bSaveAndValidate)
		m_oi.m_Record.SampleSize = CRecordDlg::GetSampleSize(m_SampleSizeCombo);
}

BEGIN_MESSAGE_MAP(COptsRecordDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsRecordDlg)
	ON_BN_CLICKED(IDC_OPT_REC_FOLDER_BROWSE, OnFolderBrowse)
	ON_CBN_SELCHANGE(IDC_OPT_REC_HOT_KEY_FUNCTION, OnSelchangeHotKeyFunction)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_UPDATE_COMMAND_UI(IDC_OPT_REC_FOLDER_EDIT, OnUpdateFolderPath)
	ON_UPDATE_COMMAND_UI(IDC_OPT_REC_FOLDER_BROWSE, OnUpdateFolderPath)
	ON_UPDATE_COMMAND_UI(IDC_OPT_REC_START_LEVEL, OnUpdateTriggers)
	ON_UPDATE_COMMAND_UI(IDC_OPT_REC_START_DURATION, OnUpdateTriggers)
	ON_UPDATE_COMMAND_UI(IDC_OPT_REC_STOP_LEVEL, OnUpdateTriggers)
	ON_UPDATE_COMMAND_UI(IDC_OPT_REC_STOP_DURATION, OnUpdateTriggers)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsRecordDlg message handlers

BOOL COptsRecordDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	CRecordDlg::InitSampleRateCombo(m_SampleRateCombo, m_oi.m_Record.SampleRate);
	CRecordDlg::InitSampleSizeCombo(m_SampleSizeCombo, m_oi.m_Record.SampleSize);
	m_HotKeyFunction.SetCurSel(0);
	// disallow unmodified keys and keys only modified by Alt, to reduce conflict
	m_HotKey.SetRules(HKCOMB_NONE | HKCOMB_A, HOTKEYF_SHIFT);
	SetSysHotKey(m_HotKey, m_oi.m_Record.HotKeys.Def[0]);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptsRecordDlg::OnOK() 
{
	m_oi.m_Record.HotKeys.Def[m_SelHotKeyFunc] = GetSysHotKey(m_HotKey);

	CPropertyPage::OnOK();
}

LRESULT COptsRecordDlg::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
    UpdateDialogControls(this, TRUE);
    return FALSE;
}

void COptsRecordDlg::OnFolderBrowse() 
{
	bool	retc = theApp.BrowseFolder(IDS_OPT_SELECT_RECORD_FOLDER, 
		m_oi.m_RecordFolderPath, m_hWnd);
	if (retc) {	// if user didn't cancel folder dialog
		GetDlgItem(IDC_OPT_REC_FOLDER_EDIT)->SetWindowText(m_oi.m_RecordFolderPath);
	}
}

void COptsRecordDlg::OnUpdateFolderPath(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActivationType() != CRecordDlg::ACT_PROMPT);
}

void COptsRecordDlg::OnUpdateTriggers(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActivationType() == CRecordDlg::ACT_SOUND);
}

void COptsRecordDlg::OnSelchangeHotKeyFunction() 
{
	m_oi.m_Record.HotKeys.Def[m_SelHotKeyFunc] = GetSysHotKey(m_HotKey);
	int	sel = m_HotKeyFunction.GetCurSel();
	ASSERT(sel >= 0 && sel < RECORD_PARMS::HOT_KEYS);
	SetSysHotKey(m_HotKey, m_oi.m_Record.HotKeys.Def[sel]);
	m_SelHotKeyFunc = sel;
}
