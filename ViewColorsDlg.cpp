// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      12feb13	initial version
		01		02apr13	move EditColor implementation to button
        02      16jun13	add excluded channel colors

		colors dialog

*/

// ViewColorsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "ViewColorsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewColorsDlg dialog

#define VIEW_COLOR_DEF(name, tag, R, G, B) IDC_VWCLR_##tag,
const int CViewColorsDlg::m_BtnResID[COLORS] = {	// button resource IDs
	#include "ViewColors.h"
};

CViewColorsDlg::CViewColorsDlg(COptionsInfo& Info, CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent), m_oi(Info)
{
	//{{AFX_DATA_INIT(CViewColorsDlg)
	//}}AFX_DATA_INIT
}

int CViewColorsDlg::FindColor(int ResID)
{
	for (int iColor = 0; iColor < COLORS; iColor++) {	// for each color
		// if button resource ID for this color matches caller's resource ID
		if (m_BtnResID[iColor] == ResID)
			return(iColor);
	}
	return(-1);	// resource ID not found
}

void CViewColorsDlg::EditColor(int ColorIdx)
{
	ASSERT(ColorIdx >= 0 && ColorIdx < COLORS);
	m_SwatchBtn[ColorIdx].EditColor(m_oi.m_CustomColors.Color);
}

void CViewColorsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewColorsDlg)
	//}}AFX_DATA_MAP
	// update swatch buttons
	for (int iColor = 0; iColor < COLORS; iColor++)	// for each color
		DDX_Control(pDX, m_BtnResID[iColor], m_SwatchBtn[iColor]);
}

/////////////////////////////////////////////////////////////////////////////
// CViewColorsDlg message map

BEGIN_MESSAGE_MAP(CViewColorsDlg, CDialog)
	//{{AFX_MSG_MAP(CViewColorsDlg)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(BN_CLICKED, IDC_VWCLR_EXCLUDED_CHAN_BK, IDC_VWCLR_WAVE_DATA, OnColorBtn)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewColorsDlg message handlers

BOOL CViewColorsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// initialize swatch buttons from palette
	for (int iColor = 0; iColor < COLORS; iColor++)
		m_SwatchBtn[iColor].SetColor(m_oi.m_ViewPalette.Color[iColor]);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CViewColorsDlg::OnOK() 
{
	// retrieve palette from swatch buttons
	for (int iColor = 0; iColor < COLORS; iColor++)	// for each color
		m_oi.m_ViewPalette.Color[iColor] = m_SwatchBtn[iColor].GetColor();
	CDialog::OnOK();
}

void CViewColorsDlg::OnColorBtn(UINT nID)
{
	int	iColor = FindColor(nID);
	ASSERT(iColor >= 0);	// else logic error
	EditColor(iColor);
}
