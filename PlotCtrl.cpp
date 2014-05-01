// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08dec12	initial version
		01		20mar13	in UpdateRulers, avoid divide by zero
        02      23mar13	add minimum minor tick gap
        03      24mar13	add baseline attribute to series
        04      25mar13	add tick arrays
        05      26mar13	add log scale
        06      28mar13	add data tip
        07      31mar13	add safe log
        08      08apr13	in UpdatePlot, remove unused var

		plot control

*/

// PlotCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PlotCtrl.h"
#include <math.h>
#include <float.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlotCtrl

IMPLEMENT_DYNAMIC(CPlotCtrl, CWnd)

const int CPlotCtrl::m_AxisRuler[2][2] = {
	{RULER_TOP, RULER_BOTTOM},	// AXIS_HORZ
	{RULER_LEFT, RULER_RIGHT},	// AXIS_VERT
};

const COLORREF CPlotCtrl::m_DefaultPalette[DEFAULT_PALETTE_SIZE] = {
	RGB(0x99, 0x99, 0xFF),	RGB(0x99, 0x33, 0x66),	RGB(0xFF, 0xFF, 0xCC),	RGB(0xCC, 0xFF, 0xFF),
	RGB(0x66, 0x00, 0x66),	RGB(0xFF, 0x80, 0x80),	RGB(0x00, 0x66, 0xCC),	RGB(0xCC, 0xCC, 0xFF),
	RGB(0x00, 0x00, 0x80),	RGB(0xFF, 0x00, 0xFF),	RGB(0xFF, 0xFF, 0x00),	RGB(0x00, 0xFF, 0xFF),
	RGB(0x80, 0x00, 0x80),	RGB(0x80, 0x00, 0x00),	RGB(0x00, 0x80, 0x80),	RGB(0x00, 0x00, 0xFF),
	RGB(0x00, 0xCC, 0xFF),	RGB(0xCC, 0xFF, 0xFF),	RGB(0xCC, 0xFF, 0xCC),	RGB(0xFF, 0xFF, 0x99),
	RGB(0x99, 0xCC, 0xFF),	RGB(0xFF, 0x99, 0xCC),	RGB(0xCC, 0x99, 0xFF),	RGB(0xFF, 0xCC, 0x99),
	RGB(0x33, 0x66, 0xFF),	RGB(0x33, 0xCC, 0xCC),	RGB(0x99, 0xCC, 0x00),	RGB(0xFF, 0xCC, 0x00),
	RGB(0xFF, 0x99, 0x00),	RGB(0xFF, 0x66, 0x00),	RGB(0x66, 0x66, 0x99),	RGB(0x96, 0x96, 0x96),
};

CPlotCtrl::CPlotCtrl()
{
	m_VisibleRulers = 0;
	m_FitToData = RM_ALL;
	m_Font = NULL;
	m_WndSize = CSize(0, 0);
	m_PlotRect = CRect(0, 0, 0, 0);
	m_Color[CLR_BKGD] = GetSysColor(COLOR_3DLIGHT);
	m_Color[CLR_PLOT_BKGD] = GetSysColor(COLOR_WINDOW);
	m_Color[CLR_GRID] = RGB(192, 192, 192);
	m_Color[CLR_TEXT] = GetSysColor(COLOR_BTNTEXT);
	m_Color[CLR_BORDER] = GetSysColor(COLOR_BTNTEXT);
	m_Margins = CRect(0, 0, 0, 0);
	m_TipPrecision = 6;
}

CPlotCtrl::~CPlotCtrl()
{
}

CPlotCtrl::CRulerInfo::CRulerInfo()
{
	m_Range = CDblRange(0, 1);
}

CPlotCtrl::CMyDataTipCtrl::CMyDataTipCtrl()
{
	m_PlotCtrl = NULL;
}

bool CPlotCtrl::CMyDataTipCtrl::FindPoint(CPoint Target, CPoint& DataPos) const
{
	ASSERT(m_PlotCtrl != NULL);
	return(m_PlotCtrl->FindPoint(Target, DataPos));
}

BOOL CPlotCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	LPCTSTR	lpszClassName = AfxRegisterWndClass(
		CS_HREDRAW | CS_VREDRAW, 
		LoadCursor(NULL, IDC_ARROW));
	if (!CWnd::Create(lpszClassName, NULL, dwStyle, rect, pParentWnd, nID))
		return FALSE;
	return TRUE;
}

CPlotCtrl::CSeries::CSeries()
{
	m_Baseline = 0;
	m_Flags = SER_LINE | SER_FILL_BASELINE;
	m_PenWidth = 1; 
	m_PenStyle = PS_SOLID;
	m_PenColor = CLR_INVALID;
	m_BrushHatch = -1;
	m_BrushColor = CLR_INVALID;
	m_BkColor = 0;
	m_MarkerStyle = MARK_SQUARE;
	m_MarkerSize = 8;
}

CPlotCtrl::CSeries& CPlotCtrl::CSeries::operator=(const CSeries& Series)
{
	if (&Series != this) {	// avoid self-assignment
		m_Name			= Series.m_Name;
		m_Point			= Series.m_Point;
		m_Baseline		= Series.m_Baseline;
		m_Flags			= Series.m_Flags;
		m_PenWidth		= Series.m_PenWidth;
		m_PenStyle		= Series.m_PenStyle;
		m_PenColor		= Series.m_PenColor;
		m_BrushHatch	= Series.m_BrushHatch;
		m_BrushColor	= Series.m_BrushColor;
		m_BkColor		= Series.m_BkColor;
		m_MarkerStyle	= Series.m_MarkerStyle;
		m_MarkerSize	= Series.m_MarkerSize;
	}
	return(*this);
}

void CPlotCtrl::InsertSeries(int SerIdx, CSeries& Series)
{
	m_Series.InsertAt(SerIdx, Series);
}

void CPlotCtrl::RemoveSeries(int SerIdx)
{
	m_Series.RemoveAt(SerIdx);
}

void CPlotCtrl::RemoveAllSeries()
{
	m_Series.RemoveAll();
}

void CPlotCtrl::DrawMarker(CDC& dc, POINT Point, int Style, int Size)
{
	if (Size > 1) {
		int	rad = Size >> 1;
		CRect	r(CPoint(Point) - CSize(rad, rad), CSize(Size, Size));
		switch (Style) {
		case MARK_SQUARE:
			dc.Rectangle(r);
			break;
		case MARK_CIRCLE:
			dc.Ellipse(r);
			break;
		case MARK_DIAMOND:
			{
				POINT	pt[] = {
					{Point.x,	r.top},
					{r.right,	Point.y},
					{Point.x,	r.bottom},
					{r.left,	Point.y}
				};
				dc.Polygon(pt, _countof(pt));
			}
			break;
		case MARK_TRIANGLE:
			{
				POINT	pt[] = {
					{Point.x,	r.top},
					{r.right,	r.bottom},
					{r.left,	r.bottom}
				};
				dc.Polygon(pt, _countof(pt));
			}
			break;
		}
	} else
		dc.SetPixelV(Point, dc.GetTextColor());
}

void CPlotCtrl::PolyMarker(CDC& dc, const POINT *Point, int Points, int Style, int Size)
{
	for (int iPt = 0; iPt < Points; iPt++)
		DrawMarker(dc, Point[iPt], Style, Size);
}

COLORREF CPlotCtrl::GetDefaultPenColor(int SeriesIdx)
{
	return(m_DefaultPalette[(SeriesIdx + 8) % DEFAULT_PALETTE_SIZE]);
}

COLORREF CPlotCtrl::GetDefaultBrushColor(int SeriesIdx)
{
	return(m_DefaultPalette[SeriesIdx % DEFAULT_PALETTE_SIZE]);
}

int CPlotCtrl::GetAxisRuler(int AxisIdx) const
{
	int	iRuler;
	if (AxisIdx == AXIS_HORZ) {	// if horizontal axis
		if (IsRulerVisible(RULER_TOP))	// if showing top ruler
			iRuler = RULER_TOP;
		else
			iRuler = RULER_BOTTOM;
	} else {	// vertical axis
		if (IsRulerVisible(RULER_RIGHT))	// if showing right ruler
			iRuler = RULER_RIGHT;
		else
			iRuler = RULER_LEFT;
	}
	return(iRuler);
}

inline double CPlotCtrl::Wrap(double Val, double Limit)
{
	double	r = fmod(Val, Limit);
	return(Val < 0 ? r + Limit : r);
}

bool CPlotCtrl::IsRulerVisible(int RulerIdx) const
{
	return((m_VisibleRulers & GetRulerMask(RulerIdx)) != 0);
}

inline void CPlotCtrl::Twiddle(UINT& Data, UINT BitMask, bool SetBits)
{
	if (SetBits)
		Data |= BitMask;
	else
		Data &= ~BitMask;
}

CPoint CPlotCtrl::GetLogScale() const
{
	CPoint	pt;
	pt.x = GetRuler(m_AxisRuler[AXIS_HORZ][0]).GetUnit() == CRulerCtrl::UNIT_LOG
		|| GetRuler(m_AxisRuler[AXIS_HORZ][1]).GetUnit() == CRulerCtrl::UNIT_LOG;
	pt.y = GetRuler(m_AxisRuler[AXIS_VERT][0]).GetUnit() == CRulerCtrl::UNIT_LOG
		|| GetRuler(m_AxisRuler[AXIS_VERT][1]).GetUnit() == CRulerCtrl::UNIT_LOG;
	return(pt);
}

inline double CPlotCtrl::SafeLog10(double x)
{
	return(x > 0 ? log10(x) : 0);
}

inline void CPlotCtrl::ApplyLogScale(CPoint IsLogScale, DPoint& Pt)
{
	if (IsLogScale.x)
		Pt.x = SafeLog10(Pt.x);
	if (IsLogScale.y)
		Pt.y = SafeLog10(Pt.y);
}

void CPlotCtrl::ShowRuler(int RulerIdx, bool Enable)
{
	if (m_hWnd != NULL)	// allow pre-create usage
		GetRuler(RulerIdx).ShowWindow(Enable ? SW_SHOW : SW_HIDE);
	Twiddle(m_VisibleRulers, GetRulerMask(RulerIdx), Enable);
}

void CPlotCtrl::SetVisibleRulers(UINT RulerMask)
{
	for (int iRuler = 0; iRuler < RULERS; iRuler++)
		ShowRuler(iRuler, (RulerMask & GetRulerMask(iRuler)) != 0);
}

void CPlotCtrl::SetRange(int RulerIdx, const CDblRange& Range)
{
	ASSERT(IsValidRuler(RulerIdx));
	CDblRange	r(Range);
	if (GetRuler(RulerIdx).GetUnit() == CRulerCtrl::UNIT_LOG) {
		r.Start = SafeLog10(r.Start);
		r.End = SafeLog10(r.End);
	}
	m_RulerInfo[RulerIdx].m_Range = r;
}

bool CPlotCtrl::FitToData()
{
	int	nSeries = GetSeriesCount();
	if (!nSeries)
		return(FALSE);
	CDblRange	RangeX(DBL_MAX, -DBL_MAX), RangeY(DBL_MAX, -DBL_MAX);
	CPoint	LogScale = GetLogScale();
	for (int iSeries = 0; iSeries < nSeries; iSeries++) {	// for each series
		const CSeries& ser = m_Series[iSeries];
		int	pts = ser.m_Point.GetSize();
		for (int iPt = 0; iPt < pts; iPt++) {	// for each point
			DPoint	pt(ser.m_Point[iPt]);
			ApplyLogScale(LogScale, pt);
			if (pt.x < RangeX.Start)
				RangeX.Start = pt.x;
			if (pt.x > RangeX.End)
				RangeX.End = pt.x;
			if (pt.y < RangeY.Start)
				RangeY.Start = pt.y;
			if (pt.y > RangeY.End)
				RangeY.End = pt.y;
		}
	}
	if (RangeX.Start == DBL_MAX)	// if range unchanged
		return(FALSE);	// assume all series were empty
	if (!RangeX.IsEmpty()) {	// if horizontal range non-empty
		if (m_FitToData & RM_TOP)
			m_RulerInfo[RULER_TOP].m_Range = RangeX;
		if (m_FitToData & RM_BOTTOM)
			m_RulerInfo[RULER_BOTTOM].m_Range = RangeX;
	}
	if (!RangeY.IsEmpty()) {	// if vertical range non-empty
		if (m_FitToData & RM_LEFT)
			m_RulerInfo[RULER_LEFT].m_Range = RangeY;
		if (m_FitToData & RM_RIGHT)
			m_RulerInfo[RULER_RIGHT].m_Range = RangeY;
	}
	return(TRUE);
}

void CPlotCtrl::SetTickGaps(CRulerCtrl& Ruler)
{
	int	MinMajorTickGap;
	if (Ruler.GetUnit() == CRulerCtrl::UNIT_LOG)	// if log scale
		MinMajorTickGap = LOG_MIN_MAJOR_TICK_GAP;
	else {	// linear scale
		if (Ruler.IsVertical())	// if vertical
			MinMajorTickGap = VERT_MIN_MAJOR_TICK_GAP;
		else	// horizontal
			MinMajorTickGap = HORZ_MIN_MAJOR_TICK_GAP;
	}
	Ruler.SetMinMajorTickGap(MinMajorTickGap);
	Ruler.SetMinMinorTickGap(MIN_MINOR_TICK_GAP);
}

void CPlotCtrl::UpdateRulers()
{
	if (m_FitToData)	// if fitting one or more rulers to data
		FitToData();
	CRect	rc(CPoint(0, 0), m_WndSize);
	CRect	rPlot(rc);	// start with client rect
	UINT	VisibleMask = GetVisibleRulers();
//printf("UpdateRulers %x\n", VisibleMask);
	if (VisibleMask & RM_TOP)
		rPlot.top += GetRuler(RULER_TOP).CalcMinHeight();
	if (VisibleMask & RM_BOTTOM)
		rPlot.bottom -= GetRuler(RULER_BOTTOM).CalcMinHeight();
	rPlot.top += m_Margins.top;
	rPlot.bottom -= m_Margins.bottom;
	const int	NominalWidth = 16;
	for (int iVertRuler = 0; iVertRuler < 2; iVertRuler++) {
		int	iRuler = m_AxisRuler[AXIS_VERT][iVertRuler];	// get index of vertical ruler
		if (VisibleMask & GetRulerMask(iRuler)) {	// if ruler is visible
			CRulerCtrl&	ruler = GetRuler(iRuler);
			SetTickGaps(ruler);
			CIntRange	margins(rPlot.top, m_WndSize.cy - rPlot.bottom);
			ruler.SetMargins(margins.Start, margins.End);
			const CRulerInfo&	info = m_RulerInfo[iRuler];
			double	zoom = -info.m_Range.Length() / (rPlot.Size().cy - 1);	// flip y-axis
			if (zoom) {	// avoid divide by zero
				double	origin = info.m_Range.End / zoom - rPlot.top;
				ruler.SetScrollPosition(origin);
				ruler.SetZoom(zoom);
			}
			CRect	r(0, 0, NominalWidth, m_WndSize.cy);
			CSize	ext(ruler.CalcTextExtent(r, &m_TickArray[AXIS_VERT]));
			if (iRuler == RULER_LEFT) {	// if left ruler
				rPlot.left += ext.cx;
				ext.cx += m_Margins.left;
				r = CRect(0, 0, ext.cx, m_WndSize.cy);
			} else {	// right ruler
				rPlot.right -= ext.cx;
				ext.cx += m_Margins.right;
				r = CRect(m_WndSize.cx - ext.cx, 0, m_WndSize.cx, m_WndSize.cy);
			}
			ruler.MoveWindow(r, FALSE);	// no repaint
		}
	}
	rPlot.left += m_Margins.left;
	rPlot.right -= m_Margins.right;
	m_PlotRect = rPlot;	// store plot rectangle
//printf("UpdateRulers rPlot %d %d %d %d\n", rPlot.left, rPlot.top, rPlot.right, rPlot.bottom);
	for (int iHorzRuler = 0; iHorzRuler < 2; iHorzRuler++) {
		int	iRuler = m_AxisRuler[AXIS_HORZ][iHorzRuler];	// get index of horizontal ruler
		if (VisibleMask & GetRulerMask(iRuler)) {	// if ruler is visible
			CRulerCtrl&	ruler = GetRuler(iRuler);
			SetTickGaps(ruler);
			CIntRange	margins(rPlot.left, m_WndSize.cx - rPlot.right);
			ruler.SetMargins(margins.Start, margins.End);
			const CRulerInfo&	info = m_RulerInfo[iRuler];
			double	zoom = info.m_Range.Length() / (rPlot.Size().cx - 1);
			if (zoom) {	// avoid divide by zero
				double	origin = info.m_Range.Start / zoom - rPlot.left;
				ruler.SetScrollPosition(origin);
				ruler.SetZoom(zoom);
			}
			CRect	r;
			if (iRuler == RULER_TOP)	// if top ruler
				r = CRect(0, 0, m_WndSize.cx, rPlot.top);
			else	// bottom ruler
				r = CRect(0, rPlot.bottom, m_WndSize.cx, m_WndSize.cy);
			ruler.MoveWindow(r, FALSE);	// no repaint
			ruler.CalcTextExtent(r, &m_TickArray[AXIS_HORZ]);
		}
	}
}

bool CPlotCtrl::FindPoint(CPoint Target, CPoint& DataPos) const
{
	CPoint	LogScale = GetLogScale();
	int	nSeries = GetSeriesCount();
	DPoint	PrevPt;
	for (int iSeries = 0; iSeries < nSeries; iSeries++) {	// for each series
		const CSeries& ser = m_Series[iSeries];
		if (ser.m_Flags) {	// if series is visible
			int	points = ser.m_Point.GetSize();
			CDblRange	RangeX, RangeY;
			GetRange(GetAxisRuler(AXIS_HORZ), RangeX);
			GetRange(GetAxisRuler(AXIS_VERT), RangeY);
			CSize	PlotSize = m_PlotRect.Size() - CSize(1, 1);
			DPoint	RangeLen(RangeX.Length(), RangeY.Length());
			for (int iPoint = 0; iPoint < points; iPoint++) {	// for each point
				DPoint	pt(ser.m_Point[iPoint]);
				ApplyLogScale(LogScale, pt);
				double	x = (pt.x - RangeX.Start) / RangeLen.x;
				double	y = (pt.y - RangeY.Start) / RangeLen.y;
				DPoint	CurPt;
				CurPt.x = m_PlotRect.left + x * PlotSize.cx;
				CurPt.y = m_PlotRect.bottom - y * PlotSize.cy;	// flip y-axis
				CPoint	iCurPt(CurPt);
				// if target point near data point
				if (m_Tip.PointsNear(Target, iCurPt)) {
					DataPos = CPoint(iSeries, iPoint);
					return(TRUE);	// target point found
				}
				if (iPoint) {	// if not first data point
					// create bounding rectangle defined by current and 
					// previous data points in diagonally opposite corners
					CRect	r(iCurPt.x, iCurPt.y, round(PrevPt.x), round(PrevPt.y));
					r.NormalizeRect();	// normalize bounds and account for epsilon
					r.InflateRect(CSize(m_Tip.m_Epsilon, m_Tip.m_Epsilon));
					if (r.PtInRect(Target)) {	// if target point within bounds
						// calculate distance from target point to line
						// intersecting current and previous data points
						double	dist = m_Tip.DistancePointToLine(Target, CurPt, PrevPt);
						// if target point near line between pair of data points
						if (dist <= m_Tip.m_Epsilon) {
							DataPos = CPoint(iSeries, iPoint);
							return(TRUE);	// target point found
						}
					}
				}
				PrevPt = CurPt;	// update previous data point
			}
		}
	}
	return(FALSE);	// target point not found
}

/////////////////////////////////////////////////////////////////////////////
// CPlotCtrl drawing

void CPlotCtrl::UpdatePlot()
{
	CRect	rPlot(m_PlotRect);
	m_DC.FillSolidRect(rPlot, m_Color[CLR_PLOT_BKGD]);
	UINT	VisibleMask = GetVisibleRulers();
	DWORD	style = GetStyle();
	// draw horizontal grid lines if desired
	if ((style & (HORZ_MAJOR_GRID | HORZ_MINOR_GRID))	// if horizontal grid lines
	&& (VisibleMask & RM_HORZ)) {	// and showing horizontal ruler
		int	ticks = m_TickArray[AXIS_HORZ].GetSize();
		for (int iTick = 0; iTick < ticks; iTick++) {
			const CRulerCtrl::TICK&	tick = m_TickArray[AXIS_HORZ][iTick];
			if ((!tick.IsMinor && (style & HORZ_MAJOR_GRID))
			|| (tick.IsMinor && (style & HORZ_MINOR_GRID)))
				m_DC.FillSolidRect(tick.Pos, rPlot.top, 1, rPlot.Height(), m_Color[CLR_GRID]);
		}
	}
	// draw vertical grid lines if desired
	if ((style & (VERT_MAJOR_GRID | VERT_MINOR_GRID))	// if vertical grid lines
	&& (VisibleMask & RM_VERT)) {	// and showing vertical ruler
		int	ticks = m_TickArray[AXIS_VERT].GetSize();
		for (int iTick = 0; iTick < ticks; iTick++) {
			const CRulerCtrl::TICK&	tick = m_TickArray[AXIS_VERT][iTick];
			if ((!tick.IsMinor && (style & VERT_MAJOR_GRID))
			|| (tick.IsMinor && (style & VERT_MINOR_GRID)))
				m_DC.FillSolidRect(rPlot.left, tick.Pos, rPlot.Width(), 1, m_Color[CLR_GRID]);
		}
	}
	CPoint	LogScale = GetLogScale();
	int	nSeries = GetSeriesCount();	// reverse iterate so first series is drawn last
	for (int iSeries = nSeries - 1; iSeries >= 0; iSeries--) {	// for each series
		const CSeries& ser = m_Series[iSeries];
		int	points = ser.m_Point.GetSize();
		CArrayEx<POINT, POINT&>	PtArray;
		int	PtArraySize;
		UINT	FillBaseline = SER_FILL | SER_FILL_BASELINE;
		if ((ser.m_Flags & FillBaseline) == FillBaseline)	// if filling to baseline
			PtArraySize = points + 2;	// allocate two extra points for baseline
		else	// not filling, or no baseline
			PtArraySize = points;
		PtArray.SetSize(PtArraySize);
		CDblRange	RangeX, RangeY;
		GetRange(GetAxisRuler(AXIS_HORZ), RangeX);
		GetRange(GetAxisRuler(AXIS_VERT), RangeY);
		CSize	PlotSize = rPlot.Size() - CSize(1, 1);
		DPoint	RangeLen(RangeX.Length(), RangeY.Length());
		if (!(RangeLen.x && RangeLen.y))
			continue;	// avoid divide by zero
		CPoint	PrevPt(INT_MAX, INT_MAX);
		for (int iPoint = 0; iPoint < points; iPoint++) {	// for each series point
			DPoint	pt(ser.m_Point[iPoint]);
			ApplyLogScale(LogScale, pt);
			double	x = (pt.x - RangeX.Start) / RangeLen.x;
			double	y = (pt.y - RangeY.Start) / RangeLen.y;
			CPoint	ClientPt;
			ClientPt.x = rPlot.left + round(x * PlotSize.cx);
			ClientPt.y = rPlot.bottom - round(y * PlotSize.cy);	// flip y-axis
			PtArray[iPoint] = ClientPt;
		}
		COLORREF	PenColor, BrushColor;
		if (ser.m_PenColor != CLR_INVALID)	// if pen color specified
			PenColor = ser.m_PenColor;	// use it
		else	// default pen color
			PenColor = GetDefaultPenColor(iSeries);
		if (ser.m_BrushColor != CLR_INVALID)	// if brush color specified
			BrushColor = ser.m_BrushColor;	// use it
		else	// default brush color
			BrushColor = GetDefaultBrushColor(iSeries);
		CPen	pen;
		if (ser.m_Flags & (SER_LINE | SER_MARKER_LINE))	// if pen needed
			pen.CreatePen(ser.m_PenStyle, ser.m_PenWidth, PenColor);
		CBrush	brush;
		if (ser.m_Flags & (SER_FILL | SER_MARKER_FILL)) {	// if brush needed
			if (ser.m_BrushHatch >= 0)
				brush.CreateHatchBrush(ser.m_BrushHatch, BrushColor);
			else
				brush.CreateSolidBrush(BrushColor);
		}
		int	BkMode = (ser.m_Flags & SER_TRANSPARENT) ? TRANSPARENT : OPAQUE;
		int	PrevBkMode = m_DC.SetBkMode(BkMode);
		COLORREF	PrevBkColor = m_DC.SetBkColor(ser.m_BkColor);
		HGDIOBJ	PrevPen = m_DC.SelectObject(GetStockObject(NULL_PEN));
		HGDIOBJ	PrevBrush = m_DC.SelectObject(brush);
		if (ser.m_Flags & SER_FILL) {	// if filling series points
			// if adding baseline, and at least one point
			if ((ser.m_Flags & SER_FILL_BASELINE) && points) {
				double	ry = (-RangeY.Start + ser.m_Baseline) / RangeLen.y * PlotSize.cy;
				int	BaselineY = rPlot.bottom - round(ry);
				// add baseline end point: last point's x, baseline y
				PtArray[points] = CPoint(PtArray[points - 1].x, BaselineY);
				// add baseline start point: first point's x, baseline y
				PtArray[points + 1] = CPoint(PtArray[0].x, BaselineY);
			}
			m_DC.Polygon(PtArray.GetData(), PtArraySize);	// fill points
		}
		m_DC.SelectObject(pen);
		if (ser.m_Flags & SER_LINE)	// if outlining series points
			m_DC.Polyline(PtArray.GetData(), points);	// outline points
		// if drawing markers at series points
		if (ser.m_Flags & (SER_MARKER_LINE | SER_MARKER_FILL)) {
			if (!(ser.m_Flags & SER_MARKER_LINE))
				m_DC.SelectObject(GetStockObject(NULL_PEN));
			if (!(ser.m_Flags & SER_MARKER_FILL))
				m_DC.SelectObject(GetStockObject(NULL_BRUSH));
			COLORREF	PrevTextColor = m_DC.SetTextColor(PenColor);
			PolyMarker(m_DC, PtArray.GetData(), points, 
				ser.m_MarkerStyle, ser.m_MarkerSize);
			m_DC.SetTextColor(PrevTextColor);
		}
		m_DC.SelectObject(PrevPen);	// restore GDI states
		m_DC.SelectObject(PrevBrush);
		m_DC.SetBkMode(PrevBkMode);
		m_DC.SetBkColor(PrevBkColor);
	}
	if ((style & PLOT_BORDER) && rPlot.Size().cy > 0) {	// if drawing plot border
		m_DC.SelectObject(GetStockObject(NULL_BRUSH));
		m_DC.SelectObject(GetStockObject(DC_PEN));
		SetDCPenColor(m_DC, m_Color[CLR_BORDER]);
		m_DC.Rectangle(rPlot);
	}
}

void CPlotCtrl::Update()
{
	UpdateRulers();	// order matters; update rulers first to set tick arrays
	UpdatePlot();
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CPlotCtrl message map

BEGIN_MESSAGE_MAP(CPlotCtrl, CWnd)
	//{{AFX_MSG_MAP(CPlotCtrl)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnNeedText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotCtrl message handlers

int CPlotCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_DC.Create(this))
		return -1;
	DWORD	style = lpCreateStruct->style;
	UINT	BaseRulerStyle = WS_CHILD | CRulerCtrl::ENFORCE_MARGINS;
	if (style & HIDE_CLIPPED_VALS)
		BaseRulerStyle |= CRulerCtrl::HIDE_CLIPPED_VALS;
	for (int iRuler = 0; iRuler < RULERS; iRuler++) {
		CRulerCtrl&	ruler = GetRuler(iRuler);
		UINT	RulerAlign = CBRS_ALIGN_LEFT << iRuler;
		UINT	RulerStyle = BaseRulerStyle | RulerAlign;
		if (m_VisibleRulers & GetRulerMask(iRuler))
			RulerStyle |= WS_VISIBLE;
		if (!ruler.Create(RulerStyle, CRect(0, 0, 0, 0), this, 0))
			return -1;
	}
	m_Tip.Create(this);	// create tooltip control
	m_Tip.m_PlotCtrl = this;
	m_Tip.AddTool(this, LPSTR_TEXTCALLBACK, CRect(0, 0, 0, 0), TOOLTIP_ID);
	m_Tip.SetMaxTipWidth(SHRT_MAX);	// enable multiline tip
	m_Tip.SetDelayTime(TTDT_AUTOPOP, SHRT_MAX);	// delay times limited to 16-bit
	m_Tip.Activate(FALSE);	// initially inactive

	return 0;
}

LRESULT CPlotCtrl::OnGetFont(WPARAM wParam, LPARAM lParam)
{
	 return (LRESULT)m_Font;
}

LRESULT CPlotCtrl::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	m_Font = (HFONT)wParam;
	for (int iRuler = 0; iRuler < RULERS; iRuler++)
		GetRuler(iRuler).SetFont(GetFont());
	if (lParam)
		Invalidate();
	return 0;
}

void CPlotCtrl::OnSize(UINT nType, int cx, int cy) 
{
//printf("CPlotCtrl::OnSize %d %d\n", cx, cy);
	CWnd::OnSize(nType, cx, cy);
	if (!m_DC.CreateBackBuffer(cx, cy))
		AfxThrowResourceException();
	m_WndSize = CSize(cx, cy);
	Update();
	m_Tip.SetToolRect(this, TOOLTIP_ID, CRect(0, 0, cx, cy));	// update tooltip area
}

BOOL CPlotCtrl::OnEraseBkgnd(CDC* pDC) 
{
	CRect	cb;
	pDC->GetClipBox(cb);
//printf("OnEraseBkgnd cb %d %d %d %d\n", cb.left, cb.top, cb.right, cb.bottom);
	CRect	rPlot(m_PlotRect);
	pDC->ExcludeClipRect(rPlot);
	pDC->FillSolidRect(cb, m_Color[CLR_BKGD]);
	CRgn	ClipRgn;
	ClipRgn.CreateRectRgnIndirect(cb);
	pDC->SelectClipRgn(&ClipRgn);
	return(TRUE);
}

void CPlotCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect	cb;
	dc.GetClipBox(cb);
//printf("OnPaint cb %d %d %d %d\n", cb.left, cb.top, cb.right, cb.bottom);
	if (!m_PlotRect.IsRectEmpty()) {
		dc.IntersectClipRect(m_PlotRect);	// restrict blit to plot area
		dc.BitBlt(cb.left, cb.top, cb.Width(), cb.Height(),	// blit view
			&m_DC, cb.left, cb.top, SRCCOPY);
	}
}

HBRUSH CPlotCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH	hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	if (nCtlColor == CTLCOLOR_STATIC) {	// if static child control, assume ruler
		hbr = HBRUSH(GetStockObject(NULL_BRUSH));	// paint rulers with null brush
		pDC->SetTextColor(m_Color[CLR_TEXT]);	// set ruler text color
	}
	return hbr;
}

BOOL CPlotCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if (GetStyle() & DATA_TIPS) {
		switch (pMsg->message) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			m_Tip.RelayEvent(pMsg);	// relay mouse events to tip
			break;
		case WM_MOUSEMOVE:
			m_Tip.OnMouseMove(pMsg);	// relay mouse move to tip
			break;
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

BOOL CPlotCtrl::OnNeedText(UINT id, NMHDR* pTTTStruct, LRESULT* pResult)
{
	TOOLTIPTEXT	*pTTT = (TOOLTIPTEXT *)pTTTStruct;
	CPoint	DataPos = m_Tip.m_DataPos;
	if (DataPos.x >= GetSeriesCount())
		return(FALSE);
	const CSeries&	ser = m_Series[DataPos.x];
	if (DataPos.y >= ser.m_Point.GetSize())
		return(FALSE);
	DPoint	pt(ser.m_Point[DataPos.y]);
	CString	sData;
	if (m_TipPrecision >= 0)	// if precision specified
		sData.Format(_T("%.*f, %.*f"), m_TipPrecision, pt.x, m_TipPrecision, pt.y);
	else	// default precision
		sData.Format(_T("%g, %g"), pt.x, pt.y);	// generic format
	if (ser.m_Name.IsEmpty())
		m_Tip.m_Text = sData;
	else
		m_Tip.m_Text = ser.m_Name + '\n' + sData;
	pTTT->lpszText = m_Tip.m_Text.GetBuffer(0);
	return(TRUE);
}
