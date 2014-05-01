// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09mar13	initial version
        01      26mar13	move double buffering to its own class
        02      30mar13	in TimerHook, fix looping
		03		08apr13	in TimerHook, check for empty audio
		04		23apr13	override OnMouseActivate to prevent activation
		05		26apr13	add wave member
		06		09may13	add clip detection
		07		22may13	in UpdateView, reset timer to avoid paint errors
		08		05jun13	in OnLButtonDown, validate channel index
		09		02sep13	in UpdateCaptions, remove unused variable

        meter view
 
*/

// MeterView.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "MeterView.h"
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
// CMeterView

IMPLEMENT_DYNCREATE(CMeterView, CScrollView)

const CMeterView::SEGMENT CMeterView::m_Segment[SEGMENTS] = {
	{RGB(0, 127, 0),	RGB(0, 255, 0),		36},	// soft (green)
	{RGB(127, 127, 0),	RGB(255, 255, 0),	9},		// loud (yellow)
	{RGB(127, 0, 0),	RGB(255, 0, 0),		3},		// peak (red)
};

CMeterView::CMeterView()
{
	m_PrevFont = NULL;
	m_ClientSize = CSize(0, 0);
	m_ViewSize = CSize(0, 0);
	m_CaptionHeight = 0;
	m_Ticks = 0;
	m_Timer = 0;
	m_TimerFrequency = 0;
	m_Wave = NULL;
}

CMeterView::~CMeterView()
{
}

BOOL CMeterView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
    ASSERT(cs.lpszClass);
	
	return CScrollView::PreCreateWindow(cs);
}

void CMeterView::CreateBackBuffer(int Width, int Height)
{
	int	chans = GetChannelCount();
	int	ViewWidth = chans * (BAR_WIDTH + GUTTER) - GUTTER + MARGIN_X * 2;
	if (!m_DC.CreateBackBuffer(ViewWidth, Height))
		AfxThrowResourceException();
	m_ClientSize = CSize(Width, Height);
	m_ViewSize = CSize(ViewWidth, Height);
	SetScrollSizes(MM_TEXT, CSize(ViewWidth, 0));
	EnableScrollBarCtrl(SB_VERT, FALSE);	// disable vertical scrolling
}

inline const CWaveEdit *CMeterView::GetWave() const
{
	if (m_Wave != NULL)
		return(m_Wave);
	return(theApp.GetMain()->GetViewWave());
}

void CMeterView::UpdateView()
{
	const CWaveEdit	*Wave = GetWave();
	int	chans;
	if (Wave != NULL)	// if active view exists
		chans = Wave->GetChannels();
	else	// no active view
		chans = 0;	// no channels either
	if (chans != GetChannelCount()) {	// if channel count changed
		m_MaxSamp.SetSize(chans);	// resize per-channel arrays
		m_Meter.SetSize(chans);
		CreateBackBuffer(m_ClientSize.cx, m_ClientSize.cy);	// recreate back buffer
	}
	ZeroMemory(m_Meter.GetData(), chans * sizeof(METER));	// reset meter state
	m_Timer = 0;	// reset TimerHook's alternating update sequence
	UpdateCaptions();
	UpdateMeters();
	Invalidate();	// repaint entire client area
}

void CMeterView::UpdateCaptions()
{
	const CWaveEdit	*Wave = GetWave();
	CRect	rCaption(0, 0, m_ViewSize.cx, m_CaptionHeight);
	m_DC.FillSolidRect(rCaption, GetSysColor(COLOR_3DFACE));
	if (Wave != NULL) {	// if active view exists
		HGDIOBJ	PrevFont = m_DC.SelectObject(
			GetStockObject(DEFAULT_GUI_FONT));	// select caption font
		// draw channel name abbreviations
		CStringArray	ChanName;
		Wave->GetChannelNames(ChanName);
		int	x = MARGIN_X + BAR_WIDTH / 2;
		int	chans = GetChannelCount();
		for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
			if (m_Meter[iChan].OnRailsCount < 0) {	// if channel clipped
				CRect	r(CPoint(x - BAR_WIDTH / 2, CAPTION_Y), 
					CSize(BAR_WIDTH, m_CaptionHeight - CAPTION_Y));
				m_DC.FillSolidRect(r, RGB(255, 0, 0));
			}
			m_DC.TextOut(x, CAPTION_Y, 
				CWaveEdit::AbbreviateChannelName(ChanName[iChan]));
			x += BAR_WIDTH + GUTTER;
		}
		m_DC.SelectObject(PrevFont);	// reselect previous font
	}
	InvalidateRect(rCaption);	// paint captions
}

inline int CMeterView::CalcBarHeight(int Height) const
{
	return(Height - m_CaptionHeight - MARGIN_TOP - MARGIN_BOTTOM);
}

void CMeterView::UpdateMeters()
{
	// fill back buffer's meter area with background color first
	CRect	rMeter(0, m_CaptionHeight, m_ViewSize.cx, m_ViewSize.cy);
	m_DC.FillSolidRect(rMeter, GetSysColor(COLOR_3DFACE));
	InvalidateRect(rMeter);	// queue paint before possible early out
	int	BarHeight = CalcBarHeight(m_ClientSize.cy);
	if (BarHeight <= 0)	// if client area too small for meters
		return;	// early out
	int	BarBottom = BarHeight + MARGIN_TOP + m_CaptionHeight;
	ASSERT(m_Ticks > 0);	// else divide by zero
	int	ticks = m_Ticks;
	int	TickGap = RANGE / ticks;
	int	x = MARGIN_X;
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		const METER&	meter = m_Meter[iChan];
		int	LevelY = round(meter.Level * BarHeight);
		int	PeakY = round(meter.PeakLevel * BarHeight);
		COLORREF	PeakColor = 0;
		int	SegY = 0;	// bottom-up and relative to bar bottom
		for (int iSeg = 0; iSeg < SEGMENTS; iSeg++) {	// for each segment
			const SEGMENT&	seg = m_Segment[iSeg];
			int	SegHeight;
			if (iSeg < SEGMENTS - 1)	// if not last segment
				SegHeight = round(double(seg.Height) / RANGE * BarHeight);
			else	// last segment is special
				SegHeight = BarHeight - SegY;	// avoid rounding error
			int	LitHeight = LevelY - SegY;
			LitHeight = max(LitHeight, 0);
			int	y = BarBottom - SegY;
			if (LitHeight) {	// if any portion of segment is lit
				m_DC.FillSolidRect(x, y - LitHeight, BAR_WIDTH, LitHeight, seg.LitColor);
			}
			int	UnlitHeight = SegHeight - LitHeight;
			if (UnlitHeight) {	// if any portion of segment is unlit
				m_DC.FillSolidRect(x, y - SegHeight, BAR_WIDTH, UnlitHeight, seg.UnlitColor);
			}
			if (PeakY > SegY)	// if peak includes this segment
				PeakColor = seg.LitColor;	// update peak color
			SegY += SegHeight;
		}
		if (PeakY)	// if peak is visible
			m_DC.FillSolidRect(x, BarBottom - PeakY, BAR_WIDTH, 1, PeakColor);
		for (int iTick = 1; iTick < ticks; iTick++) {	// for each tick
			int	y = round(BarHeight * (double(TickGap) / RANGE) * iTick) 
				+ MARGIN_TOP + m_CaptionHeight;
			CString	s;
			s.Format(_T("%d"), iTick * -TickGap);
			m_DC.TextOut(x + BAR_WIDTH / 2, y - FONT_HEIGHT / 2, s);
		}
		x += BAR_WIDTH + GUTTER;
	}
}

int CMeterView::FindClipIndicator(CPoint point) const
{
	int	x = MARGIN_X;
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		CRect	r(CPoint(x, CAPTION_Y), CSize(BAR_WIDTH, m_CaptionHeight - CAPTION_Y));
		if (r.PtInRect(point))
			return(iChan);
		x += BAR_WIDTH + GUTTER;
	}	
	return(-1);
}

void CMeterView::ResetClipping(int ChannelIdx)
{
	METER&	meter = m_Meter[ChannelIdx];
	if (meter.OnRailsCount < 0) {	// if channel clipped
		meter.OnRailsCount = 0;
		UpdateCaptions();	// update clip indicators
	}
}

void CMeterView::ResetAllClipping()
{
	bool	WasReset = FALSE;
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		METER&	meter = m_Meter[iChan];
		if (meter.OnRailsCount < 0) {	// if channel clipped
			meter.OnRailsCount = 0;
			WasReset = TRUE;
		}
	}
	if (WasReset)	// if clipping was reset on one or more channels
		UpdateCaptions();	// update clip indicators
}

double CMeterView::GetMaxLevel() const
{
	double	level = 0;
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++)	// for each channel
		level = max(m_Meter[iChan].Level, level);
	return(level);
}

inline int CMeterView::CalcTickCount(int Height) const
{
	int	BarHeight = CalcBarHeight(Height);
	int	MaxTicks = BarHeight / (FONT_HEIGHT + TEXT_LEADING);
	int	ticks;
	if (MaxTicks < RANGE) {	// if maximum ticks within meter range
		// find largest power of two that doesn't exceed MaxTicks
		ULONG	iMSB;	// index of most significant bit
		if (!_BitScanReverse(&iMSB, MaxTicks))	// if MSB not found
			iMSB = 0;	// MaxTicks must be zero; one tick minimum
		ticks = 1 << iMSB;
		ticks = min(ticks, RANGE / 3);	// minimum 3 dB steps
	} else	// maximum ticks equals or exceeds meter range
		ticks = RANGE;	// 1 dB steps
	return(ticks);
}

void CMeterView::ComputeLevels(const CWave& Wave, W64INT Offset, W64INT Frames, CW64IntRange Selection, bool Repeat)
{
	int	ClipThreshold = theApp.GetMain()->GetOptions().m_MeterClipThreshold;
	CWave::SAMPLE	NegRail, PosRail;
	Wave.GetSampleRails(NegRail, PosRail);
	W64INT	SelLen = Selection.Length();
	UINT	SampleSize = Wave.GetSampleSize();
	int chans = GetChannelCount();
	bool	ClippingFound = FALSE;
	for (W64INT iFrame = 0; iFrame < Frames; iFrame++) {	// for each frame
		if (Offset >= Selection.End) {	// if at or beyond end of range
			if (!Repeat)	// if not looping playback
				break;	// no more frames
			Offset -= SelLen;	// wrap offset to start of range
		}
		for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
			CWave::SAMPLE	samp = Wave.GetSampleAt(Offset);
			METER&	meter = m_Meter[iChan];
			if (meter.OnRailsCount >= 0) {	// if channel isn't already clipped
				if (samp <= NegRail || samp >= PosRail) {	// if sample on rails
					meter.OnRailsCount++;	// increment on rails count
					// if too many consecutive samples on rails
					if (meter.OnRailsCount >= ClipThreshold) {
						meter.OnRailsCount = -1;	// mark channel clipped
						ClippingFound = TRUE;	// set clipping detected flag
					}
				} else	// sample not on rails
					meter.OnRailsCount = 0;	// reset on rails count
			}
			if (samp == INT_MIN) {	// if most negative integer
				samp = INT_MAX;	// abs would return zero, work around it
			} else	// any other integer
				samp = abs(samp);	// convert sample to magnitude
			if (samp > m_MaxSamp[iChan])	// if magnitude exceeds maximum
				m_MaxSamp[iChan] = samp;	// update maximum
			Offset += SampleSize;	// increment offset by sample size
		}
	}
	if (ClippingFound)	// if clipping was found on one or more channels
		UpdateCaptions();	// update clip indicators
}

void CMeterView::TimerHook(W64INT Frame)
{
	CMainFrame	*main = theApp.GetMain();
	CWaveShopView	*view = main->GetView();
	if (view == NULL)	// if no active view
		return;	// nothing to do
	const CWave&	wave = view->GetWave();
	if (!wave.IsValid())	// if invalid wave
		return;	// nothing to do
	int	chans = GetChannelCount();
	W64INT	frames = wave.GetFrameCount();
	bool	UpdatingMeters = (m_Timer & 1) != 0;	// update meters every other call
	if (!UpdatingMeters)	// if not updating meters
		ZeroMemory(m_MaxSamp.GetData(), chans * sizeof(int));	// zero maximum samples
	if (frames && Frame <= frames) {	// if frame within non-empty audio
		ASSERT(UINT(chans) == wave.GetChannels());	// channel count must match audio
		ASSERT(m_TimerFrequency);	// else divide by zero
		double	fWindowFrames =  wave.GetSampleRate() / m_TimerFrequency;
		double	pitch = main->GetPitchBar().GetPitch();
		if (pitch)	// if playback pitch is shifted
			fWindowFrames *= pow(2, pitch);	// compensate window size for pitch shift
		W64INT	WindowFrames = roundW64INT(fWindowFrames);
		bool	repeat = main->GetRepeat();
		CW64IntRange	sel;
		if (repeat) {	// if looping playback
			sel = view->GetIntSelection();
			if (sel.IsEmpty())	// if selection is empty
				sel = CW64IntRange(0, frames);	// select all
		} else	// not looping
			sel = CW64IntRange(0, frames);	// select all
		W64INT	SelLen = sel.Length();
		if (SelLen < WindowFrames) {	// if selection smaller than window
			Frame = sel.Start;	// ignore frame; begin at start of selection
		} else {	// selection as least as big as window
			Frame -= WindowFrames;	// offset frame backwards by window size
			if (Frame < sel.Start) {	// if too far back
				if (repeat)	// if looping playback
					Frame += SelLen;	// wrap around
				else {	// not looping
					WindowFrames -= sel.Start - Frame;	// trim window size
					Frame = sel.Start;	// begin at start of selection
				}
			}
		}
		W64INT	offset = wave.GetByteOffset(0, Frame);
		UINT	FrameSize = wave.GetFrameSize();
		sel.Start *= FrameSize;	// convert selection from frames to byte offsets
		sel.End *= FrameSize;
		ComputeLevels(wave, offset, WindowFrames, sel, repeat);
	}
	UpdatePeaks(wave);
	if (UpdatingMeters)	// if updating meters
		UpdateMeters();
	m_Timer++;	// increment time counter
}

void CMeterView::UpdatePeaks(const CWave& Wave)
{
	CWave::SAMPLE	NegRail, PosRail;
	Wave.GetSampleRails(NegRail, PosRail);
	double	rail = -double(NegRail);	// convert to real first to avoid overflow
	int	PeakHoldTime = m_TimerFrequency / 2;
	double	PeakDecayDelta = 2.0 / PeakHoldTime;
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		double	NormMaxSamp = m_MaxSamp[iChan] / rail;
		double	Level = CWaveProcess::SafeLinearToDecibels(NormMaxSamp);
		double	NormLevel = 1 - max(Level, -RANGE) / -RANGE;
		METER&	meter = m_Meter[iChan];
		meter.Level = NormLevel;	// update level
		if (meter.PeakHold > 0)	// if peak hold in progress
			meter.PeakHold--;	// decrement peak hold timer
		else	// peak hold finished
			meter.PeakLevel -= PeakDecayDelta;	// decrement peak level
		if (NormLevel >= meter.PeakLevel) {	// if level at or above peak level
			meter.PeakLevel = NormLevel;	// reset peak level
			meter.PeakHold = PeakHoldTime;	// reset peak hold timer
		}
	}
}

void CMeterView::TimerHook()
{
	int	chans = GetChannelCount();
	ASSERT(m_Wave != NULL);	// wave pointer is required
	ASSERT(UINT(chans) == m_Wave->GetChannels());	// channel count must match audio
	W64INT	frames = m_Wave->GetFrameCount();	// process all frames
	ZeroMemory(m_MaxSamp.GetData(), chans * sizeof(int));	// zero maximum samples
	ComputeLevels(*m_Wave, 0, frames, CW64IntRange(0, W64INT_MAX), FALSE);
	UpdatePeaks(*m_Wave);
	UpdateMeters();
}

/////////////////////////////////////////////////////////////////////////////
// CMeterView drawing

void CMeterView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	SetScrollSizes(MM_TEXT, CSize(0, 0));
}

void CMeterView::OnDraw(CDC* pDC)
{
	CRect	cb;
	pDC->GetClipBox(cb);
	pDC->BitBlt(cb.left, cb.top, cb.Width(), cb.Height(),	// blit view
		&m_DC, cb.left, cb.top, SRCCOPY);
	pDC->ExcludeClipRect(CRect(CPoint(0, 0), m_ViewSize));	// exclude view
	pDC->FillSolidRect(cb, GetSysColor(COLOR_3DFACE));	// erase background
}

/////////////////////////////////////////////////////////////////////////////
// CMeterView diagnostics

#ifdef _DEBUG
void CMeterView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CMeterView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMeterView message map

BEGIN_MESSAGE_MAP(CMeterView, CScrollView)
	//{{AFX_MSG_MAP(CMeterView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_MOUSEACTIVATE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMeterView message handlers

int CMeterView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_DC.Create(this))	// create double-buffered device context
		return -1;
	m_DC.SetBkMode(TRANSPARENT);
	m_DC.SetTextAlign(TA_CENTER | TA_TOP);
	LOGFONT	lf;
	ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = FONT_HEIGHT;
	if (!m_MeterFont.CreateFontIndirect(&lf))	// create meter font
		return -1;
	m_PrevFont = m_DC.SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	m_CaptionHeight = m_DC.GetTextExtent(_T("0")).cy;	// measure caption height
	m_CaptionHeight += CAPTION_BOTTOM_MARGIN;	// include bottom margin
	m_DC.SelectObject(m_MeterFont);	// select meter font
	
	return 0;
}

void CMeterView::OnDestroy() 
{
	CScrollView::OnDestroy();
	m_DC.SelectObject(m_PrevFont);	// restore previous font
}

void CMeterView::OnSize(UINT nType, int cx, int cy) 
{
	m_Ticks = CalcTickCount(cy);
	CreateBackBuffer(cx, cy);
	UpdateCaptions();
	UpdateMeters();
	CScrollView::OnSize(nType, cx, cy);	// order matters; call base class last
}

int CMeterView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// prevent activation, else context help stops working
	return(MA_NOACTIVATE);
}

void CMeterView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (GetKeyState(VK_MENU) & GKS_DOWN)	// if menu key down
		ResetAllClipping();	// reset all clipping indicators
	else {	// reset selected clipping indicator
		int	iChan = FindClipIndicator(point);
		if (iChan >= 0)	// if indicator was found
			ResetClipping(iChan);
	}
	CScrollView::OnLButtonDown(nFlags, point);
}
