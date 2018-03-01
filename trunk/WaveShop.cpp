// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
		01		19nov12	in HandleDlgKeyMsg, forward menu key to main
		02		28jan13	add help file
		03		31jan13	in InitInstance, add special case for portable build
        04      18feb13	handle non-native file formats
		05		25feb13	add GetDLLFunctions
        06      01mar13	add PromptForInputFiles
		07      20mar13	add InitDialogCaptionView
		08      16apr13	make help context-sensitive
		09      20apr13	add OpenTempStream
		10      22apr13	override WinHelpInternal for NET version
		11		17may13	add browse folder method
		12		18may13	add command line argument for recording
		13		21may13	add MakeAbsolutePath
		14		24may13	post initial record message
		15		10jul13	in GetDLLFunctions, format error as string

		wave editor
 
*/

// WaveShop.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WaveShop.h"
#include "AboutDlg.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "WaveShopDoc.h"
#include "WaveShopView.h"
#include "Win32Console.h"
#include "FocusEdit.h"
#include "htmlhelp.h"	// needed for HTML Help API
#include "PathStr.h"
#include "DocManagerEx.h"
#include "DllWrap.h"
#include "SortStringArray.h"
#include "HelpIDs.h"
#include <io.h>		// for _open_osfhandle in OpenTempStream
#include "FolderDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaveShopApp

const CWaveShopApp::HELP_RES_MAP CWaveShopApp::m_HelpResMap[] = {
	#include "HelpResMap.h"
};

/////////////////////////////////////////////////////////////////////////////
// CWaveShopApp construction

CWaveShopApp::CWaveShopApp()
{
	// Place all significant initialization in InitInstance
	m_HelpInit = FALSE;
	m_InitialRecord = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWaveShopApp object

CWaveShopApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWaveShopApp initialization

BOOL CWaveShopApp::InitInstance()
{
	AfxEnableControlContainer();

	// initialize COM for apartment threading
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) {
		AfxMessageBox(IDS_APP_CANT_INIT_COM);
		return FALSE;
	}

#ifdef _DEBUG
	Win32Console::Create();
#endif
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MFC_VER < 0x0700
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif

#ifdef PORTABLE_APP	// if building portable application
	// Set profile name to INI file in application folder
	free((void*)m_pszProfileName);
	CPathStr	IniPath(GetAppFolder());
	IniPath.Append(CString(m_pszAppName) + _T(".ini"));
	m_pszProfileName = _tcsdup(IniPath);
#else
	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("Anal Software"));
#endif

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_WAVVIETYPE,
		RUNTIME_CLASS(CWaveShopDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CWaveShopView));
	ASSERT(m_pDocManager == NULL);
	m_pDocManager = new CDocManagerEx;
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	m_pMainWnd = pMainFrame;	// so components can use GetMain during init
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

	// Parse command line for standard shell commands, DDE, file open
	CMyCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Don't display a new MDI child window during startup
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	
	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->UpdateWindow();
	pMainFrame->ShowWindow(m_nCmdShow);

	if (m_InitialRecord)
		pMainFrame->PostMessage(UWM_INITIAL_RECORD);

	return TRUE;
}

int CWaveShopApp::ExitInstance() 
{
	// if HTML help was initialized, close all topics
	if (m_HelpInit)
		::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);
	CoUninitialize();
	return CWinAppEx::ExitInstance();
}

void CWaveShopApp::WinHelp(DWORD dwData, UINT nCmd) 
{
//printf("dwData=%d:%d nCmd=%d\n", HIWORD(dwData), LOWORD(dwData), nCmd);
	CPathStr	HelpPath(GetAppFolder());
	HelpPath.Append(CString(m_pszAppName) + _T(".chm"));
	HWND	hMainWnd = GetMain()->m_hWnd;
	UINT	ResID = LOWORD(dwData);
	int	HelpID = 0;
	int	elems = _countof(m_HelpResMap);
	for (int iElem = 0; iElem < elems; iElem++) {	// for each map element
		if (ResID == m_HelpResMap[iElem].ResID) {	// if resource ID found
			HelpID = m_HelpResMap[iElem].HelpID;	// store context help ID
			break;
		}
	}
	HWND	retc = 0;	// assume failure
	if (HelpID)	// if context help ID was found
		retc = ::HtmlHelp(hMainWnd, HelpPath, HH_HELP_CONTEXT, HelpID);
	if (!retc) {	// if context help wasn't available or failed
		retc = ::HtmlHelp(hMainWnd, HelpPath, HH_DISPLAY_TOC, 0);	// show contents
		if (!retc) {	// if help file not found
			CString	s;
			AfxFormatString1(s, IDS_APP_HELP_FILE_MISSING, HelpPath);
			AfxMessageBox(s);
			return;
		}
	}
	m_HelpInit = TRUE;
}

void CWaveShopApp::WinHelpInternal(DWORD_PTR dwData, UINT nCmd)
{
	WinHelp(static_cast<DWORD>(dwData), nCmd);	// route to our WinHelp override
}

bool CWaveShopApp::HandleDlgKeyMsg(MSG* pMsg)
{
	static const LPCSTR	EditBoxCtrlKeys = "ACHVX";	// Z reserved for app undo
	CMainFrame	*Main = theApp.GetMain();
	ASSERT(Main != NULL);	// main frame must exist
	switch (pMsg->message) {
	case WM_KEYDOWN:
		{
			int	VKey = INT64TO32(pMsg->wParam);
			bool	bTryMainAccels = FALSE;	// assume failure
			if ((VKey >= VK_F1 && VKey <= VK_F24) || VKey == VK_ESCAPE) {
				bTryMainAccels = TRUE;	// function key or escape
			} else {
				bool	IsAlpha = VKey >= 'A' && VKey <= 'Z';
				CEdit	*pEdit = CFocusEdit::GetEdit();
				if (pEdit != NULL) {	// if an edit control has focus
					if ((IsAlpha									// if (alpha key
					&& strchr(EditBoxCtrlKeys, VKey) == NULL		// and unused by edit
					&& (GetKeyState(VK_CONTROL) & GKS_DOWN)))		// and Ctrl is down)
						bTryMainAccels = TRUE;	// give main accelerators a try
				} else {	// non-edit control has focus
					if (IsAlpha										// if alpha key
					|| VKey == VK_SPACE								// or space key
					|| (GetKeyState(VK_CONTROL) & GKS_DOWN)			// or Ctrl is down
					|| (GetKeyState(VK_SHIFT) & GKS_DOWN))			// or Shift is down
						bTryMainAccels = TRUE;	// give main accelerators a try
				}
			}
			if (bTryMainAccels) {
				HACCEL	hAccel = Main->GetAccelTable();
				if (hAccel != NULL
				&& TranslateAccelerator(Main->m_hWnd, hAccel, pMsg))
					return(TRUE);	// message was translated, stop dispatching
			}
		}
		break;
	case WM_SYSKEYDOWN:
		Main->SetFocus();	// main frame must have focus to display menus
		Main->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);	// enter menu mode
		return(TRUE);	// message was translated, stop dispatching
	}
	return(FALSE);	// continue dispatching
}

// By default, CWinApp::OnIdle is called after WM_TIMER messages.  This isn't
// normally a problem, but if the application uses a short timer, OnIdle will
// be called frequently, seriously degrading performance.  Performance can be
// improved by overriding IsIdleMessage to return FALSE for WM_TIMER messages,
// which prevents them from triggering OnIdle.  This technique can be applied
// to any idle-triggering message that repeats frequently, e.g. WM_MOUSEMOVE.
//
BOOL CWaveShopApp::IsIdleMessage(MSG* pMsg)
{
	if (CWinApp::IsIdleMessage(pMsg)) {
		switch (pMsg->message) {	// don't call OnIdle after these messages
		case WM_TIMER:
			return(FALSE);
		default:
			return(TRUE);
		}
	} else
		return(FALSE);
}

BOOL CWaveShopApp::ProcessMessageFilter(int code, LPMSG lpMsg) 
{
	// If a menu is displayed while an edit control has focus, the message loop
	// pauses periodically until the menu is closed; this applies to all menus,
	// including context and system menus, and it's a problem for timer-driven
	// apps that use edit controls.  The problem is caused by the undocumented
	// WM_SYSTIMER message (0x118), which Windows uses internally for various
	// purposes including scrolling and blinking the caret in an edit control.
	// The solution is to suppress WM_SYSTIMER, but only if the filter code is
	// MSGF_MENU, otherwise the caret won't blink while scrolling.  The caret
	// doesn't blink while a menu is displayed even without this workaround.
	//
	// if displaying a menu and message is WM_SYSTIMER
	if (code == MSGF_MENU && lpMsg->message == 0x118) {
		// use GetClassName because IsKindOf fails if the edit control doesn't
		// have a CEdit instance; see Microsoft knowledge base article Q145616
		TCHAR	szClassName[6];
		if (GetClassName(lpMsg->hwnd, szClassName, 6)
		&& !_tcsicmp(szClassName, _T("Edit"))) {	// if recipient is an edit control
			return TRUE;	// suppress WM_SYSTIMER
		}
	}
	return CWinApp::ProcessMessageFilter(code, lpMsg);
}

bool CWaveShopApp::GetDLLFunctions(CDLLWrap& Lib, LPCTSTR LibPath, const int *OrdinalTbl, int Functions, CPtrArray& FuncPtr)
{
	if (!Lib.LoadLibrary(LibPath)) {	// if we can't load DLL
		CString	msg;
		AfxFormatString2(msg, IDS_CKUP_CANT_LOAD_DLL, LibPath, GetLastErrorString());
		AfxMessageBox(msg);
		return(FALSE);
	}
	FuncPtr.SetSize(Functions);
	for (int iFunc = 0; iFunc < Functions; iFunc++) {	// for each function
		int	ordinal = OrdinalTbl[iFunc];
		LPVOID	pFunc = Lib.GetProcAddress((LPCTSTR)ordinal);
		if (pFunc == NULL) {	// if we can't get address
			CString	msg;
			msg.Format(IDS_DLL_CANT_GET_FUNCTION, ordinal, LibPath, GetLastErrorString());
			AfxMessageBox(msg);
			return(FALSE);
		}
		FuncPtr[iFunc] = pFunc;	// store function address in array
	}
	return(TRUE);
}

bool CWaveShopApp::PromptForInputFiles(CStringArray& Path, int TitleID)
{
	CString	Filter;
	CDocManagerEx::GetFileFilter(Filter, TRUE);
	CString	Title((LPCTSTR)TitleID);
	CFileDialog	fd(TRUE, WAV_EXT, NULL, OFN_HIDEREADONLY, Filter);
	fd.m_ofn.lpstrTitle = Title;
	fd.m_ofn.Flags |= OFN_ALLOWMULTISELECT;	// enable multiple selection
	const int	BUFSIZE = 0x7fff;	// reserve plenty of space for paths
	CString	Buffer;
	LPTSTR	FileBuf = Buffer.GetBufferSetLength(BUFSIZE);
	ZeroMemory(FileBuf, BUFSIZE);	// must zero buffer before use
	fd.m_ofn.lpstrFile = FileBuf;
	fd.m_ofn.nMaxFile = BUFSIZE;
	if (fd.DoModal() != IDOK)	// if file not selected
		return(FALSE);
	POSITION	pos = fd.GetStartPosition();
	while (pos != NULL)	// for each file path
		Path.Add(fd.GetNextPathName(pos));	// add to path array
	CSortStringArray::Sort(Path);	// sort path array
	return(TRUE);
}

void CWaveShopApp::InitDialogCaptionView(CDialog& Dlg, const CWaveShopView *View)
{
	Dlg.SetIcon(theApp.LoadIcon(IDR_MAINFRAME), 0);
	if (View != NULL) {
		CString	s;
		Dlg.GetWindowText(s);
		Dlg.SetWindowText(s + _T(" - ") + View->GetDocument()->GetTitle());
	}
}

FILE *CWaveShopApp::OpenTempStream(LPCTSTR Path)
{
	// create temporary file; automatically delete on close
	HANDLE	hFile = CreateFile(Path,
		GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (hFile == INVALID_HANDLE_VALUE)	// if create failed
		return(NULL);
	// associate C run-time file handle with OS file handle
	int hCRTFile = _open_osfhandle((long)hFile, 0);
	if (hCRTFile < 0)	// if association failed
		return(NULL);
	// convert C run-time file handle to stream
	return(_fdopen(hCRTFile, "w+"));	// read/write access
}

bool CWaveShopApp::BrowseFolder(int TitleID, CString& Path, HWND OwnerWnd)
{
	CString	Title((LPCTSTR)TitleID);
	bool	retc = CFolderDialog::BrowseFolder(
		Title, Path, NULL, BIF_NEWDIALOGSTYLE, Path, OwnerWnd);
	return(retc);
}

void CWaveShopApp::MakeAbsolutePath(CString& Path) const
{
	if (PathIsRelative(Path)) {	// if path is relative
		CPathStr	AbsPath;
#ifdef PORTABLE_APP	// if portable app
		AbsPath = GetAppFolder();	// make path relative to app folder
#else	// not portable app; make path relative to profile's App Data folder
		GetSpecialFolderPath(CSIDL_APPDATA, AbsPath);
#endif
		AbsPath.Append(Path);
		Path = AbsPath;
	}
}

void CWaveShopApp::CMyCommandLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
	if (!_tcsicmp(lpszParam, _T("r")))
		theApp.m_InitialRecord = TRUE;
	CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
}

/////////////////////////////////////////////////////////////////////////////
// CWaveShopApp message map

BEGIN_MESSAGE_MAP(CWaveShopApp, CWinAppEx)
	//{{AFX_MSG_MAP(CWaveShopApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_APP_HOME_PAGE, OnAppHomePage)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP_FINDER, CWinApp::OnHelp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaveShopApp message handlers

// App command to run the dialog
void CWaveShopApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CWaveShopApp::OnAppHomePage() 
{
	if (!CHyperlink::GotoUrl(CAboutDlg::HOME_PAGE_URL))
		AfxMessageBox(IDS_HLINK_CANT_LAUNCH);
}
