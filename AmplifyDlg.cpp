// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02jan13	initial version
        01      25feb13	restore previous gain on cancel

		amplify dialog
 
*/

// AmplifyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "AmplifyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAmplifyDlg dialog

#define RK_AMPLIFY_GAIN _T("AmplifyGain")

CAmplifyDlg::CAmplifyDlg(double Peak, CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CAmplifyDlg)
	//}}AFX_DATA_INIT
	m_Gain = theApp.RdRegDouble(RK_AMPLIFY_GAIN, 3);
	m_PrevGain = m_Gain;
	m_Peak = Peak;
}

CAmplifyDlg::~CAmplifyDlg()
{
	theApp.WrRegDouble(RK_AMPLIFY_GAIN, m_Gain);
}

void CAmplifyDlg::UpdateUI(double Gain)
{
	CString	s;
	s.Format(_T("%.2f"), m_Peak + Gain);
	m_PeakEdit.SetWindowText(s);
	int	ShowClip = m_Peak + Gain > 0 ? SW_SHOW : SW_HIDE;
	m_Clip.ShowWindow(ShowClip);
}

void CAmplifyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAmplifyDlg)
	DDX_Control(pDX, IDC_AMPL_CLIP, m_Clip);
	DDX_Control(pDX, IDC_AMPL_PEAK, m_PeakEdit);
	DDX_Text(pDX, IDC_AMPL_GAIN, m_Gain);
	//}}AFX_DATA_MAP
	if (pDX->m_bSaveAndValidate) {	// if saving
		if (!m_Gain) {	// if no gain
			AfxMessageBox(IDS_AMPL_NO_GAIN);
			DDV_Fail(pDX, IDC_AMPL_GAIN);
		}
		if (m_Peak + m_Gain > 0) {	// if audio will be clipped
			if (AfxMessageBox(IDS_AMPL_CLIP_WARN, MB_OKCANCEL) != IDOK)	// warn user
				DDV_Fail(pDX, IDC_AMPL_GAIN);	// user chickened out
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAmplifyDlg message map

BEGIN_MESSAGE_MAP(CAmplifyDlg, CDialog)
	//{{AFX_MSG_MAP(CAmplifyDlg)
	ON_EN_KILLFOCUS(IDC_AMPL_GAIN, OnKillfocusGain)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAmplifyDlg message handlers

BOOL CAmplifyDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateUI(m_Gain);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAmplifyDlg::OnCancel() 
{
	m_Gain = m_PrevGain;	// restore previous gain
	CDialog::OnCancel();
}

void CAmplifyDlg::OnKillfocusGain() 
{
	CString	s;
	GetDlgItem(IDC_AMPL_GAIN)->GetWindowText(s);
	UpdateUI(_tstof(s));
}

HBRUSH CAmplifyDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd == &m_Clip) {
		const COLORREF	ClipColor = RGB(255, 192, 0);	// medium yellow
		hbr = (HBRUSH)GetStockObject(DC_BRUSH);
		SetDCBrushColor(pDC->m_hDC, ClipColor);
		pDC->SetBkColor(ClipColor);
	}
	
	return hbr;
}
