// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		19mar13	initial version
		01		04apr13	in OnRefresh, CAsyncJob fix obviates set focus on cancel
        02      08apr13	in MakePlot, remove unused var
		03		14jul13	add option to specify frequency range

		spectrum dialog

*/

// SpectrumDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "SpectrumDlg.h"
#include "PathStr.h"
#include "SpectrumAnal.h"
#include <math.h>
#include <afxpriv.h>	// for WM_KICKIDLE

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpectrumDlg dialog

const CCtrlResize::CTRL_LIST CSpectrumDlg::m_CtrlList[] = {
	{IDC_SPEC_PLOT,				BIND_ALL},
	{IDC_SPEC_WINDOW_FUNC_CAP,	BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_WINDOW_FUNC,		BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_WINDOW_SIZE_CAP,	BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_WINDOW_SIZE,		BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_FREQ_AXIS_CAP,	BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_FREQ_AXIS,		BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_CHANNEL_MODE_CAP,	BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_CHANNEL_MODE,		BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_VIEW_CHANNEL_CAP,	BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_VIEW_CHANNEL,		BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_REFRESH,			BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_EXPORT,			BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_FREQ_RANGE_GROUP,	BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_FREQ_RANGE_AUTO,	BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_FREQ_EDIT_START,	BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_FREQ_EDIT_END,	BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_FREQ_EDIT_START_CAP,	BIND_LEFT | BIND_BOTTOM},
	{IDC_SPEC_FREQ_EDIT_END_CAP,	BIND_LEFT | BIND_BOTTOM},
	{IDOK,						BIND_LEFT | BIND_BOTTOM},
	{0}
};

#define RK_SPEC_WINDOW_FUNC		_T("SpecWindowFunc")
#define RK_SPEC_WINDOW_SIZE		_T("SpecWindowSize")
#define RK_SPEC_FREQ_AXIS		_T("SpecFreqAxis")
#define RK_SPEC_CHANNEL_MODE	_T("SpecChannelMode")
#define RK_SPEC_FREQ_RANGE_AUTO	_T("SpecFreqRangeAuto")
#define RK_SPEC_FREQ_RANGE		_T("SpecFreqRange")

CSpectrumDlg::CSpectrumDlg(CWaveShopView *View, CWnd* pParent /*=NULL*/)
	: CPersistDlg(IDD, 0, _T("SpectrumDlg"), pParent)
{
	//{{AFX_DATA_INIT(CSpectrumDlg)
	//}}AFX_DATA_INIT
	ASSERT(View != NULL);
	m_View = View;
	m_InitSize = CSize(0, 0);
	m_WindowFunction = theApp.RdRegInt(RK_SPEC_WINDOW_FUNC, CSpectrumAnal::WF_HANN);
	m_WindowSize = theApp.RdRegInt(RK_SPEC_WINDOW_SIZE, 1024);
	m_FreqAxisType = theApp.RdRegInt(RK_SPEC_FREQ_AXIS, FAT_LINEAR);
	m_ChannelMode = theApp.RdRegInt(RK_SPEC_CHANNEL_MODE, CM_COMBINE);
	m_FreqRangeAuto = theApp.RdRegInt(RK_SPEC_FREQ_RANGE_AUTO, TRUE);
	if (m_FreqRangeAuto)	// if auto frequency range, set default range
		m_FreqRange = CDblRange(0, View->GetWave().GetSampleRate() / 2);
	else	// manual frequency range; read range from registry
		theApp.RdReg2Struct(RK_SPEC_FREQ_RANGE, m_FreqRange);
}

CSpectrumDlg::~CSpectrumDlg()
{
	theApp.WrRegInt(RK_SPEC_WINDOW_FUNC, m_WindowFunction);
	theApp.WrRegInt(RK_SPEC_WINDOW_SIZE, m_WindowSize);
	theApp.WrRegInt(RK_SPEC_FREQ_AXIS, m_FreqAxisType);
	theApp.WrRegInt(RK_SPEC_CHANNEL_MODE, m_ChannelMode);
	theApp.WrRegInt(RK_SPEC_FREQ_RANGE_AUTO, m_FreqRangeAuto);
	theApp.WrRegStruct(RK_SPEC_FREQ_RANGE, m_FreqRange);
}

inline void CSpectrumDlg::InitWindowFunctionCombo(CComboBox& Combo, int WindowFunction)
{
	for (int iFunc = 0; iFunc < CSpectrumAnal::WINDOW_FUNCTIONS; iFunc++) {
		int	pos = Combo.AddString(CSpectrumAnal::GetWindowFuncName(iFunc));
		Combo.SetItemData(pos, iFunc);
	}
	Combo.SelectString(0, CSpectrumAnal::GetWindowFuncName(WindowFunction));
}

inline void CSpectrumDlg::InitWindowSizeCombo(CComboBox& Combo, int WindowSize)
{
	int	WinSize = 1 << FIRST_WINDOW_SIZE;
	int	iSelWinSize = -1;
	CString	s;
	for (int iSize = 0; iSize < WINDOW_SIZES; iSize++) {
		s.Format(_T("%d"), WinSize);
		Combo.AddString(s);
		Combo.SetItemData(iSize, WinSize);
		if (WinSize == WindowSize)
			iSelWinSize = iSize;
		WinSize <<= 1;
	}
	if (iSelWinSize >= 0) 
		Combo.SetCurSel(iSelWinSize);
}

inline void CSpectrumDlg::InitViewChannelCombo()
{
	enum {
		IMG_WIDTH = 6,
		IMG_HEIGHT = 6,
	};
	CWaveProcess&	wave = m_View->GetWave();
	CStringArray	ChanName;
	wave.GetChannelNames(ChanName);	// get channel names
	int	chans = INT64TO32(ChanName.GetSize());
	CSize	ImgSize(IMG_WIDTH, IMG_HEIGHT);
	if (!m_ViewChanImgList.Create(ImgSize.cx, ImgSize.cy, ILC_COLOR, chans, 1))
		AfxThrowResourceException();
	if (!m_ViewChanImgList.SetImageCount(chans))	// one bitmap per channel
		AfxThrowResourceException();
	CClientDC	dc(this);
	CDC	MemDC;
	if (!MemDC.CreateCompatibleDC(&dc))	// create memory DC
		AfxThrowResourceException();
	CBitmap	bmp;
	if (!bmp.CreateCompatibleBitmap(&dc, ImgSize.cx, ImgSize.cy))	// create bitmap
		AfxThrowResourceException();
	m_ViewChannelCombo.SetImageList(&m_ViewChanImgList);	// set combo's image list
	COMBOBOXEXITEM	cbei;
	cbei.mask = CBEIF_TEXT;
	cbei.iItem = 0;
	CString	sAllChans((LPCTSTR)IDS_FIND_ALL_CHANNELS);
	LPCTSTR	pText = sAllChans;
	cbei.pszText = const_cast<LPTSTR>(pText);	// pszText also used for retrieval
	m_ViewChannelCombo.InsertItem(&cbei);	// insert ALL item
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		HGDIOBJ	PrevBmp = MemDC.SelectObject(bmp);
		COLORREF	color = CPlotCtrl::GetDefaultPenColor(iChan);
		MemDC.FillSolidRect(0, 0, ImgSize.cx, ImgSize.cy, color);	// draw bitmap
		MemDC.SelectObject(PrevBmp);
		m_ViewChanImgList.Replace(iChan, &bmp, NULL);	// copy bitmap into image list
		cbei.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
		cbei.iItem = iChan + 1;	// index off by one due to ALL item 
		cbei.iImage = iChan;
		cbei.iSelectedImage = iChan;
		LPCTSTR	pText = ChanName[iChan];
		cbei.pszText = const_cast<LPTSTR>(pText);
		m_ViewChannelCombo.InsertItem(&cbei);	// insert channel item
	}
	m_ViewChannelCombo.SetCurSel(0);	// default to view all channels
}

inline void CSpectrumDlg::InitPlotCtrl()
{
	CWnd	*PlotProxy = GetDlgItem(IDC_SPEC_PLOT);	// plot placeholder
	m_Plot.SetVisibleRulers(CPlotCtrl::RM_LEFT | CPlotCtrl::RM_BOTTOM);
	UINT	PlotStyle = WS_DLGFRAME	// for 3D raised border
		| CPlotCtrl::DEFAULT_STYLE | CPlotCtrl::HIDE_CLIPPED_VALS;
	CRect	rProxy;
	PlotProxy->GetWindowRect(rProxy);	// get placeholder's rectangle
	ScreenToClient(rProxy);
	CRect	rc;
	GetClientRect(rc);	// plot control gets our client rect
	rc.bottom = rProxy.bottom;	// except with placeholder's bottom
	m_Plot.Create(PlotStyle, rc, this, IDC_SPEC_PLOT);
	m_Plot.SetFont(GetFont());
	m_Plot.SetMargins(CRect(4, 8, 8, 4));
	m_Plot.SetDataTipPrecision(PLOT_DATA_TIP_PRECISION);
	PlotProxy->DestroyWindow();	// destroy supplanted plot placeholder
}

bool CSpectrumDlg::Refresh()
{
	int	sel;
	sel = m_WindowFuncCombo.GetCurSel();
	ASSERT(sel >= 0);
	m_WindowFunction = INT64TO32(m_WindowFuncCombo.GetItemData(sel));
	sel = m_WindowSizeCombo.GetCurSel();
	ASSERT(sel >= 0);
	m_WindowSize = INT64TO32(m_WindowSizeCombo.GetItemData(sel));
	sel = m_ChannelModeCombo.GetCurSel();
	ASSERT(sel >= 0);
	m_ChannelMode = sel;
	m_Spec.m_WindowFunction = m_WindowFunction;
	m_Spec.m_WindowSize = m_WindowSize;
	m_Spec.m_SeparateChannels = m_ChannelMode == CM_SEPARATE;
	m_Spec.m_Bin.RemoveAll();
	CWaveProcess&	wave = m_View->GetWave();
	bool	retc = wave.GetSpectrum(m_View->GetIntSelection(), m_Spec);
	MakePlot();
	m_ViewChannelCombo.EnableWindow(m_Spec.m_SeparateChannels);
	if (!m_Spec.m_SeparateChannels)
		m_ViewChannelCombo.SetCurSel(0);
	GetDlgItem(IDC_SPEC_EXPORT)->EnableWindow(retc);
	return(retc);
}

void CSpectrumDlg::MakePlot()
{
	int	sel = m_FreqAxisCombo.GetCurSel();
	ASSERT(sel >= 0);
	m_FreqAxisType = sel;
	int	ViewChannel = m_ViewChannelCombo.GetCurSel();
	bool	LogScale = m_FreqAxisType == FAT_LOG;
	int	unit;
	UINT	MinorGrid;
	if (LogScale) {	// if log scale
		unit = CRulerCtrl::UNIT_LOG;
		MinorGrid = CPlotCtrl::HORZ_MINOR_GRID;	// enable minor gridlines
	} else {	// linear scale
		unit = CRulerCtrl::UNIT_METRIC;
		MinorGrid = 0;	// disable minor gridlines
	}
	m_Plot.ModifyStyle(CPlotCtrl::HORZ_MINOR_GRID, MinorGrid);
	CRulerCtrl&	HorzRuler = m_Plot.GetRuler(CPlotCtrl::RULER_BOTTOM);
	HorzRuler.SetUnit(unit);
	CWaveProcess&	wave = m_View->GetWave();
	ASSERT(wave.IsValid());
	int	chans;
	bool	SepChans = m_Spec.m_SeparateChannels;
	CStringArray	ChanName;
	if (SepChans) {	// if channels are separated
		chans = wave.GetChannels();
		wave.GetChannelNames(ChanName);
	} else	// channels are combined
		chans = 1;
	int	TotalBins = m_Spec.m_Bin.GetSize();
	int	bins = TotalBins / chans;
	m_Plot.RemoveAllSeries();
	if (bins) {
		ASSERT(m_WindowSize);	// else logic error and divide by zero
		double	BinFreqScale = double(wave.GetSampleRate()) / m_WindowSize;
		const double	*pBin = m_Spec.m_Bin.GetData();
		for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
			CPlotCtrl::CSeries	ser;
			if (chans > 1)
				ser.m_Name = ChanName[iChan];
			if (SepChans && ViewChannel > 0 && ViewChannel - 1 != iChan)
				ser.m_Flags = 0;
			ser.m_Point.SetSize(bins - 1);	// exclude zero bin
			for (int iBin = 1; iBin < bins; iBin++) {	// for each bin except zero
				double	BinFreq = double(iBin) * BinFreqScale;
				ASSERT(CSpectrumAnal::IsValidBinPtr(m_Spec.m_Bin, &pBin[iBin]));
				ser.m_Point[iBin - 1] = DPoint(BinFreq, pBin[iBin]);
			}
			m_Plot.InsertSeries(iChan, ser);
			pBin += bins;	// point to next channel's bins
		}
	}
	int	FitToData;
	if (m_FreqRangeAuto)	// if auto frequency range
		FitToData = CPlotCtrl::RM_ALL;	// fit all axes to data
	else {	// manual frequency range; explicitly set bottom ruler's range
		FitToData = CPlotCtrl::RM_VERT;	// only fit vertical axes to data
		m_Plot.SetRange(CPlotCtrl::RULER_BOTTOM, m_FreqRange);	// set ruler range
	}
	m_Plot.SetFitToData(FitToData);	// set fit to data mask before updating plot
	m_Plot.Update();
}

void CSpectrumDlg::Export(LPCTSTR Path)
{
	CWaveProcess&	wave = m_View->GetWave();
	ASSERT(wave.IsValid());
	UINT	SampleRate = wave.GetSampleRate();
	int	chans;
	bool	SepChans = m_Spec.m_SeparateChannels;
	if (SepChans)	// if channels are separated
		chans = wave.GetChannels();
	else	// channels are combined
		chans = 1;
	int	TotalBins = m_Spec.m_Bin.GetSize();
	int	bins = TotalBins / chans;
	CStdioFile	fp(Path, CFile::modeCreate | CFile::modeWrite);
	CString	line, s;
	// output column header
	if (chans > 1) {	// if separated channels, column header varies
		line = _T("Frequency (Hz)");
		CStringArray	ChanName;
		wave.GetChannelNames(ChanName);
		for (int iChan = 0; iChan < chans; iChan++)	// for each channel
			line += '\t' + ChanName[iChan];	// add channel name to column header
		fp.WriteString(line + '\n');	// write header
	} else	// combined channels: fixed column header
		fp.WriteString(_T("Frequency (Hz)\tLevel (dB)\n"));	// write header
	for (int iBin = 1; iBin < bins; iBin++) {	// for each point
		double	BinFreq = double(iBin) * SampleRate / m_WindowSize;
		line.Format(_T("%f"), BinFreq);	// format frequency
		for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
			double	amp = m_Spec.m_Bin[iChan * bins + iBin];
			s.Format(_T("\t%f"), amp);	// format amplitude
			line += s;	// concatenate to line
		}
		fp.WriteString(line + '\n');	// write line
	}
}

void CSpectrumDlg::DoDataExchange(CDataExchange* pDX)
{
	CPersistDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpectrumDlg)
	DDX_Control(pDX, IDC_SPEC_VIEW_CHANNEL, m_ViewChannelCombo);
	DDX_Control(pDX, IDC_SPEC_CHANNEL_MODE, m_ChannelModeCombo);
	DDX_Control(pDX, IDC_SPEC_FREQ_AXIS, m_FreqAxisCombo);
	DDX_Control(pDX, IDC_SPEC_WINDOW_SIZE, m_WindowSizeCombo);
	DDX_Control(pDX, IDC_SPEC_WINDOW_FUNC, m_WindowFuncCombo);
	DDX_Check(pDX, IDC_SPEC_FREQ_RANGE_AUTO, m_FreqRangeAuto);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_SPEC_FREQ_EDIT_END, m_FreqRange.End);
	DDX_Text(pDX, IDC_SPEC_FREQ_EDIT_START, m_FreqRange.Start);
}

/////////////////////////////////////////////////////////////////////////////
// CSpectrumDlg message map

BEGIN_MESSAGE_MAP(CSpectrumDlg, CPersistDlg)
	//{{AFX_MSG_MAP(CSpectrumDlg)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_SPEC_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_SPEC_EXPORT, OnExport)
	ON_CBN_SELCHANGE(IDC_SPEC_FREQ_AXIS, OnReplot)
	ON_CBN_SELCHANGE(IDC_SPEC_VIEW_CHANNEL, OnReplot)
	ON_BN_CLICKED(IDC_SPEC_FREQ_RANGE_AUTO, OnFreqRangeAuto)
	ON_EN_KILLFOCUS(IDC_SPEC_FREQ_EDIT_START, OnKillfocusFreqEdit)
	ON_EN_KILLFOCUS(IDC_SPEC_FREQ_EDIT_END, OnKillfocusFreqEdit)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_UPDATE_COMMAND_UI_RANGE(IDC_SPEC_FREQ_EDIT_END, IDC_SPEC_FREQ_EDIT_START_CAP, OnUpdateFreqEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpectrumDlg message handlers

BOOL CSpectrumDlg::OnInitDialog() 
{
	CPersistDlg::OnInitDialog();

	// save initial size
	CRect	rc;
	GetWindowRect(rc);
	m_InitSize = rc.Size();
	// set app icon and add document title to caption
	CWaveShopApp::InitDialogCaptionView(*this, m_View);
	// init plot control
	InitPlotCtrl();
	// init resizing; order matters, must replace plot placeholder first
	m_Resize.AddControlList(this, m_CtrlList);
	// init window function combo box; items are sorted
	InitWindowFunctionCombo(m_WindowFuncCombo, m_WindowFunction);
	// init window size combo box
	InitWindowSizeCombo(m_WindowSizeCombo, m_WindowSize);
	// init view channel combo box
	InitViewChannelCombo();
	// init other combos
	m_FreqAxisCombo.SetCurSel(m_FreqAxisType);
	m_ChannelModeCombo.SetCurSel(m_ChannelMode);
	// post a refresh command; use post so dialog finishes initializing
	PostMessage(WM_COMMAND, MAKEWPARAM(IDC_SPEC_REFRESH, BN_CLICKED),
		LPARAM(::GetDlgItem(m_hWnd, IDC_SPEC_REFRESH)));
	// update UI now, else it wouldn't update until after refresh completes
    UpdateDialogControls(this, TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpectrumDlg::OnCancel()
{
	OnOK();	// save dialog data regardless of how dialog is closed
}

void CSpectrumDlg::OnSize(UINT nType, int cx, int cy) 
{
	CPersistDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

void CSpectrumDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	lpMMI->ptMinTrackSize = CPoint(m_InitSize);
	CPersistDlg::OnGetMinMaxInfo(lpMMI);
}

void CSpectrumDlg::OnRefresh() 
{
	Refresh();
}

void CSpectrumDlg::OnReplot() 
{
	MakePlot();	
}

void CSpectrumDlg::OnExport() 
{
	CString	TextFilter(LDS(IDS_TEXT_FILTER));
	CPathStr	title(m_View->GetDocument()->GetTitle());
	title.RemoveExtension();
	CString	sWinSize;
	sWinSize.Format(_T("%d"), m_WindowSize);
	CString	FuncName(CSpectrumAnal::GetWindowFuncName(m_WindowFunction));
	title += ' ' + FuncName + '=' + sWinSize;
	CFileDialog	fd(FALSE, _T(".txt"), title, OFN_OVERWRITEPROMPT, TextFilter);
	if (fd.DoModal() == IDOK)
		Export(fd.GetPathName());
}

LRESULT CSpectrumDlg::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
    UpdateDialogControls(this, TRUE);
    return FALSE;
}

void CSpectrumDlg::OnUpdateFreqEdit(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!IsDlgButtonChecked(IDC_SPEC_FREQ_RANGE_AUTO));
}

void CSpectrumDlg::OnKillfocusFreqEdit() 
{
	UpdateData();	// retrieve data from controls
	MakePlot();
}

void CSpectrumDlg::OnFreqRangeAuto() 
{
	UpdateData();	// retrieve data from controls
	if (m_FreqRangeAuto) {	// if auto frequency range
		// restore default frequency range
		m_FreqRange = CDblRange(0, m_View->GetWave().GetSampleRate() / 2);
		UpdateData(FALSE);	// update controls from data
		MakePlot();
	}
}
