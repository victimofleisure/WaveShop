// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      07may13	initial version

		sweep dialog

*/

// SweepDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "SweepDlg.h"
#include "WaveGenOscDlg.h"
#include "Oscillator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSweepDlg dialog

CSweepDlg::CSweepDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CSweepDlg)
	m_Duration = 0.0;
	m_EndFreq = 0.0;
	m_StartFreq = 0.0;
	//}}AFX_DATA_INIT
	m_Waveform = 0;
}

void CSweepDlg::DDV_GTZeroDouble(CDataExchange *pDX, int nIDC, double& value)
{
	if (pDX->m_bSaveAndValidate && value <= 0) {
		AfxMessageBox(IDS_DDV_VALUE_GT_ZERO);
		DDV_Fail(pDX, nIDC);
	}
}

void CSweepDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSweepDlg)
	DDX_Control(pDX, IDC_SWEEP_WAVEFORM, m_WaveformCombo);
	DDX_Text(pDX, IDC_SWEEP_DURATION, m_Duration);
	DDX_Text(pDX, IDC_SWEEP_END_FREQUENCY, m_EndFreq);
	DDX_Text(pDX, IDC_SWEEP_START_FREQUENCY, m_StartFreq);
	//}}AFX_DATA_MAP
	DDV_GTZeroDouble(pDX, IDC_SWEEP_DURATION, m_Duration);
	DDV_GTZeroDouble(pDX, IDC_SWEEP_START_FREQUENCY, m_StartFreq);
	DDV_GTZeroDouble(pDX, IDC_SWEEP_END_FREQUENCY, m_EndFreq);
	CWaveGenOscDlg::DDX_Combo(pDX, m_WaveformCombo, m_Waveform);
}

BEGIN_MESSAGE_MAP(CSweepDlg, CDialog)
	//{{AFX_MSG_MAP(CSweepDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSweepDlg message handlers

BOOL CSweepDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWaveGenOscDlg::InitWaveformCombo(m_WaveformCombo, m_Waveform);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
