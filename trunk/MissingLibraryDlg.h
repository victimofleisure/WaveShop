// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11apr13	initial version

		missing library dialog
 
*/

#if !defined(AFX_MISSINGLIBRARYDLG_H__FEDD862C_651A_4C46_B702_355BD724A2BC__INCLUDED_)
#define AFX_MISSINGLIBRARYDLG_H__FEDD862C_651A_4C46_B702_355BD724A2BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MissingLibraryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMissingLibraryDlg dialog

#include "FileSearchDlg.h"

class CMissingLibraryDlg : public CDialog
{
// Construction
public:
	CMissingLibraryDlg(LPCTSTR Description, LPCTSTR FileName, LONGLONG MinVersion, LPCTSTR DownloadURL, CWnd* pParent = NULL);

// Attributes
	CString	GetLibraryPath() const;

// Operations
	static	CString	MakeVersionStr(LONGLONG Version);
	static	LONGLONG	MakeVersion(USHORT v3, USHORT v2, USHORT v1, USHORT v0);
	static	bool	CheckMinimumVersion(LPCTSTR Path, LONGLONG MinVersion);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMissingLibraryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMissingLibraryDlg)
	enum { IDD = IDD_MISSING_LIBRARY };
	CString	m_FileName;
	CString	m_Platform;
	CString	m_MinVersionStr;
	CString	m_Description;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMissingLibraryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	afx_msg void OnDownload();
	afx_msg void OnSearch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	class CMyFileSearchDlg : public CFileSearchDlg {
	public:
		CString	m_LibFileName;		// file name to search for
		CString	m_LibPath;			// if library is found, its path
		LONGLONG	m_MinVersion;	// minimum acceptable version
		virtual	bool	OnFile(LPCTSTR Path);
	};

// Member data
	CString	m_LibPath;			// if library is found, its path
	CString	m_DownloadURL;		// URL to download library from
	LONGLONG	m_MinVersion;	// minimum acceptable version of library
};

inline CString CMissingLibraryDlg::GetLibraryPath() const
{
	return(m_LibPath);
}

inline LONGLONG CMissingLibraryDlg::MakeVersion(USHORT v3, USHORT v2, USHORT v1, USHORT v0)
{
	LARGE_INTEGER	ver;
	ver.LowPart = MAKELONG(v0, v1);
	ver.HighPart = MAKELONG(v2, v3);
	return(ver.QuadPart);	// return 64-bit version number
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MISSINGLIBRARYDLG_H__FEDD862C_651A_4C46_B702_355BD724A2BC__INCLUDED_)
