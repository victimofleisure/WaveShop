// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10apr13	initial version
        01      03aug13	add metadata

		MP3 encoder configuration dialog
 
*/

// MP3EncoderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "MP3EncoderDlg.h"
#include <afxpriv.h>	// for WM_KICKIDLE
#include "PathStr.h"
#include "MissingLibraryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMP3EncoderDlg dialog

const int CMP3EncoderDlg::m_AlgorithmQualityPreset[ALG_QUALITY_PRESETS] = {
	7,	// AQP_FAST
	5,	// AQP_STANDARD
	2,	// AQP_HIGH_QUALITY
};

const int CMP3EncoderDlg::m_BitRatePreset[] = {
	8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320
};

#define RK_MP3_ENC				REG_SETTINGS _T("\\MP3Encoder")
#define RK_BIT_RATE_MODE		_T("BitRateMode")
#define RK_ALGORITHM_QUALITY	_T("AlgorithmQuality")
#define RK_TARGET_BIT_RATE		_T("TargetBitRate")
#define RK_TARGET_QUALITY		_T("TargetQuality")
#define RK_LAME_PATH			_T("LamePath")

CMP3EncoderDlg::CMP3EncoderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CMP3EncoderDlg)
	//}}AFX_DATA_INIT
	m_BitRateMode = CLameWrap::BRM_VARIABLE;
	m_AlgorithmQuality = m_AlgorithmQualityPreset[AQP_HIGH_QUALITY];
	m_TargetBitRate = 320;	// best bit rate
	m_TargetQuality = 0;	// best quality
	theApp.RdRegEx2Int(RK_MP3_ENC, RK_BIT_RATE_MODE, m_BitRateMode);
	theApp.RdRegEx2Int(RK_MP3_ENC, RK_ALGORITHM_QUALITY, m_AlgorithmQuality);
	theApp.RdRegEx2Int(RK_MP3_ENC, RK_TARGET_BIT_RATE, m_TargetBitRate);
	theApp.RdRegEx2Int(RK_MP3_ENC, RK_TARGET_QUALITY, m_TargetQuality);
	m_AlgorithmQualityPresetIdx = FindAlgorithmQuality(m_AlgorithmQuality);
}

CMP3EncoderDlg::~CMP3EncoderDlg()
{
	theApp.WrRegExInt(RK_MP3_ENC, RK_BIT_RATE_MODE, m_BitRateMode);
	theApp.WrRegExInt(RK_MP3_ENC, RK_ALGORITHM_QUALITY, m_AlgorithmQuality);
	theApp.WrRegExInt(RK_MP3_ENC, RK_TARGET_BIT_RATE, m_TargetBitRate);
	theApp.WrRegExInt(RK_MP3_ENC, RK_TARGET_QUALITY, m_TargetQuality);
}

void CMP3EncoderDlg::GetParams(CLameWrap::ENCODING_PARAMS& Params) const
{
	Params.AlgorithmQuality = m_AlgorithmQuality;
	Params.BitRateMode = m_BitRateMode;
	Params.TargetBitRate = m_TargetBitRate;
	Params.TargetQuality = m_TargetQuality;
}

int CMP3EncoderDlg::FindAlgorithmQuality(int Quality) const
{
	for (int iAQP = 0; iAQP < ALG_QUALITY_PRESETS; iAQP++) {	// for each preset
		if (m_AlgorithmQualityPreset[iAQP] == Quality)	// if preset matches quality
			return(iAQP);	// return preset index
	}
	return(-1);	// match not found
}

bool CMP3EncoderDlg::LimitQuality()
{
	int	nID = GetCheckedRadioButton(IDC_MP3_BIT_RATE_MODE, IDC_MP3_BIT_RATE_MODE3);
	return(nID == IDC_MP3_BIT_RATE_MODE3);
};

bool CMP3EncoderDlg::Encode(LPCTSTR Path, const CWave& Wave, const CStringArray *Metadata)
{
	LPCTSTR	LibName = _T("libmp3lame.dll");
	CPathStr	LibPath(theApp.GetAppFolder());
	LibPath.Append(LibName);
	bool	UsingRegPath;
	if (PathFileExists(LibPath))	// if library found in app folder
		UsingRegPath = FALSE;	// registry path not used
	else {	// library not found in app folder; try registry path next
		UsingRegPath = TRUE;	// using registry path
		LibPath = theApp.RdRegExString(RK_MP3_ENC, RK_LAME_PATH);	// read registry
		if (!PathFileExists(LibPath)) {	// if library not found in registry path either
			// do missing library dialog
			LPCTSTR	Description = _T("LAME");
			LONGLONG	MinVersion = CMissingLibraryDlg::MakeVersion(3, 99, 2, 5);
			LPCTSTR	DownloadURL = _T("http://www.rarewares.org/mp3-lame-libraries.php");
			CMissingLibraryDlg	MissLibDlg(Description, LibName, MinVersion, DownloadURL);
			if (MissLibDlg.DoModal() != IDOK)	// if library still not found
 				return(FALSE);	// give up
			LibPath = MissLibDlg.GetLibraryPath();	// store path to located library
		}
	}
	CLameWrap	lame;
	if (!lame.Create(LibPath))	// create MPEG audio encoder
		return(FALSE);	// can't create encoder; don't write registry path
	if (UsingRegPath)	// if we used registry path
		theApp.WrRegExString(RK_MP3_ENC, RK_LAME_PATH, LibPath);	// write registry
	CMP3EncoderDlg	EncDlg;
	if (EncDlg.DoModal() != IDOK)	// show encoder dialog
		return(FALSE);	// user canceled
	CLameWrap::ENCODING_PARAMS	params;
	EncDlg.GetParams(params);	// get encoding parameters
	if (!lame.Write(Path, Wave, params, Metadata))	// write MPEG audio
		return(FALSE);	// write failure; error already reported
	return(TRUE);	// success
}

void CMP3EncoderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMP3EncoderDlg)
	DDX_Control(pDX, IDC_MP3_TARGET_QUALITY, m_TargetQualitySlider);
	DDX_Control(pDX, IDC_MP3_TARGET_BIT_RATE, m_TargetBitRateCombo);
	DDX_Radio(pDX, IDC_MP3_BIT_RATE_MODE, m_BitRateMode);
	DDX_CBIndex(pDX, IDC_MP3_ALGORITHM_QUALITY, m_AlgorithmQualityPresetIdx);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMP3EncoderDlg, CDialog)
	//{{AFX_MSG_MAP(CMP3EncoderDlg)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_UPDATE_COMMAND_UI_RANGE(IDC_MP3_TARGET_BIT_RATE, IDC_MP3_TARGET_BIT_RATE_UNIT, OnUpdateTargetBitRate)
	ON_UPDATE_COMMAND_UI_RANGE(IDC_MP3_TARGET_QUALITY, IDC_MP3_TARGET_QUALITY_FASTEST, OnUpdateTargetQuality)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMP3EncoderDlg message handlers

BOOL CMP3EncoderDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int	presets = _countof(m_BitRatePreset);
	int	BitRateSel = presets - 1;	// default selection
	for (int iPreset = 0; iPreset < presets; iPreset++) {	// for each bit rate preset
		CString	s;
		s.Format(_T("%d"), m_BitRatePreset[iPreset]);
		m_TargetBitRateCombo.AddString(s);	// add preset string to combo
		if (m_BitRatePreset[iPreset] == m_TargetBitRate)	// if preset matches target
			BitRateSel = iPreset;	// save selection
	}
	m_TargetBitRateCombo.SetCurSel(BitRateSel);
	m_TargetQualitySlider.SetRange(0, MAX_VBR_QUALITY);
	m_TargetQualitySlider.SetPos(MAX_VBR_QUALITY - m_TargetQuality);	// 0 == best
	int	GotoCtrlID;
	if (m_BitRateMode == CLameWrap::BRM_VARIABLE)	// if variable bit rate
		GotoCtrlID = IDC_MP3_TARGET_QUALITY;	// go to quality slider
	else	// not variable bit rate
		GotoCtrlID = IDC_MP3_TARGET_BIT_RATE;	// go to bit rate combo
	GotoDlgCtrl(GetDlgItem(GotoCtrlID));

	return FALSE;  // return TRUE unless you set the focus to a control
	               // EXCEPTION: OCX Property Pages should return FALSE
}

void CMP3EncoderDlg::OnOK() 
{
	CDialog::OnOK();
	int	iAQP = m_AlgorithmQualityPresetIdx;
	ASSERT(iAQP >= 0 && iAQP < ALG_QUALITY_PRESETS);
	if (iAQP >= 0)	// if valid selection
		m_AlgorithmQuality = m_AlgorithmQualityPreset[iAQP];
	int	BitRateSel = m_TargetBitRateCombo.GetCurSel();
	ASSERT(BitRateSel >= 0 && BitRateSel < _countof(m_BitRatePreset));
	if (BitRateSel >= 0)	// if valid selection
		m_TargetBitRate = m_BitRatePreset[BitRateSel]; 
	int	QualPos = m_TargetQualitySlider.GetPos();
	ASSERT(QualPos >= 0 && QualPos <= MAX_VBR_QUALITY);
	QualPos = CLAMP(QualPos, 0, MAX_VBR_QUALITY);
	m_TargetQuality = MAX_VBR_QUALITY - QualPos;	// 0 == best
}

LRESULT CMP3EncoderDlg::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
    UpdateDialogControls(this, TRUE);
    return FALSE;
}

void CMP3EncoderDlg::OnUpdateTargetBitRate(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!LimitQuality());	// enable control if limiting bit rate
}

void CMP3EncoderDlg::OnUpdateTargetQuality(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(LimitQuality());	// enable control if limiting quality
}
