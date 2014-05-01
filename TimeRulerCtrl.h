// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24oct12	initial version
        01      30jan13	add OnLButtonUp, remove OnLButtonDown

		time ruler control

*/

#if !defined(AFX_TIMERULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_)
#define AFX_TIMERULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TimeRulerCtrl.h : header file
//

#include "ZoomRulerCtrl.h"

class CWaveShopView;

/////////////////////////////////////////////////////////////////////////////
// CTimeRulerCtrl window

class CTimeRulerCtrl : public CZoomRulerCtrl
{
	DECLARE_DYNAMIC(CTimeRulerCtrl)
// Construction
public:
	CTimeRulerCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimeRulerCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTimeRulerCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTimeRulerCtrl)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Overrides
	virtual	void	GetTargetRect(CRect& Rect) const;
	virtual	void	OnDrop(UINT Flags, CPoint Point);

// Helpers
	CWaveShopView	*GetView();
	const CWaveShopView	*GetView() const;
	void	StepZoom(CPoint Point, bool In);
};

inline CWaveShopView *CTimeRulerCtrl::GetView()
{
	return(STATIC_DOWNCAST(CWaveShopView, m_Target));
}

inline const CWaveShopView *CTimeRulerCtrl::GetView() const
{
	return(STATIC_DOWNCAST(CWaveShopView, m_Target));
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMERULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_)
