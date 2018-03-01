// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25jun13	initial version

        plugin parameters row
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "PluginParamRow.h"
#include "ladspa.h"
#include <math.h>

CPluginParamRow::CPluginParamRow()
{
	m_Desc = NULL;
	m_PortIdx = 0;
	m_Bounds = CFloatRange(0, 0);
}

void CPluginParamRow::Init(const _LADSPA_Descriptor *Desc, UINT PortIdx, int SampleRate)
{
	ASSERT(Desc != NULL);	// plugin descriptor can't be null
	ASSERT(PortIdx < Desc->PortCount);
	ASSERT(SampleRate > 0);
	ASSERT(LADSPA_IS_PORT_CONTROL(Desc->PortDescriptors[PortIdx]));	// port must be control
	m_Desc = Desc;
	m_PortIdx = PortIdx;
	m_Name = Desc->PortNames[PortIdx];
	m_Name += ':';	// append colon to parameter name
	const LADSPA_PortRangeHint&	hint = Desc->PortRangeHints[PortIdx];
	m_Bounds = CFloatRange(hint.LowerBound, hint.UpperBound);
	if (LADSPA_IS_HINT_SAMPLE_RATE(hint.HintDescriptor)) {	// if sample rate parameter
		m_Bounds.Start *= SampleRate;
		m_Bounds.End *= SampleRate;
	}
}

static inline LADSPA_PortRangeHintDescriptor GetHintDescriptor(const _LADSPA_Descriptor *Desc, int PortIdx)
{
	return(Desc->PortRangeHints[PortIdx].HintDescriptor);
}

double CPluginParamRow::GetVal() const
{
	CString	s;
	m_Edit.GetWindowText(s);
	return(_tstof(s));
}

void CPluginParamRow::SetVal(double Val)
{
	m_Edit.SetWindowText(ValToStr(Val));
	OnEditChange();
}

int CPluginParamRow::GetTotalHeight(int Rows)
{
	return((ROW_H + V_GUTTER) * Rows + V_GUTTER);
}

int CPluginParamRow::GetBaseCtrlID(int RowIdx)
{
	return(RowIdx * COLUMNS + CTRL_ID_OFFSET);
}

int CPluginParamRow::GetRowIdx(int CtrlID)
{
	return((CtrlID - CTRL_ID_OFFSET) / COLUMNS);
}

CString CPluginParamRow::ValToStr(double Val)
{
	CString	s;
	s.Format(_T("%g"), Val);
	return(s);
}

CDblRange CPluginParamRow::GetLogBounds() const
{
	return(CDblRange(log(m_Bounds.Start), log(m_Bounds.End)));
}

double CPluginParamRow::PosToVal(int Pos) const
{
	LADSPA_PortRangeHintDescriptor	HintDesc = GetHintDescriptor(m_Desc, m_PortIdx);
	double	val;
	if (LADSPA_IS_HINT_INTEGER(HintDesc))	// if integer
		val = Pos;
	else if (LADSPA_IS_HINT_LOGARITHMIC(HintDesc)) {	// if logarithmic
		CDblRange	bounds(GetLogBounds());
		val = exp(bounds.Start + Pos * (bounds.Length() / SLIDER_RANGE));
	} else	// linear
		val = double(Pos) / SLIDER_RANGE * m_Bounds.Length() + m_Bounds.Start;
	return(val);
}

int CPluginParamRow::ValToPos(double Val) const
{
	LADSPA_PortRangeHintDescriptor	HintDesc = GetHintDescriptor(m_Desc, m_PortIdx);
	double	pos;
	if (LADSPA_IS_HINT_INTEGER(HintDesc))	// if integer
		pos = Val;
	else if (LADSPA_IS_HINT_LOGARITHMIC(HintDesc)) {	// if logarithmic
		CDblRange	bounds(GetLogBounds());
		pos = (log(Val) - bounds.Start) / (bounds.Length() / SLIDER_RANGE);
	} else	// linear
		pos = (Val - m_Bounds.Start) / m_Bounds.Length() * SLIDER_RANGE;
	return(round(pos));
}

void CPluginParamRow::OnSliderChange()
{
	m_Edit.SetWindowText(ValToStr(PosToVal(m_Slider.GetPos())));
}

void CPluginParamRow::OnEditChange()
{
	m_Slider.SetPos(ValToPos(GetVal()));
}

void CPluginParamRow::RestoreDefault()
{
	SetVal(GetDefaultVal());
}

double CPluginParamRow::GetScaledDefaultVal(int HintDesc, double Scale) const
{
	double	DefVal;
	if (LADSPA_IS_HINT_LOGARITHMIC(HintDesc))	// if logarithmic
		DefVal = PosToVal(round(SLIDER_RANGE * Scale));
	else	// linear
		DefVal = m_Bounds.Start + m_Bounds.Length() * Scale;
	return(DefVal);
}

double CPluginParamRow::GetDefaultVal() const
{
	LADSPA_PortRangeHintDescriptor	HintDesc = GetHintDescriptor(m_Desc, m_PortIdx);
	double	DefVal;
	switch (HintDesc & LADSPA_HINT_DEFAULT_MASK) {
	case LADSPA_HINT_DEFAULT_MINIMUM:
		DefVal = m_Bounds.Start;
		break;
	case LADSPA_HINT_DEFAULT_LOW:
		DefVal = GetScaledDefaultVal(HintDesc, 0.25);
		break;
	case LADSPA_HINT_DEFAULT_MIDDLE:
		DefVal = GetScaledDefaultVal(HintDesc, 0.5);
		break;
	case LADSPA_HINT_DEFAULT_HIGH:
		DefVal = GetScaledDefaultVal(HintDesc, 0.75);
		break;
	case LADSPA_HINT_DEFAULT_MAXIMUM:
		DefVal = m_Bounds.End;
		break;
	case LADSPA_HINT_DEFAULT_0:
		DefVal = 0;
		break;
	case LADSPA_HINT_DEFAULT_1:
		DefVal = 1;
		break;
	case LADSPA_HINT_DEFAULT_100:
		DefVal = 100;
		break;
	case LADSPA_HINT_DEFAULT_440:
		DefVal = 440;
		break;
	default:
		DefVal = 0;
	}
	return(DefVal);
}

bool CPluginParamRow::Create(CWnd *pParent, int RowIdx, int MaxNameWidth, CIntRange MaxBoundWidth)
{
	ASSERT(pParent != NULL);	// parent window can't be null
	LADSPA_PortRangeHintDescriptor	HintDesc = GetHintDescriptor(m_Desc, m_PortIdx);
	CString	s;
	CRect	r;
	DWORD	style;
	int	x = H_GUTTER;
	int	y = RowIdx * (ROW_H + V_GUTTER) + V_GUTTER;
	int	BaseID = GetBaseCtrlID(RowIdx);
	// create parameter name static control
	r = CRect(CPoint(x, y), CSize(MaxNameWidth, ROW_H));
	style = WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE;
	if (!m_NameCtrl.Create(m_Name, style, r, pParent, BaseID + COL_NAME))
		return(FALSE);
	x += MaxNameWidth + H_GUTTER;
	// create edit control
	r = CRect(CPoint(x, y), CSize(EDIT_W, ROW_H));
	style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL;
	double	DefVal = GetDefaultVal();
	s = ValToStr(DefVal);
	// must use CreateEx instead of Create to obtain 3D border
	if (!m_Edit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), s, style, r, pParent, BaseID + COL_EDIT))
		return(FALSE);
	m_Edit.SetWindowText(s);
	x += EDIT_W + H_GUTTER;
	// create lower bound static control
	r = CRect(CPoint(x, y), CSize(MaxBoundWidth.Start, ROW_H));
	style = WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE;
	if (LADSPA_IS_HINT_BOUNDED_BELOW(HintDesc))
		s = ValToStr(m_Bounds.Start);
	else
		s.Empty();
	if (!m_Lower.Create(s, style, r, pParent, BaseID + COL_LOWER))
		return(FALSE);
	CRect	rcParent;
	pParent->GetClientRect(rcParent);
	x += MaxBoundWidth.Start + H_GUTTER;
	// create slider control
	int	SliderWidth = rcParent.Width() - x - MaxBoundWidth.End - H_GUTTER * 2;
	r = CRect(CPoint(x, y), CSize(SliderWidth, ROW_H));
	style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
	if (!m_Slider.Create(style, r, pParent, BaseID + COL_SLIDER))
		return(FALSE);
	if (LADSPA_IS_HINT_INTEGER(HintDesc))
		m_Slider.SetRange(round(m_Bounds.Start), round(m_Bounds.End));
	m_Slider.SetPos(ValToPos(DefVal));
	x += SliderWidth + H_GUTTER;
	// create upper bound static control
	r = CRect(CPoint(x, y), CSize(MaxBoundWidth.End, ROW_H));
	style = WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE;
	if (LADSPA_IS_HINT_BOUNDED_ABOVE(HintDesc))
		s = ValToStr(m_Bounds.End);
	else
		s.Empty();
	if (!m_Upper.Create(s, style, r, pParent, BaseID + COL_UPPER))
		return(FALSE);
	return(TRUE);
}
