// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version

		ruler control bar

*/

#if !defined(AFX_RULERBAR_H__4BA66D6B_9B2D_4D4F_B604_F9B08D2CD5A9__INCLUDED_)
#define AFX_RULERBAR_H__4BA66D6B_9B2D_4D4F_B604_F9B08D2CD5A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RulerBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRulerBar window

#include "RulerCtrl.h"

class CRulerBar : public CControlBar
{
	DECLARE_DYNAMIC(CRulerBar)
// Construction
public:
	CRulerBar();
	BOOL	Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);

// Attributes
public:
	CRulerCtrl	*GetRuler();
	const CRulerCtrl	*GetRuler() const;
	int		GetHeight() const;
	void	SetHeight(int Height);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRulerBar)
	//}}AFX_VIRTUAL
	virtual void	OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual CSize	CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual bool	CreateRuler();

// Implementation
public:
	virtual ~CRulerBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CRulerBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members
	CRulerCtrl	*m_Ruler;		// pointer to child ruler window
	int		m_Height;			// bar height if horizontal, or width if vertical

// Helpers
};

inline CRulerCtrl *CRulerBar::GetRuler()
{
	return(m_Ruler);
}

inline const CRulerCtrl *CRulerBar::GetRuler() const
{
	return(m_Ruler);
}

inline int CRulerBar::GetHeight() const
{
	return(m_Height);
}

inline void CRulerBar::SetHeight(int Height)
{
	m_Height = Height;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RULERBAR_H__4BA66D6B_9B2D_4D4F_B604_F9B08D2CD5A9__INCLUDED_)
