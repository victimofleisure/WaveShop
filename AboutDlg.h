// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
        01      19feb13	add libsndfile credit
        02      25feb13	add libsamplerate credit
        03      27feb13	add libmad credit
        04      20mar13	add Kiss FFT credit
        05      10apr13	add libmp3lame credit
        06      20apr13	add libfaad2 credit

        about dialog
 
*/

#if !defined(AFX_ABOUTDLG_H__ED0ED84D_AA43_442C_85CD_A7FA518EBF90__INCLUDED_)
#define AFX_ABOUTDLG_H__ED0ED84D_AA43_442C_85CD_A7FA518EBF90__INCLUDED_

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

#include "Hyperlink.h"

class CAboutDlg : public CDialog
{
// Construction
public:
	CAboutDlg();

// Constants
	static const LPCTSTR HOME_PAGE_URL;

// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CEdit	m_License;
	CHyperlink	m_AboutUrl;
	CHyperlink	m_Libsndfile;
	CHyperlink	m_Libsamplerate;
	CHyperlink	m_Libmad;
	CHyperlink	m_Libmp3lame;
	CHyperlink	m_KissFFT;
	CHyperlink	m_Libfaad2;
	CStatic	m_AboutText;
	//}}AFX_DATA
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ABOUTDLG_H__ED0ED84D_AA43_442C_85CD_A7FA518EBF90__INCLUDED_)
