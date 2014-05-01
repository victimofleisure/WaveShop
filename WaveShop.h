// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
		01		28jan13	add help file
		02		25feb13	add GetDLLFunctions
        03      01mar13	add PromptForInputFiles
		04      20mar13	add InitDialogCaptionView
		05      16apr13	make help context-sensitive
		06      20apr13	add OpenTempStream
		07      22apr13	override WinHelpInternal for NET version
		08		17may13	add browse folder method
		09		18may13	add command line argument for recording
		10		21may13	add MakeAbsolutePath

		wave editor
 
*/

// WaveShop.h : main header file for the WAVVIEW application
//

#if !defined(AFX_WAVVIEW_H__67465E78_2A32_49B8_B617_37247230945C__INCLUDED_)
#define AFX_WAVVIEW_H__67465E78_2A32_49B8_B617_37247230945C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "WinAppEx.h"
#include "MainFrm.h"

/////////////////////////////////////////////////////////////////////////////
// CWaveShopApp:
// See WaveShop.cpp for the implementation of this class
//

class CDLLWrap;

class CWaveShopApp : public CWinAppEx
{
// Construction
public:
	CWaveShopApp();

// Attributes
	CMainFrame	*GetMain();
	CWaveEdit	m_Clipboard;

// Operations
	bool	HandleDlgKeyMsg(MSG* pMsg);
	static	bool	GetDLLFunctions(CDLLWrap& Lib, LPCTSTR LibPath, const int *OrdinalTbl, int Functions, CPtrArray& FuncPtr);
	static	bool	PromptForInputFiles(CStringArray& Path, int TitleID);
	static	void	InitDialogCaptionView(CDialog& Dlg, const CWaveShopView *View);
	static	FILE	*OpenTempStream(LPCTSTR Path);
	static	bool	BrowseFolder(int TitleID, CString& Path, HWND OwnerWnd = NULL);
	void	MakeAbsolutePath(CString& Path) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveShopApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	virtual BOOL IsIdleMessage(MSG* pMsg);
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	//}}AFX_VIRTUAL
	virtual void WinHelpInternal(DWORD_PTR dwData, UINT nCmd = HELP_CONTEXT);

protected:
// Implementation
	//{{AFX_MSG(CWaveShopApp)
	afx_msg void OnAppAbout();
	afx_msg void OnAppHomePage();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	struct HELP_RES_MAP {
		WORD	ResID;		// resource identifier
		WORD	HelpID;		// identifier of corresponding help topic
	};
	class CMyCommandLineInfo : public CCommandLineInfo {
		virtual void ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);
	};

// Constants
	static const HELP_RES_MAP	m_HelpResMap[];	// map resource IDs to help IDs

// Data members
	bool	m_HelpInit;			// true if help was initialized
	bool	m_InitialRecord;	// true if app should start recording initially
	friend class CMyCommandLineInfo;
};

extern CWaveShopApp theApp;

inline CMainFrame *CWaveShopApp::GetMain()
{
	return((CMainFrame *)m_pMainWnd);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVVIEW_H__67465E78_2A32_49B8_B617_37247230945C__INCLUDED_)
