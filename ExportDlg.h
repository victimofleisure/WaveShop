// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      20feb13	initial version

		export dialog
 
*/

#if !defined(AFX_EXPORTDLG_H__F9B2E913_FBEF_4A23_B760_74207DA151A5__INCLUDED_)
#define AFX_EXPORTDLG_H__F9B2E913_FBEF_4A23_B760_74207DA151A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

#include "SndFileEx.h"

class CWave;

class CExportDlg : public CDialog
{
// Construction
public:
	CExportDlg(const CWave& Wave, CWnd* pParent = NULL);

// Attributes
	int		GetSelFormat() const;
	CString	GetSelExtension() const;
	CString	GetSelFilter() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CExportDlg)
	enum { IDD = IDD_EXPORT };
	CComboBox	m_EndiannessCombo;
	CComboBox	m_SubtypeCombo;
	CComboBox	m_MajorFormatCombo;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CExportDlg)
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelchangeMajorFormat();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	struct ENDIANNESS_INFO {	// map endianness options to their names
		int		Option;		// sndfile endianness option
		int		Name;		// resource ID of name string
	};

// Constants
	static const ENDIANNESS_INFO	m_EndiannessInfo[];	// endianness info

// Member data
	const CWave&	m_Wave;		// reference to caller's wave
	int		m_InitSubtype;		// initial subtype guess based on sample size
	int		m_SelFormat;		// selected format's sndfile format code
	CString	m_SelExtension;		// selected format's extension string
	CString	m_SelFilter;		// selected format's filter string
	CSndFileEx	m_SndFile;		// sndfile library interface	
	CSndFileFormatArray	m_MajorFormat;	// array of major formats
	CSndFileFormatArray	m_Subtype;	// array of subtypes

// Helpers
	bool	UpdateSubtypeCombo(int MajorFormatIdx);
	int		GetFormatFromCombos() const;
};

inline int CExportDlg::GetSelFormat() const
{
	return(m_SelFormat);
}

inline CString CExportDlg::GetSelExtension() const
{
	return(m_SelExtension);
}

inline CString CExportDlg::GetSelFilter() const
{
	return(m_SelFilter);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTDLG_H__F9B2E913_FBEF_4A23_B760_74207DA151A5__INCLUDED_)
