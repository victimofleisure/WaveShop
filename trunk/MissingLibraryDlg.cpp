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

// MissingLibraryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MissingLibraryDlg.h"
#include "Hyperlink.h"
#include "VersionInfo.h"
#include "FileSearchDlg.h"
#include "shlwapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMissingLibraryDlg dialog

CMissingLibraryDlg::CMissingLibraryDlg(LPCTSTR Description, LPCTSTR FileName, LONGLONG MinVersion, LPCTSTR DownloadURL, CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CMissingLibraryDlg)
	//}}AFX_DATA_INIT
	m_MinVersion = MinVersion;
	m_Description = Description;
	m_FileName = FileName;
	m_DownloadURL = DownloadURL;
#ifdef _WIN64
	UINT	PlatformBits = 64;
#else
	UINT	PlatformBits = 32;
#endif
	m_Platform.Format(_T("%d-bit"), PlatformBits);
}

CString	CMissingLibraryDlg::MakeVersionStr(LONGLONG Version)
{
	LARGE_INTEGER	li;
	li.QuadPart = Version;
	CString	str;
	str.Format(_T("%d.%d.%d.%d"), 
		HIWORD(li.HighPart), LOWORD(li.HighPart), 
		HIWORD(li.LowPart), LOWORD(li.LowPart));
	return(str);
}

bool CMissingLibraryDlg::CheckMinimumVersion(LPCTSTR Path, LONGLONG MinVersion)
{
	VS_FIXEDFILEINFO	info;
	if (!CVersionInfo::GetFileInfo(info, Path)) {	// if can't get version info
		CString	msg;
		AfxFormatString1(msg, IDS_MISSLIB_CANT_GET_VERSION, Path);
		AfxMessageBox(msg);
		return(FALSE);
	}
	LARGE_INTEGER	ver;
	ver.HighPart = info.dwFileVersionMS;
	ver.LowPart = info.dwFileVersionLS;
	if (ver.QuadPart < MinVersion) {	// if version less than minimum version
		CString	msg;
		AfxFormatString2(msg, IDS_MISSLIB_VERSION_TOO_OLD, Path, 
			MakeVersionStr(ver.QuadPart));
		AfxMessageBox(msg);
		return(FALSE);
	}
	return(TRUE);
}

bool CMissingLibraryDlg::CMyFileSearchDlg::OnFile(LPCTSTR Path)
{
	// CAUTION: this method is called from CFileSearchDlg worker thread
	CString	name(PathFindFileName(Path));
	if (name.CompareNoCase(m_LibFileName))	// if file name doesn't match
		return(TRUE);	// keep searching
	if (!CheckMinimumVersion(Path, m_MinVersion))	// if version is too old
		return(TRUE);	// keep searching
	CString	msg;
	AfxFormatString1(msg, IDS_MISSLIB_PATH_CHECK, Path);
	int	retc = AfxMessageBox(msg, MB_YESNOCANCEL | MB_ICONQUESTION);
	if (retc == IDNO)
		return(TRUE);	// keep searching
	if (retc == IDCANCEL)
		return(FALSE);	// end search
	// assignment to shared CString is safe enough in this case, because
	// main thread doesn't access string until after worker thread exits
	m_LibPath = Path;	// success: store library path
	return(FALSE);	// end search
}

void CMissingLibraryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMissingLibraryDlg)
	DDX_Text(pDX, IDC_MISSLIB_FILE_NAME, m_FileName);
	DDX_Text(pDX, IDC_MISSLIB_PLATFORM, m_Platform);
	DDX_Text(pDX, IDC_MISSLIB_MIN_VERSION, m_MinVersionStr);
	DDX_Text(pDX, IDC_MISSLIB_DESCRIPTION, m_Description);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CMissingLibraryDlg message map

BEGIN_MESSAGE_MAP(CMissingLibraryDlg, CDialog)
	//{{AFX_MSG_MAP(CMissingLibraryDlg)
	ON_BN_CLICKED(IDC_MISSLIB_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_MISSLIB_DOWNLOAD, OnDownload)
	ON_BN_CLICKED(IDC_MISSLIB_SEARCH, OnSearch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMissingLibraryDlg message handlers

BOOL CMissingLibraryDlg::OnInitDialog() 
{
	m_MinVersionStr = MakeVersionStr(m_MinVersion);
	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMissingLibraryDlg::OnBrowse() 
{
	LPCTSTR	ext = _T(".dll");
	CString	Filter((LPCTSTR)IDS_MISSLIB_BROWSE_FILTER);
	CFileDialog	fd(TRUE, ext, m_FileName, OFN_HIDEREADONLY, Filter);
	if (fd.DoModal() == IDOK) {
		// if version equals or exceeds minimum acceptable version
		if (CheckMinimumVersion(fd.GetPathName(), m_MinVersion)) {
			m_LibPath = fd.GetPathName();
			OnOK();	// success: end dialog
		}
	}
}

void CMissingLibraryDlg::OnSearch() 
{
	CMyFileSearchDlg	dlg;
	dlg.m_LibFileName = m_FileName;
	dlg.m_MinVersion = m_MinVersion;
	dlg.DoModal();
	if (dlg.m_LibPath.IsEmpty())	// if target not found
		AfxMessageBox(IDS_MISSLIB_NOT_FOUND);
	else {	// target found
		m_LibPath = dlg.m_LibPath;
		OnOK();	// success: end dialog
	}
}

void CMissingLibraryDlg::OnDownload() 
{
	CHyperlink::GotoUrl(m_DownloadURL);
}
