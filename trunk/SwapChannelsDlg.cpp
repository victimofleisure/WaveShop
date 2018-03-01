// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      12jan13	initial version

		swap channels dialog

*/

// SwapChannelsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "SwapChannelsDlg.h"
#include "WaveProcess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSwapChannelsDlg dialog

CSwapChannelsDlg::CSwapChannelsDlg(const CWaveProcess& Wave, CWnd* pParent /*=NULL*/)
	: m_Wave(Wave), CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CSwapChannelsDlg)
	//}}AFX_DATA_INIT
	ASSERT(Wave.GetChannels() >= 2);	// need at least two channels to swap
	m_Pair.SetEmpty();
}

CUIntRange CSwapChannelsDlg::RetrievePair() const
{
	return(CUIntRange(m_ChannelA.GetCurSel(), m_ChannelB.GetCurSel()));
}

void CSwapChannelsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSwapChannelsDlg)
	DDX_Control(pDX, IDC_SWAP_CHANNEL_B, m_ChannelB);
	DDX_Control(pDX, IDC_SWAP_CHANNEL_A, m_ChannelA);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CSwapChannelsDlg message map

BEGIN_MESSAGE_MAP(CSwapChannelsDlg, CDialog)
	//{{AFX_MSG_MAP(CSwapChannelsDlg)
	ON_CBN_SELCHANGE(IDC_SWAP_CHANNEL_A, OnSelchangeChannel)
	ON_CBN_SELCHANGE(IDC_SWAP_CHANNEL_B, OnSelchangeChannel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSwapChannelsDlg message handlers

BOOL CSwapChannelsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CStringArray	ChannelName;
	m_Wave.GetChannelNames(ChannelName);
	W64INT	chans = ChannelName.GetSize();
	for (int iChan = 0; iChan < chans; iChan++) {
		m_ChannelA.AddString(ChannelName[iChan]);
		m_ChannelB.AddString(ChannelName[iChan]);
	}
	m_ChannelA.SetCurSel(0);
	m_ChannelB.SetCurSel(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSwapChannelsDlg::OnOK() 
{
	m_Pair = RetrievePair();
	CDialog::OnOK();
}

void CSwapChannelsDlg::OnSelchangeChannel() 
{
	GetDlgItem(IDOK)->EnableWindow(!RetrievePair().IsEmpty());
}
