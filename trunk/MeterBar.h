// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		09mar13	initial version
		01		29mar13	derive from custom sizing bar
		02		10may13	add ResetClipping

        meter bar
 
*/

#if !defined(AFX_METERBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_METERBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MeterBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMeterBar window

#include "MySizingControlBar.h"
#include "MeterView.h"

class CMeterBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CMeterBar);
// Construction
public:
	CMeterBar();

// Attributes
public:

// Operations
public:
	void	UpdateView();
	void	UpdateMeters();
	void	UpdateCaptions();
	void	TimerHook(W64INT Frame);
	void	ResetClipping();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeterBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMeterBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMeterBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CMeterView	*m_View;
};

inline void CMeterBar::UpdateView()
{
	m_View->UpdateView();
}

inline void CMeterBar::UpdateMeters()
{
	m_View->UpdateMeters();
}

inline void CMeterBar::UpdateCaptions()
{
	m_View->UpdateCaptions();
}

inline void CMeterBar::TimerHook(W64INT Frame)
{
	m_View->TimerHook(Frame);
}

inline void CMeterBar::ResetClipping()
{
	m_View->ResetAllClipping();
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_METERBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
