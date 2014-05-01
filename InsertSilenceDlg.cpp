// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02jan13	initial version
        01      16apr13	rename

        insert silence dialog
 
*/

// InsertSilenceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "InsertSilenceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInsertSilenceDlg dialog

#define RK_INSERT_DURATION _T("InsertDuration")

CInsertSilenceDlg::CInsertSilenceDlg(UINT SampleRate, CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	ASSERT(SampleRate);	// sample rate must be non-zero
	//{{AFX_DATA_INIT(CInsertSilenceDlg)
	m_LengthEdit = _T("");
	m_Unit = _T("");
	//}}AFX_DATA_INIT
	m_SampleRate = SampleRate;
	m_Duration = theApp.RdRegDouble(RK_INSERT_DURATION, 1);
}

CInsertSilenceDlg::~CInsertSilenceDlg()
{
	theApp.WrRegDouble(RK_INSERT_DURATION, m_Duration);
}

W64INT CInsertSilenceDlg::GetFrameCount() const
{
	return(roundW64INT(m_Duration * m_SampleRate));
}

void CInsertSilenceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsertSilenceDlg)
	DDX_Text(pDX, IDC_INS_LENGTH, m_LengthEdit);
	DDX_Text(pDX, IDC_INS_UNIT, m_Unit);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CInsertSilenceDlg message map

BEGIN_MESSAGE_MAP(CInsertSilenceDlg, CDialog)
	//{{AFX_MSG_MAP(CInsertSilenceDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsertSilenceDlg message handlers

BOOL CInsertSilenceDlg::OnInitDialog() 
{
	CMainFrame	*main = theApp.GetMain();
	double	frame = m_Duration * m_SampleRate;
	m_LengthEdit = main->GetNavBar().FrameToStr(frame);
	int	UnitStrID;
	if (main->GetOptions().m_TimeInFrames)	// if showing time in frames
		UnitStrID = IDS_UNIT_SAMPLE_FRAMES;
	else	// showing time in seconds
		UnitStrID = IDS_UNIT_SECONDS;
	m_Unit.LoadString(UnitStrID);
	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInsertSilenceDlg::OnOK() 
{
	UpdateData();
	CMainFrame	*main = theApp.GetMain();
	double	frame;
	if (!main->GetNavBar().StrToFrame(m_LengthEdit, frame) || frame <= 0) {
		AfxMessageBox(IDS_INS_BAD_LENGTH);
		GotoDlgCtrl(GetDlgItem(IDC_INS_LENGTH));
		return;
	}
	m_Duration = frame / m_SampleRate;
	CDialog::OnOK();
}
