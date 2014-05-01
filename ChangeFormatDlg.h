// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21jan13	initial version

		change format dialog
 
*/

#if !defined(AFX_CHANGEFORMATDLG_H__ACF26E58_948A_4EA4_8F15_46694593E500__INCLUDED_)
#define AFX_CHANGEFORMATDLG_H__ACF26E58_948A_4EA4_8F15_46694593E500__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChangeFormatDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChangeFormatDlg dialog

class CChangeFormatDlg : public CDialog
{
// Construction
public:
	CChangeFormatDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChangeFormatDlg)
	enum { IDD = IDD_CHANGE_FORMAT };
	UINT	m_SampleRate;
	UINT	m_SampleBits;
	UINT	m_Channels;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangeFormatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChangeFormatDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANGEFORMATDLG_H__ACF26E58_948A_4EA4_8F15_46694593E500__INCLUDED_)
