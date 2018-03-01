// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      31jan07	initial version
        01      18apr08	add show window handler
		02		14mar09	add style changed handler
		03		25may10	add size valid flag
		04		29mar13	remove FastIsVisible
		05      23apr13	handle command help

        wrapper for Cristi Posea's sizable control bar
 
*/

#if !defined(AFX_MYSIZINGCONTROLBAR_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_)
#define AFX_MYSIZINGCONTROLBAR_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MySizingControlBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMySizingControlBar dialog

class CMySizingControlBar : public CSizingControlBarG
{
	DECLARE_DYNAMIC(CMySizingControlBar);
// Construction
public:
	CMySizingControlBar();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMySizingControlBar)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CMySizingControlBar)
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	//}}AFX_MSG
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Member data
	bool	m_IsSizeValid;		// if true, OnSize arguments are valid
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSIZINGCONTROLBAR_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_)
