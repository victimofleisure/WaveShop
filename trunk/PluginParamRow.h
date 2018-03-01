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

#ifndef CPLUGINPARAMROW_INCLUDED
#define	CPLUGINPARAMROW_INCLUDED

#include "ClickSliderCtrl.h"

struct _LADSPA_Descriptor;

class CPluginParamRow : public WObject {
public:
// Construction
	CPluginParamRow();
	void	Init(const _LADSPA_Descriptor *Desc, UINT PortIdx, int SampleRate);
	bool	Create(CWnd *Parent, int RowIdx, int MaxNameWidth, CIntRange MaxBoundWidth);

// Constants
	enum {	// column indices
		COL_NAME,		// parameter name
		COL_EDIT,		// edit control
		COL_LOWER,		// lower bound
		COL_SLIDER,		// slider control
		COL_UPPER,		// uppper bound
		COLUMNS,
	};

// Attributes
	double	GetVal() const;
	void	SetVal(double Val);
	CFloatRange	GetBounds() const;
	CDblRange	GetLogBounds() const;
	CString	GetName() const;
	double	GetDefaultVal() const;
	static	int		GetBaseCtrlID(int RowIdx);
	static	int		GetRowIdx(int CtrlID);
	static	int		GetTotalHeight(int Rows);

// Operations
	void	OnSliderChange();
	void	OnEditChange();
	double	PosToVal(int Pos) const;
	int		ValToPos(double Val) const;
	void	RestoreDefault();
	static	CString	ValToStr(double Val);

protected:
// Constants
	enum {
		H_GUTTER = 5,			// horizontal gutter, in pixels
		V_GUTTER = 5,			// vertical gutter, in pixels
		EDIT_W = 100,			// edit control width, in pixels
		ROW_H = 24,				// row height, in pixels
		CTRL_ID_OFFSET = 1000,	// offset for all control IDs
		SLIDER_RANGE = 100,		// slider range
	};

// Data members
	const _LADSPA_Descriptor *m_Desc;	// plugin descriptor
	UINT	m_PortIdx;			// port descriptor array index
	CString	m_Name;				// parameter name, including trailing colon
	CFloatRange	m_Bounds;		// parameter's lower and upper bounds
	CStatic	m_NameCtrl;			// name static control
	CEdit	m_Edit;				// value edit control
	CStatic	m_Lower;			// lower bound static control
	CClickSliderCtrl	m_Slider;	// value slider control
	CStatic	m_Upper;			// upper bound static control

// Helpers
	double	GetScaledDefaultVal(int HintDesc, double Scale) const;
};

inline CFloatRange CPluginParamRow::GetBounds() const
{
	return(m_Bounds);
}

inline CString CPluginParamRow::GetName() const
{
	return(m_Name);
}

#endif
