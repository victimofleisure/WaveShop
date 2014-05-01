// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01apr13	initial version
		01		28apr13	remove persistence

        real-time spectrum analyzer options dialog
 
*/

#if !defined(AFX_OPTSRTSADLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
#define AFX_OPTSRTSADLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsRTSADlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsRTSADlg dialog

#include "OptionsInfo.h"
#include "SwatchButton.h"

class COptsRTSADlg : public CPropertyPage
{
// Construction
public:
	COptsRTSADlg(COptionsInfo& Info);

// Attributes

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptsRTSADlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(COptsRTSADlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnColorPlotBkgnd();
	afx_msg void OnColorPlotGrid();
	//}}AFX_MSG
	afx_msg LRESULT	OnKickIdle(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Dialog data
	//{{AFX_DATA(COptsRTSADlg)
	enum { IDD = IDD_OPT_RTSA };
	CSwatchButton	m_GridSwatch;
	CSwatchButton	m_BkgndSwatch;
	CSpinButtonCtrl	m_AveragingSpin;
	CComboBox	m_WindowSizeCombo;
	CComboBox	m_WindowFuncCombo;
	//}}AFX_DATA

// Member data
	COptionsInfo&	m_oi;		// reference to parent's options info 
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSRTSADLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
