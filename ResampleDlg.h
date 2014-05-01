// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25feb13	initial version

		resample dialog

*/

#if !defined(AFX_RESAMPLEDLG_H__D1175F87_17EB_43AF_8596_D79272B5AD25__INCLUDED_)
#define AFX_RESAMPLEDLG_H__D1175F87_17EB_43AF_8596_D79272B5AD25__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResampleDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResampleDlg dialog

class CResampleDlg : public CDialog
{
// Construction
public:
	CResampleDlg(UINT SampleRate, CWnd* pParent = NULL);
	~CResampleDlg();

// Attributes
	UINT	GetSampleRate() const;
	int		GetQuality() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResampleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CResampleDlg)
	enum { IDD = IDD_RESAMPLE };
	int		m_Quality;
	UINT	m_SampleRate;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CResampleDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members
	UINT	m_PrevSampleRate;
};

inline UINT CResampleDlg::GetSampleRate() const
{
	return(m_SampleRate);
}

inline int	CResampleDlg::GetQuality() const
{
	return(m_Quality);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESAMPLEDLG_H__D1175F87_17EB_43AF_8596_D79272B5AD25__INCLUDED_)
