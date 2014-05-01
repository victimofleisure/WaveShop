// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05dec12	initial version
        01      28jan13	add sort header
		02		08feb13	in OnGoto, search from start of selection if any
        03      20mar13	in OnInitDialog, use InitDialogCaptionView

		peak statistics dialog
 
*/

// PeakStatsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "PeakStatsDlg.h"
#include "NumFormat.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPeakStatsDlg dialog

const CReportCtrl::RES_COL CPeakStatsDlg::m_ColInfo[COLS] = {
	{IDS_PKST_COL_CHANNEL,		LVCFMT_LEFT,	80,		CReportCtrl::DIR_ASC},
	{IDS_PKST_COL_MIN_SAMPLE,	LVCFMT_RIGHT,	100,	CReportCtrl::DIR_ASC},
	{IDS_PKST_COL_MIN_PCT,		LVCFMT_RIGHT,	50,		CReportCtrl::DIR_ASC},
	{IDS_PKST_COL_MAX_SAMPLE,	LVCFMT_RIGHT,	100,	CReportCtrl::DIR_ASC},
	{IDS_PKST_COL_MAX_PCT,		LVCFMT_RIGHT,	50,		CReportCtrl::DIR_ASC},
	{IDS_PKST_COL_PEAK_DB,		LVCFMT_RIGHT,	60,		CReportCtrl::DIR_ASC},
	{IDS_PKST_COL_DC_BIAS,		LVCFMT_RIGHT,	100,	CReportCtrl::DIR_ASC},
	{IDS_PKST_COL_BIAS_PCT,		LVCFMT_RIGHT,	50,		CReportCtrl::DIR_ASC},
};

const CCtrlResize::CTRL_LIST CPeakStatsDlg::m_CtrlList[] = {
	{IDC_PEAK_STATS_LIST, BIND_ALL},
	{0}
};

CPeakStatsDlg::CPeakStatsDlg(CWaveProcess::CPeakStats& Stats, CWaveShopView *View, CWnd* pParent /*=NULL*/)
	: CPersistDlg(IDD, 0, _T("PeakStatsDlg"), pParent), m_Stats(Stats)
{
	//{{AFX_DATA_INIT(CPeakStatsDlg)
	//}}AFX_DATA_INIT
	m_View = View;
}

#define SORT_CMP(x) retc = m_List.SortCmp(m_Stats.x(p1), m_Stats.x(p2));

int CPeakStatsDlg::SortCompare(int p1, int p2) const
{
	int	retc;
	switch (m_List.SortCol()) {
	case COL_CHANNEL:
		retc = m_List.SortCmp(p1, p2);
		break;
	case COL_MIN_SAMPLE:
	case COL_MIN_PCT:
		SORT_CMP(GetNormMin);
		break;
	case COL_MAX_SAMPLE:
	case COL_MAX_PCT:
		SORT_CMP(GetNormMax);
		break;
	case COL_PEAK_DB:
		SORT_CMP(GetPeakDecibels);
		break;
	case COL_DC_BIAS:
	case COL_BIAS_PCT:
		SORT_CMP(GetBias);
		break;
	default:
		NODEFAULTCASE;	// logic error
		retc = 0;
	}
	return(retc);
}

int CALLBACK CPeakStatsDlg::SortCompare(LPARAM p1, LPARAM p2, LPARAM This)
{
	return(((CPeakStatsDlg *)This)->SortCompare(INT64TO32(p1), INT64TO32(p2)));
}

void CPeakStatsDlg::DoDataExchange(CDataExchange* pDX)
{
	CPersistDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPeakStatsDlg)
	DDX_Control(pDX, IDC_PEAK_STATS_LIST, m_List);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CPeakStatsDlg message map

BEGIN_MESSAGE_MAP(CPeakStatsDlg, CPersistDlg)
	//{{AFX_MSG_MAP(CPeakStatsDlg)
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PEAK_STATS_GOTO, OnGoto)
	ON_UPDATE_COMMAND_UI(ID_PEAK_STATS_GOTO, OnUpdateGoto)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPeakStatsDlg message handlers

BOOL CPeakStatsDlg::OnInitDialog() 
{
	CPersistDlg::OnInitDialog();
	// set app icon and add document title to caption
	CWaveShopApp::InitDialogCaptionView(*this, m_View);
	m_List.SetColumns(COLS, m_ColInfo);
	m_List.InitControl(0, CReportCtrl::SORT_ARROWS);
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_List.SetSortCallback(SortCompare, this);
	int	chans = m_Stats.m_Chan.GetSize();
	CNumFormat	fmt;
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		const CWaveProcess::CPeakStats::CHAN_STATS& chst = m_Stats.m_Chan[iChan];
		CString	s;
		m_List.InsertItem(iChan, m_Stats.m_ChanName[iChan], 0);
		m_List.SetItemData(iChan, iChan);	// set sort key
		m_List.SetItemText(iChan, COL_MIN_SAMPLE, fmt.Format(chst.Min));
		s.Format(_T("%.2f"), m_Stats.GetNormMin(iChan) * 100);
		m_List.SetItemText(iChan, COL_MIN_PCT, s);
		m_List.SetItemText(iChan, COL_MAX_SAMPLE, fmt.Format(chst.Max));
		s.Format(_T("%.2f"), m_Stats.GetNormMax(iChan) * 100);
		m_List.SetItemText(iChan, COL_MAX_PCT, s);
		s.Format(_T("%.2f"), m_Stats.GetPeakDecibels(iChan)); 
		m_List.SetItemText(iChan, COL_PEAK_DB, s);
		m_List.SetItemText(iChan, COL_DC_BIAS, fmt.Format(m_Stats.GetBias(iChan), 2));
		s.Format(_T("%.2f"), m_Stats.GetNormBias(iChan) * 100);
		m_List.SetItemText(iChan, COL_BIAS_PCT, s);
	}
	m_Resize.AddControlList(this, m_CtrlList);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPeakStatsDlg::OnSize(UINT nType, int cx, int cy) 
{
	CPersistDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();	
}

void CPeakStatsDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (point.x == -1 && point.y == -1)	// if menu triggered via keyboard
		return;	// not supported
	CMenu	menu;
	menu.LoadMenu(IDR_PEAK_STATS_CTX);
	CPoint	pt(point);
	m_List.ScreenToClient(&pt);	// convert point to list client coords
	m_ContextPt = pt;	// store context point before updating menu
	theApp.UpdateMenu(this, &menu);
	CMenu	*mp = menu.GetSubMenu(0);
	mp->TrackPopupMenu(0, point.x, point.y, this);
}

void CPeakStatsDlg::OnGoto() 
{
	int	iChan;
	int	iTarget = GetGotoTarget(m_ContextPt, iChan);
	CWaveShopView	*View = theApp.GetMain()->GetView();
	if (View != NULL) {
		CWave::SAMPLE	target;
		if (iTarget == GOTO_MIN)
			target = m_Stats.m_Chan[iChan].Min;
		else
			target = m_Stats.m_Chan[iChan].Max;
		CWaveProcess::FIND_SAMPLE_INFO	info;
		ZeroMemory(&info, sizeof(info));
		info.ChannelIdx = iChan;
		info.TargetStart = target;
		info.TargetEnd = target;
		if (View->HaveSelection())	// if selection exists
			info.StartFrame = View->GetIntSelection().Start;	// search from its start
		if (View->GetWave().FindSample(info)) {	// find sample
			if (info.MatchFrame >= 0)	// if matching sample was found
				theApp.GetMain()->SetNow(double(info.MatchFrame), TRUE);	// center
		}
	}
}

int CPeakStatsDlg::GetGotoTarget(CPoint Point, int& ChannelIdx)
{
	LVHITTESTINFO	hti;
	hti.pt = Point;
	if (m_List.SubItemHitTest(&hti) >= 0) {
		ChannelIdx = hti.iItem;
		switch (hti.iSubItem) {
		case COL_MIN_SAMPLE:
		case COL_MIN_PCT:
			return(GOTO_MIN);
		case COL_MAX_SAMPLE:
		case COL_MAX_PCT:
			return(GOTO_MAX);
		}
	}
	return(GOTO_NONE);
}

void CPeakStatsDlg::OnUpdateGoto(CCmdUI* pCmdUI) 
{
	int	iChan;
	pCmdUI->Enable(GetGotoTarget(m_ContextPt, iChan));
}
