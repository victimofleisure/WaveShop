// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24oct12	initial version
        01      30jan13	add StepZoom helper

		channel ruler control

*/

#if !defined(AFX_CHANNELRULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_)
#define AFX_CHANNELRULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChannelRulerCtrl.h : header file
//

#include "ZoomRulerCtrl.h"
#include "RefPtr.h"

/////////////////////////////////////////////////////////////////////////////
// CChannelRulerCtrl window

class CChannelRulerCtrl : public CZoomRulerCtrl
{
	DECLARE_DYNAMIC(CChannelRulerCtrl)
// Construction
public:
	CChannelRulerCtrl();

// Attributes
public:
	void	SetOrigin(double Origin);
	void	SetAmplitude(double Amplitude);

// Operations
public:
	void	UpdateZoom(int Height);
	void	StepZoom(CPoint Point, bool In);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChannelRulerCtrl)
	//}}AFX_VIRTUAL
	virtual	void	GetTargetRect(CRect& Rect) const;
	virtual	void	OnDrop(UINT Flags, CPoint Point);

// Implementation
public:
	virtual ~CChannelRulerCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CChannelRulerCtrl)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	double	m_Origin;		// origin relative to center; 1 == height
	double	m_Amplitude;	// amplitude; 1 == fit to channel height

// Helpers
	int		GetRulerIndex() const;
};

inline void CChannelRulerCtrl::SetOrigin(double Origin)
{
	m_Origin = Origin;
}

inline void CChannelRulerCtrl::SetAmplitude(double Amplitude)
{
	m_Amplitude = Amplitude;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANNELRULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_)
