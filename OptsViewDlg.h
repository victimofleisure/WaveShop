// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version
		01		12feb13	add view palette
		02		28apr13	remove persistence

        view options dialog
 
*/

#if !defined(AFX_OPTSVIEWDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
#define AFX_OPTSVIEWDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsViewDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsViewDlg dialog

#include "OptionsInfo.h"

class COptsViewDlg : public CPropertyPage
{
// Construction
public:
	COptsViewDlg(COptionsInfo& Info);

// Attributes

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptsViewDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(COptsViewDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnColors();
	//}}AFX_MSG
	afx_msg LRESULT	OnKickIdle(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Dialog data
	//{{AFX_DATA(COptsViewDlg)
	enum { IDD = IDD_OPT_VIEW };
	//}}AFX_DATA

// Constants

// Member data
	COptionsInfo&	m_oi;		// reference to parent's options info 

// Helpers
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSVIEWDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
