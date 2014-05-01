// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21jan13	initial version

		change format dialog
 
*/

// ChangeFormatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ChangeFormatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChangeFormatDlg dialog

CChangeFormatDlg::CChangeFormatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangeFormatDlg)
	m_SampleRate = 0;
	m_SampleBits = 0;
	m_Channels = 0;
	//}}AFX_DATA_INIT
}

void CChangeFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangeFormatDlg)
	DDX_Text(pDX, IDC_CFMT_SAMPLE_RATE, m_SampleRate);
	DDV_MinMaxUInt(pDX, m_SampleRate, 1, 4294967295);
	DDX_Text(pDX, IDC_CFMT_SAMPLE_BITS, m_SampleBits);
	DDV_MinMaxUInt(pDX, m_SampleBits, 1, 32);
	DDX_Text(pDX, IDC_CFMT_CHANNELS, m_Channels);
	DDV_MinMaxUInt(pDX, m_Channels, 1, 65535);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CChangeFormatDlg message map

BEGIN_MESSAGE_MAP(CChangeFormatDlg, CDialog)
	//{{AFX_MSG_MAP(CChangeFormatDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangeFormatDlg message handlers
