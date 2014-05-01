// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16nov12	initial version
		01		12jul13	remove main frame member pointer

        volume dialog bar
 
*/

// VolumeBar.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "VolumeBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVolumeBar dialog

IMPLEMENT_DYNAMIC(CVolumeBar, CDialogBarEx);

#define RK_VOLUME	_T("Volume")
#define RK_MUTE		_T("Mute")

const CCtrlResize::CTRL_LIST CVolumeBar::m_CtrlList[] = {
	{IDC_VOLUME_MUTE,	BIND_LEFT},
	{IDC_VOLUME_SLIDER,	BIND_LEFT | BIND_RIGHT},
	{0}
};

CVolumeBar::CVolumeBar(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(CVolumeBar)
	//}}AFX_DATA_INIT
	m_Volume = theApp.RdRegDouble(RK_VOLUME, 1.0);
	m_Mute = theApp.RdRegBool(RK_MUTE, FALSE);
}

CVolumeBar::~CVolumeBar()
{
	theApp.WrRegDouble(RK_VOLUME, m_Volume);
	theApp.WrRegBool(RK_MUTE, m_Mute);
}

void CVolumeBar::SetVolume(double Volume)
{
	m_VolumeSlider.SetPos(round(Volume * double(VOLUME_STEPS)));
	m_Volume = Volume;
}

double CVolumeBar::GetVolume() const
{
	if (m_Mute)
		return(0);
	return(m_Volume);
}

void CVolumeBar::UpdatePlayer()
{
	theApp.GetMain()->SetVolume(GetVolume());
}

void CVolumeBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBarEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVolumeBar)
	DDX_Control(pDX, IDC_VOLUME_MUTE, m_MuteBtn);
	DDX_Control(pDX, IDC_VOLUME_SLIDER, m_VolumeSlider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVolumeBar, CDialogBarEx)
	//{{AFX_MSG_MAP(CVolumeBar)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_VOLUME_MUTE, OnVolumeMute)
	ON_BN_DOUBLECLICKED(IDC_VOLUME_MUTE, OnVolumeMute)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
	ON_UPDATE_COMMAND_UI(IDC_VOLUME_MUTE, OnUpdateMute)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVolumeBar message handlers

LRESULT CVolumeBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	CDialogBarEx::OnInitDialog(wParam, lParam);
	m_VolumeSlider.SetRange(0, VOLUME_STEPS);
	SetVolume(m_Volume);
	m_MuteBtn.SetIcons(IDI_MUTE_UP, IDI_MUTE_DOWN);
	m_MuteBtn.SetCheck(m_Mute);
	m_Resize.AddControlList(this, m_CtrlList);
	return 0;
}

void CVolumeBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	m_Volume = 	m_VolumeSlider.GetPos() / double(VOLUME_STEPS);
	UpdatePlayer();
	CDialogBarEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CVolumeBar::OnVolumeMute() 
{
	m_Mute ^= 1;
	UpdatePlayer();
}

void CVolumeBar::OnUpdateMute(CCmdUI *pCmdUI)
{
	// this handler is mandatory, else button is automatically disabled
	pCmdUI->SetCheck(m_Mute);
}

void CVolumeBar::OnSize(UINT nType, int cx, int cy) 
{
	CDialogBarEx::OnSize(nType, cx, cy);
	int	xshift = IsFloating() ? 0 : GRIPPER_SIZE;
	m_Resize.SetOriginShift(CSize(xshift, 0));
	m_Resize.OnSize();
}
