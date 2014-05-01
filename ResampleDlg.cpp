// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25feb13	initial version

		resample dialog

*/

// ResampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "ResampleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResampleDlg dialog

#define RK_RESAMPLE_QUALITY	_T("ResampleQuality")

CResampleDlg::CResampleDlg(UINT SampleRate, CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CResampleDlg)
	m_Quality = 0;
	m_SampleRate = 0;
	//}}AFX_DATA_INIT
	theApp.RdReg2Int(RK_RESAMPLE_QUALITY, m_Quality);
	m_SampleRate = SampleRate;
	m_PrevSampleRate = SampleRate;
}

CResampleDlg::~CResampleDlg()
{
	theApp.WrRegInt(RK_RESAMPLE_QUALITY, m_Quality);
}

void CResampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResampleDlg)
	DDX_CBIndex(pDX, IDC_RESAMP_QUALITY, m_Quality);
	DDX_Text(pDX, IDC_RESAMP_SAMPLE_RATE, m_SampleRate);
	DDV_MinMaxUInt(pDX, m_SampleRate, 1, 4294967295);
	//}}AFX_DATA_MAP
	if (pDX->m_bSaveAndValidate) {
		if (m_SampleRate == m_PrevSampleRate) {
			AfxMessageBox(IDS_RESAMP_CHANGE_RATE);
			DDV_Fail(pDX, IDC_RESAMP_SAMPLE_RATE);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CResampleDlg message map

BEGIN_MESSAGE_MAP(CResampleDlg, CDialog)
	//{{AFX_MSG_MAP(CResampleDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResampleDlg message handlers
