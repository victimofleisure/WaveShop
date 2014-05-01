// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25oct12	initial version
		01		02mar13	add MP3 import quality
		02		04mar13	add VBR encoding quality
        03      20apr13	add MP4 import
        04      27apr13	add record device
		05		28apr13	remove persistence
		06		05may13	use GUID instead of description for device persistence
		07		10may13	add meter clip threshold

        audio options dialog
 
*/

// OptsAudioDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "OptsAudioDlg.h"
#include <afxpriv.h>	// for WM_KICKIDLE
#include "DSCapture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsAudioDlg dialog

COptsAudioDlg::COptsAudioDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsAudioDlg)
	//}}AFX_DATA_INIT
}

void COptsAudioDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsAudioDlg)
	DDX_Control(pDX, IDC_OPT_VBR_QUALITY_SLIDER, m_VBRQualitySlider);
	DDX_Control(pDX, IDC_OPT_VBR_QUALITY_EDIT, m_VBRQualityEdit);
	DDX_Control(pDX, IDC_OPT_AUDIO_PLAY_DEVICE, m_PlayDeviceCombo);
	DDX_Control(pDX, IDC_OPT_AUDIO_RECORD_DEVICE, m_RecordDeviceCombo);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_OPT_AUDIO_PLAY_BUF_SIZE, m_oi.m_PlayBufferSize);
	DDV_MinMaxInt(pDX, m_oi.m_PlayBufferSize, 1, SHRT_MAX);
	DDX_Text(pDX, IDC_OPT_AUDIO_RECORD_BUF_SIZE, m_oi.m_RecordBufferSize);
	DDV_MinMaxInt(pDX, m_oi.m_RecordBufferSize, 1, SHRT_MAX);
	DDX_CBIndex(pDX, IDC_OPT_MP3_IMPORT_QUALITY, m_oi.m_MP3ImportQuality);
	DDX_CBIndex(pDX, IDC_OPT_MP4_IMPORT_QUALITY, m_oi.m_MP4ImportQuality);
	DDX_Check(pDX, IDC_OPT_MP4_DOWNMIX, m_oi.m_MP4Downmix);
	DDX_Text(pDX, IDC_OPT_METER_CLIP_THRESHOLD, m_oi.m_MeterClipThreshold);
	DDV_MinMaxInt(pDX, m_oi.m_MeterClipThreshold, 1, INT_MAX);
}

bool COptsAudioDlg::PopulatePlayDeviceCombo()
{
	m_PlayDeviceCombo.ResetContent();
	if (!CDSPlayer::EnumDevices(m_PlayDevInfo))	// enumerate playback devices
		return(FALSE);
	PopulateDeviceCombo(m_PlayDeviceCombo, m_PlayDevInfo, m_oi.m_PlayDeviceGuid);
	return(TRUE);
}

bool COptsAudioDlg::PopulateRecordDeviceCombo()
{
	m_RecordDeviceCombo.ResetContent();
	if (!CDSCapture::EnumDevices(m_RecordDevInfo))	// enumerate capture devices
		return(FALSE);
	PopulateDeviceCombo(m_RecordDeviceCombo, m_RecordDevInfo, m_oi.m_RecordDeviceGuid);
	return(TRUE);
}

void COptsAudioDlg::PopulateDeviceCombo(CComboBox& Combo, const CDSDeviceInfoArray& DevInfo, const GUID& DeviceGuid)
{
	int	iSelDev = -1;
	int	iDefaultDev = -1;
	int	devs = DevInfo.GetSize();
	for (int iDev = 0; iDev < devs; iDev++) {	// for each device
		const CDSPlayer::CDSDeviceInfo&	info = DevInfo[iDev];
		Combo.AddString(info.m_Description);
		if (IsEqualGUID(info.m_Guid, DeviceGuid))	// if selected device
			iSelDev = iDev;
		if (IsEqualGUID(info.m_Guid, GUID_NULL))	// if default device
			iDefaultDev = iDev;
	}
	if (iSelDev < 0) {	// if selected device not found
		if (iDefaultDev >= 0)	// if default device was found
			iSelDev = iDefaultDev;	// select default device
		else {	// no default device; shouldn't happen
			Combo.InsertString(0, _T("<default>"));
			iSelDev = 0;
		}
	}
	Combo.SetCurSel(iSelDev);
}

CString COptsAudioDlg::GetSelectedDeviceName(CComboBox& Combo, const CDSDeviceInfoArray& DevInfo)
{
	CString	name;
	int	iSelDev = Combo.GetCurSel();
	if (iSelDev >= 0 && iSelDev < DevInfo.GetSize())	// if valid selection
		name = DevInfo[iSelDev].m_Description;
	return(name);
}

BEGIN_MESSAGE_MAP(COptsAudioDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsAudioDlg)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsAudioDlg message handlers

BOOL COptsAudioDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	PopulatePlayDeviceCombo();
	PopulateRecordDeviceCombo();
	m_VBRQualitySlider.SetEditCtrl(&m_VBRQualityEdit);
	m_VBRQualityEdit.SetVal(m_oi.m_VBREncodingQuality);
	m_VBRQualityEdit.SetFormat(CNumEdit::DF_INT);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptsAudioDlg::OnOK() 
{
	int	sel = m_PlayDeviceCombo.GetCurSel();
	if (sel >= 0)
		m_oi.m_PlayDeviceGuid = m_PlayDevInfo[sel].m_Guid;
	sel = m_RecordDeviceCombo.GetCurSel();
	if (sel >= 0)
		m_oi.m_RecordDeviceGuid = m_RecordDevInfo[sel].m_Guid;
	m_oi.m_VBREncodingQuality = m_VBRQualityEdit.GetIntVal();

	CPropertyPage::OnOK();
}

LRESULT COptsAudioDlg::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
    UpdateDialogControls(this, TRUE);
    return FALSE;
}
