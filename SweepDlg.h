// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      07may13	initial version

		sweep dialog

*/

#if !defined(AFX_SWEEPDLG_H__5C95AFC3_EA1D_4DED_BEF6_599047F85189__INCLUDED_)
#define AFX_SWEEPDLG_H__5C95AFC3_EA1D_4DED_BEF6_599047F85189__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SweepDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSweepDlg dialog

class CSweepDlg : public CDialog
{
// Construction
public:
	CSweepDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSweepDlg)
	enum { IDD = IDD_SWEEP };
	CComboBox	m_WaveformCombo;
	double	m_Duration;
	double	m_EndFreq;
	double	m_StartFreq;
	//}}AFX_DATA
	int		m_Waveform;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSweepDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CSweepDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Helpers
	static	void	DDV_GTZeroDouble(CDataExchange *pDX, int nIDC, double& value);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SWEEPDLG_H__5C95AFC3_EA1D_4DED_BEF6_599047F85189__INCLUDED_)
