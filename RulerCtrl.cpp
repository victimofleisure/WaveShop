// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version
        01      20mar13	in SetZoom, throw instead of returning false
        02      22mar13	variable minor tick count
        03      24mar13	merge text measurement into draw
		04		25mar13	add log unit and tick array

		ruler control

*/

// RulerCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "RulerCtrl.h"
#include <math.h>
#include "Range.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRulerCtrl

IMPLEMENT_DYNAMIC(CRulerCtrl, CWnd)

const CRulerCtrl::DIV_INFO CRulerCtrl::m_DivMetric[] = {	// base 10
//	major	minor
	{1,		2},	// 0, 1, 2, 3, ...
	{2,		5},	// 0, 5, 10, 15, ...
	{5,		4},	// 0, 2, 4, 6, ...
};
const CRulerCtrl::DIV_INFO CRulerCtrl::m_DivEnglish[] = {	// base 2
//	major	minor
	{1,		2}	// 0, 1, 2, 3, ...
};
const CRulerCtrl::DIV_INFO CRulerCtrl::m_DivTime[] = {	// base 60
//	major	minor
	{1,		2},	// 0, 1, 2, 3, ...
	{2,		3},	// 0, 30, 60, 90, ...
	{3,		4},	// 0, 20, 40, 60, ...
	{6,		2},	// 0, 10, 20, 30, ...
	{12,	5},	// 0, 5, 10, 15, ...
	{30,	4},	// 0, 2, 4, 6, ...
};

const CRulerCtrl::UNIT_INFO CRulerCtrl::m_UnitInfo[UNITS] = {
//	divisions array		number of divisions		base
	{m_DivMetric,		_countof(m_DivMetric),	10},	// UNIT_METRIC
	{m_DivEnglish,		_countof(m_DivEnglish),	2},		// UNIT_ENGLISH
	{m_DivTime,			_countof(m_DivTime),	60},	// UNIT_TIME
	{m_DivEnglish,		_countof(m_DivEnglish),	10},	// UNIT_LOG
};

const TCHAR CRulerCtrl::m_NumFmtCode[NUMERIC_FORMATS] = {'f', 'e', 'g'};


CRulerCtrl::CRulerCtrl()
{
	m_ScrollPos = 0;
	m_Zoom = 1;
	m_MajorTickStep = 0;
	m_MajorTickGap = 0;
	m_Font = NULL;
	m_Unit = UNIT_METRIC;
	m_MinorTicks = 0;
	m_MinMajorTickGap = 80;
	m_MinMinorTickGap = 16;
	m_TickLen[MAJOR] = 2;
	m_TickLen[MINOR] = 2;
	m_MarginStart = 0;
	m_MarginEnd = 0;
	SetNumericFormat(NF_GENERIC, 6);
}

CRulerCtrl::~CRulerCtrl()
{
}

BOOL CRulerCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	LPCTSTR	lpszClassName = AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW));
	if (!CWnd::Create(lpszClassName, NULL, dwStyle, rect, pParentWnd, nID))
		return FALSE;
	return TRUE;
}

void CRulerCtrl::SetUnit(int Unit)
{
	m_Unit = Unit;
	UpdateSpacing();
	Invalidate();
}

void CRulerCtrl::SetNumericFormat(int NumericFormat, int Precision)
{
	ASSERT(NumericFormat >= 0 && NumericFormat < NUMERIC_FORMATS);
	m_NumericFormat = NumericFormat;
	m_Precision = Precision;
	m_NumFormatStr.Format(_T("%%.*%c"), m_NumFmtCode[m_NumericFormat]);
	if (m_hWnd)	// allows us to be called from ctor
		Invalidate();
}

void CRulerCtrl::SetZoom(double Zoom)
{
	if (!Zoom)	// if zoom is zero
		AfxThrowNotSupportedException();	// invalid argument
//printf("CRulerCtrl::SetZoom %f\n", Zoom);
	m_Zoom = Zoom;
	UpdateSpacing();
	Invalidate();
}

void CRulerCtrl::SetZoom(int Pos, double Zoom)
{
	if (!Zoom)	// if zoom is zero
		AfxThrowNotSupportedException();	// invalid argument
//printf("CRulerCtrl::SetZoom %d %f\n", Pos, Zoom);
	double	r = (m_ScrollPos + Pos) * m_Zoom;
	m_Zoom = Zoom;
	UpdateSpacing();
	m_ScrollPos = r / Zoom - Pos;	// keep zoom positive
	Invalidate();
}

void CRulerCtrl::ScrollToPosition(double ScrollPos)
{
//printf("CRulerCtrl::ScrollToPosition %f\n", ScrollPos);
	UINT	style = GetStyle();
	if ((style & (HIDE_CLIPPED_VALS | ENFORCE_MARGINS)) || m_Unit == UNIT_LOG) {
		Invalidate();	// scrolling would cause artifacts; paint entire window
	} else {	// clipped values allowed
		double	ScrollDelta = m_ScrollPos - ScrollPos;
		double	ScrollSize = fabs(ScrollDelta);
		CRect	rc;
		GetClientRect(rc);
		CSize	sz;
		int	len;
		int	iScrollDelta = round(ScrollDelta);
		if (style & CBRS_ORIENT_HORZ) {	// if horizontal ruler
			sz = CSize(iScrollDelta, 0);
			len = rc.Width();
		} else {	// vertical ruler
			sz = CSize(0, iScrollDelta);
			len = rc.Height();
		}
		if (ScrollSize < len) {	// if any portion of window is still valid
			ScrollWindow(sz.cx, sz.cy);	// scroll valid portion of window
		} else	// entire window is invalid
			Invalidate();
	}
	m_ScrollPos = ScrollPos;
}

void CRulerCtrl::UpdateSpacing()
{
	ASSERT(m_Zoom);	// zoom must be non-zero
//printf("CRulerCtrl::UpdateSpacing zoom=%f unit=%d\n", m_Zoom, m_Unit);
	double	AbsZoom = fabs(m_Zoom);
	int	iUnit;
	// if unit is time, and zoom is less than a second
	if (m_Unit == UNIT_TIME && AbsZoom * m_MinMajorTickGap < 1)
		iUnit = UNIT_METRIC;	// override unit to metric for ticks
	else	// default to current unit
		iUnit = m_Unit;
	const UNIT_INFO&	UnitInfo = m_UnitInfo[iUnit];
	double	base = UnitInfo.Base;
	double	NearestExp = log(AbsZoom * m_MinMajorTickGap) / log(base);
	int	NearestIntExp = trunc(NearestExp);
	if (NearestExp >= 0)	// if nearest exponent is positive
		NearestIntExp++;	// chop it up; otherwise chop it down
	double	NearestPow = pow(base, NearestIntExp);
	double	CurGap = NearestPow / AbsZoom;
	double	InitGap = CurGap;
	int	divs = UnitInfo.NumDivs;
	const DIV_INFO	*pDiv = UnitInfo.Div;
	int	iDiv;
	for (iDiv = 0; iDiv < divs - 1; iDiv++) {	// for each division but last one
		double	NextGap = InitGap / pDiv[iDiv + 1].Major;	// compute next gap
		if (NextGap < m_MinMajorTickGap)	// if next gap is too small
			break;	// we're done
		CurGap = NextGap;
	}
	m_MajorTickGap = CurGap;
	int	MajorTicks = pDiv[iDiv].Major;
	m_MajorTickStep = NearestPow / MajorTicks;
	if (m_Zoom < 0)	// if negative zoom
		m_MajorTickStep = -m_MajorTickStep;	// negate major tick step
	// major tick spacing complete; now do minor ticks
	int	MinorTicks = pDiv[iDiv].Minor;	// start with nominal tick count
	if (iDiv + 1 < divs) {	// if not last division
		int	NextMinorTicks = UnitInfo.Div[iDiv + 1].Minor;
		if (NextMinorTicks > MinorTicks	// if next division yields more ticks 
		&& CurGap / NextMinorTicks >= m_MinMinorTickGap)	// and far enough apart
			MinorTicks = NextMinorTicks;	// use next division's tick count
	}
	if (CurGap / MinorTicks < m_MinMinorTickGap) {	// if ticks too close together
		int	HalfMinorTicks = MinorTicks >> 1;	// try half as many
		if (CurGap / HalfMinorTicks >= m_MinMinorTickGap)	// if far enough apart
			MinorTicks = HalfMinorTicks;	// use half as many
		else	// still too close together
			MinorTicks = 1;	// last resort: no minor ticks
	} else {	// ticks far enough apart
		int	DblMinorTicks = MinorTicks << 1;	// try twice as many
		if (CurGap / DblMinorTicks >= m_MinMinorTickGap)	// if far enough apart
			MinorTicks = DblMinorTicks;	// use twice as many
	}
	m_MinorTicks = MinorTicks;
//printf("exp=%g pow=%g gap=%f iDiv=%d majTs=%d minTs=%d step=%g\n", NearestExp, NearestPow, m_MajorTickGap, iDiv, MajorTicks, m_MinorTicks, m_MajorTickStep);
}

double CRulerCtrl::PositionToValue(double Pos) const
{
	return(round64((m_ScrollPos + Pos) * m_Zoom / m_MajorTickStep) * m_MajorTickStep);
}

inline double CRulerCtrl::Wrap(double Val, double Limit)
{
	double	r = fmod(Val, Limit);
	return(Val < 0 ? r + Limit : r);
}

int CRulerCtrl::TrimTrailingZeros(CString& Str)
{
	int	len = Str.GetLength();
	if (len) {
		LPCTSTR	pStr = Str.GetBuffer(0);
		int	iPos = len - 1;
		while (iPos >= 0 && pStr[iPos] == '0')
			iPos--;
		len = iPos + 1;
		Str.ReleaseBuffer(len);
	}
	return(len);
}

CString CRulerCtrl::FormatTime(double TimeSecs)
{
	const int TICKS_PER_SEC = 1000000;	// maximum tick precision
	LONGLONG	TimeTicks = round64(TimeSecs * TICKS_PER_SEC);
	bool	IsNeg;
	if (TimeTicks < 0) {
		IsNeg = TRUE;
		TimeTicks = -TimeTicks;
	} else
		IsNeg = FALSE;
	int	ticks = int(TimeTicks % TICKS_PER_SEC);
	TimeTicks /= TICKS_PER_SEC;
	int	seconds = int(TimeTicks % 60);
	TimeTicks /= 60;
	int	minutes = int(TimeTicks % 60);
	int	hours = int(TimeTicks / 60);
	CString	sResult, sTicks;
	sResult.Format(_T("%d:%02d:%02d"), hours, minutes, seconds);
	// remove trailing zeros from tick string
	sTicks.Format(_T("%06d"), ticks);	// must match TICKS_PER_SEC
	if (TrimTrailingZeros(sTicks)) {	// if digits remain
		sResult += '.';	// append decimal point
		sResult += sTicks;	// append tick string to result
	}
	if (IsNeg)	// if negative
		sResult.Insert(0, '-');	// insert sign
	return(sResult);
}

CString CRulerCtrl::FormatValue(double Val) const
{
	CString	s;
	switch (m_Unit) {
	case UNIT_TIME:
		s = FormatTime(Val);
		break;
	case UNIT_LOG:
		s.Format(m_NumFormatStr, m_Precision, Val);
		break;
	default:
		if (fabs(Val) < fabs(m_MajorTickStep) / 2)	// if too close to zero
			Val = 0;	// truncate to zero
		s.Format(m_NumFormatStr, m_Precision, Val);
	}
	return(s);
}

inline CSize CRulerCtrl::SizeMax(CSize a, CSize b)
{
	return(CSize(max(a.cx, b.cx), max(a.cy, b.cy)));
}

void CRulerCtrl::AddTick(CTickArray *TickArray, int Pos, bool IsMinor)
{
	TICK	tick = {Pos, IsMinor};
	TickArray->Add(tick);
}

CSize CRulerCtrl::CalcTextExtent(CTickArray *TickArray)
{
	CRect	rc;
	GetClientRect(rc);
	CClientDC	dc(this);
	return(Draw(dc, rc, rc, TRUE, TickArray));	// measure text
}

CSize CRulerCtrl::CalcTextExtent(CRect& rc, CTickArray *TickArray)
{
	CClientDC	dc(this);
	return(Draw(dc, rc, rc, TRUE, TickArray));	// measure text
}

int CRulerCtrl::CalcMinHeight()
{
	ASSERT(!IsVertical());	// works for horizontal rulers only
	CClientDC	dc(this);
	HGDIOBJ	PrevFont = dc.SelectObject(m_Font);
	CSize	ext(dc.GetTextExtent(_T("0")));
	dc.SelectObject(PrevFont);
	if (ext.cy)
		ext.cy += m_TickLen[MAJOR];	// font's leading provides tick/text gap
	return(ext.cy);
}

/////////////////////////////////////////////////////////////////////////////
// CRulerCtrl drawing

CSize CRulerCtrl::Draw(CDC& dc, const CRect& cb, const CRect& rc, bool MeasureText, CTickArray *TickArray)
{
	CSize	TotExt(0, 0);
	// if either loop delta input is zero and unit is linear
	if ((!m_MajorTickGap || !m_MinorTicks) && m_Unit != UNIT_LOG)
		return(TotExt);	// avoid infinite loop
	HGDIOBJ	PrevFont = dc.SelectObject(m_Font);	// restore font before exiting
	COLORREF	ReticleColor = dc.GetTextColor();
	int	MinorTicks;
	if (MeasureText) {	// if measuring text
		if (TickArray != NULL) {	// if returning ticks
			TickArray->RemoveAll();	// empty tick array
			MinorTicks = m_MinorTicks;	// iterate minor ticks
		} else	// not returning ticks
			MinorTicks = 1;	// don't waste time iterating minor ticks
	} else	// not measuring text
		MinorTicks = m_MinorTicks;	// iterate minor ticks
	UINT	style = GetStyle();
	if (style & CBRS_ORIENT_HORZ) {	// if horizontal ruler
		int	TextY, TextAlign;
		int	TickY[TICK_TYPES];
		if (style & CBRS_ALIGN_TOP) {	// if docked top
			TextY = rc.bottom - m_TickLen[MAJOR];
			TextAlign = TA_CENTER | TA_BOTTOM;
			for (int iTickType = 0; iTickType < TICK_TYPES; iTickType++)
				TickY[iTickType] = rc.bottom - m_TickLen[iTickType];
		} else {	// docked bottom
			TextY = m_TickLen[MAJOR];
			TextAlign = TA_CENTER | TA_TOP;
			for (int iTickType = 0; iTickType < TICK_TYPES; iTickType++)
				TickY[iTickType] = 0;
		}
		dc.SetTextAlign(TextAlign);
		CRange<int>	span(rc.left, rc.right);
		if (style & ENFORCE_MARGINS)	// if enforcing margins
			span += CRange<int>(m_MarginStart, -m_MarginEnd);	// deduct margins
		if (m_Unit == UNIT_LOG) {	// if log unit
			int	PrevTextX2 = 0;
			CString	sTickVal;
			bool	reverse = m_Zoom < 0;
			double	ScrollPos;
			if (reverse)
				ScrollPos = -m_ScrollPos - rc.right;
			else
				ScrollPos = m_ScrollPos;
			double	Zoom = fabs(m_Zoom) / LOG_UNIT_SCALE;
			int iExp = round(floor(ScrollPos * Zoom));
//printf("iExp=%d\n", iExp);
			while (1) {
				double	CurPow = pow(LOG_UNIT_BASE, double(iExp));
				double	NextPow = CurPow * LOG_UNIT_BASE;
				double	NextPowX = log10(NextPow) / Zoom - ScrollPos;
				sTickVal = FormatValue(NextPow);
				CSize	ext(dc.GetTextExtent(sTickVal));
				int	NextPowTextX1 = round(NextPowX) - (ext.cx >> 1);
				for (int iDiv = 0; iDiv < LOG_UNIT_BASE - 1; iDiv++) {
					double	n = CurPow + iDiv * CurPow;
					double	rx = log10(n) / Zoom - ScrollPos;
					sTickVal = FormatValue(n);
					CSize	ext(dc.GetTextExtent(sTickVal));
					int	HalfTextWidth = ext.cx >> 1;
					int	x = round(rx);
					int	x1 = x - HalfTextWidth;
					int	x2 = x + HalfTextWidth;
					if (x1 >= rc.right)
						goto HorzLogDone;
					if (style & HIDE_CLIPPED_VALS) {	// if hiding clipped values
						if (x1 < span.Start || x2 > span.End)
							continue;
					}
					if (reverse)	// if reversing
						x = rc.right - x;	// flip x-axis
					bool	IsMinor;
					// if label fits between previous label and label at next power
					if ((!PrevTextX2 || x1 - PrevTextX2 >= m_MinMajorTickGap)
					&& (!iDiv || NextPowTextX1 - x2 >= m_MinMajorTickGap)) {
						PrevTextX2 = x2;
						if (MeasureText)	// if measuring text
							TotExt = SizeMax(ext, TotExt);	// update total size
						else	// drawing
							dc.TextOut(x, TextY, sTickVal);
						IsMinor = FALSE;
					} else
						IsMinor = TRUE;
					if (MeasureText) {	// if measuring text
						if (TickArray != NULL)	// if returning ticks
							AddTick(TickArray, x, IsMinor);	// add tick to array
					} else	// drawing
						dc.FillSolidRect(x, TickY[IsMinor], 1, m_TickLen[IsMinor], ReticleColor);
				}
				iExp++;
			}
HorzLogDone:
			;
		} else {	// linear scale
			double	xmod = Wrap(cb.left + m_ScrollPos, m_MajorTickGap);
			double	x1 = cb.left - xmod;
			double	x2 = cb.right + m_MajorTickGap;
			double	n = PositionToValue(x1);
//printf("xmod=%f x1=%f x2=%f n=%f\n", xmod, x1, x2, n);
			CString	sTickVal;
			int	iTick = 0;
			double	dx = m_MajorTickGap / MinorTicks;
			ASSERT(dx);	// else infinite loop ensues
			for (double rx = x1; rx < x2; rx += dx) {
				bool	IsMinor = iTick++ % MinorTicks != 0;
				int	x = round(rx);
				if (style & ENFORCE_MARGINS) {	// if enforcing margins
					if (x < span.Start || x >= span.End) {	// if tick outside margins
						if (!IsMinor)	// if major tick
							n += m_MajorTickStep;	// increment value
						continue;	// skip tick
					}
				}
				if (!IsMinor) {	// if major tick
					sTickVal = FormatValue(n);
					n += m_MajorTickStep;	// increment value
					CSize	ext(0, 0);
					if (style & HIDE_CLIPPED_VALS) {	// if hiding clipped values
						ext = dc.GetTextExtent(sTickVal);
						int	TextHCenter = ext.cx >> 1;
						if (x - TextHCenter < span.Start || x + TextHCenter > span.End)
							continue;	// value would clip, so skip it
					}
					if (MeasureText) {	// if measuring text instead of drawing
						if (!ext.cx)	// if text wasn't measured above
							ext = dc.GetTextExtent(sTickVal);	// measure text
						TotExt = SizeMax(ext, TotExt);	// update total size
					} else	// drawing
						dc.TextOut(x, TextY, sTickVal);	// draw text
				}
				if (MeasureText) {	// if measuring text
					if (TickArray != NULL)	// if returning ticks
						AddTick(TickArray, x, IsMinor);	// add tick to array
				} else	// drawing
					dc.FillSolidRect(x, TickY[IsMinor], 1, m_TickLen[IsMinor], ReticleColor);
			}
		}
		if (TotExt.cy)	// if total text height non-zero
			TotExt.cy += m_TickLen[MAJOR];	// font's leading provides tick/text gap
	} else {	// vertical ruler
		CSize	TextExt = dc.GetTextExtent(_T("0"));
		int	TextVCenter = TextExt.cy >> 1;
		int	TextX, TextAlign;
		int	TickX[TICK_TYPES];
		if (style & CBRS_ALIGN_LEFT) {	// if docked left
			TextX = rc.right - m_TickLen[MAJOR] - TICK_TEXT_GAP;
			TextAlign = TA_RIGHT | TA_TOP;
			for (int iTickType = 0; iTickType < TICK_TYPES; iTickType++)
				TickX[iTickType] = rc.right - m_TickLen[iTickType];
		} else {	// docked right
			TextX = m_TickLen[MAJOR] + TICK_TEXT_GAP;
			TextAlign = TA_LEFT | TA_TOP;
			for (int iTickType = 0; iTickType < TICK_TYPES; iTickType++)
				TickX[iTickType] = 0;
		}
		dc.SetTextAlign(TextAlign);
		CRange<int>	span(rc.top, rc.bottom);
		if (style & ENFORCE_MARGINS)	// if enforcing margins
			span += CRange<int>(m_MarginStart, -m_MarginEnd);	// deduct margins
		if (m_Unit == UNIT_LOG) {	// if log unit
			int	PrevTextY2 = 0;
			CString	sTickVal;
			bool	reverse = m_Zoom < 0;
			double	ScrollPos;
			if (reverse)
				ScrollPos = -m_ScrollPos - rc.bottom;
			else
				ScrollPos = m_ScrollPos;
			double	Zoom = fabs(m_Zoom) / LOG_UNIT_SCALE;
			int iExp = round(floor(ScrollPos * Zoom));
//printf("iExp=%d\n", iExp);
			while (1) {
				double	CurPow = pow(LOG_UNIT_BASE, double(iExp));
				double	NextPow = CurPow * LOG_UNIT_BASE;
				double	NextPowY = log10(NextPow) / Zoom - ScrollPos;
				sTickVal = FormatValue(NextPow);
				CSize	ext(dc.GetTextExtent(sTickVal));
				int	NextPowTextY1 = round(NextPowY) - (ext.cy >> 1);
				for (int iDiv = 0; iDiv < LOG_UNIT_BASE - 1; iDiv++) {
					double	n = CurPow + iDiv * CurPow;
					double	ry = log10(n) / Zoom - ScrollPos;
					sTickVal = FormatValue(n);
					CSize	ext(dc.GetTextExtent(sTickVal));
					int	HalfTextHeight = ext.cy >> 1;
					int	y = round(ry);
					int	y1 = y - HalfTextHeight;
					int	y2 = y + HalfTextHeight;
					if (y1 >= rc.bottom)
						goto VertLogDone;
					if (style & HIDE_CLIPPED_VALS) {	// if hiding clipped values
						if (y1 < span.Start || y2 > span.End)
							continue;
					}
					if (reverse)	// if reversing
						y = rc.bottom - y;	// flip y-axis
					bool	IsMinor;
					// if label fits between previous label and label at next power
					if ((!PrevTextY2 || y1 - PrevTextY2 >= m_MinMajorTickGap)
					&& (!iDiv || NextPowTextY1 - y2 >= m_MinMajorTickGap)) {
						PrevTextY2 = y2;
						if (MeasureText)	// if measuring text
							TotExt = SizeMax(ext, TotExt);	// update total size
						else	// drawing
							dc.TextOut(TextX, y - TextVCenter, sTickVal);	// draw text
						IsMinor = FALSE;
					} else
						IsMinor = TRUE;
					if (MeasureText) {	// if measuring text
						if (TickArray != NULL)	// if returning ticks
							AddTick(TickArray, y, IsMinor);	// add tick to array
					} else	// drawing
						dc.FillSolidRect(TickX[IsMinor], y, m_TickLen[IsMinor], 1, ReticleColor);
				}
				iExp++;
			}
VertLogDone:
			;
		} else {	// linear scale
			double	ymod = Wrap(cb.top + m_ScrollPos, m_MajorTickGap);
			double	y1 = cb.top - ymod;
			double	y2 = cb.bottom + m_MajorTickGap;
			double	n = PositionToValue(y1);
//printf("ymod=%f y1=%f y2=%f n=%f\n", ymod, y1, y2, n);
			CString	sTickVal;
			int	iTick = 0;
			double	dy = m_MajorTickGap / MinorTicks;
			ASSERT(dy);	// else infinite loop ensues
			for (double ry = y1; ry < y2; ry += dy) {
				bool	IsMinor = iTick++ % MinorTicks != 0;
				int	y = round(ry);
				if (style & ENFORCE_MARGINS) {	// if enforcing margins
					if (y < span.Start || y >= span.End) {	// if tick outside margins
						if (!IsMinor)	// if major tick
							n += m_MajorTickStep;	// increment value
						continue;	// skip tick
					}
				}
				if (!IsMinor) {	// if major tick
					sTickVal = FormatValue(n);
					n += m_MajorTickStep;	// increment value
					if (style & HIDE_CLIPPED_VALS) {	// if hiding clipped values
						if (y - TextVCenter < span.Start || y + TextVCenter > span.End)
							continue;	// value would clip, so skip it
					}
					if (MeasureText) {	// if measuring text instead of drawing
						CSize	ext(dc.GetTextExtent(sTickVal));	// measure text
						TotExt = SizeMax(ext, TotExt);	// update total size
					} else	// drawing
						dc.TextOut(TextX, y - TextVCenter, sTickVal);	// draw text
				}
				if (MeasureText) {	// if measuring text
					if (TickArray != NULL)	// if returning ticks
						AddTick(TickArray, y, IsMinor);	// add tick to array
				} else	// drawing
					dc.FillSolidRect(TickX[IsMinor], y, m_TickLen[IsMinor], 1, ReticleColor);
			}
		}
		if (TotExt.cx)	// if total text width non-zero
			TotExt.cx += m_TickLen[MAJOR] + TICK_TEXT_GAP;
	}
	dc.SelectObject(PrevFont);	// reselect previous font
	return(TotExt);
}

void CRulerCtrl::OnDraw(CDC& dc)
{
	CRect	cb;
	dc.GetClipBox(cb);
//printf("CRulerCtrl::OnPaint %d %d %d %d\n", cb.left, cb.top, cb.Width(), cb.Height());
	HBRUSH	hBkBrush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORSTATIC,
		WPARAM(dc.m_hDC), LPARAM(m_hWnd));	// get background brush from parent
	FillRect(dc.m_hDC, cb, hBkBrush);	// fill with background brush
	dc.SetBkMode(TRANSPARENT);
	CRect	rc;
	GetClientRect(rc);
	Draw(dc, cb, rc, FALSE, NULL);	// not measuring text
}

/////////////////////////////////////////////////////////////////////////////
// CRulerCtrl message map

BEGIN_MESSAGE_MAP(CRulerCtrl, CWnd)
	//{{AFX_MSG_MAP(CRulerCtrl)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRulerCtrl message handlers

LRESULT CRulerCtrl::OnGetFont(WPARAM wParam, LPARAM lParam)
{
	 return (LRESULT)m_Font;
}

LRESULT CRulerCtrl::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	m_Font = (HFONT)wParam;
	if (lParam)
		Invalidate();
	return 0;
}

void CRulerCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	// if we're aligned top or left, ticks are aligned to bottom or right edge
	// and move with it, so entire window must be repainted to avoid artifacts;
	// entire window must also be painted if hiding clipped values or enforcing
	// margins, because in either case window size affects which items are drawn
	DWORD	NEED_REPAINT = CBRS_ALIGN_TOP | CBRS_ALIGN_LEFT 
		| HIDE_CLIPPED_VALS | ENFORCE_MARGINS;
	if (GetStyle() & NEED_REPAINT)	// if repaint needed
		Invalidate();	// repaint entire window
}

void CRulerCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	OnDraw(dc);
}
