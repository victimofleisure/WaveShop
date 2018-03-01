// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
		01		28jan13	add GetNowXClampEnd
		02		30jan13	add OnNextPane and OnPrevPane
		03		07feb13	in OnSize, constrain scroll position
		04		07feb13	in GetNowXClampEnd, fix false positives
		05		11feb13	in undo handlers, throw wave failures
		06		11feb13	in InsertChannel, use progress read
        07      18feb13	in InsertChannel, use import
        08      25feb13	paste and insert should move cursor to end of selection
        09      25feb13	add Resample
        10      28feb13	add audio insert
		11		01mar13	in Insert and InsertChannel, add dual progress
		12		03mar13	in OnMouseMove, handle cursor at edge of screen
		13		04mar13	in FindClipping, add clipping level
		14		04mar13	in Paste, remove player open
		15		04mar13	in Insert and InsertChannel, add outer CPlay
		16		08mar13	in SetChannelMask, replace Open with UpdatePlayerState 
		17		09mar13	in UpdateChannelCount, notify meter bar
        18      12mar13	add channel captions
        19      19mar13	add spectrum dialog
        20      30mar13	add spectrum bar
        21      16apr13	rename insert silence dialog
		22		20may13	fix format change handling
        23		04jun13	add channel selection
        24		17jun13	in OnDraw, skip channels above or below clip box
		25		28jun13	add plugins
		26		28jul13	add metadata

		wave editor view
 
*/

// WaveShopView.cpp : implementation of the CWaveShopView class
//

#include "stdafx.h"
#include "WaveShop.h"

#include "WaveShopDoc.h"
#include "WaveShopView.h"
#include "TimeRulerCtrl.h"
#include "Benchmark.h"
#include "ChannelBar.h"
#include "NumFormat.h"
#include "FocusEdit.h"
#include <math.h>	// for ceil
#include "NormalizeDlg.h"
#include "PeakStatsDlg.h"
#include "RMSStatsDlg.h"
#include "InsertSilenceDlg.h"
#include "AmplifyDlg.h"
#include "FadeDlg.h"
#include "SpeakersDlg.h"
#include "PathStr.h"
#include "FolderDialog.h"
#include "SwapChannelsDlg.h"
#include "FindClippingDlg.h"
#include "ChangeFormatDlg.h"
#include "DocManagerEx.h"
#include "ExportDlg.h"
#include "ResampleDlg.h"
#include "SpectrumDlg.h"
#include "PluginParamDlg.h"
#include "MetadataDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaveShopView

IMPLEMENT_DYNCREATE(CWaveShopView, CScrollView)

const double CWaveShopView::MIN_ZOOM = 1e-2;
const double CWaveShopView::MAX_ZOOM = 1e8;
const double CWaveShopView::DEFAULT_ZOOM = 1e5;
const double CWaveShopView::MIN_AMP = 1;
const double CWaveShopView::MAX_AMP = 1e3;
const double CWaveShopView::DEFAULT_AMP = 1;

#define UCODE_DEF(name) IDS_UC_##name, 
const int CWaveShopView::m_UndoTitleID[UNDO_CODES] = {
#include "UndoCodeData.h"
};

const WORD CWaveShopView::m_ChanSelUndoCode[] = {
	UCODE_AMPLIFY,
	UCODE_FADE,
	UCODE_INVERT,
	UCODE_NORMALIZE,
	UCODE_REVERSE,
};

/////////////////////////////////////////////////////////////////////////////
// CWaveShopView construction/destruction

CWaveShopView::CWaveShopView()
{
	m_Main = NULL;
	m_Wave = NULL;
	m_TimeRuler = NULL;
	m_ChannelBar = NULL;
	m_WndSize = CSize(0, 0);
	m_Zoom = DEFAULT_ZOOM;
	m_WaveWidth = 0;
	m_ScrollPos = 0;
	m_ScrollScale = 1;
	m_PagePercent = 100;
	m_LinePercent = 5;
	m_PageSize = 0;
	m_LineSize = 0;
	m_BucketMargin = 0;
	m_TimeRulerOffset = 0;
	m_DragState = DRAG_NONE;
	m_DragOrigin = CPoint(0, 0);
	m_DragChannel = 0;
	m_DragChanOrigin = 0;
	m_DragSelOrigin = 0;
	m_DragScrollDelta = 0;
	m_IsZooming = FALSE;
	m_IsTrackingMouse = FALSE;
	m_DeferBuckets = FALSE;
	m_ShowCaptions = FALSE;
	m_Now = 0;
	m_Selection.SetEmpty();
	m_PrevSelection.SetEmpty();
	m_ContextTarget = CPoint(0, 0);
	m_LastPluginCmdID = 0;
}

CWaveShopView::~CWaveShopView()
{
}

BOOL CWaveShopView::PreCreateWindow(CREATESTRUCT& cs)
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
    ASSERT(cs.lpszClass);
	cs.style |= WS_CLIPCHILDREN;	// avoids caption flicker
	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CWaveShopView operations

inline bool CWaveShopView::IsValidBucket(const BUCKET *pBucket) const
{
	return(pBucket >= m_Bucket.GetData() 
		&& pBucket < m_Bucket.GetData() + m_Bucket.GetSize());
}

inline int CWaveShopView::Min64To32(LONGLONG x, int y)
{
	ASSERT(x >= 0 && y >= 0);	// both arguments must be positive
	return(x < y ? int(x) : y);
}

double CWaveShopView::XToFrame(double x) const
{
	return((x + m_ScrollPos) / m_WaveWidth * m_Wave->GetFrameCount());
}

double CWaveShopView::FrameToX(double FrameIdx) const
{
	return(FrameIdx / m_Wave->GetFrameCount() * m_WaveWidth - m_ScrollPos);
}

inline LONGLONG CWaveShopView::GetMaxScrollPos() const
{
	return(max(m_WaveWidth - m_WndSize.cx, 0));
}

inline CDblRange CWaveShopView::IntToDblRange(const CW64IntRange& Range)
{
	return(CDblRange(double(Range.Start), double(Range.End)));
}

CWaveShopView::CClipboardUndoInfo::CClipboardUndoInfo()
{
	m_ChanSel = NULL;
}

CWaveShopView::CClipboardUndoInfo::~CClipboardUndoInfo()
{
	delete [] m_ChanSel;
}

void CWaveShopView::GetClipboardUndoInfo(CUndoState& State, const CDblRange& Selection) const
{
	CRefPtr<CClipboardUndoInfo>	uip;
	uip.CreateObj();
	uip->m_Now = GetNow();
	uip->m_Selection = Selection;
	uip->m_EditSel = GetIntSelection();
	if (!m_Wave->Copy(uip->m_Wave, uip->m_EditSel))
		AfxThrowUserException();
	if (!AllChannelsSelected()) {	// if channel(s) are excluded
		CByteArray	ChanSel;
		GetChannelSelection(&ChanSel);
		W64INT	chans = ChanSel.GetSize();
		uip->m_ChanSel = new BYTE[chans];
		CopyMemory(uip->m_ChanSel, ChanSel.GetData(), chans);
	}
	State.SetObj(uip);
}

void CWaveShopView::SaveUndoState(CUndoState& State)
{
//printf("SaveUndoState %d %d\n", State.GetCode(), State.GetCtrlID());
	switch (State.GetCode()) {
	case UCODE_CUT:
	case UCODE_DELETE:
		if (UndoMgrIsIdle()) {	// if initial state
			GetClipboardUndoInfo(State, GetSelection());	// current selection is valid
			UVInsert(State) = CUndoManager::UA_UNDO;	// undo inserts, redo deletes
		}
		break;
	case UCODE_PASTE:
	case UCODE_INSERT:
	case UCODE_INSERT_SILENCE:
		if (UndoMgrIsIdle()) {	// if initial state
			GetClipboardUndoInfo(State, m_PrevSelection);	// selection is altered pre-notify
			UVInsert(State) = CUndoManager::UA_REDO;	// undo deletes, redo inserts
		}
		break;
	case UCODE_SPEAKERS:
		UVSpeakers(State) = m_Wave->GetChannelMask();
		break;
	case UCODE_INSERT_CHANNEL:
	case UCODE_DELETE_CHANNEL:
	case UCODE_CHANGE_FORMAT:
	case UCODE_RESAMPLE:
		{
			CDblRange	sel = m_Selection;
			m_Selection = CDblRange(0, double(m_Wave->GetFrameCount()));
			GetClipboardUndoInfo(State, sel);
			m_Selection = sel;
		}
		break;
	case UCODE_METADATA:
		{
			CRefPtr<CMetadataUndoInfo>	uip;
			uip.CreateObj();
			uip->m_Metadata.Copy(GetDocument()->m_Metadata);
			State.SetObj(uip);
		}
		break;
	default:
		if (!UndoMgrIsIdle()) {	// if not initial state
			const CClipboardUndoInfo	*uip = (CClipboardUndoInfo *)State.GetObj();
			SetNow(uip->m_Now);	// restore now
			SetSelection(uip->m_Selection);	// restore selection
			SetChannelSelection(uip->m_ChanSel);	// restore channel selection
		}
		GetClipboardUndoInfo(State, GetSelection());
	}
}

void CWaveShopView::SetUIState(double Now, const CDblRange& Sel, const BYTE *ChanSel, bool Center)
{
	m_Main->SetNow(Now, Center);	// restore now
	m_Main->SetSelection(Sel);	// restore selection
	SetChannelSelection(ChanSel);	// restore channel selection
}

void CWaveShopView::SetUIState(const CClipboardUndoInfo& Info, bool Center)
{
	SetUIState(Info.m_Now, Info.m_Selection, Info.m_ChanSel, Center);
}

void CWaveShopView::RestoreUndoState(const CUndoState& State)
{
//printf("RestoreUndoState %d %d\n", State.GetCode(), State.GetCtrlID());
	switch (State.GetCode()) {
	case UCODE_CUT:
	case UCODE_PASTE:
	case UCODE_DELETE:
	case UCODE_INSERT:
	case UCODE_INSERT_SILENCE:
		{
			CWaitCursor	wc;
			const CClipboardUndoInfo	*uip = 
				(const CClipboardUndoInfo *)State.GetObj();
			CMainFrame::CPlay	stop(this);	// suspend playing our wave
			CDblRange	sel;
			double	now;
			if (GetUndoAction() == UVInsert(State)) {	// if inserting
				if (!m_Wave->Insert(uip->m_Wave, uip->m_EditSel.Start))
					AfxThrowUserException();
				switch (State.GetCode()) {
				case UCODE_PASTE:
				case UCODE_INSERT:	
				case UCODE_INSERT_SILENCE:
					sel = CDblRange(double(uip->m_EditSel.Start),
						double(uip->m_EditSel.End));
					now = sel.End;
					break;
				default:
					sel = uip->m_Selection;
					now = uip->m_Now;
				}
			} else {	// deleting
				if (!m_Wave->Delete(uip->m_EditSel))
					AfxThrowUserException();
				switch (State.GetCode()) {
				case UCODE_PASTE:
				case UCODE_INSERT:	
				case UCODE_INSERT_SILENCE:
					sel = uip->m_Selection;
					now = uip->m_Now;
					break;
				default:
					sel = CDblRange(uip->m_Selection.Start, uip->m_Selection.Start);
					now = sel.Start;
				}
			}
			Update();
			SetUIState(now, sel, uip->m_ChanSel, TRUE);	// center
		}
		break;
	case UCODE_SPEAKERS:
		SetChannelMask(UVSpeakers(State));
		break;
	case UCODE_INSERT_CHANNEL:
	case UCODE_DELETE_CHANNEL:
	case UCODE_CHANGE_FORMAT:
	case UCODE_RESAMPLE:
		{
			CWaitCursor	wc;
			const CClipboardUndoInfo	*uip = 
				(const CClipboardUndoInfo *)State.GetObj();
			CMainFrame::CPlay	stop(this);	// suspend playing our wave
			m_Wave->Init();	// so insert bypasses compatibility check
			if (!m_Wave->Insert(uip->m_Wave, 0))
				AfxThrowUserException();
			Update(HINT_WAVE_UPDATE | HINT_WAVE_FORMAT);
			SetUIState(*uip, TRUE);	// center
		}
		break;
	case UCODE_METADATA:
		{
			const CMetadataUndoInfo	*uip = 
				(const CMetadataUndoInfo *)State.GetObj();
			GetDocument()->m_Metadata.Copy(uip->m_Metadata);
		}
		break;
	default:
		CWaitCursor	wc;
		const CClipboardUndoInfo	*uip = 
			(const CClipboardUndoInfo *)State.GetObj();
		if (!m_Wave->Replace(uip->m_Wave, uip->m_EditSel.Start))
			AfxThrowUserException();
		Update();
		SetUIState(*uip, TRUE);	// center
	}
}

CString	CWaveShopView::GetUndoTitle(const CUndoState& State)
{
	ASSERT(State.GetCode() >= 0 && State.GetCode() < UNDO_CODES);
	CString	Title;
	if (State.GetCode() == UCODE_PLUGIN)
		m_Main->GetPluginManager().GetPluginName(State.GetCtrlID(), Title);
	else
		Title.LoadString(m_UndoTitleID[State.GetCode()]);
	return(Title);
}

bool CWaveShopView::Copy()
{
	CWaitCursor	wc;
	return(m_Wave->Copy(theApp.m_Clipboard, GetIntSelection()));
}

bool CWaveShopView::Delete(bool Cut)
{
	CWaitCursor	wc;
	static const WORD	UndoCode[2] = {UCODE_DELETE, UCODE_CUT};
	NotifyUndoableEdit(0, UndoCode[Cut]);
	CW64IntRange	iSel = GetIntSelection();
	CMainFrame::CPlay	stop(this);	// suspend playing our wave
	bool	retc;
	if (Cut)
		retc = m_Wave->Cut(theApp.m_Clipboard, iSel);
	else
		retc = m_Wave->Delete(iSel);
	if (!retc)	// if failure
		return(FALSE);
	Update();
	EnsureVisible(m_Selection.Start);
	m_Selection.End = m_Selection.Start;
	m_Main->SetSelection(m_Selection);
	m_Main->SetNow(m_Selection.Start);
	return(TRUE);
}

bool CWaveShopView::Paste(const CWaveEdit& Wave, WORD UndoCode)
{
	// ensure formats are compatible; warn if sample rates differ
	m_Wave->MatchFormat(Wave,	// throws on failure
		CWaveEdit::MF_COMPATIBLE | CWaveEdit::MF_SAMPLE_RATE);
	CWaitCursor	wc;
	LPARAM	hint;
	if (m_Wave->IsValid())	// if pre-paste wave is valid
		hint = HINT_WAVE_UPDATE;
	else	// pre-paste wave is invalid
		hint = HINT_INITIAL_UPDATE;	// do initial update
	W64INT	iNow = GetIntNow();
	CMainFrame::CPlay	stop(this);	// suspend playing our wave
	if (!m_Wave->Insert(Wave, iNow))
		return(FALSE);
	Update(hint);
	double	now = double(iNow);
	m_PrevSelection = GetSelection();
	CDblRange	sel(now, now + Wave.GetFrameCount());
	m_Main->SetSelection(sel);
	NotifyUndoableEdit(0, UndoCode);
	m_Main->SetNow(sel.End, TRUE);	// center
	return(TRUE);
}

bool CWaveShopView::Paste()
{
	return(Paste(theApp.m_Clipboard, UCODE_PASTE));
}

bool CWaveShopView::Insert(const CWaveEdit& Wave)
{
	return(Paste(Wave, UCODE_INSERT));
}

bool CWaveShopView::Insert() 
{
	CStringArray	Path;
	if (!CWaveShopApp::PromptForInputFiles(Path, IDS_UC_INSERT))
		return(FALSE);
	CWaitCursor	wc;
	CWaveEdit	InsWave;
	CProgressDlg	ProgDlg(IDD_PROGRESS_DUAL);	// allow nested progress
	int	files = INT64TO32(Path.GetSize());	// W64: force to 32-bit
	if (files > 1) {	// if multiple files
		if (!ProgDlg.Create())	// create progress dialog
			AfxThrowResourceException();
		ProgDlg.SetWindowText(LDS(IDS_DOC_INSERTING));
		ProgDlg.SetRange(0, files);
	}
	CMainFrame::CPlay	stop(this);	// suspend playing our wave before loop
	for (int iFile = 0; iFile < files; iFile++) {	// for each file
		if (ProgDlg.m_hWnd) {	// if progress dialog exists
			ProgDlg.SetPos(iFile);
			if (ProgDlg.Canceled())
				return(FALSE);
		}
		int	format;
		if (!InsWave.Import(Path[iFile], format))	// read insert wave
			return(FALSE);
		if (!Insert(InsWave))	// insert wave into our wave
			return(FALSE);
	}
	return(TRUE);
}

bool CWaveShopView::InsertSilence(W64INT FrameCount)
{
	CWaitCursor	wc;
	W64INT	iNow = GetIntNow();
	CMainFrame::CPlay	stop(this);	// suspend playing our wave
	if (!m_Wave->InsertSilence(iNow, FrameCount))
		return(FALSE);
	m_PrevSelection = GetSelection();
	double	now = double(iNow);
	CDblRange	sel(now, now + FrameCount);
	m_Main->SetSelection(sel);
	Update();
	NotifyUndoableEdit(0, UCODE_INSERT_SILENCE);
	m_Main->SetNow(sel.End, TRUE);	// center
	return(TRUE);
}

void CWaveShopView::EnsureVisible(double FrameIdx, bool Center)
{
	double	NowX = FrameToX(FrameIdx);
	if (NowX < 0 || NowX > m_WndSize.cx) {
		LONGLONG	x = round64(NowX) + m_ScrollPos;
		if (Center)
			x -= m_WndSize.cx / 2;
		ScrollToPosition(x);
	}
}

void CWaveShopView::ForceSelection()
{
	if (m_Selection.IsEmpty())
		OnEditSelectAll();
}

void CWaveShopView::InvalidateChannel(int ChannelIdx)
{
	const CChannelInfo&	info = m_ChanInfo[ChannelIdx];
	InvalidateRect(CRect(CPoint(0, info.m_y), CSize(m_WndSize.cx, info.m_Height)));
}

void CWaveShopView::SelectChannel(int ChannelIdx, bool Enable)
{
	CChannelInfo&	info = m_ChanInfo[ChannelIdx];
	if (info.m_Selected != Enable) {	// if channel's selection state changed
		info.m_Selected = Enable;	// update channel's state
		InvalidateChannel(ChannelIdx);	// redraw channel
	}
}

int CWaveShopView::GetChannelSelection(CByteArray *ChanSel) const
{
	int	SelChans = 0;
	int	chans = GetChannelCount();
	if (ChanSel != NULL)	// if channel selection array specified
		ChanSel->SetSize(chans);	// resize array
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		bool	IsSel = m_ChanInfo[iChan].m_Selected;
		if (ChanSel != NULL)	// if channel selection array specified
			(*ChanSel)[iChan] = IsSel;	// store selection state
		if (IsSel)	// if channel selected
			SelChans++;	// increment selected count
	}
	return(SelChans);	// return selected count
}

void CWaveShopView::SetChannelSelection(const BYTE *ChanSel)
{
	if (ChanSel != NULL) {	// if channel selection array specified
		int	chans = GetChannelCount();
		for (int iChan = 0; iChan < chans; iChan++)	// for each channel
			SelectChannel(iChan, ChanSel[iChan] != 0);	// set channel's selection state
	} else	// null channel selection
		SelectAllChannels();
}

bool CWaveShopView::AllChannelsSelected() const
{
	return(GetChannelSelection() == GetChannelCount());
}

void CWaveShopView::SelectAllChannels()
{
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++)	// for each channel
		SelectChannel(iChan, TRUE);	// set channel's selection state
}

int CWaveShopView::ToggleChannelSelection(UINT nFlags, CPoint point)
{
	UINT	KeyState = nFlags & (MK_CONTROL | MK_SHIFT);
	if (KeyState == MK_CONTROL) {	// if desired modifier keys
		if (GetChannelCount() >= 2) {	// if at least two channels
			int	iChan = FindChannel(point.y);
			if (iChan >= 0) {	// if channel found
				// toggle channel's selection state
				SelectChannel(iChan, !IsChannelSelected(iChan));
				if (!GetChannelSelection()) {	// if no channels selected
					if (iChan < GetChannelCount() - 1)	// if not last channel
						iChan++;	// select next channel
					else // last channel
						iChan--;	// select previous channel
					SelectChannel(iChan, TRUE);	// set channel's selection state
				}
				return(iChan);	// return index of toggled channel
			}
		}
	}
	return(-1);	// no channel toggled
}

bool CWaveShopView::RespectsChannelSelection(WORD UndoCode)
{
	int	cmds = _countof(m_ChanSelUndoCode);
	for (int iCmd = 0; iCmd < cmds; iCmd++) {
		if (m_ChanSelUndoCode[iCmd] == UndoCode)
			return(TRUE);
	}
	return(FALSE);
}

void CWaveShopView::ApplyChannelSelection()
{
	int	pos = GetUndoManager()->GetPos() - 1;	// get current undo state
	const CUndoState&	state = GetUndoManager()->GetState(pos);
	// if current command respects channel selection
	if (RespectsChannelSelection(state.GetCode())) {
		const CClipboardUndoInfo	*uip = (CClipboardUndoInfo *)state.GetObj();
		if (uip->m_ChanSel != NULL) {	// if channel(s) are excluded
			// invert channel selection to represent excluded channels
			int	chans = uip->m_Wave.GetChannels();
			CByteArray	ChanSel;
			ChanSel.SetSize(chans);
			for (int iChan = 0; iChan < chans; iChan++)	// for each channel
				ChanSel[iChan] = !uip->m_ChanSel[iChan];	// invert selection
			// restore excluded channels from undo state
			m_Wave->Replace(uip->m_Wave, uip->m_EditSel.Start, ChanSel.GetData());
		}
	}
}

void CWaveShopView::OnAudioProcessResult(bool Result, LPARAM Hint)
{
	if (Result) {	// if process succeeded
		ApplyChannelSelection();
		Update(Hint);	// update view(s)
	} else	// process canceled or failed; data is incomplete
		GetUndoManager()->UndoNoRedo();	// restore data prior to process
}

void CWaveShopView::OnAudioProcessResult(bool Result)
{
	OnAudioProcessResult(Result, HINT_WAVE_UPDATE);
}

bool CWaveShopView::InsertChannel(int ChannelIdx)
{
	CStringArray	Path;
	if (!CWaveShopApp::PromptForInputFiles(Path, IDS_UC_INSERT_CHANNEL))
		return(FALSE);
	CWaitCursor	wc;
	CWaveEdit	InsWave;
	CProgressDlg	ProgDlg(IDD_PROGRESS_DUAL);	// allow nested progress
	int	files = INT64TO32(Path.GetSize());	// W64: force to 32-bit
	if (files > 1) {	// if multiple files
		if (!ProgDlg.Create())	// create progress dialog
			AfxThrowResourceException();
		ProgDlg.SetWindowText(LDS(IDS_WPRO_INSERT_CHANNEL));
		ProgDlg.SetRange(0, files);
	}
	CMainFrame::CPlay	stop(this);	// suspend playing our wave before loop
	for (int iFile = 0; iFile < files; iFile++) {	// for each file
		if (ProgDlg.m_hWnd) {	// if progress dialog exists
			ProgDlg.SetPos(iFile);
			if (ProgDlg.Canceled())
				return(FALSE);
		}
		int	format;
		if (!InsWave.Import(Path[iFile], format))	// read insert wave
			return(FALSE);
		if (!InsertChannel(ChannelIdx, InsWave))	// insert channels(s)
			return(FALSE);
		ChannelIdx += InsWave.GetChannels();	// increment insert position
	}
	return(TRUE);
}

bool CWaveShopView::InsertChannel(int ChannelIdx, const CWaveEdit& Wave)
{
	if (!m_Wave->IsValid())	// if our wave is invalid
		return(Insert(Wave));	// insert is faster; also avoids undo problems
	// ensure sample sizes match; warn if sample rates differ
	m_Wave->MatchFormat(Wave,	// throws on failure
		CWaveEdit::MF_SAMPLE_SIZE | CWaveEdit::MF_SAMPLE_RATE);
	NotifyUndoableEdit(0, UCODE_INSERT_CHANNEL);
	CMainFrame::CPlay	stop(this);	// suspend playing our wave
	bool	retc = m_Wave->InsertChannel(ChannelIdx, Wave);	// insert channel(s)
	OnAudioProcessResult(retc);
	return(retc);
}

bool CWaveShopView::DeleteChannel(int ChannelIdx)
{
	NotifyUndoableEdit(0, UCODE_DELETE_CHANNEL);
	CMainFrame::CPlay	stop(this);	// suspend playing our wave
	bool	retc = m_Wave->DeleteChannel(ChannelIdx);	// delete channel
	OnAudioProcessResult(retc);
	return(retc);
}

bool CWaveShopView::SetChannelMask(UINT ChannelMask)
{
	m_Wave->SetChannelMask(ChannelMask);
	m_Main->OnWaveFormatChange();
	if (m_ShowCaptions)	// if showing captions
		UpdateCaptions();	// update captions
	if (m_Main->ActivePlayer()) {	// should always be true
		if (!m_Main->UpdatePlayerState())	// notify player of speaker change
			return(FALSE);
	}
	return(TRUE);
}

double CWaveShopView::CalcFitZoom() const
{
	if (!m_WndSize.cx || m_Wave->IsEmpty())
		return(DEFAULT_ZOOM);	// avoid divide by zero
	return(double(m_Wave->GetFrameCount()) / m_WndSize.cx);
}

bool CWaveShopView::SetZoom(int x, double Zoom)
{
	if (Zoom < MIN_ZOOM || Zoom > MAX_ZOOM)	// if out of range
		return(FALSE);
	if (!m_Wave->IsValid())	// if invalid wave
		return(FALSE);
	if (m_ShowCaptions)	// if showing captions
		RepositionAllCaptions(FALSE);	// hide captions while zooming
//printf("SetZoom x=%d Zoom=%f\n", x, Zoom);
	double	FrameIdx = XToFrame(x);	// before updating width
	CalcWaveWidth(Zoom);
	UpdateScrollSize();
	m_Zoom = Zoom;
	// prevent scroll from needlessly updating buckets; OnWaveUpdate updates them
	m_IsZooming = TRUE;
	ScrollToPosition(round64(FrameToX(FrameIdx)) + m_ScrollPos - x);
	OnWaveUpdate();
	m_IsZooming = FALSE;
	if (m_ShowCaptions)	// if showing captions
		RepositionAllCaptions();	// restore captions
	return(TRUE);	
}

bool CWaveShopView::StepZoom(int x, bool In)
{
	double	zoom = m_Zoom;
	double	ZoomStep = m_Main->GetOptions().m_ZoomStepHorz / 100.0;
	if (In)
		zoom /= ZoomStep;
	else
		zoom *= ZoomStep;
	return(SetZoom(x, zoom));
}

void CWaveShopView::FitInWindow()
{
	SetZoom(0, CalcFitZoom());
}

void CWaveShopView::FitVertically()
{
	ResetChannelInfo();
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++) {
		SetVerticalOrigin(iChan, 0);
		SetAmplitude(iChan, 1);
	}
}

void CWaveShopView::VertSyncChannels(int ChannelIdx)
{
	const CChannelInfo& src = m_ChanInfo[ChannelIdx];
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++) {
		if (iChan != ChannelIdx) {
			CChannelInfo& dst = m_ChanInfo[iChan];
			dst.m_Origin = src.m_Origin;
			dst.m_Amplitude = src.m_Amplitude;
			m_ChannelBar->SetAmplitude(iChan, src.m_Origin, src.m_Amplitude);
		}
	}
	Invalidate();	// repaint all channels
}

bool CWaveShopView::SetAmplitude(int ChannelIdx, double Amplitude)
{
	Amplitude = CLAMP(Amplitude, MIN_AMP, MAX_AMP);
	if (!m_Wave->IsValid())	// if invalid wave
		return(FALSE);
	CChannelInfo&	info = m_ChanInfo[ChannelIdx];
	info.m_Amplitude = Amplitude;
	if (Amplitude <= MIN_AMP)	// if minimum amplitude
		info.m_Origin = 0;	// reset origin
	m_ChannelBar->SetAmplitude(ChannelIdx, info.m_Origin, Amplitude);
	InvalidateChannel(ChannelIdx);
	if (m_Main->GetOptions().m_VertSyncChans)
		VertSyncChannels(ChannelIdx);
	return(TRUE);
}

void CWaveShopView::SetVerticalOrigin(int ChannelIdx, double Origin)
{
	CChannelInfo&	info = m_ChanInfo[ChannelIdx];
	info.m_Origin = Origin;
	m_ChannelBar->SetAmplitude(ChannelIdx, Origin, info.m_Amplitude);
	InvalidateChannel(ChannelIdx);
	if (m_Main->GetOptions().m_VertSyncChans)
		VertSyncChannels(ChannelIdx);
}

void CWaveShopView::ApplyVertZoomStep(bool In, double& Amplitude) const
{
	double	AmpStep = m_Main->GetOptions().m_ZoomStepVert / 100.0;
	if (In)
		Amplitude *= AmpStep;
	else
		Amplitude /= AmpStep;
}

bool CWaveShopView::StepAmplitude(int ChannelIdx, bool In)
{
	double	amp = GetAmplitude(max(ChannelIdx, 0));
	ApplyVertZoomStep(In, amp);
	return(SetAmplitude(ChannelIdx, amp));
}

bool CWaveShopView::SetVerticalZoom(int ChannelIdx, int y, double Amplitude)
{
	Amplitude = CLAMP(Amplitude, MIN_AMP, MAX_AMP);
	if (!m_Wave->IsValid())	// if invalid wave
		return(FALSE);
	CChannelInfo&	info = m_ChanInfo[ChannelIdx];
	double	PrevAmp = info.m_Amplitude;
	double	pos = double(y - info.m_y) / info.m_Height - .5;
	double	NewOrigin = (info.m_Origin - pos) / PrevAmp * Amplitude + pos;
	info.m_Origin = NewOrigin;
	SetAmplitude(ChannelIdx, Amplitude);
	return(TRUE);
}

bool CWaveShopView::StepVerticalZoom(int ChannelIdx, int y, bool In)
{
	double	amp = GetAmplitude(max(ChannelIdx, 0));
	ApplyVertZoomStep(In, amp);
	return(SetVerticalZoom(ChannelIdx, y, amp));
}

int CWaveShopView::GetNowX() const
{
	double	NowX = FrameToX(m_Now);
	NowX = CLAMP(NowX, INT_MIN, INT_MAX);	// limit to integer range
	return(round(NowX));
}

int CWaveShopView::GetNowXClampEnd() const
{
	int	NowX = GetNowX();
	if (NowX >= 0 && NowX >= m_WaveWidth - m_ScrollPos)	// if now at end of page
		NowX--;	// shift x left one pixel to keep now visible
	return(NowX);
}

void CWaveShopView::SetNow(double Now)
{
	if (Now == m_Now)	// if now unchanged
		return;	// nothing to do
	int	OldNowX = GetNowXClampEnd();
	CRect	rNow(CPoint(OldNowX, 0), CSize(1, m_WndSize.cy));
	m_Now = Now;	// order matters: update now member before updating window
	CRect	rInvalid;
	GetUpdateRect(rInvalid);
	if ((rInvalid & rNow) != rNow) {	// if old now marker not already invalidated
		InvalidateRect(rNow);	// invalidate old now marker
		// update window to avoid repainting entire area between old and new markers
		UpdateWindow();	// paint ASAP, erasing old now marker
	}
	EnsureVisible(Now, FALSE);	// don't center
	int	NowX = GetNowXClampEnd();
	rNow.left = NowX;
	rNow.right = NowX + 1;
	InvalidateRect(rNow);	// invalidate new now marker
}

void CWaveShopView::SetSelection(const CDblRange& Sel)
{
	if (Sel == m_Selection)	// if selection unchanged
		return;	// nothing to do
	CIntRange	OldSelX = GetSelectionX();	// before updating m_Selection
	m_Selection = Sel;
	CRect	rSel(CPoint(OldSelX.Start, 0), CSize(OldSelX.Length(), m_WndSize.cy));
	CRect	rClient(CPoint(0, 0), m_WndSize);
	rSel = rSel & rClient;	// intersect with client area to keep GDI happy
	InvalidateRect(rSel);	// invalidate previous selection
	CIntRange	SelX = GetSelectionX();	// after updating m_Selection
	rSel = CRect(CPoint(SelX.Start, 0), CSize(SelX.Length(), m_WndSize.cy));
	rSel = rSel & rClient;	// intersect with client area to keep GDI happy
	InvalidateRect(rSel);	// invalidate new selection
}

W64INT CWaveShopView::GetIntNow() const
{
	W64INT	now = roundW64INT(ceil(m_Now));
	return(CLAMP(now, 0, m_Wave->GetFrameCount()));
}

CW64IntRange CWaveShopView::GetIntSelection() const
{
	W64INT	Start = roundW64INT(ceil(m_Selection.Start));
	W64INT	End = roundW64INT(ceil(m_Selection.End));
	return(CW64IntRange(max(Start, 0), min(End, m_Wave->GetFrameCount())));
}

CIntRange CWaveShopView::GetSelectionX() const
{
	CDblRange	SelX(FrameToX(m_Selection.Start), FrameToX(m_Selection.End));
	SelX.Start = CLAMP(SelX.Start, INT_MIN, INT_MAX);	// limit to integer range
	SelX.End = CLAMP(SelX.End, INT_MIN, INT_MAX);
	return(CIntRange(round(SelX.Start), round(SelX.End)));
}

double CWaveShopView::FindZeroCrossing(double Frame) const
{
	CWaitCursor	wc;	// find can take time, e.g. if all samples are zero
	W64INT	iFrame = roundW64INT(Frame);
	if (!m_Wave->FindZeroCrossing(iFrame))	// if zero crossing not found
		return(Frame);	// return start frame unmodified
	return(double(iFrame) + .5);	// return center of crossing span
}

CSize CWaveShopView::GetPageSize() const
{
	return(CSize(CalcPageWidth(m_WndSize.cx), m_WndSize.cy));
}

void CWaveShopView::CalcWaveWidth(double Zoom)
{
	ASSERT(Zoom > 0);	// avoid divide by zero or negative zoom
	double	RealWaveWidth = m_Wave->GetFrameCount() / Zoom;
	m_WaveWidth = round64(RealWaveWidth);
	if (m_WaveWidth > MAX_SCROLL_SIZE)	// if width exceeds scrolling capacity
		m_ScrollScale = double(m_WaveWidth) / MAX_SCROLL_SIZE;	// scale scrolling
	else
		m_ScrollScale = 1;
//printf("m_WaveWidth=%I64d m_ScrollScale=%f\n", m_WaveWidth, m_ScrollScale);
}

inline int CWaveShopView::CalcPageWidth(int WndWidth) const
{
	return(Min64To32(m_WaveWidth, WndWidth));
}

inline int CWaveShopView::CalcNumBuckets(int PageWidth) const
{
	return((PageWidth + m_BucketMargin * 2) * GetChannelCount());
}

void CWaveShopView::UpdateScrollSize()
{
	int	ScrollSize = CalcPageWidth(MAX_SCROLL_SIZE);
	SetScrollSizes(MM_TEXT, CSize(ScrollSize, 0));
	m_PageSize = round(m_WndSize.cx * (m_PagePercent / 100.0));
	m_LineSize = round(m_WndSize.cx * (m_LinePercent / 100.0));
//printf("ScrollSize=%d PageSize=%d LineSize=%d\n", ScrollSize, m_PageSize, m_LineSize);
}

int CWaveShopView::FindChannel(int y) const
{
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		const CChannelInfo&	info = m_ChanInfo[iChan];
		if (y >= info.m_y && y < info.m_y + info.m_Height)	// if within channel
			return(iChan);
	}
	return(-1);
}

int CWaveShopView::FindChannelFuzzy(int y) const
{
	int	chans = GetChannelCount();
	if (chans > 0) {
		if (y < 0)	// if above window
			return(0);	// first channel
		if (y >= m_WndSize.cy)	// if below window
			return(chans - 1);	// last channel
		for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
			const CChannelInfo&	info = m_ChanInfo[iChan];
			// if within channel or gutter below it
			if (y >= info.m_y && y < info.m_y + info.m_Height + GUTTER_HEIGHT)
				return(iChan);
		}
	}
	return(-1);
}

int CWaveShopView::FindGutter(int y) const
{
	int	chans = GetChannelCount();
	for (int iChan = 1; iChan < chans; iChan++) {	// skip first channel
		const CChannelInfo&	info = m_ChanInfo[iChan];
		if (y >= info.m_y - GUTTER_HEIGHT && y < info.m_y)
			return(iChan);
	}
	return(-1);
}

int CWaveShopView::FindInsertPosition(int y) const
{
	int iChan = FindChannel(y);
	if (iChan < 0) {	// if not on a channel
		iChan = FindGutter(y);	// try finding gutter
		if (iChan < 0)	// if not on a gutter either
			iChan = m_Wave->GetChannels();	// append
	}
	return(iChan);
}

int CWaveShopView::HitTest(CPoint Point, int& ItemIdx) const
{
	ItemIdx = FindGutter(Point.y);
	if (ItemIdx >= 0)
		return(HT_GUTTER);
	int	DragThreshold = GetSystemMetrics(SM_CXDRAG);
	if (!m_Selection.IsEmpty()) {
		CIntRange	SelX = GetSelectionX();
		if (abs(SelX.Start - Point.x) < DragThreshold)
			return(HT_SEL_START);
		if (abs(SelX.End - Point.x) < DragThreshold)
			return(HT_SEL_END);
	}
	return(HT_NOWHERE);
}

void CWaveShopView::UpdateCaptions()
{
	int	chans = m_Wave->GetChannels();
	// if mono audio with no speaker assignment
	if (chans == 1 && !m_Wave->GetChannelMask()) {
		m_ChanInfo[0].m_Caption.SetEmpty();	// delete caption if any
		return;	// early out
	}
	CStringArray	ChanName;
	m_Wave->GetChannelNames(ChanName);
	CClientDC	dc(this);
	HGDIOBJ	hFont = GetStockObject(DEFAULT_GUI_FONT);
	dc.SelectObject(hFont);
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		CChannelInfo&	info = m_ChanInfo[iChan];
		CString	name(CWaveEdit::AbbreviateChannelName(ChanName[iChan]));
		CSize	TextSize = dc.GetTextExtent(name);
		TextSize.cx += CAPTION_HORZ_MARGIN * 2;
		CRect	r(CPoint(0, info.m_y), TextSize);
		if (info.m_Caption.IsEmpty()) {	// if caption doesn't exist
			info.m_Caption.CreateObj();	// create instance
			UINT	style = WS_CHILD | WS_VISIBLE | SS_CENTER;
			if (!info.m_Caption->Create(name, style, r, this))	// create control
				AfxThrowResourceException();
			info.m_Caption->SendMessage(WM_SETFONT, WPARAM(hFont));	// set font
		} else {	// caption exists
			info.m_Caption->MoveWindow(r);	// update size
			info.m_Caption->SetWindowText(name);	// update text
		}
	}
}

void CWaveShopView::RepositionCaption(int ChannelIdx) const
{
	const CChannelInfo&	info = m_ChanInfo[ChannelIdx];
	if (!info.m_Caption.IsEmpty()) {	// if caption exists
		CRect	r;
		info.m_Caption->GetClientRect(r);
		CSize	sz = r.Size();
		info.m_Caption->MoveWindow(CRect(CPoint(0, info.m_y), sz));
	}
}

void CWaveShopView::RepositionAllCaptions(bool Show) const
{
	int	chans = GetChannelCount();
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		const CChannelInfo&	info = m_ChanInfo[iChan];
		if (!info.m_Caption.IsEmpty()) {	// if caption exists
			if (Show)	// if showing
				RepositionCaption(iChan);	// update caption position
			info.m_Caption->ShowWindow(Show ? SW_SHOW : SW_HIDE);
		}
	}
}

void CWaveShopView::ShowCaptions(bool Enable)
{
	if (Enable == m_ShowCaptions)
		return;	// nothing to do
	m_ShowCaptions = Enable;
	if (Enable) {	// if showing captions
		UpdateCaptions();	// update captions
	} else {	// hiding captions
		int	chans = GetChannelCount();
		for (int iChan = 0; iChan < chans; iChan++)	// for each channel
			m_ChanInfo[iChan].m_Caption.SetEmpty();	// destroy caption
	}
}

void CWaveShopView::OnWaveFormatChange()
{
//printf("OnWaveFormatChange\n");
	int	chans = m_Wave->GetChannels();
	if (chans != GetChannelCount())	// if channel count changed
		UpdateChannelCount();
	else	// same channel count; assume sample rate/size change
		m_Main->OnWaveFormatChange();
}

void CWaveShopView::UpdateChannelCount()
{
	int	chans = m_Wave->GetChannels();
	if (chans == GetChannelCount())	// if channel count unchanged
		return;	// nothing to do
	m_ChannelBar->SetChannelCount(chans);
	m_ChanInfo.SetSize(chans);
	double	HeightFrac = 1.0 / chans;	// make all channels the same height
	for (int iChan = 0; iChan < chans; iChan++) {
		CChannelInfo&	info = m_ChanInfo[iChan];
		info.m_HeightFrac = HeightFrac;
		info.m_Origin = 0;
		info.m_Amplitude = MIN_AMP;
		info.m_Selected = TRUE;
	}
	if (m_ShowCaptions)	// if showing captions
		UpdateCaptions();	// update captions
	UpdateChannelHeights(m_WndSize.cy);
	m_Main->OnWaveFormatChange();
}

void CWaveShopView::UpdateChannelHeights(int WndHeight)
{
	if (m_ShowCaptions)	// if showing captions
		RepositionAllCaptions(FALSE);	// hide captions before resizing channels
	int	chans = GetChannelCount();
	int	TotalGutter = (chans - 1) * GUTTER_HEIGHT;
	int	AvailHeight = WndHeight - TotalGutter;
	int	y = 0;
	for (int iChan = 0; iChan < chans; iChan++) {
		CChannelInfo&	info = m_ChanInfo[iChan];
		info.m_Height = trunc(AvailHeight * info.m_HeightFrac);
		info.m_y = y;
		m_ChannelBar->SetChannelPos(iChan, y, info.m_Height);
		y += info.m_Height + GUTTER_HEIGHT;
	}
	if (m_ShowCaptions)	// if showing captions
		RepositionAllCaptions();	// show repositioned captions
}

void CWaveShopView::DragGutter(int y)
{
	int	DragChan = m_DragChannel;
	CChannelInfo&	upr = m_ChanInfo[DragChan - 1];	// upper channel
	CChannelInfo&	lwr = m_ChanInfo[DragChan];		// lower channel
	int	HeightBoth = upr.m_Height + lwr.m_Height;
	int	y1 = upr.m_y;
	int	y2 = y1 + HeightBoth;
	if (y2 - y1 > MIN_CHAN_HEIGHT * 2) {	// if room to drag
		y = CLAMP(y, y1 + MIN_CHAN_HEIGHT, y2 - MIN_CHAN_HEIGHT);
		if (y + GUTTER_HEIGHT != lwr.m_y) {	// if position changed
			upr.m_Height = y - y1;
			lwr.m_y = y + GUTTER_HEIGHT;
			lwr.m_Height = y2 - y;
			double	FracBoth = upr.m_HeightFrac + lwr.m_HeightFrac;
			upr.m_HeightFrac = FracBoth * (double(upr.m_Height) / HeightBoth);
			lwr.m_HeightFrac = FracBoth - upr.m_HeightFrac;
			m_ChannelBar->SetChannelPos(DragChan - 1, upr.m_y, upr.m_Height);
			m_ChannelBar->SetChannelPos(DragChan, lwr.m_y, lwr.m_Height);
			if (m_ShowCaptions)	// if showing captions
				RepositionCaption(DragChan);	// move dragged channel's caption
			Invalidate();
		}
	}
}

void CWaveShopView::EndDrag()
{
	if (m_DragState != DRAG_NONE) {
		ReleaseCapture();	// release mouse input
		m_DragState = DRAG_NONE;	// reset drag state
		if (m_DragScrollDelta)
			KillTimer(SCROLL_TIMER_ID);
	}
}

void CWaveShopView::ResetChannelInfo()
{
	m_ChanInfo.RemoveAll();
	UpdateChannelCount();
	Invalidate();
}

void CWaveShopView::MaximizeChannel(int ChannelIdx)
{
	int	chans = GetChannelCount();
	if (chans <= 1)
		return;	// nothing to do
	int	MinHeight = (MIN_CHAN_HEIGHT + GUTTER_HEIGHT) * (chans - 1);
	int	MaxHeight = m_WndSize.cy - MinHeight;
	if (MaxHeight <= MIN_CHAN_HEIGHT)
		return;
	double	MaxFrac = double(MaxHeight) / m_WndSize.cy;
	double	MinFrac = (1 - MaxFrac) / (chans - 1);
	for (int iChan = 0; iChan < chans; iChan++) {
		CChannelInfo&	info = m_ChanInfo[iChan];
		if (iChan == ChannelIdx)
			info.m_HeightFrac = MaxFrac;
		else
			info.m_HeightFrac = MinFrac;
	}
	UpdateChannelHeights(m_WndSize.cy);
	Invalidate();	// repaint all channels
}

bool CWaveShopView::Find(bool First)
{
	CWaveProcess::FIND_SAMPLE_INFO	info;
	if (!m_Main->Find(info, First))
		return(FALSE);	// error if find
	if (info.MatchFrame >= 0)	// if matching sample found
		m_Main->SetNow(double(info.MatchFrame), TRUE);	// center
	else	// no match
		AfxMessageBox(IDS_FIND_MATCH_NOT_FOUND);
	return(TRUE);
}

void CWaveShopView::OnRecalcLayout()
{
//printf("OnRecalcLayout\n");
	CPoint	RulerOrigin(0, 0);
	m_TimeRuler->MapWindowPoints(this, &RulerOrigin, 1);	// convert to view coords
	m_TimeRulerOffset = RulerOrigin.x;	// offset of ruler from view
	m_TimeRuler->ScrollToPosition(double(m_ScrollPos + m_TimeRulerOffset));
}

void CWaveShopView::ZoomRuler(double Zoom)
{
	if (m_TimeRuler->GetUnit() == CRulerCtrl::UNIT_TIME)	// if time ruler unit is time
		Zoom /= m_Wave->GetSampleRate();	// convert from samples to seconds
	if (Zoom == m_TimeRuler->GetZoom())	// if zoom unchanged
		return;	// nothing to do
	m_TimeRuler->SetZoom(Zoom);
	m_TimeRuler->SetScrollPosition(double(m_ScrollPos + m_TimeRulerOffset));
}

void CWaveShopView::OnTimeUnitChange()
{
//printf("OnTimeUnitChange\n");
	if (!m_Wave->IsValid())	// if invalid wave
		return;	// nothing to do
	int	TimeUnit;
	if (m_Main->GetOptions().m_TimeInFrames)	// if showing sample frames
		TimeUnit = CRulerCtrl::UNIT_METRIC;
	else	// showing time in seconds as usual
		TimeUnit = CRulerCtrl::UNIT_TIME;
	if (TimeUnit != m_TimeRuler->GetUnit()) {	// if unit changed
		m_TimeRuler->SetUnit(TimeUnit);	// update time ruler
		ZoomRuler(m_Zoom);
	}
}

void CWaveShopView::Update(LPARAM Hint)
{
	OnUpdate(this, Hint, NULL);	// update this view
	GetDocument()->UpdateAllViews(this, Hint);	// update any other views of our doc
}

void CWaveShopView::OnWaveUpdate()
{
//printf("OnWaveUpdate\n");
	if (!m_Wave->IsValid())	// if invalid wave
		return;	// nothing to do
	UpdateChannelCount();
	if (!m_IsZooming) {	// if not zooming (otherwise this work was already done)
		CalcWaveWidth(m_Zoom);
		UpdateScrollSize();
	}
	m_BucketMargin = trunc(1 / m_Zoom) + 1;
	m_ScrollPos = CLAMP(m_ScrollPos, 0, GetMaxScrollPos());	// constrain scroll position
	int	PageWidth = CalcPageWidth(m_WndSize.cx);
	int	buckets = CalcNumBuckets(PageWidth);	// calculate number of buckets
	m_Bucket.SetSize(buckets);	// resize buckets array
	UpdateBuckets(0, PageWidth);	// update buckets
	Invalidate();
	ZoomRuler(m_Zoom);
}

inline void CWaveShopView::DeferFitInWindow()	// single caller, so inline for now
{
//printf("DeferFitInWindow\n");
	m_DeferBuckets = TRUE;	// defer updating buckets; reset by OnFitInWindow
	UpdateChannelCount();
	m_Zoom = CalcFitZoom();
	PostMessage(UWM_VIEW_FIT_IN_WINDOW);	// fit after things settle down
}

void CWaveShopView::ScrollToPosition(LONGLONG ScrollPos)
{
	LONGLONG	MaxScrollPos = GetMaxScrollPos();
	ScrollPos = CLAMP(ScrollPos, 0, MaxScrollPos);	// constrain scroll position
	if (ScrollPos == m_ScrollPos)	// if scroll position unchanged
		return;	// nothing to do
//printf("ScrollToPosition %I64d\n", ScrollPos);
	if (m_IsZooming) {	// if zooming
		m_ScrollPos = ScrollPos;	// set position without actually scrolling
	} else {	// not zooming
		if (m_ShowCaptions)	// if showing captions
			RepositionAllCaptions(FALSE);	// hide captions while scrolling
		LONGLONG	ScrollDelta = ScrollPos - m_ScrollPos;
		LONGLONG	ScrollSize = _abs64(ScrollDelta);
		BUCKET	*pSrc, *pDst;
		int	width = CalcPageWidth(m_WndSize.cx);
		if (ScrollSize < width) {	// if any existing buckets are still valid
			int	offset = int(ScrollSize) * GetChannelCount();
			if (ScrollDelta > 0) {	// if scrolling ahead
				pSrc = &m_Bucket[offset];	// shift valid buckets down
				pDst = &m_Bucket[0];
			} else {	// scrolling back
				pSrc = &m_Bucket[0];		// shift valid buckets up
				pDst = &m_Bucket[offset];
			}
			MoveMemory(pDst, pSrc, (m_Bucket.GetSize() - offset) * sizeof(BUCKET));
			ScrollWindow(-int(ScrollDelta), 0);
		} else	// scrolled beyond all buckets
			Invalidate();
		m_ScrollPos = ScrollPos;	// update scroll position; order matters
		int	x1, x2;
		int	ScrollSize32 = int(min(ScrollSize, width));
		if (ScrollDelta > 0) {	// if scrolling ahead
			x1 = width - ScrollSize32;
			x2 = width;
		} else {	// scrolling back
			x1 = 0;
			x2 = ScrollSize32;
		}
		UpdateBuckets(x1, x2);	// fill buckets invalidated by scrolling
		m_TimeRuler->ScrollToPosition(double(ScrollPos + m_TimeRulerOffset));
		if (m_ShowCaptions)	// if showing captions
			RepositionAllCaptions();	// restore captions
	}
	int	ScrollPos32 = round(ScrollPos / m_ScrollScale);
//printf("ScrollPos32=%d\n", ScrollPos32);
	SetScrollPos(SB_HORZ, ScrollPos32);
}

void CWaveShopView::UpdateBuckets(int x1, int x2)
{
	if (x1 >= x2)	// if empty or invalid update
		return;	// nothing to do
	if (m_DeferBuckets)	// if deferring update
		return;	// nothing to do
//printf("UpdateBuckets %d %d\n", x1, x2);
//CBenchmark	b;
	int	chans = GetChannelCount();
	ASSERT(m_WaveWidth);	// wave can't be empty
	ASSERT(x1 >= 0);
	int	mx1 = x1 - m_BucketMargin;
	int	mx2 = x2 + m_BucketMargin;
	ASSERT(mx2 * chans <= m_Bucket.GetSize());	// x2 is excluded
	W64INT	frames = m_Wave->GetFrameCount();
	UINT	SampSize = m_Wave->GetSampleSize();
	int	FrameStep = round(m_Zoom / m_Main->GetOptions().m_MaxDensity);
	FrameStep = max(FrameStep, 1);
	int	SampSkip = m_Wave->GetFrameSize() * (FrameStep - 1);
	BUCKET	*pBucket = &m_Bucket[x1 * chans];
	int	buckets = (mx2 - mx1) * chans;
	ASSERT(IsValidBucket(pBucket + buckets - 1));
	BUCKET	BucketInit = {INT_MAX, INT_MIN};
	for (int iBucket = 0; iBucket < buckets; iBucket++)
		pBucket[iBucket] = BucketInit;
	if (m_Zoom < 1) {	// if less than one frame per pixel, compensate for rounding
		int	OriginShift = trunc(0.5 / m_Zoom);	// trunc keeps first sample visible
		mx1 += OriginShift;
		mx2 += OriginShift;
	}
	LONGLONG	sx1 = mx1 + m_ScrollPos;
	LONGLONG	sx2 = mx2 + m_ScrollPos;
	ASSERT(m_WaveWidth);	// wave width must be non-zero
	W64INT	StartFrame = roundW64INT(double(sx1) / m_WaveWidth * frames);
//printf("sx1=%I64d sx2=%I64d StartFrame=%d\n", sx1, sx2, StartFrame);
	for (LONGLONG x = sx1; x < sx2; x++) {	// x2 is excluded
		if (StartFrame >= frames)	// if beyond last frame (due to step)
			break;	// early out
		W64INT	EndFrame = roundW64INT(double(x + 1) / m_WaveWidth * frames);
		if (StartFrame >= 0) {
			W64INT	ByteOffset = m_Wave->GetByteOffset(0, StartFrame);
			for (W64INT iFrame = StartFrame; iFrame < EndFrame; iFrame += FrameStep) {
				BUCKET	*pb = pBucket;
				for (int iChan = 0; iChan < chans; iChan++) {
					CWave::SAMPLE	samp = m_Wave->GetSampleAt(ByteOffset);
					ASSERT(IsValidBucket(pb));
					if (samp < pb->Min)
						pb->Min = samp;
					if (samp > pb->Max)
						pb->Max = samp;
					ByteOffset += SampSize;
					pb++;
				}
				ByteOffset += SampSkip;
			}
		}
		StartFrame = EndFrame;
		pBucket += chans;
	}
//printf("\tUpdateBuckets T=%f\n", b.Elapsed());
}

/////////////////////////////////////////////////////////////////////////////
// CWaveShopView drawing

void CWaveShopView::OnDraw(CDC* pDC)
{
	COLORREF	EmptyColor = GetSysColor(COLOR_3DSHADOW);
	COLORREF	GutterColor = GetSysColor(COLOR_3DFACE);
	COLORREF	OriginColor = GetSysColor(COLOR_3DSHADOW);
	COLORREF	InterpolationColor;
	VIEW_PALETTE	Palette = m_Main->GetOptions().m_ViewPalette;
	if (m_Main->GetOptions().m_ShowInterpolation)	// if highlighting interpolation
		InterpolationColor = Palette.Interpolation;	// use interpolation color
	else
		InterpolationColor = Palette.SelectedWaveData;	// use wave data color
	CRect	cb;
	pDC->GetClipBox(cb);
//printf("OnDraw %d %d %d %d\n", cb.left, cb.top, cb.Width(), cb.Height());
	if (m_Wave->IsEmpty()) {	// if empty wave
		pDC->FillSolidRect(cb, EmptyColor);	// just fill background
		return;
	}
	int	chans = GetChannelCount();
//CBenchmark	b;
	SAMPLE	NegRail, PosRail;
	m_Wave->GetSampleRails(NegRail, PosRail);
	int	ClipWaveWidth = CalcPageWidth(cb.right);
	int	x1 = cb.left - m_BucketMargin;
	int	x2 = ClipWaveWidth + m_BucketMargin;
//printf("x1=%d x2=%d\n", x1, x2);
	CPen	InterpolationPen(PS_SOLID, 1, InterpolationColor);
	HGDIOBJ	PrevPen = pDC->SelectObject(InterpolationPen);
	CRgn	ClipBoxRgn;
	ClipBoxRgn.CreateRectRgnIndirect(cb);	// save clip box as a region
	int	NowX = GetNowXClampEnd();
	CIntRange	SelX = GetSelectionX();
	if (x1 < x2) {
		for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
			const CChannelInfo& info = m_ChanInfo[iChan];
			int	y = info.m_y;
			int	ChanHeight = info.m_Height;
			// if channel is entirely above or below clip box
			if (y + ChanHeight + GUTTER_HEIGHT - 1 < cb.top || y >= cb.bottom)
				continue;	// skip channel
			double	RealChanOrg = double(ChanHeight - 1) / 2;
			double	VertScale = RealChanOrg * info.m_Amplitude;
			RealChanOrg += ChanHeight * info.m_Origin;
			int	ChanOrg = round(RealChanOrg);
			pDC->IntersectClipRect(x1, y, x2, y + ChanHeight);	// clip to channel
			const BUCKET	*pBucket = &m_Bucket[(x1 + m_BucketMargin) * chans + iChan];
			int	psx = INT_MAX, psy1 = 0, psy2 = 0;
			bool	ChannelSelected = m_ChanInfo[iChan].m_Selected;
			for (int x = x1; x < x2; x++) {
				COLORREF	WaveBkColor, WaveDataColor;
				// if x is within selection, and channel selected
				if (SelX.InRange(x) && ChannelSelected) {
					WaveBkColor = Palette.SelectedWaveBk;
					WaveDataColor = Palette.SelectedWaveData;
				} else {	// not within selection, or channel excluded
					if (ChannelSelected) {	// if channel selected
						WaveBkColor = Palette.WaveBk;
						WaveDataColor = Palette.WaveData;
					} else {	// channel excluded
						WaveBkColor = Palette.ExcludedChanBk;
						WaveDataColor = Palette.ExcludedChanData;
					}
				}
				if (x == NowX) {	// if x is now
					WaveBkColor ^= 0xffffff;	// invert palette
					WaveDataColor ^= 0xffffff;
				}
				ASSERT(IsValidBucket(pBucket));
				SAMPLE	bmin = pBucket->Min;
				SAMPLE	bmax = pBucket->Max;
				if (bmin <= bmax) {	// if valid bucket
					// compute vertical span's endpoints in channel-relative y-coords
					int	sy1 = round(RealChanOrg - double(bmax) / PosRail * VertScale);
					int	sy2 = round(RealChanOrg - double(bmin) / PosRail * VertScale);
					// erase any background above vertical span
					pDC->FillSolidRect(x, y, 1, sy1, WaveBkColor);
					// erase any background below vertical span
					pDC->FillSolidRect(x, y + sy2 + 1, 1, ChanHeight - sy2, WaveBkColor);
					// if vertical span doesn't cross origin, draw origin 
					if (!(sy1 < 0 && sy2 > ChanOrg))
						pDC->FillSolidRect(x, y + ChanOrg, 1, 1, OriginColor);
					// if vertical spans aren't adjacent in x, or don't overlap in y
					if ((x != psx + 1 || sy1 > psy2 || sy2 < psy1) && psx != INT_MAX) {
						int	sy, psy;
						if (sy1 > psy2) {	// if current span is below previous span
							sy = y + sy1;		// draw from current span's sy1
							psy = y + psy2;		// to previous span's sy2
						} else {			// current span is above previous span
							sy = y + sy2;		// draw from current span's sy2
							psy = y + psy1;		// to previous span's sy1
						}
//printf("line (%d,%d)(%d,%d)\n", x, y, psx, py);
						pDC->MoveTo(x, sy);	// draw from current to previous point
						pDC->LineTo(psx, psy);	// so that previous point is excluded
					}
					// draw vertical span
					pDC->FillSolidRect(x, y + sy1, 1, sy2 - sy1 + 1, WaveDataColor);
					psx = x;
					psy1 = sy1;
					psy2 = sy2;
				} else {	// empty bucket
					pDC->FillSolidRect(x, y, 1, ChanHeight, WaveBkColor);
					pDC->SetPixelV(x, y + ChanOrg, OriginColor);
				}
				pBucket += chans;
			}
			pDC->SelectClipRgn(&ClipBoxRgn);	// restore original clip box
			pDC->FillSolidRect(cb.left, y + ChanHeight,	// draw gutter
				ClipWaveWidth, GUTTER_HEIGHT, GutterColor);
		}
	}
	if (chans) {
		const CChannelInfo&	LastChan = m_ChanInfo[chans - 1];
		int	LastChanBottom = LastChan.m_y + LastChan.m_Height + GUTTER_HEIGHT;
		if (LastChanBottom < cb.bottom) {	// if last gutter doesn't reach bottom edge
			pDC->FillSolidRect(cb.left, LastChanBottom,	// extend last gutter
				ClipWaveWidth, cb.bottom - LastChanBottom, GutterColor);
		}
	}
	if (ClipWaveWidth < cb.right) {	// if wave doesn't reach right edge
		cb.left = ClipWaveWidth;
		pDC->FillSolidRect(cb, EmptyColor);
	}
	pDC->SelectObject(PrevPen);
//printf("\tOnDraw T=%f\n", b.Elapsed());
}

BOOL CWaveShopView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;	// no need to erase
}

/////////////////////////////////////////////////////////////////////////////
// CWaveShopView diagnostics

#ifdef _DEBUG
void CWaveShopView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CWaveShopView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CWaveShopDoc* CWaveShopView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWaveShopDoc)));
	return (CWaveShopDoc*)m_pDocument;
}

const CWaveShopDoc* CWaveShopView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWaveShopDoc)));
	return (CWaveShopDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWaveShopView message map

BEGIN_MESSAGE_MAP(CWaveShopView, CScrollView)
	//{{AFX_MSG_MAP(CWaveShopView)
	ON_COMMAND(ID_AUDIO_AMPLIFY, OnAudioAmplify)
	ON_COMMAND(ID_AUDIO_CHANGE_FORMAT, OnAudioChangeFormat)
	ON_COMMAND(ID_AUDIO_EXTRACT_CHANNELS, OnAudioExtractChannels)
	ON_COMMAND(ID_AUDIO_FADE, OnAudioFade)
	ON_COMMAND(ID_AUDIO_FIND_CLIPPING, OnAudioFindClipping)
	ON_COMMAND(ID_AUDIO_INSERT_CHANNEL, OnAudioInsertChannel)
	ON_COMMAND(ID_AUDIO_INVERT, OnAudioInvert)
	ON_COMMAND(ID_AUDIO_NORMALIZE, OnAudioNormalize)
	ON_COMMAND(ID_AUDIO_PEAK_STATS, OnAudioPeakStats)
	ON_COMMAND(ID_AUDIO_RMS_STATS, OnAudioRMSStats)
	ON_COMMAND(ID_AUDIO_RESAMPLE, OnAudioResample)
	ON_COMMAND(ID_AUDIO_REVERSE, OnAudioReverse)
	ON_COMMAND(ID_AUDIO_SPEAKERS, OnAudioSpeakers)
	ON_COMMAND(ID_AUDIO_SPECTRUM, OnAudioSpectrum)
	ON_COMMAND(ID_AUDIO_SWAP_CHANNELS, OnAudioSwapChannels)
	ON_WM_CONTEXTMENU()
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_EDIT_BEGIN_SELECTION, OnEditBeginSelection)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_DESELECT, OnEditDeselect)
	ON_COMMAND(ID_EDIT_END_SELECTION, OnEditEndSelection)
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_COMMAND(ID_EDIT_FIND_NEXT, OnEditFindNext)
	ON_COMMAND(ID_EDIT_FIND_ZERO_CROSSING, OnEditFindZeroCrossing)
	ON_COMMAND(ID_EDIT_GOTO_FIRST_FRAME, OnEditGotoFirstFrame)
	ON_COMMAND(ID_EDIT_GOTO_LAST_FRAME, OnEditGotoLastFrame)
	ON_COMMAND(ID_EDIT_GOTO_NOW, OnEditGotoNow)
	ON_COMMAND(ID_EDIT_GOTO_SEL_END, OnEditGotoSelEnd)
	ON_COMMAND(ID_EDIT_GOTO_SEL_START, OnEditGotoSelStart)
	ON_COMMAND(ID_EDIT_INSERT, OnEditInsert)
	ON_COMMAND(ID_EDIT_INSERT_SILENCE, OnEditInsertSilence)
	ON_COMMAND(ID_EDIT_METADATA, OnEditMetadata)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_FILE_INFO, OnFileInfo)
	ON_WM_HSCROLL()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_PREV_PANE, OnNextPane)
	ON_COMMAND(ID_NEXT_PANE, OnNextPane)
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(ID_AUDIO_SWAP_CHANNELS, OnUpdateAudioSwapChannels)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO_SEL_END, OnUpdateEditDeselect)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DESELECT, OnUpdateEditDeselect)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO_SEL_START, OnUpdateEditDeselect)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CTX_PASTE, OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_NEXT_PANE, OnUpdateNextPane)
	ON_UPDATE_COMMAND_UI(ID_PREV_PANE, OnUpdateNextPane)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CTX_DELETE_CHANNEL, OnUpdateViewCtxDeleteChannel)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CTX_MAXIMIZE_CHANNEL, OnUpdateViewCtxMaximizeChannel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_BEGIN_SELECTION, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_REVERSE, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CTX_END_SELECTION, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CTX_START_SELECTION, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_FIND_CLIPPING, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_ZERO_CROSSING, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_AMPLIFY, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_SPECTRUM, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_NEXT, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO_FIRST_FRAME, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_FADE, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_PEAK_STATS, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO_NOW, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_INVERT, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO_LAST_FRAME, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_NORMALIZE, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_EDIT_END_SELECTION, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_RMS_STATS, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_RESAMPLE, OnUpdateWaveNotEmpty)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CTX_INSERT_CHANNEL, OnUpdateWaveValid)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateWaveValid)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT, OnUpdateWaveValid)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_CHANGE_FORMAT, OnUpdateWaveValid)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_SPEAKERS, OnUpdateWaveValid)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_EXTRACT_CHANNELS, OnUpdateWaveValid)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_SILENCE, OnUpdateWaveValid)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateWaveValid)
	ON_COMMAND(ID_VIEW_CTX_DELETE_CHANNEL, OnViewCtxDeleteChannel)
	ON_COMMAND(ID_VIEW_CTX_END_SELECTION, OnViewCtxEndSelection)
	ON_COMMAND(ID_VIEW_CTX_INSERT_CHANNEL, OnViewCtxInsertChannel)
	ON_COMMAND(ID_VIEW_CTX_MAXIMIZE_CHANNEL, OnViewCtxMaximizeChannel)
	ON_COMMAND(ID_VIEW_CTX_PASTE, OnViewCtxPaste)
	ON_COMMAND(ID_VIEW_CTX_START_SELECTION, OnViewCtxStartSelection)
	ON_COMMAND(ID_VIEW_CTX_ZOOM_IN, OnViewCtxZoomIn)
	ON_COMMAND(ID_VIEW_CTX_ZOOM_OUT, OnViewCtxZoomOut)
	ON_COMMAND(ID_VIEW_FIT_VERTICALLY, OnViewFitVertically)
	ON_COMMAND(ID_VIEW_FIT_WINDOW, OnViewFitWindow)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	ON_COMMAND(ID_VIEW_ZOOM_IN, OnViewZoomIn)
	ON_COMMAND(ID_VIEW_ZOOM_OUT, OnViewZoomOut)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(UWM_VIEW_FIT_IN_WINDOW, OnFitInWindow)
	ON_COMMAND_RANGE(CPluginManager::CMD_ID_FIRST, CPluginManager::CMD_ID_LAST, OnPlugin)
	ON_UPDATE_COMMAND_UI_RANGE(CPluginManager::CMD_ID_FIRST, CPluginManager::CMD_ID_LAST, OnUpdateWaveNotEmpty)
	ON_COMMAND(ID_PLUGIN_REPEAT, OnPluginRepeat)
	ON_UPDATE_COMMAND_UI(ID_PLUGIN_REPEAT, OnUpdatePluginRepeat)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaveShopView message handlers

int CWaveShopView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
//printf("CWaveShopView::OnCreate %x\n", this);
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_Main = theApp.GetMain();
	
	return 0;
}

void CWaveShopView::OnInitialUpdate() 
{
//printf("OnInitialUpdate %s\n", GetDocument()->GetTitle());
	CWaveShopDoc	*pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	m_Wave = &pDoc->m_Wave;
	EnableScrollBarCtrl(SB_VERT, FALSE);	// disable vertical scrolling
	m_TimeRuler->SetNumericFormat(CRulerCtrl::NF_FIXED, 0);
	HGDIOBJ	hFont = GetStockObject(DEFAULT_GUI_FONT);
	m_TimeRuler->SendMessage(WM_SETFONT, WPARAM(hFont));
	DeferFitInWindow();
	OnUpdate(NULL, HINT_INITIAL_UPDATE, NULL);
}

void CWaveShopView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
//printf("OnUpdate %d\n", lHint);
	Invalidate();	// always invalidate
	if (lHint & HINT_WAVE_FORMAT)
		OnWaveFormatChange();	// order matters
	if (lHint & HINT_TIME_UNIT)
		OnTimeUnitChange();
	if (lHint & HINT_WAVE_UPDATE)
		OnWaveUpdate();
	if (lHint & HINT_SHOW_CAPTIONS)
		ShowCaptions(m_Main->GetOptions().m_ShowChannelNames);
}

LRESULT	CWaveShopView::OnFitInWindow(WPARAM wParam, LPARAM lParam)
{
	m_DeferBuckets = FALSE;	// order matters
	FitInWindow();
	return(0);
}

void CWaveShopView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	// Don't call scroll view's override; it plays viewport games to compensate
	// the DC for scrolling, but it breaks for huge scrolls (e.g. 100M pixels),
	// causing GetClipBox to return a rectangle with wrapped negative values.
	CView::OnPrepareDC(pDC, pInfo);	// doesn't compensate DC for scrolling
}

void CWaveShopView::OnSize(UINT nType, int cx, int cy) 
{
//printf("OnSize %d %d\n", cx, cy);
	if (cx != m_WndSize.cx) {	// if width changed
		int	PageWidth = CalcPageWidth(cx);
		int	buckets = CalcNumBuckets(PageWidth);
		int	PrevBuckets = m_Bucket.GetSize();	// save bucket count
		m_Bucket.SetSize(buckets);	// resize bucket array
		int	PrevWndWidth = m_WndSize.cx;	// save previous window width
		m_WndSize.cx = cx;	// update window width member before scrolling
		ScrollToPosition(m_ScrollPos);	// constrain scroll position
		if (buckets > PrevBuckets) {	// if buckets were added
			int	PrevPageWidth = CalcPageWidth(PrevWndWidth);
			UpdateBuckets(PrevPageWidth, PageWidth);	// fill new buckets
		}
	}
	if (cy != m_WndSize.cy) {	// if height changed
		UpdateChannelHeights(cy);
		Invalidate();	// repaint entire view
		m_WndSize.cy = cy;	// update window height member
	}
	UpdateScrollSize();
	CScrollView::OnSize(nType, cx, cy);
}

BOOL CWaveShopView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint	CursorPt;
	GetCursorPos(&CursorPt);
	ScreenToClient(&CursorPt);
	int	ItemIdx;
	int	HitCode = HitTest(CursorPt, ItemIdx);
	LPCTSTR	StdCursor = NULL;
	switch (HitCode) {
	case HT_SEL_START:
	case HT_SEL_END:
		StdCursor = IDC_SIZEWE;
		break;
	case HT_GUTTER:
		StdCursor = IDC_SIZENS;
		break;
	}
	if (StdCursor != NULL) {
		HCURSOR	hCursor = theApp.LoadStandardCursor(StdCursor);
		SetCursor(hCursor);
		return TRUE;
	}
	return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

void CWaveShopView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// nPos is 16-bit and may wrap if scroll view size exceeds 32767
	if (nSBCode == SB_THUMBTRACK || nSBCode == SB_THUMBPOSITION) {
		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_TRACKPOS;
		::GetScrollInfo(m_hWnd, SB_HORZ, &info);	// get horizontal scroll info
		nPos = info.nTrackPos;	// get 32-bit position
	}
//printf("OnHScroll %d %d\n", nSBCode, nPos);
	CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CWaveShopView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll) 
{
//printf("OnScroll code=%d nPos=%d\n", LOBYTE(nScrollCode), nPos);
	switch (LOBYTE(nScrollCode)) {
	case SB_TOP:
		ScrollToPosition(0);
		break;
	case SB_BOTTOM:
		ScrollToPosition(m_WaveWidth);
		break;
	case SB_LINEUP:
		ScrollToPosition(m_ScrollPos - m_LineSize);
		break;
	case SB_LINEDOWN:
		ScrollToPosition(m_ScrollPos + m_LineSize);
		break;
	case SB_PAGEUP:
		ScrollToPosition(m_ScrollPos - m_PageSize);
		break;
	case SB_PAGEDOWN:
		ScrollToPosition(m_ScrollPos + m_PageSize);
		break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		{
			LONGLONG	ScrollPos = round64(nPos * m_ScrollScale);
			if (m_ScrollScale > 1) {	// if scrolling is scaled
				// if unscaled scroll position is within a screen width of maximum
				if (nPos + m_WndSize.cx >= round64(GetMaxScrollPos() / m_ScrollScale))
					ScrollPos = m_WaveWidth;	// force scroll to end
			}
			ScrollToPosition(ScrollPos);
		}
		break;
	}
	return TRUE;
}

BOOL CWaveShopView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll) 
{
	return FALSE;
}

void CWaveShopView::OnLButtonDown(UINT nFlags, CPoint point) 
{
//printf("OnLButtonDown\n");
	m_DragOrigin = point;	// store initial point
	int	iChan = FindGutter(point.y);
	if (iChan > 0) {	// if on a gutter
		m_DragState = DRAG_GUTTER;	// set drag state to dragging gutter
		m_DragChanOrigin = m_ChanInfo[iChan].m_y - GUTTER_HEIGHT - point.y;
		m_DragChannel = iChan;
	} else {	// not on a gutter
		iChan = ToggleChannelSelection(nFlags, point);
		if (iChan >= 0) {	// if channel selection was toggled
			if (GetChannelCount() > 2) {	// if more than two channels
				m_DragState = DRAG_TRACK_CHAN_SELECT;	// track drag selection
				m_DragChannel = iChan;
			}
		} else {	// normal case
			int	ItemIdx;
			int	HitCode = HitTest(point, ItemIdx);
			switch (HitCode) {
			case HT_SEL_START:	// if on a selection boundary
			case HT_SEL_END:
				{
					m_DragState = DRAG_SELECTION;	// set drag state to selecting
					// override drag origin to be selection's other boundary
					if (HitCode == HT_SEL_START)
						m_DragSelOrigin = m_Selection.End;
					else
						m_DragSelOrigin = m_Selection.Start;
				}
				break;
			default:	// nowhere special
				if (!m_Wave->IsEmpty())	// if non-empty wave
					m_Main->SetNow(XToFrame(point.x));	// set now marker
				m_DragState = DRAG_TRACK;	// begin tracking possible drag
			}
		}
	}
	if (m_DragState != DRAG_NONE)
		SetCapture();	// capture mouse input
	CScrollView::OnLButtonDown(nFlags, point);
}

void CWaveShopView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int	iChan = FindGutter(point.y);
	if (iChan > 0) {	// if on a gutter
		CChannelInfo&	upr = m_ChanInfo[iChan - 1];
		CChannelInfo&	lwr = m_ChanInfo[iChan];
		double	avg = (upr.m_HeightFrac + lwr.m_HeightFrac) / 2;
		upr.m_HeightFrac = avg;	// make adjacent channels same height
		lwr.m_HeightFrac = avg;
		UpdateChannelHeights(m_WndSize.cy);
		Invalidate();
	} else	// not on a gutter
		ToggleChannelSelection(nFlags, point);	// try channel selection
	CScrollView::OnLButtonDblClk(nFlags, point);
}

void CWaveShopView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	EndDrag();
	CScrollView::OnLButtonUp(nFlags, point);
}

void CWaveShopView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CScrollView::OnRButtonDown(nFlags, point);
}

void CWaveShopView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CMainFrame	*pMain = m_Main;
	if (pMain->GetStatusBar().IsWindowVisible()) {
		if (!m_Wave->IsEmpty()) {	// if non-empty wave
			double	FrameIdx = XToFrame(point.x);	// convert x to sample frame index
			CString	s;
			if (m_TimeRuler->GetUnit() == CRulerCtrl::UNIT_TIME) {	// if showing seconds
				double	secs = FrameIdx /= m_Wave->GetSampleRate();	// convert to seconds
				s = CRulerCtrl::FormatTime(secs);
			} else	// showing time in sample frames
				s.Format(_T("%I64d"), trunc64(FrameIdx));
			m_Main->SetCursorWavePos(s);	// display cursor wave position
		}
		if (!m_IsTrackingMouse) {	// if not already tracking mouse
			TRACKMOUSEEVENT	tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = m_hWnd;
			_TrackMouseEvent(&tme);	// request WM_MOUSELEAVE notification
			m_IsTrackingMouse = TRUE;
		}
		switch (m_DragState) {
		case DRAG_GUTTER:
			DragGutter(point.y + m_DragChanOrigin);
			break;
		case DRAG_TRACK:
			{
				// if mouse motion relative to origin exceeds drag threshold
				int	DragThreshold = GetSystemMetrics(SM_CXDRAG);
				if (abs(point.x - m_DragOrigin.x) >= DragThreshold) {
					m_DragState = DRAG_SELECTION;	// set drag state to selecting
					HCURSOR	hCursor = theApp.LoadStandardCursor(IDC_SIZEWE);
					SetCursor(hCursor);
					m_DragSelOrigin = XToFrame(m_DragOrigin.x);
					// recurse to draw selection; safe due to changed drag state
					OnMouseMove(nFlags, point);
				}
			}
			break;
		case DRAG_SELECTION:
			{
				CDblRange	sel(XToFrame(point.x), m_DragSelOrigin);
				sel.Normalize();
				m_Main->SetSelection(sel);
				bool	HaveTimer = m_DragScrollDelta != 0;
				CPoint	ScreenPt(point);
				ClientToScreen(&ScreenPt);	// convert cursor position to screen coords
				int	ScreenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
				int	ScreenRight = ScreenLeft + GetSystemMetrics(SM_CXVIRTUALSCREEN);
				// special case if cursor at left or right edge of virtual screen
				if (ScreenPt.x <= ScreenLeft)	// if cursor at left edge
					m_DragScrollDelta = -m_LineSize;	// scroll left by line size
				else if (ScreenPt.x >= ScreenRight - 1)	// if cursor at right edge
					m_DragScrollDelta = m_LineSize;	// scroll right by line size
				else {	// scroll speed is proportional to distance beyond view
					if (point.x < 0)	// if cursor left of view
						m_DragScrollDelta = point.x;	// scroll left
					else if (point.x >= m_WndSize.cx)	// if cursor right of view
						m_DragScrollDelta  = point.x - m_WndSize.cx;	// scroll right
					else	// cursor within view
						m_DragScrollDelta = 0;	// stop scrolling
				}
				bool	NeedTimer = m_DragScrollDelta != 0;
				if (NeedTimer != HaveTimer) {
					if (m_DragScrollDelta)
						SetTimer(SCROLL_TIMER_ID, SCROLL_TIMER_PERIOD, NULL);
					else
						KillTimer(SCROLL_TIMER_ID);
				}
			}
			break;
		case DRAG_TRACK_CHAN_SELECT:
			{
				// if mouse motion relative to origin exceeds drag threshold
				int	DragThreshold = GetSystemMetrics(SM_CYDRAG);
				if (abs(point.y - m_DragOrigin.y) >= DragThreshold) {
					m_DragState = DRAG_CHANNEL_SELECTION;	// set drag state
					HCURSOR	hCursor = theApp.LoadStandardCursor(IDC_SIZENS);
					SetCursor(hCursor);
					// recurse to draw selection; safe due to changed drag state
					OnMouseMove(nFlags, point);
				}
			}
			break;
		case DRAG_CHANNEL_SELECTION:
			{
				bool	IsSel = m_ChanInfo[m_DragChannel].m_Selected;
				// if selecting or at least one channel currently selected
				if (IsSel || GetChannelSelection() > 1)  {
					CIntRange	range(m_DragChannel, FindChannelFuzzy(point.y));
					range.Normalize();	// enforce ascending order
					// for each channel between drag origin and channel at cursor
					for (int iChan = range.Start; iChan <= range.End; iChan++)
						SelectChannel(iChan, IsSel);	// set channel's selection state
				}
			}
			break;
		}
	}
	CScrollView::OnMouseMove(nFlags, point);
}

void CWaveShopView::OnTimer(W64UINT nIDEvent) 
{
	if (nIDEvent == SCROLL_TIMER_ID) {
		ASSERT(m_DragScrollDelta);	// else logic error
		ScrollToPosition(m_ScrollPos + m_DragScrollDelta);
		if (m_DragState == DRAG_SELECTION) {	// if dragging selection
			CPoint	CursorPos;
			GetCursorPos(&CursorPos);	// get cursor position
			ScreenToClient(&CursorPos);
			CDblRange	sel(XToFrame(CursorPos.x), m_DragSelOrigin);
			sel.Normalize();
			m_Main->SetSelection(sel);	// update selection
		}
	}
	CScrollView::OnTimer(nIDEvent);
}

LRESULT CWaveShopView::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_IsTrackingMouse = FALSE;
	CMainFrame	*pMain = m_Main;
	if (pMain->GetStatusBar().IsWindowVisible())
		pMain->SetCursorWavePos(_T(""));
	return(0);
}

BOOL CWaveShopView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (GetKeyState(VK_CONTROL) & GKS_DOWN) {
		CPoint	ClientPt(pt);
		ScreenToClient(&ClientPt);
		StepZoom(ClientPt.x, zDelta > 0);
		return(TRUE);
	} else if (GetKeyState(VK_SHIFT) & GKS_DOWN) {
		CPoint	ClientPt(pt);
		ScreenToClient(&ClientPt);
		int iChan = FindChannel(ClientPt.y);
		if (iChan >= 0) {
			if (m_Main->GetOptions().m_VertZoomCursor)
				StepVerticalZoom(iChan, ClientPt.y, zDelta > 0);
			else
				StepAmplitude(iChan, zDelta > 0);
		}
		return(TRUE);
	}
	UINT	nWheelScrollLines = 3;
    ::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nWheelScrollLines, 0);
	int	ScrollDelta = zDelta * m_LineSize / WHEEL_DELTA * nWheelScrollLines;
	ScrollToPosition(m_ScrollPos + ScrollDelta);
	return TRUE;
}

void CWaveShopView::OnKillFocus(CWnd* pNewWnd) 
{
	EndDrag();
	CScrollView::OnKillFocus(pNewWnd);
}

void CWaveShopView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	int	nSBCode = -1;
	switch (nChar) {
	case VK_LEFT:
	case VK_UP:
		nSBCode = SB_LINELEFT;
		break;
	case VK_RIGHT:
	case VK_DOWN:
		nSBCode = SB_LINERIGHT;
		break;
	case VK_PRIOR:
		nSBCode = SB_PAGELEFT;
		break;
	case VK_NEXT:
		nSBCode = SB_PAGERIGHT;
		break;
	case VK_HOME:
		nSBCode = SB_LEFT;
		break;
	case VK_END:
		nSBCode = SB_RIGHT;
		break;
	}
	if (nSBCode >= 0)
		OnHScroll(nSBCode, 0, NULL);
	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CWaveShopView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (point.x == -1 && point.y == -1) {	// if menu triggered via keyboard
		CRect	rc(CPoint(0, 0), m_WndSize);
		point = rc.CenterPoint();
		ClientToScreen(&point);
	}
	m_ContextTarget = point;
	ScreenToClient(&m_ContextTarget);
	CMenu	menu;
	menu.LoadMenu(IDR_VIEW_CTX);
	CMenu	*mp = menu.GetSubMenu(0);
	theApp.UpdateMenu(this, &menu);
	mp->TrackPopupMenu(0, point.x, point.y, m_Main);	// owner is main so hints work
}

void CWaveShopView::OnNextPane() 
{
	CWnd	*FocusWnd = GetFocus();
	if (FocusWnd == this)	// if we have focus
		FocusWnd = &m_Main->GetNavBar();	// give focus to navigation bar
	else
		FocusWnd = this;	// take focus
	FocusWnd->SetFocus();
}

void CWaveShopView::OnUpdateNextPane(CCmdUI* pCmdUI) 
{
	// base class disables ID_NEXT_PANE if we're not a splitter
	pCmdUI->Enable(m_Main->GetNavBar().IsWindowVisible());
}

HBRUSH CWaveShopView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	return (HBRUSH)GetStockObject(WHITE_BRUSH);
}

void CWaveShopView::OnFileInfo() 
{
	CWave&	wave = GetWave();
	double	secs;
	if (wave.IsValid())
		secs = double(wave.GetFrameCount() / wave.GetSampleRate());	// nearest second
	else	// avoid divide by zero
		secs = 0;
	CString	duration(CRulerCtrl::FormatTime(secs));
	LONGLONG	TotalBytes = wave.GetFrameCount() * wave.GetFrameSize();
	CString	sTotalBytes = CNumFormat::FormatByteSize(TotalBytes);
	CString	msg;
	CNumFormat	fmt;
	msg.Format(IDS_FILE_INFO,
		duration,
		fmt.Format(wave.GetFrameCount()),
		wave.GetChannels(),
		fmt.Format(wave.GetSampleRate()),
		wave.GetSampleBits(),
		wave.GetFrameSize(),
		sTotalBytes,
		fmt.Format(TotalBytes)
	);
	MessageBox(msg, GetDocument()->GetTitle(), MB_ICONINFORMATION);
}

void CWaveShopView::OnFileExport() 
{
	CExportDlg	dlg(*m_Wave);
	if (dlg.DoModal() == IDOK) {
		CPathStr	FileName(GetDocument()->GetTitle());
		FileName.RemoveExtension();
		FileName += dlg.GetSelExtension();
		CFileDialog	fd(FALSE, dlg.GetSelExtension(), FileName, 
			OFN_OVERWRITEPROMPT, dlg.GetSelFilter());
		CString	title((LPCTSTR)IDS_FILE_EXPORT_AS);
		fd.m_ofn.lpstrTitle = title;
		if (fd.DoModal() == IDOK) {
			m_Wave->Export(fd.GetPathName(), dlg.GetSelFormat(), 
				&GetDocument()->m_Metadata);
		}
	}
}

void CWaveShopView::OnUpdateWaveValid(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_Wave->IsValid());
}

void CWaveShopView::OnUpdateWaveNotEmpty(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_Wave->IsEmpty());
}

void CWaveShopView::OnEditUndo() 
{
	GetDocument()->m_UndoMgr.Undo();
}

void CWaveShopView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	CWaveShopDoc	*pDoc = GetDocument();
	CString	Text;
	Text.Format(LDS(IDS_EDIT_UNDO_FMT), pDoc->m_UndoMgr.GetUndoTitle());
	pCmdUI->SetText(Text);
	pCmdUI->Enable(pDoc->m_UndoMgr.CanUndo());
}

void CWaveShopView::OnEditRedo() 
{
	GetDocument()->m_UndoMgr.Redo();
}

void CWaveShopView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	CWaveShopDoc	*pDoc = GetDocument();
	CString	Text;
	Text.Format(LDS(IDS_EDIT_REDO_FMT), pDoc->m_UndoMgr.GetRedoTitle());
	pCmdUI->SetText(Text);
	pCmdUI->Enable(pDoc->m_UndoMgr.CanRedo());
}

void CWaveShopView::OnEditCopy() 
{
	if (!CFocusEdit::Copy()) {
		if (!Copy())
			AfxMessageBox(IDS_VIEW_CANT_COPY);
	}
}

void CWaveShopView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	if (!CFocusEdit::UpdateCopy(pCmdUI))
		pCmdUI->Enable(HaveSelection());
}

void CWaveShopView::OnEditCut() 
{
	if (!CFocusEdit::Cut()) {
		if (!Delete(TRUE))	// cut to clipboard
			AfxMessageBox(IDS_VIEW_CANT_CUT);
	}
}

void CWaveShopView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	if (!CFocusEdit::UpdateCut(pCmdUI))
		pCmdUI->Enable(HaveSelection());
}

void CWaveShopView::OnEditPaste() 
{
	if (!CFocusEdit::Paste()) {
		if (!Paste())
			AfxMessageBox(IDS_VIEW_CANT_PASTE);
	}
}

void CWaveShopView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	if (!CFocusEdit::UpdatePaste(pCmdUI))
		pCmdUI->Enable(!theApp.m_Clipboard.IsEmpty());
}

void CWaveShopView::OnEditDelete() 
{
	if (!CFocusEdit::Delete()) {
		if (!Delete(FALSE))	// don't cut to clipboard
			AfxMessageBox(IDS_VIEW_CANT_DELETE);
	}
}

void CWaveShopView::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
	if (!CFocusEdit::UpdateDelete(pCmdUI))
		pCmdUI->Enable(HaveSelection());
}

void CWaveShopView::OnEditInsert() 
{
	Insert();
}

void CWaveShopView::OnEditInsertSilence() 
{
	CInsertSilenceDlg	dlg(m_Wave->GetSampleRate());
	if (dlg.DoModal() == IDOK) {
		if (!InsertSilence(dlg.GetFrameCount()))
			AfxMessageBox(IDS_VIEW_CANT_INSERT);
	}
}

void CWaveShopView::OnEditBeginSelection() 
{
	m_Main->SetSelection(CDblRange(m_Now, max(m_Selection.End, m_Now)));
}

void CWaveShopView::OnEditEndSelection() 
{
	m_Main->SetSelection(CDblRange(min(m_Selection.Start, m_Now), m_Now));
}

void CWaveShopView::OnEditSelectAll() 
{
	m_Main->SetSelection(CDblRange(0, double(m_Wave->GetFrameCount())));
}

void CWaveShopView::OnEditDeselect() 
{
	m_Main->SetSelection(CDblRange(0, 0));
}

void CWaveShopView::OnUpdateEditDeselect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(HaveSelection());
}

void CWaveShopView::OnEditGotoFirstFrame() 
{
	m_Main->SetNow(0);
}

void CWaveShopView::OnEditGotoLastFrame() 
{
	m_Main->SetNow(double(m_Wave->GetFrameCount()));
}

void CWaveShopView::OnEditGotoNow() 
{
	EnsureVisible(m_Now);
}

void CWaveShopView::OnEditGotoSelStart() 
{
	EnsureVisible(m_Selection.Start);
	m_Main->SetNow(m_Selection.Start);
}

void CWaveShopView::OnEditGotoSelEnd() 
{
	EnsureVisible(m_Selection.End);
	m_Main->SetNow(m_Selection.End);
}

void CWaveShopView::OnEditFindZeroCrossing() 
{
	m_Main->SetNow(FindZeroCrossing(double(GetIntNow())));
	if (HaveSelection()) {
		CW64IntRange	sel = GetIntSelection();
		m_Main->SetSelection(CDblRange(
			FindZeroCrossing(double(sel.Start)), 
			FindZeroCrossing(double(sel.End))));
	}
}

void CWaveShopView::OnEditFind() 
{
	Find(TRUE);	// find first
}

void CWaveShopView::OnEditFindNext() 
{
	Find(FALSE);	// find next
}

void CWaveShopView::OnEditMetadata() 
{
	CStringArray	metadata;
	metadata.Copy(GetDocument()->m_Metadata);
	CMetadataDlg	dlg(metadata);
	if (dlg.DoModal() == IDOK) {
		NotifyUndoableEdit(0, UCODE_METADATA);
		GetDocument()->m_Metadata.Copy(metadata);
	}
}

void CWaveShopView::OnViewFitWindow() 
{
	FitInWindow();
}

void CWaveShopView::OnViewFitVertically() 
{
	FitVertically();
}

void CWaveShopView::OnViewZoomIn() 
{
	EnsureVisible(m_Now);
	StepZoom(GetNowX(), TRUE);
}

void CWaveShopView::OnViewZoomOut() 
{
	EnsureVisible(m_Now);
	StepZoom(GetNowX(), FALSE);
}

void CWaveShopView::OnViewCtxZoomIn()
{
	StepZoom(m_ContextTarget.x, TRUE);
}

void CWaveShopView::OnViewCtxZoomOut()
{
	StepZoom(m_ContextTarget.x, FALSE);
}

void CWaveShopView::OnViewCtxStartSelection()
{
	double	start = XToFrame(m_ContextTarget.x);
	m_Main->SetSelection(CDblRange(start, max(m_Selection.End, start)));
}

void CWaveShopView::OnViewCtxEndSelection()
{
	double	end = XToFrame(m_ContextTarget.x);
	m_Main->SetSelection(CDblRange(min(m_Selection.Start, end), end));
}

void CWaveShopView::OnViewCtxPaste()
{
	m_Main->SetNow(XToFrame(m_ContextTarget.x));
	Paste();
}

void CWaveShopView::OnViewCtxMaximizeChannel()
{
	int iChan = FindChannel(m_ContextTarget.y);
	if (iChan >= 0)
		MaximizeChannel(iChan);
}

void CWaveShopView::OnUpdateViewCtxMaximizeChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetChannelCount() > 1 && FindChannel(m_ContextTarget.y) >= 0);
}

void CWaveShopView::OnViewCtxInsertChannel()
{
	int iChan = FindInsertPosition(m_ContextTarget.y);	// get insert channel index
	InsertChannel(iChan);
}

void CWaveShopView::OnViewCtxDeleteChannel()
{
	int iChan = FindChannel(m_ContextTarget.y);
	if (iChan >= 0)
		DeleteChannel(iChan);
}

void CWaveShopView::OnUpdateViewCtxDeleteChannel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FindChannel(m_ContextTarget.y) >= 0);
}

void CWaveShopView::OnViewRefresh() 
{
	Invalidate();
}

void CWaveShopView::OnAudioAmplify() 
{
	ForceSelection();
	CW64IntRange	sel = GetIntSelection();
	CWaveProcess::CPeakStats	stats;
	CByteArray	ChanSel;
	GetChannelSelection(&ChanSel);
	if (!m_Wave->GetPeakStats(sel, stats, ChanSel.GetData()))
		return;
	CAmplifyDlg	dlg(stats.GetPeakDecibels());
	if (dlg.DoModal() != IDOK)
		return;
	NotifyUndoableEdit(0, UCODE_AMPLIFY);
	bool	retc = m_Wave->Amplify(sel, dlg.GetGain());
	OnAudioProcessResult(retc);
}

void CWaveShopView::OnAudioExtractChannels() 
{
	CPathStr	DocFolder(GetDocument()->GetPathName());
	DocFolder.RemoveFileSpec();	// folder path only
	CPathStr	DstPath;
	UINT	flags = BIF_NEWDIALOGSTYLE;
	CString	Title(LPCTSTR(IDS_MAIN_DESTINATION_FOLDER));
	if (CFolderDialog::BrowseFolder(Title, DstPath, NULL, flags, DocFolder)) {
		DstPath.Append(GetDocument()->GetTitle());
		m_Wave->ExtractChannels(DstPath);
	}
}

void CWaveShopView::OnAudioFade() 
{
	ForceSelection();
	CFadeDlg	dlg;
	if (dlg.DoModal() != IDOK)
		return;
	CW64IntRange	sel = GetIntSelection();
	NotifyUndoableEdit(0, UCODE_FADE);
	bool	retc = m_Wave->Fade(sel, dlg.GetRange(), dlg.IsLog());
	OnAudioProcessResult(retc);
}

void CWaveShopView::OnAudioFindClipping() 
{
	CFindClippingDlg	dlg;
	if (dlg.DoModal() != IDOK)
		return;
	ForceSelection();
	CW64IntRange	sel = GetIntSelection();
	CWaveProcess::CClipSpanArray	ClipSpan;
	CWaveProcess::CLIP_THRESHOLD	threshold = {
		dlg.m_StartThreshold, dlg.m_StopThreshold, dlg.m_ClippingLevel};
	bool	retc = m_Wave->FindClipping(sel, threshold, ClipSpan);
	if (retc) {
		// clipping stats are passed to bar via pointer swap instead of copying
		if (m_Main->GetResultsBar().ReportClipping(ClipSpan, this))
			m_Main->ShowControlBar(&m_Main->GetResultsBar(), TRUE, 0);	// show bar
	}
}

void CWaveShopView::OnAudioChangeFormat() 
{
	CChangeFormatDlg	dlg;
	dlg.m_Channels = m_Wave->GetChannels();
	dlg.m_SampleRate = m_Wave->GetSampleRate();
	dlg.m_SampleBits = m_Wave->GetSampleBits();
	if (dlg.DoModal() != IDOK)
		return;
	if (dlg.m_Channels == m_Wave->GetChannels()
	&& dlg.m_SampleRate == m_Wave->GetSampleRate()
	&& dlg.m_SampleBits == m_Wave->GetSampleBits())
		return;	// format unchanged
	NotifyUndoableEdit(0, UCODE_CHANGE_FORMAT);
	CMainFrame::CPlay	stop(this);	// suspend playing our wave
	bool	retc = m_Wave->ChangeFormat(
		dlg.m_Channels, dlg.m_SampleRate, dlg.m_SampleBits);
	OnAudioProcessResult(retc, HINT_WAVE_UPDATE | HINT_WAVE_FORMAT);
}

void CWaveShopView::OnAudioInsertChannel() 
{
	InsertChannel(m_Wave->GetChannels());	// append
}

void CWaveShopView::OnAudioInvert() 
{
	ForceSelection();
	NotifyUndoableEdit(0, UCODE_INVERT);
	bool	retc = m_Wave->Invert(GetIntSelection());
	OnAudioProcessResult(retc);
}

void CWaveShopView::OnAudioNormalize() 
{
	ForceSelection();
	CNormalizeDlg	dlg;
	if (dlg.DoModal() != IDOK)
		return;
	NotifyUndoableEdit(0, UCODE_NORMALIZE);
	UINT	flags = 0;
	if (dlg.m_Unbias)
		flags |= CWaveProcess::NORM_UNBIAS;
	if (dlg.m_Normalize)
		flags |= CWaveProcess::NORM_NORMALIZE;
	if (dlg.m_Independent)
		flags |= CWaveProcess::NORM_INDEPENDENT;
	CByteArray	ChanSel;
	GetChannelSelection(&ChanSel);
	bool	retc = m_Wave->Normalize(GetIntSelection(), flags, dlg.m_PeakLevel, 
		ChanSel.GetData());
	OnAudioProcessResult(retc);
}

void CWaveShopView::OnAudioPeakStats() 
{
	ForceSelection();
	CWaveProcess::CPeakStats	stats;
	if (m_Wave->GetPeakStats(GetIntSelection(), stats)) {
		CPeakStatsDlg	dlg(stats, this);
		dlg.DoModal();
	}
}

void CWaveShopView::OnAudioResample() 
{
	UINT	SampleRate = m_Wave->GetSampleRate(); 
	CResampleDlg	dlg(SampleRate);
	if (dlg.DoModal() != IDOK)
		return;
	NotifyUndoableEdit(0, UCODE_RESAMPLE);
	CMainFrame::CPlay	stop(this);	// suspend playing our wave
	OnEditDeselect();	// clear selection since frame count may change
	OnEditGotoFirstFrame();	// move audio cursor somewhere safe
	bool	retc = m_Wave->Resample(dlg.GetSampleRate(), dlg.GetQuality());
	OnAudioProcessResult(retc, HINT_WAVE_UPDATE | HINT_WAVE_FORMAT);
}

void CWaveShopView::OnAudioReverse() 
{
	ForceSelection();
	NotifyUndoableEdit(0, UCODE_REVERSE);
	bool	retc = m_Wave->Reverse(GetIntSelection());
	OnAudioProcessResult(retc);
}

void CWaveShopView::OnAudioRMSStats() 
{
	ForceSelection();
	CRMSStatsDlg	dlg(this);
	dlg.DoModal();
}

void CWaveShopView::OnAudioSpeakers() 
{
	CSpeakersDlg	dlg;
	dlg.SetChannelMask(m_Wave->GetChannels(), m_Wave->GetChannelMask());
	if (dlg.DoModal() == IDOK) {
		NotifyUndoableEdit(0, UCODE_SPEAKERS);
		SetChannelMask(dlg.GetChannelMask());
	}
}

void CWaveShopView::OnAudioSpectrum() 
{
	ForceSelection();
	CSpectrumDlg	dlg(this);
	dlg.DoModal();
}

void CWaveShopView::OnAudioSwapChannels() 
{
	ASSERT(m_Wave->GetChannels() >= 2);	// need at least two channels to swap
	CUIntRange	Pair;
	if (m_Wave->GetChannels() > 2) {
		CSwapChannelsDlg	dlg(*m_Wave);
		if (dlg.DoModal() != IDOK)
			return;
		Pair = dlg.GetPair();
	} else
		Pair = CUIntRange(0, 1);
	ForceSelection();
	NotifyUndoableEdit(0, UCODE_SWAP_CHANNELS);
	bool	retc = m_Wave->SwapChannels(GetIntSelection(), Pair);
	OnAudioProcessResult(retc);
}

void CWaveShopView::OnUpdateAudioSwapChannels(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_Wave->GetChannels() >= 2);
}

void CWaveShopView::OnPluginRepeat()
{
	OnPlugin(m_LastPluginCmdID);
}

void CWaveShopView::OnUpdatePluginRepeat(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_LastPluginCmdID && !m_Wave->IsEmpty());
}

void CWaveShopView::OnPlugin(UINT nID)
{
	CPluginManager&	PluginMgr = m_Main->GetPluginManager();
	int	PluginIdx = PluginMgr.GetPluginIndex(nID);
	CString	PluginFileName = PluginMgr.GetFileName(PluginIdx);
	CString	PluginFolder(PluginMgr.GetPluginFolder());
	CPathStr	path(PluginFolder);
	path.Append(PluginFileName);
	path += _T(".dll");
	CPlugin::CParamArray	Param;
	PluginMgr.GetParams(PluginIdx, Param);
	W64INT	retc;
	{
		CPluginParamDlg	dlg(path, m_Wave->GetSampleRate(), Param);
		retc = dlg.DoModal();
	}	// destroy parameters dialog before proceeding, to conserve memory
	if (retc == IDOK) {
		PluginMgr.SetParams(PluginIdx, Param);
		CPlugin	plug;
		if (plug.Create(path, m_Wave->GetSampleRate())) {	// load plugin
			CByteArray	ChanSel;
			GetChannelSelection(&ChanSel);
			// if plugin is compatible with channel selection
			if (plug.IsChannelCountCompatible(ChanSel)) {
				m_LastPluginCmdID = nID;
				ForceSelection();
				NotifyUndoableEdit(static_cast<WORD>(PluginIdx), UCODE_PLUGIN);
				CMainFrame::CPlay	stop(this);	// suspend playing our wave
				bool	retc = plug.Run(*m_Wave, GetIntSelection(), ChanSel, Param);
				OnAudioProcessResult(retc);
			}
		} else {	// can't load plugin
			CString	msg;
			AfxFormatString2(msg, IDS_PLUGIN_CANT_LOAD, path,
				CWinAppEx::GetLastErrorString());
			AfxMessageBox(msg);
		}
	}
}
