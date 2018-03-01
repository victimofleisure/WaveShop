// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
        01      24jan13	in SetNow, add Center flag
        02      26jan13	add check for updates DLL
        03      27jan13	in CheckForUpdates, use absolute path
        04      30jan13	in CheckForUpdates, add DLL wrapper
        05      31jan13	in CheckForUpdates, support portable build
        06      11feb13	add history bar
        07      02mar13	show audio format in status bar
        08      04mar13	in SetPlaying, fix playing at end of audio
		09      04mar13	in CPlay, add instance count
		10		07mar13	in CreateSizingBars, don't pass dock style to Create
		11      09mar13	add meter bar
		12		12mar13	add show channel names
		13      30mar13	add spectrum bar
		14      16apr13	make control bar indices public
        15      17apr13	add temporary files folder
		16		26apr13	move DX error reporting into player
		17		27apr13	add recording
		18		05may13	in OnTimer, close player if get position fails
		19		07may13	construct wave generator dialog on stack
		20		10may13	in SetPlaying, reset meter clipping
		21		17may13	add record parameters
		22		20may13	add OnWaveFormatChange
		23		24may13	add OnInitialRecord
		24		28jun13	add plugins
		25		10jul13	in CheckForUpdates, format error as string

		wave editor main frame
 
*/

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "WaveShop.h"
#include "MainFrm.h"
#include "WaveShopDoc.h"
#include "WaveShopView.h"
#include "Persist.h"
#include "afxpriv.h"	// needed for VerifyDockState
#include "ProgressDlg.h"
#include "ChildFrm.h"
#include "DocIter.h"
#include "FindDlg.h"
#include "PathStr.h"
#include "DLLWrap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

int CMainFrame::CPlay::m_Instances;

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_SAMPLE_BITS,
	ID_INDICATOR_CHANNELS,
	ID_INDICATOR_SAMPLE_RATE,
	ID_INDICATOR_POSITION,
};

#define MAKEBAROFS(bar) offsetof(CMainFrame, bar)

const CMainFrame::BAR_INFO CMainFrame::m_ToolbarInfo[TOOLBARS] = {
//	Offset							BarID						BarResID				CaptionID			DockStyle		LeftOf						InitShow
	{MAKEBAROFS(m_ToolBar),			AFX_IDW_TOOLBAR,			IDR_MAINFRAME,			IDS_TOOLBAR,		0,				0,							TRUE},
	{MAKEBAROFS(m_TransportBar),	MAKEBARID(CBI_TRANSPORT),	IDR_TRANSPORT,			IDS_TRANSPORT,		0,				MAKEBAROFS(m_ToolBar),		TRUE},
};

const CMainFrame::BAR_INFO CMainFrame::m_DlgBarInfo[DIALOG_BARS] = {
//	Offset							BarID						BarResID				CaptionID			DockStyle		LeftOf						InitShow
	{MAKEBAROFS(m_NavBar),			MAKEBARID(CBI_NAV),			IDD_NAV_BAR,			IDD_NAV_BAR,		0,				MAKEBAROFS(m_TransportBar),	TRUE},
	{MAKEBAROFS(m_VolumeBar),		MAKEBARID(CBI_VOLUME),		IDD_VOLUME_BAR,			IDD_VOLUME_BAR,		0,				MAKEBAROFS(m_NavBar),		TRUE},
	{MAKEBAROFS(m_PitchBar),		MAKEBARID(CBI_PITCH),		IDD_PITCH_BAR,			IDD_PITCH_BAR,		0,				0,							FALSE},
};

const CMainFrame::BAR_INFO CMainFrame::m_SizingBarInfo[SIZING_BARS] = {
//	Offset							BarID						BarResID				CaptionID			DockStyle		LeftOf						InitShow
	{MAKEBAROFS(m_ResultsBar),		MAKEBARID(CBI_RESULTS),		AFX_IDW_DOCKBAR_RIGHT,	IDS_RESULTS_BAR,	CBRS_ALIGN_ANY,	0,							FALSE},
	{MAKEBAROFS(m_HistoryBar),		MAKEBARID(CBI_HISTORY),		AFX_IDW_DOCKBAR_LEFT,	IDS_HISTORY_BAR,	CBRS_ALIGN_ANY,	0,							FALSE},
	{MAKEBAROFS(m_MeterBar),		MAKEBARID(CBI_METER),		AFX_IDW_DOCKBAR_RIGHT,	IDS_METER_BAR,		CBRS_ALIGN_ANY,	0,							FALSE},
	{MAKEBAROFS(m_SpectrumBar),		MAKEBARID(CBI_SPECTRUM),	AFX_IDW_DOCKBAR_BOTTOM,	IDS_SPECTRUM_BAR,	CBRS_ALIGN_ANY,	0,							FALSE},
};

#define RK_MAIN_FRAME		_T("MainFrame")
#define RK_SHOW_CHANNEL_BAR	_T("ShowChannelBar")

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame() :
	m_Options(COptionsInfo::m_Defaults)
{
	m_WasShown = FALSE;
	m_ShowChannelBar = theApp.RdRegBool(RK_SHOW_CHANNEL_BAR, TRUE);
	m_View = NULL;
	m_PlayerView = NULL;
	ZeroMemory(&m_WaveGenParms, sizeof(m_WaveGenParms));
	m_OptionsPage = 0;
	ZeroMemory(&m_FindInfo, sizeof(m_FindInfo));
	FillMemory(&m_IndicatorCache, sizeof(m_IndicatorCache), -1);
	m_RecordDlg = NULL;
	m_Options.Load();
}

CMainFrame::~CMainFrame()
{
	theApp.WrRegBool(RK_SHOW_CHANNEL_BAR, m_ShowChannelBar);
	m_Options.Store();
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_StatusBar.Create(this) || 
	!m_StatusBar.SetIndicators(indicators, _countof(indicators)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	EnableDocking(CBRS_ALIGN_ANY);
	m_pFloatingFrameClass = RUNTIME_CLASS(CSCBMiniDockFrameWnd);	// custom dock frame
	if (!CreateToolbars()) {
		TRACE0("Failed to create tool bar\n");
		return -1;      // fail to create
	}
	ShowButton(m_TransportBar, ID_TRANSPORT_PAUSE, FALSE);	// avoids gap after bar
	if (!CreateDialogBars()) {
		TRACE0("Failed to create dialog bar\n");
		return -1;      // fail to create
	}
	if (!CreateSizingBars()) {
		TRACE0("Failed to create sizing control bar\n");
		return -1;      // fail to create
	}
	if (VerifyBarState(REG_SETTINGS)) {
		for (int iBar = 0; iBar < SIZING_BARS; iBar++) {
			CSizingControlBarG	*pBar = STATIC_DOWNCAST(CSizingControlBarG, 
				GetBarAtOffset(m_SizingBarInfo[iBar].Offset));
			pBar->LoadState(REG_SETTINGS);
		}
		LoadBarState(REG_SETTINGS);
	}
	SetTimer(TIMER_ID, 1000 / TIMER_FREQUENCY, NULL);	// start timer
	DragAcceptFiles();
	SetCursorWavePos(_T(""));
	ApplyOptions(COptionsInfo::m_Defaults);
	if (m_Options.m_CheckForUpdates)	// if automatically checking for updates
		AfxBeginThread(CheckForUpdatesThreadFunc, this);	// launch thread to check
	AfxBeginThread(IteratePluginsThread, this);

	return 0;
}

bool CMainFrame::CreateToolbars()
{
	DWORD	style = WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC;
	CRect	rc(0, 0, 0, 0);
	for (int iBar = 0; iBar < TOOLBARS; iBar++) {
		const BAR_INFO&	info = m_ToolbarInfo[iBar];
		CToolBar	*pBar = STATIC_DOWNCAST(CToolBar, GetBarAtOffset(info.Offset));
		if (!pBar->CreateEx(this, TBSTYLE_FLAT, style, rc, info.BarID))
			return(FALSE);
		if (!pBar->LoadToolBar(info.BarResID))
			return(FALSE);
		pBar->SetWindowText(LDS(info.CaptionID));
		pBar->EnableDocking(CBRS_ALIGN_ANY);
		if (info.LeftOf) {
			CControlBar	*LeftOf = GetBarAtOffset(info.LeftOf);
			CWinAppEx::DockControlBarLeftOf(this, pBar, LeftOf);
		} else
			DockControlBar(pBar);
	}
	return(TRUE);
}

bool CMainFrame::CreateDialogBars()
{
	DWORD	DockStyle = CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM;
	for (int iBar = 0; iBar < DIALOG_BARS; iBar++) {
		const BAR_INFO&	info = m_DlgBarInfo[iBar];
		CDialogBarEx	*pBar = STATIC_DOWNCAST(CDialogBarEx, GetBarAtOffset(info.Offset));
		CControlBar	*LeftOf;
		if (info.LeftOf)
			LeftOf = GetBarAtOffset(info.LeftOf);
		else
			LeftOf = NULL;
		if (!pBar->Create(this, info.BarResID, 0, DockStyle, info.BarID, LeftOf))
			return(FALSE);
		ShowControlBar(pBar, info.InitShow, 0);
	}
	return(TRUE);
}

bool CMainFrame::CreateSizingBars()
{
	for (int iBar = 0; iBar < SIZING_BARS; iBar++) {
		const BAR_INFO&	info = m_SizingBarInfo[iBar];
		CSizingControlBarG	*pBar = STATIC_DOWNCAST(
			CSizingControlBarG, GetBarAtOffset(info.Offset));
		CString	title((LPCTSTR)info.CaptionID);
		UINT	style = WS_CHILD | WS_VISIBLE | CBRS_TOP;
		if (!pBar->Create(title, this, info.BarID, style))
			return(FALSE);
		UINT	BarStyle = CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC;
		pBar->SetBarStyle(pBar->GetBarStyle() | BarStyle);
		pBar->EnableDocking(info.DockStyle);
		DockControlBar(pBar, info.BarResID);
		ShowControlBar(pBar, info.InitShow, 0);
	}
	return(TRUE);
}

inline CControlBar *CMainFrame::GetBarAtOffset(int Offset)
{
	CObject	*pObj = reinterpret_cast<CObject *>(LPBYTE(this) + Offset);
	return(STATIC_DOWNCAST(CControlBar, pObj));
}

BOOL CMainFrame::VerifyBarState(LPCTSTR lpszProfileName)
{
	CDockState	state;
	state.LoadState(lpszProfileName);
	return(VerifyDockState(state, this));
}

BOOL CMainFrame::VerifyDockState(const CDockState& state, CFrameWnd *Frm)
{
	// thanks to Cristi Posea at codeproject.com
	for (int i = 0; i < state.m_arrBarInfo.GetSize(); i++) {
		CControlBarInfo* pInfo = (CControlBarInfo*)state.m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		int nDockedCount = INT64TO32(pInfo->m_arrBarID.GetSize());
		if (nDockedCount > 0) {
			// dockbar
			for (int j = 0; j < nDockedCount; j++)
			{
				UINT	nID = (UINT) pInfo->m_arrBarID[j];
				if (nID == 0)
					continue; // row separator
				if (nID > 0xFFFF)
					nID &= 0xFFFF; // placeholder - get the ID
				if (Frm->GetControlBar(nID) == NULL)
					return FALSE;
			}
		}
		if (!pInfo->m_bFloating) // floating dockbars can be created later
			if (Frm->GetControlBar(pInfo->m_nBarID) == NULL)
				return FALSE; // invalid bar ID
	}
    return TRUE;
}

void CMainFrame::OnActivateView(CWaveShopView *View)
{
	m_View = View;
	if (!IsPlaying())
		UpdatePlayerState();
	m_NavBar.OnActivateView(View);
	if (m_HistoryBar.IsWindowVisible())	// if history bar is visible
		m_HistoryBar.UpdateList();
	if (m_MeterBar.IsWindowVisible())	// if meter bar is visible
		m_MeterBar.UpdateView();
	if (m_SpectrumBar.IsWindowVisible())	// if spectrum bar is visible
		m_SpectrumBar.UpdateView();
}

void CMainFrame::OnDestroyView(CWaveShopView *View)
{
	if (View == m_PlayerView)	// if doomed view owns player
		Close();
	m_ResultsBar.OnDestroyView(View);
}

CUndoManager *CMainFrame::GetUndoManager()
{
	if (m_View == NULL)
		return(NULL);
	return(&m_View->GetDocument()->m_UndoMgr);
}

void CMainFrame::OnUpdateUndoTitles()
{
	if (m_HistoryBar.IsWindowVisible())	// if history bar is visible
		m_HistoryBar.UpdateList();	// update its list
}

void CMainFrame::OnWaveFormatChange()
{
	if (m_MeterBar.IsWindowVisible())	// if meter bar visible
		m_MeterBar.UpdateView();	// update meter bar view
	if (m_SpectrumBar.IsWindowVisible())	// if spectrum bar visible
		m_SpectrumBar.UpdateView();	// update spectrum bar view
}

bool CMainFrame::MakeWave(const WAVEGEN_PARMS& Parms)
{
	POSITION	pos = theApp.GetFirstDocTemplatePosition();
	CDocTemplate	*pTpl = theApp.GetNextDocTemplate(pos);
	CWaveShopDoc	*pDoc = DYNAMIC_DOWNCAST(CWaveShopDoc, pTpl->CreateNewDocument());
	if (pDoc == NULL)
		return(FALSE);
	bool	retc, canceled;
	{
		CProgressDlg	ProgDlg;
		if (!ProgDlg.Create())	// create progress dialog
			AfxThrowResourceException();
		ProgDlg.SetWindowText(LDS(IDS_MAIN_GENERATING_AUDIO));
		retc = CWaveGenDlg::MakeWave(Parms, pDoc->m_Wave, &ProgDlg);
		canceled = ProgDlg.Canceled();
	}	// destroy progress dialog
	if (!retc) {	// if generation failed
		if (!canceled)	// if user canceled
			AfxMessageBox(IDS_MAIN_CANT_MAKE_WAVE);
		return(FALSE);
	}
	CDocument	*pEmptyDoc = pTpl->OpenDocumentFile(NULL);	// create new view
	if (pEmptyDoc == NULL || m_View == NULL)
		return(FALSE);
	CString	title = pEmptyDoc->GetTitle();
	pEmptyDoc->RemoveView(m_View);	// remove empty document from view
	pDoc->SetTitle(title);	// copy empty document's title to generated document
	pDoc->AddView(m_View);	// add generated document to view
	m_View->OnInitialUpdate();
	OnActivateView(m_View);
	// view is still linked to empty document's undo manager; must relink
	m_View->SetUndoManager(&pDoc->m_UndoMgr);	// link view to undo manager
	pDoc->m_UndoMgr.SetRoot(m_View);	// link undo manager to view
	return(TRUE);
}

void CMainFrame::SetCursorWavePos(CString Pos)
{
	m_StatusBar.SetPaneText(SBP_CURSOR_WAVE_POS, Pos);
}

void CMainFrame::SetOptions(const COptionsInfo& Options)
{
	COptionsInfo	PrevOpts(m_Options);
	m_Options = Options;
	ApplyOptions(PrevOpts);
}

void CMainFrame::ApplyOptions(const COptionsInfo& PrevOpts)
{
	// if playback device or buffer size changed
	if (m_Options.m_PlayDeviceGuid != PrevOpts.m_PlayDeviceGuid
	|| m_Options.m_PlayBufferSize != PrevOpts.m_PlayBufferSize
	|| !m_Player.IsCreated()) {	// or player not created
		if (m_Player.Create(&m_Options.m_PlayDeviceGuid, m_hWnd)) {	// create player
			m_Player.SetBufferDuration(m_Options.m_PlayBufferSize);
			UpdatePlayerState();
		}
	}
	// if recording device or buffer size changed
	if (m_Options.m_RecordDeviceGuid != PrevOpts.m_RecordDeviceGuid
	|| m_Options.m_RecordBufferSize != PrevOpts.m_RecordBufferSize) {
		if (m_RecordDlg != NULL)	// if record dialog exists
			m_RecordDlg->OnDeviceChange();	// notify it of device change
	}
	if (m_Options.m_UndoLevels != PrevOpts.m_UndoLevels) {
		CAllDocIter	iter;
		CDocument	*pDoc;
		while ((pDoc = iter.GetNextDoc()) != NULL)
			((CWaveShopDoc *)pDoc)->m_UndoMgr.SetLevels(m_Options.m_UndoLevels);
	}
	LPARAM	Hint = CWaveShopView::HINT_NONE;
	if (m_Options.m_TimeInFrames != PrevOpts.m_TimeInFrames) {
		m_NavBar.OnTimeFormatChange();
		Hint |= CWaveShopView::HINT_TIME_UNIT;
	}
	if (m_Options.m_MaxDensity != PrevOpts.m_MaxDensity)
		Hint |= CWaveShopView::HINT_WAVE_UPDATE;
	if (m_Options.m_ShowChannelNames != PrevOpts.m_ShowChannelNames)
		Hint |= CWaveShopView::HINT_SHOW_CAPTIONS;
	theApp.UpdateAllViews(NULL, Hint, NULL);
	if (m_Options.m_CheckForUpdates && !PrevOpts.m_CheckForUpdates)
		PostMessage(WM_COMMAND, ID_APP_CHECK_FOR_UPDATES);
	if (memcmp(&m_Options.m_RTSA, &PrevOpts.m_RTSA, sizeof(RTSA_PARMS)))
		m_SpectrumBar.SetParms(m_Options.m_RTSA);
	theApp.m_TempFolderPath = m_Options.GetTempFolderPath();
	RegisterHotKeys();
}

void CMainFrame::ShowChannelBar(bool Enable)
{
	CMDIChildIter	iter;
	CMDIChildWnd	*pChild;
	while ((pChild = iter.GetNextChild()) != NULL)
		((CChildFrame *)pChild)->ShowChannelBar(Enable);
	m_ShowChannelBar = Enable;
}

void CMainFrame::SetNow(double Now, bool Center)
{
	if (m_View != NULL) {	// if active view exists
		W64INT	frames = m_View->GetWave().GetFrameCount();
		Now = CLAMP(Now, 0, frames);
		if (ActivePlayer())	// if active view owns player
			SetPosition(Now);	// set player position
		if (Center)
			m_View->EnsureVisible(Now);
		m_View->SetNow(Now);
		m_NavBar.SetNow(Now);
	}
}

void CMainFrame::SetSelection(const CDblRange& Sel)
{
	if (m_View != NULL) {	// if active view exists
		CDblRange	SafeSel = Sel;
		W64INT	frames = m_View->GetWave().GetFrameCount();
		SafeSel.Start = CLAMP(SafeSel.Start, 0, frames);
		SafeSel.End = CLAMP(SafeSel.End, 0, frames);
		if (ActivePlayer())	// if active view owns player
			SetLoopPoints(SafeSel);	// set player loop points
		m_View->SetSelection(SafeSel);
		m_NavBar.SetSelection(SafeSel);
	}
}

void CMainFrame::ShowButton(CToolBar& ToolBar, int CmdID, bool Enable)
{
	TBBUTTONINFO	tbi;
    tbi.cbSize = sizeof(tbi);
    tbi.dwMask = TBIF_STATE;
    ToolBar.SendMessage(TB_GETBUTTONINFO, CmdID, (LPARAM)&tbi);	// get button state
	BYTE	PrevState = tbi.fsState;
	if (Enable)	// if showing button
		tbi.fsState &= ~TBSTATE_HIDDEN;	// unhide it
	else
		tbi.fsState |= TBSTATE_HIDDEN;	// hide it
	if (tbi.fsState != PrevState)	// if button state changed
		ToolBar.SendMessage(TB_SETBUTTONINFO, CmdID, (LPARAM)&tbi);	// set button state
}

bool CMainFrame::SetVolume(double Volume)
{
	return(m_Player.SetVolume(Volume));
}

bool CMainFrame::SetFrequency(double Frequency)
{
	return(m_Player.SetFrequency(Frequency));
}

bool CMainFrame::SetPosition(double Pos)
{
	return(m_Player.SetPosition(roundW64INT(Pos)));
}

bool CMainFrame::SetRepeat(bool Enable)
{
	return(m_Player.SetRepeat(Enable));
}

bool CMainFrame::SetLoopPoints(const CDblRange& Sel)
{
	W64INT	start, end;
	if (Sel.Start < Sel.End) {	// if selection is valid loop
		start = roundW64INT(Sel.Start);
		end = roundW64INT(Sel.End);
	} else {	// invalid or empty loop
		start = 0;	// set loop to entire wave
		end = m_View->GetWave().GetFrameCount();
	}
	return(m_Player.SetLoopPoints(start, end));
}

bool CMainFrame::Seek(double Frame)
{
	if (!IsPlaying() || ActivePlayer()) {	// if stopped, or active view owns player
		SetNow(Frame);	// normal case
	} else {	// player is playing but owned by inactive view
		if (!SetPosition(Frame))	// just set player position
			return(FALSE);
	}
	return(TRUE);
}

bool CMainFrame::UpdatePlayerState()
{
	CWaveShopView	*View = m_View;
	m_PlayerView = View;	// update player view first in case of early exit
	if (View == NULL)
		return(FALSE);
	CWave&	wave = View->GetWave();
	if (!wave.IsValid()) {	// if wave is invalid
		Close();	// close previous wave if any
		return(FALSE);
	}
	if (!Open(wave))	// open wave
		return(FALSE);
	if (!SetVolume(m_VolumeBar.GetVolume()))	// set volume
		return(FALSE);
	if (!SetFrequency(m_PitchBar.GetPitch()))	// set pitch
		return(FALSE);
	CDblRange	sel = View->GetSelection();
	if (!SetLoopPoints(sel))	// set loop points
		return(FALSE);
	if (!SetPosition(View->GetNow()))	// set position
		return(FALSE);
	return(TRUE);
}

bool CMainFrame::Open(CWave& Wave)
{
	return(m_Player.Open(Wave));
}

bool CMainFrame::Close()
{
	m_PlayerView = NULL;
	return(m_Player.Close());
}

bool CMainFrame::SetPlaying(bool Enable)
{
	if (Enable == IsPlaying())	// if already in requested state
		return(TRUE);	// nothing to do
	if (m_View == NULL)
		return(FALSE);	// nothing to play
	if (Enable) {	// if playing
		if (!ActivePlayer()) {	// if active view doesn't own player
			if (!UpdatePlayerState())	// take ownership before playing
				return(FALSE);
		}
		// if cursor is at end of audio
		if (m_View->GetIntNow() >= m_View->GetWave().GetFrameCount())
			m_Player.Rewind();	// rewind so non-repeat play works
		if (m_MeterBar.IsWindowVisible())
			m_MeterBar.ResetClipping();
	}
	return(m_Player.SetPlaying(Enable));
}

bool CMainFrame::Stop()
{
	if (!SetPlaying(FALSE))
		return(FALSE);
	if (!m_Player.Rewind())
		return(FALSE);
	if (ActivePlayer())	// if active view owns player
		m_NavBar.SetNow(0);
	if (m_PlayerView != NULL)
		m_PlayerView->SetNow(0);
	return(TRUE);
}

bool CMainFrame::IsViewPlaying(const CWaveShopView *View) const
{
	return(View != NULL && m_PlayerView != NULL
		&& View->GetDocument() == m_PlayerView->GetDocument());
}

CMainFrame::CPlay::CPlay(CWaveShopView *View, bool Enable)
{
	ASSERT(m_Instances >= 0);	// instance count should always be positive
	if (m_Instances++ > 0)	// post-increment; if nested instance
		return;	// defer to outermost instance; no operation
	m_View = View;	// save pointer to caller's view
	CMainFrame	*Main = theApp.GetMain();
	m_WasPlaying = Main->IsPlaying();
	ZeroMemory(&m_WaveFormat, sizeof(WAVEFORMATEXTENSIBLE));
	if (View != NULL) {
		if (!Main->IsViewPlaying(View))	// if caller doesn't own player
			return;	// leave player alone
		CWave&	PlayerWave = Main->GetPlayerView()->GetWave();
		PlayerWave.GetFormat(m_WaveFormat);	// save current audio format
	}
	Main->SetPlaying(Enable);	// set requested playing state
}

CMainFrame::CPlay::~CPlay()
{
	ASSERT(m_Instances >= 0);	// instance count should always be positive
	if (--m_Instances > 0)	// pre-decrement; if nested instance
		return;	// defer to outermost instance; no operation
	CMainFrame	*Main = theApp.GetMain();
	if (Main->IsViewPlaying(m_View)) {	// if caller owns player
		WAVEFORMATEXTENSIBLE	WaveFormat;
		CWave&	PlayerWave = Main->GetPlayerView()->GetWave();
		PlayerWave.GetFormat(WaveFormat);	// get current audio format
		// if audio format changed since our ctor saved it
		if (memcmp(&WaveFormat, &m_WaveFormat, sizeof(WAVEFORMATEXTENSIBLE))) {
			Main->Close();	// close audio and reset player view state
			if (PlayerWave.IsValid())	// if new audio is valid
				Main->Open(PlayerWave);		// reopen audio in new format
		}
	} else {	// caller doesn't own player
		if (Main->GetPlayerView() == NULL	// if player is unowned
		&& m_View != NULL && m_View->GetWave().IsValid())	// and caller has valid wave
			Main->Open(m_View->GetWave());	// open caller's audio
	}
	Main->SetPlaying(m_WasPlaying);	// restore previous playing state
}

bool CMainFrame::Find(CWaveProcess::FIND_SAMPLE_INFO& FindInfo, bool First)
{
	if (m_View == NULL)
		return(FALSE);
	if (First) {	// if initial find
		CFindDlg	dlg;
		if (dlg.DoModal() != IDOK)	// display find dialog
			return(FALSE);
		m_FindInfo.ChannelIdx = dlg.m_Channel;
		CWaveProcess::CSampleRange	target = dlg.GetTargetRange();
		m_FindInfo.TargetStart = target.Start;
		m_FindInfo.TargetEnd = target.End;
		m_FindInfo.StartFrame = m_View->GetIntNow();	// start searching from now
		m_FindInfo.Flags = 0;
		if (dlg.m_Wrap)
			m_FindInfo.Flags |= CWaveProcess::FIND_SAMPLE_INFO::WRAP;
		if (dlg.m_Match)
			m_FindInfo.Flags |= CWaveProcess::FIND_SAMPLE_INFO::INVERT;
		if (dlg.m_Dir)
			m_FindInfo.Flags |= CWaveProcess::FIND_SAMPLE_INFO::REVERSE;
		if (dlg.m_Unit == CWaveProcess::CConvert::DECIBELS)	// if target in dB
			m_FindInfo.Flags |= CWaveProcess::FIND_SAMPLE_INFO::ABS_VAL;
	} else {	// find next
		W64INT	now = m_View->GetIntNow();
		// if cursor still on previous matching frame
		if (now == m_FindInfo.MatchFrame) {
			if (m_FindInfo.Flags & CWaveProcess::FIND_SAMPLE_INFO::REVERSE)	// if reverse
				m_FindInfo.StartFrame = now - 1;	// start one sample before cursor
			else	// forward
				m_FindInfo.StartFrame = now + 1;	// start one sample after cursor
		} else	// cursor was moved
			m_FindInfo.StartFrame = now;	// start at cursor
	}
	if (!m_View->GetWave().FindSample(m_FindInfo))	// find matching sample
		return(FALSE);
	FindInfo = m_FindInfo;	// return find info to caller
	return(TRUE);
}

bool CMainFrame::Record(bool Enable)
{
	bool	retc = TRUE;	// assume success
	if (m_RecordDlg != NULL) {	// if record dialog exists
		m_RecordDlg->Show();	// show and focus record dialog
		if (!Enable) {	// if stop requested
			if (m_RecordDlg->IsRecording())	// if recording
				retc = m_RecordDlg->Stop();	// stop recording
		}
	} else {	// record dialog doesn't exist
		if (Enable) {	// if start requested
			m_RecordDlg = CRecordDlg::Record();	// create record dialog
			retc = m_RecordDlg != NULL;	// true if dialog created
		}
	}
	return(retc);
}

void CMainFrame::RegisterHotKeys()
{
	for (int iHotKey = 0; iHotKey < RECORD_PARMS::HOT_KEYS; iHotKey++) {
		DWORD	def = m_Options.m_Record.HotKeys.Def[iHotKey];
		if (def)
			RegisterHotKey(m_hWnd, iHotKey, HIWORD(def), LOWORD(def));
		else
			UnregisterHotKey(m_hWnd, iHotKey);
	}
}

void CMainFrame::UnregisterHotKeys()
{
	for (int iHotKey = 0; iHotKey < RECORD_PARMS::HOT_KEYS; iHotKey++)
		UnregisterHotKey(m_hWnd, iHotKey);
}

bool CMainFrame::CheckForUpdates(bool Explicit)
{
	enum {	// update check flags
		UF_EXPLICIT	= 0x01,	// explicit check (as opposed to automatic)
		UF_X64		= 0x02,	// target application is 64-bit
		UF_PORTABLE	= 0x04,	// target application is portable (no installer)
	};
	CPathStr	DLLPath(theApp.GetAppFolder());
	DLLPath.Append(_T("CKUpdate.dll"));
	CDLLWrap	dll;
	if (!dll.LoadLibrary(DLLPath)) {	// if we can't load DLL
		if (Explicit) {
			CString	msg;
			AfxFormatString2(msg, IDS_CKUP_CANT_LOAD_DLL, DLLPath,
				CWinAppEx::GetLastErrorString());
			AfxMessageBox(msg);
		}
		return(FALSE);
	}
	LPCTSTR	ProcName = _T("CKUpdate");
	CKUPDATE_PTR	CKUpdate = (CKUPDATE_PTR)dll.GetProcAddress(ProcName);
	if (CKUpdate == NULL) {	// if we can't get address
		if (Explicit) {
			CString	msg;
			AfxFormatString2(msg, IDS_CKUP_CANT_GET_ADDR, ProcName,
				CWinAppEx::GetLastErrorString());
			AfxMessageBox(msg);
		}
		return(FALSE);
	}
	UINT	flags = 0;
	if (Explicit)
		flags |= UF_EXPLICIT;	// explicit check (as opposed to automatic)
#ifdef _WIN64
	flags |= UF_X64;	// target application is 64-bit
#endif
#ifdef PORTABLE_APP
	flags |= UF_PORTABLE;	// target application is portable (no installer)
#endif
	// if this app uses Unicode, the CKUpdate DLL must also use Unicode,
	// else CKUpdate only receives the first character of TargetAppName.
	UINT	retc = CKUpdate(m_hWnd, theApp.m_pszAppName, flags);
	return(retc != 0);
}

UINT CMainFrame::CheckForUpdatesThreadFunc(LPVOID Param)
{
	CMainFrame	*pMain = (CMainFrame *)Param;
	TRY {
		Sleep(1000);	// give app a chance to finish initializing
		pMain->CheckForUpdates(FALSE);	// automatic check
	}
	CATCH (CException, e) {
		e->ReportError();
	}
	END_CATCH
	return(0);
}

UINT CMainFrame::IteratePluginsThread(LPVOID Param)
{
	CMainFrame	*pMain = (CMainFrame *)Param;
	TRY {
		if (pMain->m_PluginMgr.IteratePlugins())	// if any plugins found
			pMain->PostMessage(UWM_PLUGINSFOUND);	// post notification
	}
	CATCH (CException, e) {
		e->ReportError();
	}
	END_CATCH
	return(0);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message map

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_COMMAND(ID_APP_CHECK_FOR_UPDATES, OnAppCheckForUpdates)
	ON_COMMAND(ID_AUDIO_GENERATE, OnAudioGenerate)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_EDIT_OPTIONS, OnEditOptions)
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_COMMAND(ID_TRANSPORT_LOOP, OnTransportLoop)
	ON_COMMAND(ID_TRANSPORT_PAUSE, OnTransportPlay)
	ON_COMMAND(ID_TRANSPORT_PLAY, OnTransportPlay)
	ON_COMMAND(ID_TRANSPORT_RECORD, OnTransportRecord)
	ON_COMMAND(ID_TRANSPORT_REWIND, OnTransportRewind)
	ON_COMMAND(ID_TRANSPORT_STOP, OnTransportStop)
	ON_UPDATE_COMMAND_UI(ID_TRANSPORT_LOOP, OnUpdateTransportLoop)
	ON_UPDATE_COMMAND_UI(ID_TRANSPORT_PAUSE, OnUpdateTransportPlay)
	ON_UPDATE_COMMAND_UI(ID_TRANSPORT_PLAY, OnUpdateTransportPlay)
	ON_UPDATE_COMMAND_UI(ID_TRANSPORT_RECORD, OnUpdateTransportRecord)
	ON_UPDATE_COMMAND_UI(ID_TRANSPORT_REWIND, OnUpdateTransportRewind)
	ON_UPDATE_COMMAND_UI(ID_TRANSPORT_STOP, OnUpdateTransportStop)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CHANNEL_BAR, OnUpdateViewChannelBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HISTORY_BAR, OnUpdateViewHistoryBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_METER_BAR, OnUpdateViewMeterBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NAVIGATION_BAR, OnUpdateViewNavigationBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PITCH_BAR, OnUpdateViewPitchBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RESULTS_BAR, OnUpdateViewResultsBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SPECTRUM_BAR, OnUpdateViewSpectrumBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TRANSPORT_BAR, OnUpdateViewTransportBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VOLUME_BAR, OnUpdateViewVolumeBar)
	ON_COMMAND(ID_VIEW_CHANNEL_BAR, OnViewChannelBar)
	ON_COMMAND(ID_VIEW_HISTORY_BAR, OnViewHistoryBar)
	ON_COMMAND(ID_VIEW_METER_BAR, OnViewMeterBar)
	ON_COMMAND(ID_VIEW_NAVIGATION_BAR, OnViewNavigationBar)
	ON_COMMAND(ID_VIEW_PITCH_BAR, OnViewPitchBar)
	ON_COMMAND(ID_VIEW_RESULTS_BAR, OnViewResultsBar)
	ON_COMMAND(ID_VIEW_SPECTRUM_BAR, OnViewSpectrumBar)
	ON_COMMAND(ID_VIEW_TRANSPORT_BAR, OnViewTransportBar)
	ON_COMMAND(ID_VIEW_VOLUME_BAR, OnViewVolumeBar)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SAMPLE_BITS, OnUpdateIndicatorSampleBits)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CHANNELS, OnUpdateIndicatorChannels)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SAMPLE_RATE, OnUpdateIndicatorSampleRate)
	ON_MESSAGE(UWM_HANDLEDLGKEY, OnHandleDlgKey)
	ON_MESSAGE(UWM_MODELESS_DESTROY, OnModelessDestroy)
	ON_MESSAGE(UWM_INITIAL_RECORD, OnInitialRecord)
	ON_MESSAGE(UWM_PLUGINSFOUND, OnPluginsFound)
	ON_MESSAGE(WM_HOTKEY, OnHotKey)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnClose() 
{
	if (IsRecording() && !m_RecordDlg->StopCheck())
		return;
	CMDIFrameWnd::OnClose();
}

void CMainFrame::OnDestroy() 
{
	CPersist::SaveWnd(REG_SETTINGS, this, RK_MAIN_FRAME);
	m_Player.Destroy();
	KillTimer(TIMER_ID);
	CMDIFrameWnd::OnDestroy();
}

BOOL CMainFrame::DestroyWindow() 
{
	for (int iBar = 0; iBar < SIZING_BARS; iBar++) {
		CSizingControlBarG	*pBar = STATIC_DOWNCAST(CSizingControlBarG, 
			GetBarAtOffset(m_SizingBarInfo[iBar].Offset));
		pBar->SaveState(REG_SETTINGS);
	}
	SaveBarState(REG_SETTINGS);
	return CMDIFrameWnd::DestroyWindow();
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CMDIFrameWnd::OnShowWindow(bShow, nStatus);
	if (!m_WasShown && !IsWindowVisible()) {
		m_WasShown = TRUE;
		CPersist::LoadWnd(REG_SETTINGS, this, RK_MAIN_FRAME, CPersist::NO_MINIMIZE);
	}
}

void CMainFrame::OnUpdateIndicatorSampleBits(CCmdUI *pCmdUI)
{
	UINT	SampleBits;
	if (m_View != NULL)	// if active view exists
		SampleBits = m_View->GetWave().GetSampleBits();
	else
		SampleBits = 0;
	if (SampleBits != m_IndicatorCache.SampleBits) {	// if value changed
		CString	s;
		if (SampleBits)	// if valid wave
			s = CWaveEdit::GetSampleBitsString(SampleBits);	// get string
		pCmdUI->SetText(s);	// update indicator
		m_IndicatorCache.SampleBits = SampleBits;	// update cached value
	}
}

void CMainFrame::OnUpdateIndicatorChannels(CCmdUI *pCmdUI)
{
	UINT	Channels;
	if (m_View != NULL)	// if active view exists
		Channels = m_View->GetWave().GetChannels();
	else
		Channels = 0;
	if (Channels != m_IndicatorCache.Channels) {	// if value changed
		CString	s;
		if (Channels)	// if valid wave
			s = CWaveEdit::GetChannelCountString(Channels);	// get string
		pCmdUI->SetText(s);	// update indicator
		m_IndicatorCache.Channels = Channels;	// update cached value
	}
}

void CMainFrame::OnUpdateIndicatorSampleRate(CCmdUI *pCmdUI)
{
	UINT	SampleRate;
	if (m_View != NULL)	// if active view exists
		SampleRate = m_View->GetWave().GetSampleRate();
	else
		SampleRate = 0;
	if (SampleRate != m_IndicatorCache.SampleRate) {	// if value changed
		CString	s;
		if (SampleRate)	// if valid wave
			s = CWaveEdit::GetSampleRateString(SampleRate);	// get string
		pCmdUI->SetText(s);	// update indicator
		m_IndicatorCache.SampleRate = SampleRate;	// update cached value
	}
}

LRESULT	CMainFrame::OnHandleDlgKey(WPARAM wParam, LPARAM lParam)
{
	return(theApp.HandleDlgKeyMsg((MSG *)wParam));
}

LRESULT	CMainFrame::OnModelessDestroy(WPARAM wParam, LPARAM lParam)
{
	CWnd	*pWnd = reinterpret_cast<CWnd *>(wParam);
	if (pWnd == m_RecordDlg)
		m_RecordDlg = NULL;	// mark record dialog destroyed
	return(0);
}

LRESULT	CMainFrame::OnInitialRecord(WPARAM wParam, LPARAM lParam)
{
	// if activation type is prompt, force one-touch recording
	if (m_Options.m_Record.ActivationType == CRecordDlg::ACT_PROMPT)
		m_Options.m_Record.ActivationType = CRecordDlg::ACT_ONE_TOUCH;
	Record(TRUE);	// start recording
	return(0);
}

LRESULT CMainFrame::OnPluginsFound(WPARAM wParam, LPARAM lParam)
{
	CDocTemplateIter	iter;
	CMultiDocTemplate	*tpl = (CMultiDocTemplate *)iter.GetNextDocTemplate();
	ASSERT(tpl != NULL);	// at least one document template must exist
	// insert plugin popup menu into main menu associated with document type
	HMENU	PluginMenu = m_PluginMgr.GetMenu();
	UINT	flags = MF_BYPOSITION | MF_POPUP;
	if (!InsertMenu(tpl->m_hMenuShared, 5, flags, int(PluginMenu), _T("&Plugin")))
		AfxThrowResourceException();
	if (m_View != NULL)	// if at least one document open
		DrawMenuBar();	// document menu must be showing, so repaint it
	return(0);
}

LRESULT	CMainFrame::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	if (m_Options.m_Record.ActivationType == CRecordDlg::ACT_PROMPT) {
		if (IsIconic())	// if minimized
			ShowWindow(SW_RESTORE);	// restore so prompts are visible
	}
	switch (wParam) {
	case RECORD_PARMS::HK_START:
		Record(TRUE);	// start recording
		break;
	case RECORD_PARMS::HK_STOP:
		if (IsRecording()) {
			m_RecordDlg->Stop(CRecordDlg::SF_DEFAULT	// stop recording
				& ~CRecordDlg::SF_OPEN_RECORDING);	// don't open recording
		}
		break;
	}
	return(0);
}

void CMainFrame::OnTimer(W64UINT nIDEvent) 
{
	BOOL	ShowingMeters = m_MeterBar.IsWindowVisible();
	BOOL	ShowingSpectrum = m_SpectrumBar.IsWindowVisible();
	if (IsPlaying()) {	// if player is playing
		W64INT	Frame;
		if (m_Player.GetPosition(Frame)) {
			double	fFrame = double(Frame);
			if (ActivePlayer()) {	// if active view owns player
				m_NavBar.SetNow(fFrame);	// update nav bar now
				if (ShowingMeters)
					m_MeterBar.TimerHook(Frame);
				if (ShowingSpectrum)
					m_SpectrumBar.TimerHook(Frame);
			}
			if (m_PlayerView != NULL)
				m_PlayerView->SetNow(fFrame);	// update playing view's now
			// if not looping and end of audio was reached
			if (!m_Player.GetRepeat() && Frame >= m_Player.GetWave()->GetFrameCount()) {
				Stop();	// stop playing
				// timer doesn't update UI for performance reasons; see IsIdleMessage
				m_TransportBar.OnUpdateCmdUI(this, FALSE);	// explictly update UI
			}
		} else	// can't get position
			Close();	// close player to avoid message infinite loop
	} else {	// not playing
		if (ShowingMeters)
			m_MeterBar.TimerHook(W64INT_MAX);	// reserved frame for hold decay
	}
	CMDIFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnEditOptions() 
{
	COptionsInfo	PrevOpts(m_Options);
	COptionsDlg	dlg(IDS_OPTIONS, m_Options, this, m_OptionsPage);
	if (dlg.DoModal() == IDOK) {
		m_OptionsPage = dlg.GetCurPage();	// save selected page index
		ApplyOptions(PrevOpts);	// apply edited options
	} else	// canceled
		m_Options = PrevOpts;	// restore previous options
}

void CMainFrame::OnViewChannelBar() 
{
	ShowChannelBar(!m_ShowChannelBar);
}

void CMainFrame::OnUpdateViewChannelBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ShowChannelBar);
}

void CMainFrame::OnAudioGenerate() 
{
	CWaveGenDlg	dlg;	// construct wave generator dialog on stack
	if (m_WaveGenParms.m_Channels)	// if generation parameters are valid
		dlg.SetParms(m_WaveGenParms);	// init dialog from parameters
	if (dlg.DoModal() == IDOK) {
		dlg.GetParms(m_WaveGenParms);	// get generation parameters from dialog
		MakeWave(m_WaveGenParms);	// generate wave according to parameters
	}
}

void CMainFrame::OnTransportPlay() 
{
	SetPlaying(!IsPlaying());
}

void CMainFrame::OnUpdateTransportPlay(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsOpen());
	pCmdUI->SetCheck(IsPlaying());
	ShowButton(m_TransportBar, ID_TRANSPORT_PLAY, !IsPlaying());
	ShowButton(m_TransportBar, ID_TRANSPORT_PAUSE, IsPlaying());
}

void CMainFrame::OnTransportStop() 
{
	Stop();
}

void CMainFrame::OnUpdateTransportStop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsOpen());
}

void CMainFrame::OnTransportLoop() 
{
	m_Player.SetRepeat(!m_Player.GetRepeat());
}

void CMainFrame::OnUpdateTransportLoop(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_Player.GetRepeat());
}

void CMainFrame::OnTransportRewind() 
{
	Seek(0);
}

void CMainFrame::OnUpdateTransportRewind(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsOpen());
}

void CMainFrame::OnTransportRecord() 
{
	Record(!IsRecording());
}

void CMainFrame::OnUpdateTransportRecord(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(IsRecording());
}

void CMainFrame::OnViewTransportBar() 
{
	ShowControlBar(&m_TransportBar, !m_TransportBar.IsWindowVisible(), 0);
}

void CMainFrame::OnUpdateViewTransportBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_TransportBar.IsWindowVisible());
}

void CMainFrame::OnViewNavigationBar() 
{
	ShowControlBar(&m_NavBar, !m_NavBar.IsWindowVisible(), 0);
}

void CMainFrame::OnUpdateViewNavigationBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_NavBar.IsWindowVisible());
}

void CMainFrame::OnViewVolumeBar() 
{
	ShowControlBar(&m_VolumeBar, !m_VolumeBar.IsWindowVisible(), 0);
}

void CMainFrame::OnUpdateViewVolumeBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_VolumeBar.IsWindowVisible());
}

void CMainFrame::OnViewPitchBar() 
{
	ShowControlBar(&m_PitchBar, !m_PitchBar.IsWindowVisible(), 0);
}

void CMainFrame::OnUpdateViewPitchBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_PitchBar.IsWindowVisible());
}

void CMainFrame::OnViewResultsBar() 
{
	ShowControlBar(&m_ResultsBar, !m_ResultsBar.IsWindowVisible(), 0);
}

void CMainFrame::OnUpdateViewResultsBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ResultsBar.IsWindowVisible());
}

void CMainFrame::OnViewHistoryBar() 
{
	BOOL	bShow = !m_HistoryBar.IsWindowVisible();
	ShowControlBar(&m_HistoryBar, bShow, 0);
	if (bShow)	// if showing bar
		m_HistoryBar.UpdateList();	// update list
}

void CMainFrame::OnUpdateViewHistoryBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_HistoryBar.IsWindowVisible());
}

void CMainFrame::OnViewMeterBar() 
{
	BOOL	bShow = !m_MeterBar.IsWindowVisible();
	ShowControlBar(&m_MeterBar, bShow, 0);
	if (bShow)	// if showing bar
		m_MeterBar.UpdateView();	// update view
}

void CMainFrame::OnUpdateViewMeterBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_MeterBar.IsWindowVisible());
}

void CMainFrame::OnViewSpectrumBar() 
{
	BOOL	bShow = !m_SpectrumBar.IsWindowVisible();
	ShowControlBar(&m_SpectrumBar, bShow, 0);
	if (bShow)	// if showing bar
		m_SpectrumBar.UpdateView();	// update view
}

void CMainFrame::OnUpdateViewSpectrumBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_SpectrumBar.IsWindowVisible());
}

void CMainFrame::OnAppCheckForUpdates() 
{
	CWaitCursor	wc;
	CheckForUpdates(TRUE);	// explicit check
}
