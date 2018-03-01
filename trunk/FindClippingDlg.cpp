// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14jan13	initial version
		01		04mar13	add clipping level

		find clipping dialog

*/

// FindClippingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "FindClippingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindClippingDlg dialog

#define RK_FCLIP_START_THRESHOLD _T("FClipStartThreshold")
#define RK_FCLIP_STOP_THRESHOLD _T("FClipStopThreshold")
#define RK_FCLIP_CLIPPING_LEVEL _T("FClipClippingLevel")

CFindClippingDlg::CFindClippingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CFindClippingDlg)
	m_StopThreshold = 3;
	m_StartThreshold = 3;
	m_ClippingLevel = 0.0;
	//}}AFX_DATA_INIT
	theApp.RdReg2UInt(RK_FCLIP_START_THRESHOLD, m_StartThreshold);
	theApp.RdReg2UInt(RK_FCLIP_STOP_THRESHOLD, m_StopThreshold);
	theApp.RdReg2Double(RK_FCLIP_CLIPPING_LEVEL, m_ClippingLevel);
}

CFindClippingDlg::~CFindClippingDlg()
{
	theApp.WrRegInt(RK_FCLIP_START_THRESHOLD, m_StartThreshold);
	theApp.WrRegInt(RK_FCLIP_STOP_THRESHOLD, m_StopThreshold);
	theApp.WrRegDouble(RK_FCLIP_CLIPPING_LEVEL, m_ClippingLevel);
}

void CFindClippingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindClippingDlg)
	DDX_Text(pDX, IDC_FCLIP_STOP_THRESHOLD, m_StopThreshold);
	DDV_MinMaxUInt(pDX, m_StopThreshold, 1, 4294967295);
	DDX_Text(pDX, IDC_FCLIP_START_THRESHOLD, m_StartThreshold);
	DDV_MinMaxUInt(pDX, m_StartThreshold, 1, 4294967295);
	DDX_Text(pDX, IDC_FCLIP_CLIPPING_LEVEL, m_ClippingLevel);
	DDV_MinMaxDouble(pDX, m_ClippingLevel, -100., 0.);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFindClippingDlg message map

BEGIN_MESSAGE_MAP(CFindClippingDlg, CDialog)
	//{{AFX_MSG_MAP(CFindClippingDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindClippingDlg message handlers
