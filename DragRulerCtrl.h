// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26dec12	initial version

		drag ruler control

*/

#if !defined(AFX_DRAGRULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_)
#define AFX_DRAGRULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DragRulerCtrl.h : header file
//

#include "RulerCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CDragRulerCtrl window

class CDragRulerCtrl : public CRulerCtrl
{
	DECLARE_DYNAMIC(CDragRulerCtrl)
// Construction
public:
	CDragRulerCtrl();

// Attributes
public:

// Operations
public:
	virtual	void	EndDrag();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDragRulerCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDragRulerCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDragRulerCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types

// Constants
	enum {	// drag states
		DRAG_NONE,			// default state
		DRAG_TRACK,			// tracking potential drag
		DRAG_ACTIVE,		// drag in progress
	};

// Data members
	int		m_DragState;		// drag state, see enum above
	CPoint	m_DragOrigin;		// drag initial point, in client coords

// Overridables
	virtual	void	OnDragBegin(UINT Flags, CPoint Point);
	virtual	void	OnDrop(UINT Flags, CPoint Point);

// Helpers
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAGRULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_)
