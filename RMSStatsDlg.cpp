// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17dec12	initial version
        01      28jan13	add sort header
        02      20mar13	in OnInitDialog, use InitDialogCaptionView
        03      26mar13	adapt to enhanced plot control
		04		04apr13	in OnRefresh, CAsyncJob fix obviates set focus on cancel

		RMS statistics dialog

*/

// RMSStatsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "RMSStatsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRMSStatsDlg dialog

const CCtrlResize::CTRL_LIST CRMSStatsDlg::m_CtrlList[] = {
	{IDC_RMS_TAB_CTRL,			BIND_ALL},
	{IDC_RMS_TOP_CHANNEL,		BIND_RIGHT | BIND_TOP},
	{IDC_RMS_TOP_CHANNEL_CAP,	BIND_RIGHT | BIND_TOP},
	{IDC_RMS_ACCOUNT_FOR_DC,	BIND_LEFT | BIND_BOTTOM},
	{IDC_RMS_ZERO_WAVE,			BIND_LEFT | BIND_BOTTOM},
	{IDC_RMS_ZERO_WAVE2,		BIND_LEFT | BIND_BOTTOM},
	{IDC_RMS_WINDOW_SIZE,		BIND_LEFT | BIND_BOTTOM},
	{IDC_RMS_WINDOW_SIZE_CAP,	BIND_LEFT | BIND_BOTTOM},
	{IDC_RMS_WINDOW_SIZE_UNIT,	BIND_LEFT | BIND_BOTTOM},
	{IDC_RMS_REFRESH,			BIND_RIGHT | BIND_BOTTOM},
	{IDOK,						BIND_RIGHT | BIND_BOTTOM},
	{0}
};

const CReportCtrl::RES_COL CRMSStatsDlg::m_ColInfo[COLS] = {
	{IDS_RMS_COL_CHANNEL,		LVCFMT_LEFT,	80,		CReportCtrl::DIR_ASC},
	{IDS_RMS_COL_MIN,			LVCFMT_RIGHT,	80,		CReportCtrl::DIR_ASC},
	{IDS_RMS_COL_MAX,			LVCFMT_RIGHT,	80,		CReportCtrl::DIR_ASC},
	{IDS_RMS_COL_AVG,			LVCFMT_RIGHT,	80,		CReportCtrl::DIR_ASC},
	{IDS_RMS_COL_TOTAL,			LVCFMT_RIGHT,	80,		CReportCtrl::DIR_ASC},
};

#define RK_RMS_TAB_SEL			_T("RMSTabSel")
#define RK_RMS_ACCOUNT_FOR_DC	_T("RMSAccountForDC")
#define RK_RMS_WINDOW_SIZE		_T("RMSWindowSize")
#define RK_RMS_ZERO_WAVE		_T("RMSZeroWave")

#define DUMP_HISTOGRAM	FALSE

CRMSStatsDlg::CRMSStatsDlg(CWaveShopView *View, CWnd* pParent /*=NULL*/)
	: CPersistDlg(IDD, 0, _T("RMSStatsDlg"), pParent)
{
	//{{AFX_DATA_INIT(CRMSStatsDlg)
	m_AccountForDC = TRUE;
	m_WindowSize = 50;
	m_ZeroWave = 0;
	//}}AFX_DATA_INIT
	m_View = View;
	m_InitSize = CSize(0, 0);
	m_TabChild[TAB_STATS] = &m_List;
	m_TabChild[TAB_HISTOGRAM] = &m_Plot;
	m_TabSel = theApp.RdRegInt(RK_RMS_TAB_SEL, TAB_STATS);
	theApp.RdReg2Int(RK_RMS_ACCOUNT_FOR_DC, m_AccountForDC);
	theApp.RdReg2Int(RK_RMS_WINDOW_SIZE, m_WindowSize);
	theApp.RdReg2Int(RK_RMS_ZERO_WAVE, m_ZeroWave);
}

CRMSStatsDlg::~CRMSStatsDlg()
{
	theApp.WrRegInt(RK_RMS_TAB_SEL, m_TabSel);
	theApp.WrRegInt(RK_RMS_ACCOUNT_FOR_DC, m_AccountForDC);
	theApp.WrRegInt(RK_RMS_WINDOW_SIZE, m_WindowSize);
	theApp.WrRegInt(RK_RMS_ZERO_WAVE, m_ZeroWave);
}

bool CRMSStatsDlg::CalcStats()
{
	if (m_View == NULL)
		return(FALSE);
	CWaveProcess&	wave = m_View->GetWave();
	// only get peak statistics once, and only if they're needed
	if (!m_PeakStats.m_Chan.GetSize() && m_AccountForDC) {
		if (!wave.GetPeakStats(m_View->GetIntSelection(), m_PeakStats))
			return(FALSE);
	}
	m_Parms.WindowSize = m_WindowSize / 1000.0;	// convert milliseconds to seconds
	// sliding window overlap = (PanesPerWindow - 1) / PanesPerWindow
	// PanesPerWindow of one means windows don't overlap; higher values
	// oversample data, yielding smoother results for increased overhead
	m_Parms.PanesPerWindow = 50;	// number of panes per RMS window
	m_Parms.HistogramRange = 100;	// histogram range in decibels
	m_Parms.HistogramSubdiv = 10;	// histogram subdivisions per decibel
	m_Parms.Flags = 0;
	if (m_AccountForDC)
		m_Parms.Flags |= CWaveProcess::RMS_ACCOUNT_FOR_DC;
	if (m_ZeroWave)
		m_Parms.Flags |= CWaveProcess::RMS_0_DB_FS_SQUARE;
	if (!wave.GetRMSStats(m_View->GetIntSelection(), m_RMSStats, m_Parms, &m_PeakStats))
		return(FALSE);
	if (DUMP_HISTOGRAM)
		DumpHistogram();
	return(TRUE);
}

void CRMSStatsDlg::OnNewStats()
{
	m_List.DeleteAllItems();
	int	chans = m_RMSStats.m_Chan.GetSize();
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		const CWaveProcess::CRMSStats::CChanInfo&	info = m_RMSStats.m_Chan[iChan];
		CString	s;
		LPCTSTR	fmt = _T("%.2f dB");
		m_List.InsertItem(iChan, m_RMSStats.m_ChanName[iChan], 0);
		m_List.SetItemData(iChan, iChan);	// set sort key
		double	ColVal[COLS] = {0, info.m_Min, info.m_Max, info.m_Avg, info.m_Total};
		for (int iCol = COL_MIN; iCol <= COL_TOTAL; iCol++) {
			// for all statistics but total, check for non-zero average count,
			// to ensure selected data was enough for at least one full window
			if (iCol == COL_TOTAL || info.m_AvgCount)
				s.Format(fmt, ColVal[iCol]);
			else	// not enough data and/or window size too small
				s = _T("N/A");
			m_List.SetItemText(iChan, iCol, s);
		}
	}
	if (m_List.SortCol() >= 0)	// if non-default sort
		m_List.SortRows();	// sort rows
	PlotSeries();
}

bool CRMSStatsDlg::PlotSeries()
{
	m_Plot.RemoveAllSeries();
	CPlotCtrl::CSeries	ser;
	// fill and outline area for more precise coverage
	ser.m_Flags = CPlotCtrl::SER_FILL | CPlotCtrl::SER_LINE 
		| CPlotCtrl::SER_FILL_BASELINE;	// add baseline to fill points
	int	chans = m_RMSStats.m_Chan.GetSize();
	COLORREF	palette[] = {	// HLS
		RGB( 85, 128, 255),	// 150, 160, 240
		RGB(255,  85, 128),	// 230, 160, 240
		RGB(255, 213,  85),	//  30, 160, 240
		RGB(128, 255,  85),	//  70, 160, 240
		RGB(170,  85, 255),	// 180, 160, 240
		RGB( 85, 255, 255),	// 120, 160, 240
		RGB(  0,  43, 170),	// 150,  80, 240
		RGB(170,   0,  43),	// 230,  80, 240
		RGB(170, 128,   0),	//  30,  80, 240
		RGB( 43, 170,   0), //  70,  80, 240
		RGB( 85,   0, 170), // 180,  80, 240
		RGB(  0, 170, 170),	// 120,  80, 240
	};
	int	iSelChan = m_TopChannelCombo.GetCurSel();
	if (iSelChan < 0)
		return(FALSE);
	double	HistoMax = 0;	// find maximum histogram bin
	int	iChan;
	for (iChan = 0; iChan < chans; iChan++) {	// for each channel
		const CWaveProcess::CRMSStats::CChanInfo&	info = m_RMSStats.m_Chan[iChan];
		int	bins = info.m_HistoBin.GetSize();
		for (int iBin = 0; iBin < bins; iBin++) {	// for each bin
			if (info.m_HistoBin[iBin] > HistoMax)
				HistoMax = info.m_HistoBin[iBin];
		}
	}
	int	HistoMargin = m_Parms.HistogramSubdiv;
	for (iChan = 0; iChan < chans; iChan++) {	// for each channel
		const CWaveProcess::CRMSStats::CChanInfo&	info = m_RMSStats.m_Chan[iSelChan];
		if (chans > 1)	// if multiple channels
			ser.m_Name = m_RMSStats.m_ChanName[iChan];	// set series name to channel name
		if (HistoMax) {	// avoid divide by zero
			int	bins = info.m_HistoBin.GetSize();
			CIntRange	range = m_RMSStats.m_HistoRange;
			range.Start -= HistoMargin;	// add leading margin for looks
			range.Start = max(range.Start, 0);	// stay within array bounds
			ser.m_Point.SetSize(range.Length() + 1);
			int	iPt = 0;
			for (int iBin = range.Start; iBin <= range.End; iBin++) {	// for each bin
				double	x = double(iBin - (bins - 1)) / m_Parms.HistogramSubdiv;
				double	y = info.m_HistoBin[iBin] / HistoMax * 100;
				ser.m_Point[iPt++] = DPoint(x, y);
			}
			int	iColor = iSelChan % _countof(palette);
			COLORREF	color = palette[iColor];
			ser.m_PenColor = color;
			ser.m_BrushColor = color;
		}
		m_Plot.InsertSeries(iChan, ser);
		iSelChan++;
		if (iSelChan >= chans)
			iSelChan = 0;	// wrap selected channel
	}
	m_Plot.Update();
	return(TRUE);
}

bool CRMSStatsDlg::DumpHistogram()
{
	int	chans = m_RMSStats.m_Chan.GetSize();
	CString	path;
	if (!theApp.GetTempPath(path))
		return(FALSE);
	path += _T("RMSHistogram.txt");
	CStdioFile	fp(path, CFile::modeCreate | CFile::modeWrite);
	int	bins = m_RMSStats.m_Chan[0].m_HistoBin.GetSize();
	CIntRange	range = m_RMSStats.m_HistoRange;
	range.Start -= m_Parms.HistogramSubdiv;	// add 1dB leading margin just for looks
	range.Start = max(range.Start, 0);	// stay within array bounds
	for (int iBin = range.Start; iBin <= range.End; iBin++) {	// for each bin
		CString	line;
		line.Format(_T("%g"), double(iBin - (bins - 1)) / m_Parms.HistogramSubdiv);
		for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
			const CWaveProcess::CRMSStats::CChanInfo&	info = m_RMSStats.m_Chan[iChan];
			CString	s;
			s.Format(_T("%g"), info.m_HistoBin[iBin]); 
			line += '\t';
			line += s;
		}
		fp.WriteString(line + "\n");
	}
	return(TRUE);
}

void CRMSStatsDlg::ResizeTabChild()
{
	CRect	rc;
	m_Tab.GetClientRect(rc);	// client rect is input to AdjustRect
	m_Tab.AdjustRect(FALSE, rc);	// get tab control's display area
	for (int iTab = 0; iTab < TABS; iTab++) {
		m_TabChild[iTab]->MoveWindow(rc);
		m_TabChild[iTab]->Invalidate();
	}
}

void CRMSStatsDlg::ShowTabChild(int TabIdx)
{
	for (int iTab = 0; iTab < TABS; iTab++) {
		int	ShowCmd = (iTab == TabIdx) ? SW_SHOW : SW_HIDE;
		m_TabChild[iTab]->ShowWindow(ShowCmd);
	}
	m_TabSel = TabIdx;
	int	ShowChanCombo = (TabIdx == TAB_HISTOGRAM) ? SW_SHOW : SW_HIDE;
	m_TopChannelCombo.ShowWindow(ShowChanCombo);
	m_TopChannelCap.ShowWindow(ShowChanCombo);
}

#define SORT_CMP(x) retc = m_List.SortCmp(m_RMSStats.m_Chan[p1].x, m_RMSStats.m_Chan[p2].x);

int CRMSStatsDlg::SortCompare(int p1, int p2) const
{
	int	retc;
	switch (m_List.SortCol()) {
	case COL_CHANNEL:
		retc = m_List.SortCmp(p1, p2);
		break;
	case COL_MIN:
		SORT_CMP(m_Min);
		break;
	case COL_MAX:
		SORT_CMP(m_Max);
		break;
	case COL_AVG:
		SORT_CMP(m_Avg);
		break;
	case COL_TOTAL:
		SORT_CMP(m_Total);
		break;
	default:
		NODEFAULTCASE;	// logic error
		retc = 0;
	}
	return(retc);
}

int CALLBACK CRMSStatsDlg::SortCompare(LPARAM p1, LPARAM p2, LPARAM This)
{
	return(((CRMSStatsDlg *)This)->SortCompare(INT64TO32(p1), INT64TO32(p2)));
}

void CRMSStatsDlg::DoDataExchange(CDataExchange* pDX)
{
	CPersistDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRMSStatsDlg)
	DDX_Control(pDX, IDC_RMS_TOP_CHANNEL, m_TopChannelCombo);
	DDX_Control(pDX, IDC_RMS_TOP_CHANNEL_CAP, m_TopChannelCap);
	DDX_Control(pDX, IDC_RMS_TAB_CTRL, m_Tab);
	DDX_Check(pDX, IDC_RMS_ACCOUNT_FOR_DC, m_AccountForDC);
	DDX_Text(pDX, IDC_RMS_WINDOW_SIZE, m_WindowSize);
	DDV_MinMaxInt(pDX, m_WindowSize, 1, 10000);
	DDX_Radio(pDX, IDC_RMS_ZERO_WAVE, m_ZeroWave);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CRMSStatsDlg message map

BEGIN_MESSAGE_MAP(CRMSStatsDlg, CPersistDlg)
	//{{AFX_MSG_MAP(CRMSStatsDlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_RMS_REFRESH, OnRefresh)
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(TCN_SELCHANGE, IDC_RMS_TAB_CTRL, OnSelchangeTabCtrl)
	ON_CBN_SELCHANGE(IDC_RMS_TOP_CHANNEL, OnSelchangeTopChannel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRMSStatsDlg message handlers

BOOL CRMSStatsDlg::OnInitDialog() 
{
	CPersistDlg::OnInitDialog();

	// save initial size
	CRect	rc;
	GetWindowRect(rc);
	m_InitSize = rc.Size();
	// set app icon and add document title to caption
	CWaveShopApp::InitDialogCaptionView(*this, m_View);
	// init resizing
	m_Resize.AddControlList(this, m_CtrlList);
	// init tab control
	m_Tab.InsertItem(TAB_STATS, LDS(IDS_RMS_TAB_STATS));
	m_Tab.InsertItem(TAB_HISTOGRAM, LDS(IDS_RMS_TAB_HISTOGRAM));
	m_Tab.ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	m_Tab.SetCurSel(m_TabSel);
	// channel combo overlaps tab control, so position it on top
	m_TopChannelCombo.SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	m_TopChannelCap.SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	// init list control
	UINT	ListStyle = WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS;
	m_List.Create(ListStyle, CRect(0, 0, 0, 0), &m_Tab, IDC_RMS_STATS_LIST);
	m_List.SetColumns(COLS, m_ColInfo);
	m_List.InitControl(0, CReportCtrl::SORT_ARROWS);
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_List.SetSortCallback(SortCompare, this);
	// init plot control
	m_Plot.SetVisibleRulers(CPlotCtrl::RM_RIGHT | CPlotCtrl::RM_BOTTOM);
	UINT	PlotStyle = CPlotCtrl::DEFAULT_STYLE;
	m_Plot.Create(PlotStyle, CRect(0, 0, 0, 0), &m_Tab, IDC_RMS_STATS_PLOT);
	m_Plot.SetFont(GetFont());
	m_Plot.SetMargins(CRect(10, 10, 0, 0));
	m_Plot.SetDataTipPrecision(PLOT_DATA_TIP_PRECISION);
	// populate channel combo
	CWaveProcess&	wave = m_View->GetWave();
	CStringArray	ChanName;
	wave.GetChannelNames(ChanName);
	int	chans = wave.GetChannels();
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		m_TopChannelCombo.AddString(ChanName[iChan]);
	}
	m_TopChannelCombo.SetCurSel(0);
	// update tab controls
	m_Tab.SetCurSel(m_TabSel);
	ShowTabChild(m_TabSel);
	ResizeTabChild();
	// post a refresh command; use post so dialog finishes initializing
	PostMessage(WM_COMMAND, MAKEWPARAM(IDC_RMS_REFRESH, BN_CLICKED),
		LPARAM(::GetDlgItem(m_hWnd, IDC_RMS_REFRESH)));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRMSStatsDlg::OnSize(UINT nType, int cx, int cy) 
{
	CPersistDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
	if (m_Tab.m_hWnd)
		ResizeTabChild();
}

void CRMSStatsDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	lpMMI->ptMinTrackSize = CPoint(m_InitSize);
	CPersistDlg::OnGetMinMaxInfo(lpMMI);
}

void CRMSStatsDlg::OnRefresh() 
{
	if (!UpdateData())
		return;
	if (!CalcStats()) {	// if calculation fails
		m_RMSStats.m_Chan.RemoveAll();	// empty stats
	}
	OnNewStats();
}

void CRMSStatsDlg::OnSelchangeTabCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int	iTab = m_Tab.GetCurSel();
	if (iTab >= 0)
		ShowTabChild(iTab);
	*pResult = 0;
}

void CRMSStatsDlg::OnSelchangeTopChannel() 
{
	PlotSeries();
}
