// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version

		time ruler control bar

*/

#if !defined(AFX_TIMERULERBAR_H__4BA66D6B_9B2D_4D4F_B604_F9B08D2CD5A9__INCLUDED_)
#define AFX_TIMERULERBAR_H__4BA66D6B_9B2D_4D4F_B604_F9B08D2CD5A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TimeRulerBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTimeRulerBar window

#include "RulerBar.h"
#include "TimeRulerCtrl.h"

class CTimeRulerBar : public CRulerBar
{
	DECLARE_DYNAMIC(CTimeRulerBar);
// Construction
public:
	CTimeRulerBar();

// Attributes
public:
	CTimeRulerCtrl	*GetRuler();
	const CTimeRulerCtrl	*GetRuler() const;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimeRulerBar)
	//}}AFX_VIRTUAL
	virtual bool	CreateRuler();

// Implementation
public:
	virtual ~CTimeRulerBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTimeRulerBar)
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members

// Helpers
};

inline CTimeRulerCtrl *CTimeRulerBar::GetRuler()
{
	return((CTimeRulerCtrl *)m_Ruler);
}

inline const CTimeRulerCtrl *CTimeRulerBar::GetRuler() const
{
	return((const CTimeRulerCtrl *)m_Ruler);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMERULERBAR_H__4BA66D6B_9B2D_4D4F_B604_F9B08D2CD5A9__INCLUDED_)
