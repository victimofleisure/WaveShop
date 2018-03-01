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

        pitch dialog bar
 
*/

// PitchBar.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "PitchBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPitchBar dialog

IMPLEMENT_DYNAMIC(CPitchBar, CDialogBarEx);

const CCtrlResize::CTRL_LIST CPitchBar::m_CtrlList[] = {
	{IDC_PITCH_SLIDER,	BIND_LEFT | BIND_RIGHT},
	{IDC_PITCH_EDIT,	BIND_RIGHT},
	{0}
};

const CEditSliderCtrl::INFO CPitchBar::m_SliderInfo = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	-200,	200,	0,		100,	0,		0,		1,		2
};

CPitchBar::CPitchBar(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(CPitchBar)
	//}}AFX_DATA_INIT
}

CPitchBar::~CPitchBar()
{
}

void CPitchBar::SetPitch(double Pitch)
{
	m_PitchEdit.SetVal(Pitch);
}

void CPitchBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBarEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPitchBar)
	DDX_Control(pDX, IDC_PITCH_SLIDER, m_PitchSlider);
	DDX_Control(pDX, IDC_PITCH_EDIT, m_PitchEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPitchBar, CDialogBarEx)
	//{{AFX_MSG_MAP(CPitchBar)
	ON_WM_SIZE()
	ON_NOTIFY(NEN_CHANGED, IDC_PITCH_EDIT, OnChangedPitch)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPitchBar message handlers

LRESULT CPitchBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	CDialogBarEx::OnInitDialog(wParam, lParam);
	m_PitchSlider.SetInfo(m_SliderInfo, &m_PitchEdit);
	m_Resize.AddControlList(this, m_CtrlList);
	return 0;
}

void CPitchBar::OnSize(UINT nType, int cx, int cy) 
{
	CDialogBarEx::OnSize(nType, cx, cy);
	int	xshift = IsFloating() ? 0 : GRIPPER_SIZE;
	m_Resize.SetOriginShift(CSize(xshift, 0));
	m_Resize.OnSize();
}

void CPitchBar::OnChangedPitch(NMHDR* pNMHDR, LRESULT* pResult)
{
	theApp.GetMain()->SetFrequency(m_PitchEdit.GetVal());
	*pResult = 0;
}
