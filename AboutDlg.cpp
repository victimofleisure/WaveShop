// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
        01      28jan13	add x64 to caption
        02      19feb13	add libsndfile credit
        03      25feb13	add libsamplerate credit
        04      27feb13	add libmad credit
        05      20mar13	add Kiss FFT credit
        06      10apr13	add libmp3lame credit
        07      20apr13	add libfaad2 credit

        about dialog
 
*/

#include "stdafx.h"
#include "WaveShop.h"
#include "AboutDlg.h"
#include "VersionInfo.h"

const LPCTSTR CAboutDlg::HOME_PAGE_URL = _T("http://waveshop.sourceforge.net");

CAboutDlg::CAboutDlg() 
	: CDialog(IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_ABOUT_KISS_FFT, m_KissFFT);
	DDX_Control(pDX, IDC_ABOUT_LICENSE, m_License);
	DDX_Control(pDX, IDC_ABOUT_URL, m_AboutUrl);
	DDX_Control(pDX, IDC_ABOUT_LIBSNDFILE, m_Libsndfile);
	DDX_Control(pDX, IDC_ABOUT_LIBSAMPLERATE, m_Libsamplerate);
	DDX_Control(pDX, IDC_ABOUT_LIBMAD, m_Libmad);
	DDX_Control(pDX, IDC_ABOUT_LIBMP3LAME, m_Libmp3lame);
	DDX_Control(pDX, IDC_ABOUT_TEXT, m_AboutText);
	DDX_Control(pDX, IDC_ABOUT_LIBFAAD2, m_Libfaad2);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg message handlers

BOOL CAboutDlg::OnInitDialog() 
{
	m_AboutUrl.SetUrl(HOME_PAGE_URL);
	m_Libsndfile.SetUrl(_T("http://www.mega-nerd.com/libsndfile/"));
	m_Libsamplerate.SetUrl(_T("http://www.mega-nerd.com/SRC/"));
	m_Libmad.SetUrl(_T("http://www.underbit.com/products/mad/"));
	m_Libmp3lame.SetUrl(_T("http://lame.sourceforge.net/"));
	m_KissFFT.SetUrl(_T("http://sourceforge.net/projects/kissfft/"));
	m_Libfaad2.SetUrl(_T("http://www.audiocoding.com/"));
	CDialog::OnInitDialog();
	
	CString	s;
#ifdef _WIN64
	GetWindowText(s);
	s += _T(" (x64)");
	SetWindowText(s);
#endif
	VS_FIXEDFILEINFO	AppInfo;
	CVersionInfo::GetFileInfo(AppInfo, NULL);
	s.Format(IDS_APP_ABOUT_TEXT, theApp.m_pszAppName,
		HIWORD(AppInfo.dwFileVersionMS), LOWORD(AppInfo.dwFileVersionMS),
		HIWORD(AppInfo.dwFileVersionLS), LOWORD(AppInfo.dwFileVersionLS));
	m_AboutText.SetWindowText(s);
	m_License.SetWindowText(LDS(IDS_APP_LICENSE));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
