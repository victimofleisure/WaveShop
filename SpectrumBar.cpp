// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		30mar13	initial version
		01		07apr13	add ResetView
		02		28apr13	adapt to options refactoring
		03		09may13	remove spurious blank line

        real-time spectrum analyzer bar
 
*/

// SpectrumBar.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "SpectrumBar.h"
#include "SpectrumAnal.h"
#include <math.h>

#if _MFC_VER >= 0x0700
#include "intrin.h"	// for _BitScanReverse
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpectrumBar

IMPLEMENT_DYNAMIC(CSpectrumBar, CMySizingControlBar);

const UINT CSpectrumBar::m_PlotStyleFlags[PLOT_STYLES] = {
	CPlotCtrl::SER_LINE,
	CPlotCtrl::SER_FILL,
	CPlotCtrl::SER_LINE | CPlotCtrl::SER_FILL,
};

CSpectrumBar::CSpectrumBar()
{
	m_szHorz = CSize(0, 100);	// default height when horizontally docked
	ZeroMemory(&m_Parms, sizeof(m_Parms));
	m_Parms.WindowSize = 1024;
	m_OutputChannels = 0;
	m_InputDuration = 0;
	m_PeakHoldTicks = 0;
	m_PeakDecay = 0;
}

CSpectrumBar::~CSpectrumBar()
{
}

void CSpectrumBar::SetParms(const RTSA_PARMS& Parms)
{
	m_Parms = Parms;
	OnParmsChange();
}

void CSpectrumBar::OnParmsChange()
{
	theApp.GetMain()->SetRTSAParms(m_Parms);
	if (IsWindowVisible())
		UpdateView();
}

void CSpectrumBar::UpdateView()
{
	m_OutputChannels = 0;
	m_Plot.RemoveAllSeries();
	CWaveShopView	*view = theApp.GetMain()->GetView();
	m_Plot.ShowWindow(view != NULL ? SW_SHOW : SW_HIDE);
	if (view != NULL && view->GetWave().IsValid()) {
		const CWaveEdit&	wave = view->GetWave();
		ASSERT(m_Parms.WindowSize);	// else logic error
		int	WinSize = m_Parms.WindowSize;
		int	bins = WinSize / 2;
		int	unit;
		UINT	MinorGrid;
		if (m_Parms.FreqAxisType == FAT_LOG) {	// if log scale
			unit = CRulerCtrl::UNIT_LOG;
			MinorGrid = CPlotCtrl::HORZ_MINOR_GRID;	// enable minor gridlines
		} else {	// linear scale
			unit = CRulerCtrl::UNIT_METRIC;
			MinorGrid = 0;	// disable minor gridlines
		}
		m_Plot.ModifyStyle(CPlotCtrl::HORZ_MINOR_GRID, MinorGrid);
		CRulerCtrl&	HorzRuler = m_Plot.GetRuler(CPlotCtrl::RULER_BOTTOM);
		HorzRuler.SetUnit(unit);
		double	fSampleRate = wave.GetSampleRate();
		m_Plot.SetRange(CPlotCtrl::RULER_LEFT, 
			CDblRange(CWaveProcess::MIN_LEVEL, 0));
		m_Plot.SetRange(CPlotCtrl::RULER_BOTTOM, 
			CDblRange(fSampleRate / WinSize, fSampleRate / 2));
		const COptionsInfo&	opts = theApp.GetMain()->GetOptions();
		m_Plot.SetColor(CPlotCtrl::CLR_PLOT_BKGD, opts.m_RTSA.PlotBkgndColor);
		m_Plot.SetColor(CPlotCtrl::CLR_GRID, opts.m_RTSA.PlotGridColor);
		int	chans;
		if (m_Parms.ChannelMode == CM_SEPARATE)	// if channels are separated
			chans = wave.GetChannels();
		else	// channels are combined
			chans = 1;
		UINT	Flags;
		int	PlotStyle = m_Parms.PlotStyle;
		ASSERT(PlotStyle >= 0 && PlotStyle < PLOT_STYLES);
		Flags = m_PlotStyleFlags[PlotStyle];
		Flags |= CPlotCtrl::SER_FILL_BASELINE;
		CStringArray	ChanName;
		wave.GetChannelNames(ChanName);
		int	series = chans * 2;
		m_Plot.SetSeriesCount(series);	// allocate series
		double	BinFreqScale = double(wave.GetSampleRate()) / m_Parms.WindowSize;
		for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
			CPlotCtrl::CSeries&	ser = m_Plot.GetSeries(iChan);
			if (chans > 1)	// if multichannel audio
				ser.m_Name = ChanName[iChan];
			else	// mono
				ser.m_Name.Empty();
			ser.m_Flags = Flags;
			ser.m_Point.SetSize(bins - 1);	// zero bin is excluded
			ser.m_Baseline = CWaveProcess::MIN_LEVEL;
			for (int iBin = 1; iBin < bins; iBin++) {	// for each bin except zero
				double	BinFreq = double(iBin) * BinFreqScale;
				ser.m_Point[iBin - 1] = DPoint(BinFreq, CWaveProcess::MIN_LEVEL);
			}
		}
		if (m_Parms.ShowPeaks) {	// if showing peaks
			m_PeakHold.SetSize(chans * (bins - 1));
			for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
				const CPlotCtrl::CSeries&	ser = m_Plot.GetSeries(iChan);
				CPlotCtrl::CSeries&	peak = m_Plot.GetSeries(iChan + chans);
				peak = ser;
				peak.m_PenColor = CPlotCtrl::GetDefaultPenColor(iChan);
				peak.m_BrushColor = peak.m_PenColor;
				if (m_Parms.ShowPeaks == SPS_DOTS) {
					peak.m_Flags = CPlotCtrl::SER_MARKER_FILL;
					peak.m_MarkerStyle = CPlotCtrl::MARK_CIRCLE;
					peak.m_MarkerSize = 5;
				} else
					peak.m_Flags = CPlotCtrl::SER_LINE;
			}
			int	TimerPeriod = 1000 / CMainFrame::GetTimerFrequency();
			m_PeakHoldTicks = m_Parms.PeakHoldTime / TimerPeriod;
			// peak hold counter is a byte, limiting peak hold time to 255 ticks
			m_PeakHoldTicks = CLAMP(m_PeakHoldTicks, 0, UCHAR_MAX);
			m_PeakDecay = double(m_Parms.PeakDecay) / CMainFrame::GetTimerFrequency();
		}
		// initialize spectrum analyzer
		int	windows = 1 + m_Parms.Averaging;	// number of windows to analyze
		// compute input duration in frames, assuming windows overlap by half
		m_InputDuration = (windows + 1) * m_Parms.WindowSize / 2;
		m_Anal.SetParms(windows, WinSize, m_Parms.WindowFunction, 
			m_Parms.ChannelMode == CM_SEPARATE);
		m_Anal.Setup(&wave);
		m_OutputChannels = chans;	// set channels last as exception failsafe
	}
	m_Plot.Update();	// update plot control
}

void CSpectrumBar::TimerHook(W64INT Frame)
{
	if (!m_OutputChannels)	// if no output channels
		return;	// exceptional condition; nothing to do
	CMainFrame	*main = theApp.GetMain();
	CWaveShopView	*view = main->GetView();
	const CWaveEdit&	wave = view->GetWave();
	CW64IntRange	sel;
	if (main->GetRepeat()) {	// if looping playback
		sel = view->GetIntSelection();
		if (sel.IsEmpty())	// if selection is empty
			sel = CW64IntRange(0, wave.GetFrameCount());	// select all
	} else	// not looping
		sel = CW64IntRange(0, wave.GetFrameCount());	// select all
	W64INT	SelLen = sel.Length();
	if (SelLen < m_InputDuration) {	// if selection smaller than input duration
		if (!SelLen) {	// if empty audio
			ResetView();	// clear plot
			return;	// early out
		}
		Frame = sel.Start;	// ignore frame; begin at start of selection
	} else {	// selection at least as big as input duration
		Frame -= m_InputDuration;	// offset frame backwards by input duration
		if (Frame < sel.Start) {	// if too far back
			if (main->GetRepeat())	// if looping playback
				Frame += SelLen;	// wrap around
			else {	// not looping
				ResetView();	// clear plot
				return;	// early out
			}
		}
	}
	UINT	FrameSize = wave.GetFrameSize();
	sel.Start *= FrameSize;	// convert selection from frames to byte offsets
	sel.End *= FrameSize;
	m_Anal.SetSelection(sel);
	m_Anal.SetRepeat(main->GetRepeat());
	m_Anal.ResetOutput();
	m_Anal.Analyze(Frame);
	int	chans = m_OutputChannels;
	const double	*pBin = m_Anal.GetOutput();
	int	bins = m_Parms.WindowSize / 2;
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		CPlotCtrl::CSeries&	ser = m_Plot.GetSeries(iChan);
		for (int iBin = 1; iBin < bins; iBin++) {	// for each bin except zero
			ASSERT(m_Anal.IsValidBinPtr(&pBin[iBin]));
			ser.m_Point[iBin - 1].y = pBin[iBin];	// set magnitude coordinate
		}
		pBin += bins;
	}
	if (m_Parms.ShowPeaks) {	// if showing peaks
		BYTE	*pPeakHold = m_PeakHold.GetData();
		for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
			const CPlotCtrl::CSeries&	ser = m_Plot.GetSeries(iChan);
			CPlotCtrl::CSeries&	peak = m_Plot.GetSeries(iChan + chans);
			for (int iPt = 0; iPt < bins - 1; iPt++) {	// for each point
				ASSERT(IsValidArrayPtr(m_PeakHold, pPeakHold));
				const double&	Level = ser.m_Point[iPt].y;
				double&	PeakLevel = peak.m_Point[iPt].y;
				if (*pPeakHold > 0)	// if peak hold in progress
					(*pPeakHold)--;	// decrement peak hold counter
				else	// peak hold finished
					PeakLevel -= m_PeakDecay;	// decrement peak level
				if (Level >= PeakLevel) {	// if level at or above peak level
					PeakLevel = Level;
					*pPeakHold = static_cast<BYTE>(m_PeakHoldTicks);
				}
				pPeakHold++;
			}
		}
	}
	m_Plot.UpdateAndInvalidatePlot();
}

void CSpectrumBar::ResetView()
{
	int	series = m_Plot.GetSeriesCount();
	for (int iSer = 0; iSer < series; iSer++) {	// for each series
		CPlotCtrl::CSeries&	ser = m_Plot.GetSeries(iSer);
		int	points = ser.m_Point.GetSize();
		for (int iPt = 0; iPt < points; iPt++)	// for each point
			ser.m_Point[iPt].y = CWaveProcess::MIN_LEVEL;	// reset y-coord
	}
	m_Plot.UpdateAndInvalidatePlot();
}

/////////////////////////////////////////////////////////////////////////////
// CSpectrumBar message map

BEGIN_MESSAGE_MAP(CSpectrumBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CSpectrumBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(ID_RTSA_AVERAGING, ID_RTSA_AVERAGING6, OnUpdateAveraging)
	ON_COMMAND_RANGE(ID_RTSA_AVERAGING, ID_RTSA_AVERAGING6, OnAveraging)
	ON_UPDATE_COMMAND_UI_RANGE(ID_RTSA_PLOT_STYLE, ID_RTSA_PLOT_STYLE3, OnUpdatePlotStyle)
	ON_COMMAND_RANGE(ID_RTSA_PLOT_STYLE, ID_RTSA_PLOT_STYLE3, OnPlotStyle)
	ON_UPDATE_COMMAND_UI_RANGE(ID_RTSA_FREQ_AXIS, ID_RTSA_FREQ_AXIS2, OnUpdateFreqAxis)
	ON_COMMAND_RANGE(ID_RTSA_FREQ_AXIS, ID_RTSA_FREQ_AXIS2, OnFreqAxis)
	ON_UPDATE_COMMAND_UI_RANGE(ID_RTSA_CHANNEL_MODE, ID_RTSA_CHANNEL_MODE2, OnUpdateChannelMode)
	ON_COMMAND_RANGE(ID_RTSA_CHANNEL_MODE, ID_RTSA_CHANNEL_MODE2, OnChannelMode)
	ON_UPDATE_COMMAND_UI_RANGE(ID_RTSA_SHOW_PEAKS, ID_RTSA_SHOW_PEAKS3, OnUpdateShowPeaks)
	ON_COMMAND_RANGE(ID_RTSA_SHOW_PEAKS, ID_RTSA_SHOW_PEAKS3, OnShowPeaks)
	ON_COMMAND(ID_RTSA_OPTIONS, OnOptions)
	ON_COMMAND(ID_RTSA_RESET, OnReset)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpectrumBar message handlers

int CSpectrumBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	UINT	style = CPlotCtrl::DEFAULT_STYLE | CPlotCtrl::HIDE_CLIPPED_VALS;
	style &= ~WS_VISIBLE;	// initially hidden
    CRect r(0, 0, 0, 0);	// arbitrary initial size
    if (!m_Plot.Create(style, r, this, 0))
		return -1;
	m_Plot.SetFitToData(0);
	HGDIOBJ	hFont = GetStockObject(DEFAULT_GUI_FONT);
	m_Plot.SendMessage(WM_SETFONT, WPARAM(hFont));
	m_Plot.SetVisibleRulers(CPlotCtrl::RM_LEFT | CPlotCtrl::RM_BOTTOM);
	m_Plot.SetDataTipPrecision(PLOT_DATA_TIP_PRECISION);
	m_Parms = theApp.GetMain()->GetOptions().m_RTSA;

	return 0;
}

void CSpectrumBar::OnSize(UINT nType, int cx, int cy) 
{
	if (m_IsSizeValid)	// if size is valid
		m_Plot.MoveWindow(0, 0, cx, cy);
	CMySizingControlBar::OnSize(nType, cx, cy);
}

void CSpectrumBar::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (point.x == -1 && point.y == -1)	// if menu triggered via keyboard
		return;	// not supported
	CMenu	menu;
	menu.LoadMenu(IDR_RTSA_CTX);
	CPoint	pt(point);
	theApp.UpdateMenu(this, &menu);
	CMenu	*mp = menu.GetSubMenu(0);
	mp->TrackPopupMenu(0, point.x, point.y, this);
}

void CSpectrumBar::OnUpdateAveraging(CCmdUI* pCmdUI)
{
	int	AvgPreset = pCmdUI->m_nID - ID_RTSA_AVERAGING;
	ULONG	iMSB = 0;
	int	CurAvgPreset;
	if (_BitScanReverse(&iMSB, m_Parms.Averaging))
		CurAvgPreset = iMSB + 1;
	else
		CurAvgPreset = 0;
	pCmdUI->SetRadio(AvgPreset == CurAvgPreset);
}

void CSpectrumBar::OnAveraging(UINT nID)
{
	int	AvgPreset = nID - ID_RTSA_AVERAGING;
	ASSERT(AvgPreset >= 0 && AvgPreset < AVERAGING_PRESETS);
	m_Parms.Averaging = AvgPreset ? (1 << (AvgPreset - 1)) : 0;
	OnParmsChange();
}

void CSpectrumBar::OnUpdatePlotStyle(CCmdUI* pCmdUI)
{
	int	PlotStyle = pCmdUI->m_nID - ID_RTSA_PLOT_STYLE;
	pCmdUI->SetRadio(PlotStyle == m_Parms.PlotStyle);
}

void CSpectrumBar::OnPlotStyle(UINT nID)
{
	int	PlotStyle = nID - ID_RTSA_PLOT_STYLE;
	ASSERT(PlotStyle >= 0 && PlotStyle < PLOT_STYLES);
	m_Parms.PlotStyle = PlotStyle;
	OnParmsChange();
}

void CSpectrumBar::OnUpdateFreqAxis(CCmdUI* pCmdUI)
{
	int	AxisType = pCmdUI->m_nID - ID_RTSA_FREQ_AXIS;
	pCmdUI->SetRadio(AxisType == m_Parms.FreqAxisType);
}

void CSpectrumBar::OnFreqAxis(UINT nID)
{
	int	AxisType = nID - ID_RTSA_FREQ_AXIS;
	ASSERT(AxisType >= 0 && AxisType < FREQ_AXIS_TYPES);
	m_Parms.FreqAxisType = AxisType;
	OnParmsChange();
}

void CSpectrumBar::OnUpdateChannelMode(CCmdUI* pCmdUI)
{
	int	ChanMode = pCmdUI->m_nID - ID_RTSA_CHANNEL_MODE;
	pCmdUI->SetRadio(ChanMode == m_Parms.ChannelMode);
}

void CSpectrumBar::OnChannelMode(UINT nID)
{
	int	ChanMode = nID - ID_RTSA_CHANNEL_MODE;
	ASSERT(ChanMode >= 0 && ChanMode < CHANNEL_MODES);
	m_Parms.ChannelMode = ChanMode;
	OnParmsChange();
}

void CSpectrumBar::OnUpdateShowPeaks(CCmdUI* pCmdUI)
{
	int	ShowPeaks = pCmdUI->m_nID - ID_RTSA_SHOW_PEAKS;
	pCmdUI->SetRadio(ShowPeaks == m_Parms.ShowPeaks);
}

void CSpectrumBar::OnShowPeaks(UINT nID)
{
	int	ShowPeaks = nID - ID_RTSA_SHOW_PEAKS;
	ASSERT(ShowPeaks >= 0 && ShowPeaks < SHOW_PEAKS_STYLES);
	m_Parms.ShowPeaks = ShowPeaks;
	OnParmsChange();
}

void CSpectrumBar::OnOptions()
{
	// set options page to RTSA
	theApp.GetMain()->SetOptionsPage(COptionsDlg::PAGE_RTSA);
	SendMessage(WM_COMMAND, ID_EDIT_OPTIONS);	// edit options
}

void CSpectrumBar::OnReset()
{
	ResetView();
}
