// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01apr13	initial version
		01		28apr13	remove persistence

        real-time spectrum analyzer options dialog
 
*/

// OptsRTSADlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "OptsRTSADlg.h"
#include <afxpriv.h>	// for WM_KICKIDLE
#include "SpectrumDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsRTSADlg dialog

COptsRTSADlg::COptsRTSADlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsRTSADlg)
	//}}AFX_DATA_INIT
}

void COptsRTSADlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsRTSADlg)
	DDX_Control(pDX, IDC_RTSA_COLOR_PLOT_GRID, m_GridSwatch);
	DDX_Control(pDX, IDC_RTSA_COLOR_PLOT_BKGND, m_BkgndSwatch);
	DDX_Control(pDX, IDC_RTSA_AVERAGING_SPIN, m_AveragingSpin);
	DDX_Control(pDX, IDC_RTSA_WINDOW_SIZE, m_WindowSizeCombo);
	DDX_Control(pDX, IDC_RTSA_WINDOW_FUNC, m_WindowFuncCombo);
	//}}AFX_DATA_MAP
	DDX_CBIndex(pDX, IDC_RTSA_CHANNEL_MODE, m_oi.m_RTSA.ChannelMode);
	DDX_CBIndex(pDX, IDC_RTSA_FREQ_AXIS, m_oi.m_RTSA.FreqAxisType);
	DDX_CBIndex(pDX, IDC_RTSA_PLOT_STYLE, m_oi.m_RTSA.PlotStyle);
	DDX_Text(pDX, IDC_RTSA_AVERAGING_EDIT, m_oi.m_RTSA.Averaging);
	DDV_MinMaxInt(pDX, m_oi.m_RTSA.Averaging, CSpectrumBar::MIN_AVERAGING, CSpectrumBar::MAX_AVERAGING);
	DDX_CBIndex(pDX, IDC_RTSA_SHOW_PEAKS, m_oi.m_RTSA.ShowPeaks);
	DDX_Text(pDX, IDC_RTSA_PEAK_HOLD, m_oi.m_RTSA.PeakHoldTime);
	// peak hold counter is a byte, limiting hold time to around 13 seconds
	int	MaxPeakHoldTime = 255 * 1000 / CMainFrame::GetTimerFrequency();
	DDV_MinMaxInt(pDX, m_oi.m_RTSA.PeakHoldTime, 0, MaxPeakHoldTime);
	DDX_Text(pDX, IDC_RTSA_PEAK_DECAY, m_oi.m_RTSA.PeakDecay);
	DDV_MinMaxInt(pDX, m_oi.m_RTSA.PeakDecay, 0, 10000);
}

BEGIN_MESSAGE_MAP(COptsRTSADlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsRTSADlg)
	ON_BN_CLICKED(IDC_RTSA_COLOR_PLOT_BKGND, OnColorPlotBkgnd)
	ON_BN_CLICKED(IDC_RTSA_COLOR_PLOT_GRID, OnColorPlotGrid)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsRTSADlg message handlers

BOOL COptsRTSADlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	CSpectrumDlg::InitWindowFunctionCombo(m_WindowFuncCombo, m_oi.m_RTSA.WindowFunction);
	CSpectrumDlg::InitWindowSizeCombo(m_WindowSizeCombo, m_oi.m_RTSA.WindowSize);
	GetDlgItem(IDC_RTSA_AVERAGING_SPIN)->SendMessage(	// set spin control range
		UDM_SETRANGE32, CSpectrumBar::MIN_AVERAGING, CSpectrumBar::MAX_AVERAGING);
	m_BkgndSwatch.SetColor(m_oi.m_RTSA.PlotBkgndColor);
	m_GridSwatch.SetColor(m_oi.m_RTSA.PlotGridColor);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptsRTSADlg::OnOK() 
{
	int	sel = m_WindowFuncCombo.GetCurSel();
	ASSERT(sel >= 0);
	m_oi.m_RTSA.WindowFunction = INT64TO32(m_WindowFuncCombo.GetItemData(sel));
	sel = m_WindowSizeCombo.GetCurSel();
	ASSERT(sel >= 0);
	m_oi.m_RTSA.WindowSize = INT64TO32(m_WindowSizeCombo.GetItemData(sel));
	m_oi.m_RTSA.PlotBkgndColor = m_BkgndSwatch.GetColor();
	m_oi.m_RTSA.PlotGridColor = m_GridSwatch.GetColor();
	CPropertyPage::OnOK();
}

LRESULT COptsRTSADlg::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
    UpdateDialogControls(this, TRUE);
    return FALSE;
}

void COptsRTSADlg::OnColorPlotBkgnd() 
{
	m_BkgndSwatch.EditColor(m_oi.m_CustomColors.Color);
}

void COptsRTSADlg::OnColorPlotGrid() 
{
	m_GridSwatch.EditColor(m_oi.m_CustomColors.Color);
}
