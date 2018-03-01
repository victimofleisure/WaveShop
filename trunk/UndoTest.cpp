// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jul12	initial version
        01      28feb13	rename insert to insert silence
		02		01mar13	add dual progress
		03		28apr13	adapt to options refactoring
		04		08may13	adapt to wave generator refactoring
		05		07jun13	add channel selection

		automated undo test
 
*/


#include "stdafx.h"

#define UNDO_TEST 0	// set non-zero to enable undo test

#if UNDO_TEST

#include "WaveShop.h"
#include "UndoTest.h"
#include "UndoCodes.h"
#include "RandList.h"
#include "SortArray.h"
#include "Oscillator.h"

static CUndoTest gUndoTest(TRUE);	// one and only instance, initially running

enum {	// view columns; offset past undo codes
	VCOL_NAME = 1000,	// use UCODE_RENAME instead
	VCOL_BYPASS,		// use UCODE_BYPASS instead
	VCOL_DELAY,
	VCOL_THREADS,
	VCOL_PLUGIN_TYPE,
	VCOL_PROCESS_COPY,	// interferes with VCOL_NUM_INPUTS forcing process copy
	VCOL_COPY_PREF,
	VCOL_NUM_INPUTS,
};

const CUndoTest::EDIT_INFO CUndoTest::m_EditInfo[] = {
	{UCODE_CUT,				2},
	{UCODE_PASTE,			5},
	{UCODE_INSERT_SILENCE,	1},
	{UCODE_DELETE,			2},
	{UCODE_AMPLIFY,			2},
	{UCODE_FADE,			2},
	{UCODE_NORMALIZE,		2},
	{UCODE_REVERSE,			2},
	{UCODE_SWAP_CHANNELS,	1},
	{UCODE_INSERT_CHANNEL,	.5},
	{UCODE_DELETE_CHANNEL,	.5},
	{UCODE_CHANGE_FORMAT,	.1f},
};
const int CUndoTest::m_EditInfoCount = sizeof(m_EditInfo) / sizeof(EDIT_INFO);

const LPCTSTR CUndoTest::m_StateName[STATES] = {
	_T("Stop"), 
	_T("Edit"), 
	_T("Undo"), 
	_T("Redo"),
};

#define LOG_TO_FILE 0	// set non-zero to redirect log to a file
#define LOG_SAFE_MODE 0	// set non-zero to flush log after every write
#if LOG_TO_FILE
#define PRINTF LogPrintf
#else
#if UNDO_NATTER
#define PRINTF _tprintf
#else
#define PRINTF sizeof
#endif
#endif

#define LOG_FILE_PATH _T("UndoTest%s.log")

CUndoTest::CUndoTest(bool InitRunning) :
	m_ProgressDlg(IDD_PROGRESS_DUAL)	// allow nested progress
{
	Init();
	m_InitRunning = InitRunning;
	m_Timer = SetTimer(NULL, 0, TIMER_PERIOD, TimerProc);	// start timer loop
}

CUndoTest::~CUndoTest()
{
}

void CUndoTest::Init()
{
	m_InitRunning = FALSE;
	m_LogFile = NULL;
	m_Main = NULL;
	m_View = NULL;
	m_Wave = NULL;
	m_UndoMgr = NULL;
	m_State = STOP;
	m_CyclesDone = 0;
	m_PassesDone = 0;
	m_EditsDone = 0;
	m_UndosToDo = 0;
	m_UndosDone = 0;
	m_StepsDone = 0;
	m_LastResult = FAIL;
	m_InTimer = FALSE;
	m_UndoCode.RemoveAll();
	m_ErrorMsg.Empty();
}

int CUndoTest::LogPrintf(LPCTSTR Format, ...)
{
	if (m_LogFile == NULL)
		return(-1);
	va_list arglist;
	va_start(arglist, Format);
	int	retc = _vftprintf(m_LogFile, Format, arglist);
	va_end(arglist);
	if (LOG_SAFE_MODE)
		fflush(m_LogFile);
	return(retc);
}

inline int CUndoTest::Random(int Vals)
{
	return(CRandList::Rand(Vals));
}

W64INT CUndoTest::RandW64INT(W64INT Vals)
{
	if (Vals <= 0)
		return(-1);
	W64INT	i = truncW64INT(rand() / double(RAND_MAX) * Vals);
	return(min(i, Vals - 1));
}

double CUndoTest::RandomFloat(double Limit)
{
	return((double)rand() / RAND_MAX * Limit);	// poor granularity but usable
}

int CUndoTest::GetRandomEdit() const
{
	return(m_UndoCode[Random(static_cast<int>(m_UndoCode.GetSize()))]);
}

bool CUndoTest::GetRandomFrame(W64INT& Frame) const
{
	W64INT	frames = m_Wave->GetFrameCount();
	if (frames <= 0)
		return(FALSE);
	Frame = RandW64INT(frames);
	return(TRUE);
}

bool CUndoTest::GetRandomFrameRange(CW64IntRange& Range, W64INT MinLength) const
{
	W64INT	frames = m_Wave->GetFrameCount();
	if (frames < MinLength)
		return(FALSE);
	frames -= MinLength;
	W64INT	start, end;
	if (frames)
		start = RandW64INT(frames);
	else
		start = 0;
	frames -= start;
	if (frames)
		end = RandW64INT(frames);
	else
		end = 0;
	end += start + MinLength;
	Range = CW64IntRange(start, end);
	return(TRUE);
}

bool CUndoTest::RandomNow(W64INT& Now) const
{
	if (!GetRandomFrame(Now))
		return(FALSE);
	ASSERT(Now >= 0 && Now <= m_Wave->GetFrameCount());
	m_Main->SetNow(double(Now));
	return(TRUE);
}

bool CUndoTest::RandomSelection(CW64IntRange& Sel, W64INT MinLength) const
{
	if (!GetRandomFrameRange(Sel, MinLength))
		return(FALSE);
	ASSERT(Sel.Start >= 0 && Sel.End <= m_Wave->GetFrameCount());
	ASSERT(Sel.Length() >= MinLength);
	m_Main->SetSelection(CDblRange(double(Sel.Start), double(Sel.End)));
	return(TRUE);
}

bool CUndoTest::RandomChannelSelection(WORD UndoCode)
{
	int	chans = m_Wave->GetChannels();
	if (chans < 2 || !CWaveShopView::RespectsChannelSelection(UndoCode)) {
		m_View->SelectAllChannels();
		return(FALSE);
	}
	int	SelChans = Random(chans) + 1;	// select at least one channel
	CRandList	list(chans);
	CByteArray	ChanSel;
	ChanSel.SetSize(chans);
	for (int iSel = 0; iSel < SelChans; iSel++)	// for each selection
		ChanSel[list.GetNext()] = TRUE;	// select random channel
	m_View->SetChannelSelection(ChanSel.GetData());
	return(TRUE);
}

bool CUndoTest::MakeWave(CWaveEdit *Wave, UINT Channels, UINT SampleBits, double Duration)
{
	WAVEGEN_PARMS	Parms;
	ZeroMemory(&Parms, sizeof(Parms));
	Parms.m_Channels = Channels;
	Parms.m_SampleRate = SAMPLE_RATE;
	Parms.m_SampleBits = SampleBits;
	Parms.m_Duration = Duration;
	Parms.m_Volume = 100;
	Parms.m_Attack = 100;
	Parms.m_Osc[CWaveGenDlg::CARRIER].m_Waveform = COscillator::SINE;
	Parms.m_Osc[CWaveGenDlg::CARRIER].m_Frequency = 60;
	if (Wave != NULL) {
		CProgressDlg	ProgDlg;
		if (!ProgDlg.Create())	// create progress dialog
			return(FALSE);
		ProgDlg.SetWindowText(LDS(IDS_MAIN_GENERATING_AUDIO));
		if (!CWaveGenDlg::MakeWave(Parms, *Wave, &ProgDlg))
			return(FALSE);
	} else {
		if (!m_Main->MakeWave(Parms))
			return(FALSE);
	}
	return(TRUE);
}

LONGLONG fletcher64(const void *Buffer, DWORD Length)
{
	DWORD	sum1 = 0, sum2 = 0;
	DWORD	scrap = INT_PTR(Buffer) % sizeof(DWORD);
	if (scrap) {	// if buffer not dword-aligned
		scrap = sizeof(DWORD) - scrap;
		scrap = min(scrap, Length);
		DWORD	val = 0;
		memcpy(&val, Buffer, scrap);
		sum1 += val;
		sum2 += sum1;
		Buffer = ((BYTE *)Buffer) + scrap;
		Length -= scrap;
	}
	// process full dwords
	DWORD	words = Length / sizeof(DWORD);
	DWORD	*pWord = (DWORD *)Buffer;
	for (DWORD i = 0; i < words; i++) {
		sum1 += pWord[i];
		sum2 += sum1;
	}
	scrap = Length % sizeof(DWORD);
	if (scrap) {	// if any single bytes remain
		DWORD	val = 0;
		memcpy(&val, &pWord[words], scrap);
		sum1 += val;
		sum2 += sum1;
	}
	LARGE_INTEGER	result;
	result.LowPart = sum1;
	result.HighPart = sum2;
	return(result.QuadPart);
}

LONGLONG CUndoTest::GetSnapshot() const
{
	CWaitCursor	wc;
	return(fletcher64(m_Wave->GetData(), static_cast<DWORD>(m_Wave->GetDataSize())));
}

void CUndoTest::NatterSelection(LPCTSTR Tag, CW64IntRange Sel)
{
	PRINTF(_T("%s %I64d %I64d\n"), Tag, LONGLONG(Sel.Start), LONGLONG(Sel.End));
}

void CUndoTest::UpdateView()
{
	m_View->GetDocument()->UpdateAllViews(NULL, CWaveShopView::HINT_WAVE_UPDATE);
}

int CUndoTest::ApplyEdit(int UndoCode)
{
	CW64IntRange	sel;
	W64INT	now, frames;
	int	iChan;
	CString	UndoTitle(LDS(m_View->GetUndoTitleID(UndoCode)));
	bool	HaveChanSel;
	if (CHANNEL_SELECTION)
		HaveChanSel = RandomChannelSelection(UndoCode);
	else
		HaveChanSel = FALSE;
	switch (UndoCode) {
	case UCODE_CUT:
		if (!RandomSelection(sel))
			return(DISABLED);
		NatterSelection(UndoTitle, sel);
		if (!m_View->Delete(TRUE))
			return(ABORT);
		break;
	case UCODE_PASTE:
		if ((GEN_PASTE_ODDS && !Random(GEN_PASTE_ODDS))
		|| theApp.m_Clipboard.GetChannels() != m_Wave->GetChannels()
		|| theApp.m_Clipboard.GetSampleBits() != m_Wave->GetSampleBits()) {
			double	duration = RandomFloat(MAX_GEN_PASTE);
			CWaveEdit	tmp;
			if (!MakeWave(&tmp, m_Wave->GetChannels(), m_Wave->GetSampleBits(), duration)) {
				PRINTF(_T("can't make wave\n"));
				return(ABORT);
			}
			if (!tmp.Copy(theApp.m_Clipboard, CW64IntRange(0, tmp.GetFrameCount())))
				return(ABORT);
		}
		if (!RandomNow(now))
			return(DISABLED);
		frames = theApp.m_Clipboard.GetFrameCount();
		if (!frames || m_Wave->GetFrameCount() + frames > MAX_FRAMES)
			return(DISABLED);
		PRINTF(_T("%s %I64d %I64d\n"), UndoTitle, LONGLONG(now), LONGLONG(frames));
		if (!m_View->Paste())
			return(ABORT);
		break;
	case UCODE_INSERT_SILENCE:
		if (!RandomNow(now))
			return(DISABLED);
		frames = RandW64INT(MAX_INSERT) * SAMPLE_RATE;
		if (!frames || m_Wave->GetFrameCount() + frames > MAX_FRAMES)
			return(DISABLED);
		PRINTF(_T("%s %I64d %I64d\n"), UndoTitle, LONGLONG(now), LONGLONG(frames));
		if (!m_View->InsertSilence(frames))
			return(ABORT);
		break;
	case UCODE_DELETE:
		if (!RandomSelection(sel))
			return(DISABLED);
		NatterSelection(UndoTitle, sel);
		if (!m_View->Delete(FALSE))
			return(ABORT);
		break;
	case UCODE_AMPLIFY:
		if (!RandomSelection(sel, 1))
			return(DISABLED);
		NatterSelection(UndoTitle, sel);
		m_UndoMgr->NotifyEdit(0, static_cast<WORD>(UndoCode));
		if (!m_Wave->Amplify(sel, -6))
			return(ABORT);
		if (HaveChanSel)
			m_View->ApplyChannelSelection();
		UpdateView();
		break;
	case UCODE_FADE:
		if (!RandomSelection(sel, 1))
			return(DISABLED);
		NatterSelection(UndoTitle, sel);
		m_UndoMgr->NotifyEdit(0, static_cast<WORD>(UndoCode));
		if (!m_Wave->Fade(sel, CDblRange(-100, 0), 0))
			return(ABORT);
		if (HaveChanSel)
			m_View->ApplyChannelSelection();
		UpdateView();
		break;
	case UCODE_NORMALIZE:
		if (!RandomSelection(sel, 1))
			return(DISABLED);
		NatterSelection(UndoTitle, sel);
		m_UndoMgr->NotifyEdit(0, static_cast<WORD>(UndoCode));
		if (!m_Wave->Normalize(sel, 0, -6))
			return(ABORT);
		if (HaveChanSel)
			m_View->ApplyChannelSelection();
		UpdateView();
		break;
	case UCODE_REVERSE:
		if (!RandomSelection(sel, 2))
			return(DISABLED);
		NatterSelection(UndoTitle, sel);
		m_UndoMgr->NotifyEdit(0, static_cast<WORD>(UndoCode));
		if (!m_Wave->Reverse(sel))
			return(ABORT);
		if (HaveChanSel)
			m_View->ApplyChannelSelection();
		UpdateView();
		break;
	case UCODE_SWAP_CHANNELS:
		{
			UINT	chans = m_Wave->GetChannels();
			if (chans < 2)
				return(DISABLED);
			if (!RandomSelection(sel, 1))
				return(DISABLED);
			CRandList	ChanList(chans);
			CUIntRange	range(ChanList.GetNext(), ChanList.GetNext());
			CString	s;
			s.Format(_T("%s %d %d: "), UndoTitle, range.Start, range.End);
			NatterSelection(s, sel);
			m_UndoMgr->NotifyEdit(0, static_cast<WORD>(UndoCode));
			if (!m_Wave->SwapChannels(sel, range))
				return(ABORT);
			UpdateView();
		}
		break;
	case UCODE_INSERT_CHANNEL:
		{
			UINT	chans = m_Wave->GetChannels();
			int	room = MAX_CHANNELS - chans;
			if (room <= 0)	// if no room for more channels
				return(DISABLED);
			UINT	AddChans = Random(room) + 1;
			CWaveEdit	InsWave;
			double	CurDur = double(m_Wave->GetFrameCount()) / m_Wave->GetSampleRate();
			if (!MakeWave(&InsWave, AddChans, m_Wave->GetSampleBits(), CurDur))
				return(ABORT);
			iChan = Random(chans + 1);	// append is supported
			PRINTF(_T("%s %d %d\n"), UndoTitle, iChan, AddChans);
			if (!m_View->InsertChannel(iChan, InsWave))
				return(ABORT);
		}
		break;
	case UCODE_DELETE_CHANNEL:
		{
			if (m_Wave->GetChannels() == 1)	// maintain at least one channel
				return(DISABLED);
			iChan = Random(m_Wave->GetChannels());
			PRINTF(_T("%s %d\n"), UndoTitle, iChan);
			if (!m_View->DeleteChannel(iChan))
				return(ABORT);
		}
		break;
	case UCODE_CHANGE_FORMAT:
		{
			if (MIN_CHANNELS == MAX_CHANNELS && MAX_SAMPLE_SIZE <= 1)
				return(DISABLED);
			UINT	Channels, SampleSize;
			do {
				Channels = Random(MAX_CHANNELS - MIN_CHANNELS + 1) + MIN_CHANNELS;
				SampleSize = (Random(MAX_SAMPLE_SIZE) + 1) << 3;
			} while (Channels == m_Wave->GetChannels() 
				&& SampleSize == m_Wave->GetSampleSize());
			PRINTF(_T("%s %d %d\n"), UndoTitle, Channels, SampleSize);
			m_UndoMgr->NotifyEdit(0, static_cast<WORD>(UndoCode));
			if (!m_Wave->ChangeFormat(Channels, m_Wave->GetSampleRate(), SampleSize))
				return(ABORT);
			UpdateView();
		}
		break;
	default:
		NODEFAULTCASE;
		return(ABORT);
	}
	return(SUCCESS);
}

bool CUndoTest::Create()
{
	ASSERT(m_Main == NULL);
	m_Main = theApp.GetMain();
	if (MIN_CHANNELS > MAX_CHANNELS) {
		PRINTF(_T("invalid channel range\n"));
		return(FALSE);
	}
	if (!m_ProgressDlg.Create()) {	// create master progress dialog
		PRINTF(_T("can't create progress dialog\n"));
		return(FALSE);
	}
	if (!MakeWave(NULL, MIN_CHANNELS, SAMPLE_BITS, INIT_DURATION)) {
		PRINTF(_T("can't make initial wave\n"));
		return(FALSE);
	}
	m_View = m_Main->GetView();
	m_Wave = &m_View->GetDocument()->m_Wave;
	m_UndoMgr = &m_View->GetDocument()->m_UndoMgr;
	COptionsInfo	Options(m_Main->GetOptions());
	m_PrevOpts = Options;
	Options.m_DiskThreshold = DISK_THRESHOLD;
	m_Main->SetOptions(Options);
	m_UndoMgr->SetLevels(-1);	// unlimited undo
	theApp.m_Clipboard.Empty();
	if (MAKE_SNAPSHOTS) {
		m_Snapshot.RemoveAll();
		m_Snapshot.SetSize(PASS_EDITS * PASSES + 1);
		m_Snapshot[0] = GetSnapshot();
	}
	m_Main->EnableWindow();	// reenable parent window
	if (PLAY_DURING_TEST) {
		m_Main->SetRepeat(TRUE);
		m_Main->SetPlaying(TRUE);
	}
	int	Steps;
	Steps = PASS_EDITS * (PASSES + 1);
	if (PASSES > 1)
		Steps += (PASSES - 1) * (PASS_EDITS + PASS_UNDOS * 2);
	Steps *= CYCLES;
	m_ProgressDlg.SetRange(0, Steps);
#if LOG_TO_FILE
	CString	LogName;
	LogName.Format(LOG_FILE_PATH,
		CTime::GetCurrentTime().Format(_T("_%Y_%m_%d_%H_%M_%S")));
	m_LogFile = _tfopen(LogName, _T("wc"));	// commit flag
#endif
	return(TRUE);
}

void CUndoTest::Destroy()
{
	m_Main->SetOptions(m_PrevOpts);
	if (m_LogFile != NULL) {
		fclose(m_LogFile);
		m_LogFile = NULL;
	}
	Init();	// reset defaults
}

void CUndoTest::SetState(int State)
{
	if (State == m_State)
		return;
	CString	s;
	s.Format(_T("Undo Test - Pass %d of %d - %s"), 
		m_PassesDone + 1, PASSES, m_StateName[State]);
	m_ProgressDlg.SetWindowText(s);
	m_State = State;
}

bool CUndoTest::Run(bool Enable)
{
	if (Enable == IsRunning())
		return(TRUE);
	if (Enable) {	// if running
		if (!Create())	// create instance
			return(FALSE);
		srand(RAND_SEED);
		// build array of undo codes
		for (int i = 0; i < m_EditInfoCount; i++) {
			// set probability of edits by duplicating them
			int	dups = round(m_EditInfo[i].Probability * 10);
			for (int j = 0; j < dups; j++)
				m_UndoCode.Add(m_EditInfo[i].UndoCode);
		}
		SetState(EDIT);
	} else {	// stopping
		SetState(STOP);
		if (m_LastResult == DONE) {	// if normal completion
			m_CyclesDone++;
			if (m_CyclesDone < CYCLES) {	// if more cycles to go
				PRINTF(_T("cycle %d\n"), m_CyclesDone);
				m_PassesDone = 0;
				m_EditsDone = 0;
				if (MAKE_SNAPSHOTS) {
					m_Snapshot.RemoveAll();
					m_Snapshot.SetSize(PASS_EDITS * PASSES + 1);
					m_Snapshot[0] = GetSnapshot();
				}
				SetState(EDIT);	// resume editing
				return(TRUE);
			}
			// success: display test results
			m_ProgressDlg.DestroyWindow();
			CString	s;
			bool	pass = m_PassesDone >= PASSES;
			s.Format(_T("UndoTest %s: seed=%d edits=%d passes=%d cycles=%d"),
				pass ? _T("pass") : _T("FAIL"),
				RAND_SEED, m_EditsDone, m_PassesDone, m_CyclesDone);
			PRINTF(_T("%s\n"), s);
			AfxMessageBox(s, pass ? MB_ICONINFORMATION : MB_ICONEXCLAMATION);
		} else {	// abnormal completion
			m_ProgressDlg.DestroyWindow();
			if (m_LastResult != CANCEL) {	// if not canceled
				if (m_ErrorMsg.IsEmpty())
					m_ErrorMsg = _T("Generic error.");
				AfxMessageBox(m_ErrorMsg);
			}
		}
		Destroy();	// destroy instance
	}
	return(TRUE);
}

VOID CALLBACK CUndoTest::TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	gUndoTest.OnTimer();
}

void CUndoTest::OnTimer()
{
	TRY {
		if (m_InTimer)	// if already in OnTimer
			return;	// prevent reentrance
		m_InTimer = TRUE;
		if (m_InitRunning) {
			if (theApp.GetMain() == NULL) {
				PRINTF(_T("undo test can't initialize\n"));
				ASSERT(0);	// fatal logic error
				return;
			}
			if (m_Main == NULL)	// if not already initialized
				Run(TRUE);
			m_InitRunning = FALSE;
		}
		if (IsRunning()) {	// if test is running
			if (TIMER_PERIOD) {	// if using timer
				m_LastResult = DoPass();	// do a test pass
				if (m_LastResult != PASS)	// if it failed
					Run(FALSE);	// stop test, or start another cycle
			} else {	// not using timer
				// run entire test from this function
				KillTimer(NULL, m_Timer);	// kill timer
				m_Timer = 0;
				while (IsRunning()) {
					// do test passes until failure occurs
					while ((m_LastResult = DoPass()) == PASS) {
						// pump messages after each pass to keep UI responsive
						MSG msg;
						while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}
					Run(FALSE);	// stop test, or start another cycle
				}
			}
		}
	}
	CATCH (CUserException, e) {
		m_LastResult = CANCEL;
		Run(FALSE);	// abort test
	}
	CATCH (CException, e) {
		TCHAR	msg[256];
		e->GetErrorMessage(msg, _countof(msg));
		PRINTF(_T("%s\n"), msg);
		m_ErrorMsg = msg;
		m_LastResult = FAIL;
		Run(FALSE);	// abort test
	}
	END_CATCH;
	m_InTimer = FALSE;
}

int CUndoTest::DoPass()
{
	if (m_ProgressDlg.Canceled())
		return(CANCEL);
	switch (m_State) {
	case EDIT:
		{
			int	UndoCode = 0, retc = DISABLED;
			int	tries = 0;
			while (retc == DISABLED && tries < INT_MAX) {
				UndoCode = GetRandomEdit();
				if (!tries)
					PRINTF(_T("%d: "), m_EditsDone);
				retc = ApplyEdit(UndoCode); 
				tries++;
			} 
			switch (retc) {
			case SUCCESS:	// edit succeeded
				m_EditsDone++;
				if (MAKE_SNAPSHOTS)
					m_Snapshot[m_EditsDone] = GetSnapshot();
				if (m_EditsDone < MAX_EDITS) {
					if (!(m_EditsDone % PASS_EDITS)) {	// if undo boundary
						// for final pass, undo all the way back, else
						// do number of undos specified by PASS_UNDOS 
						m_UndosToDo = LastPass() ? INT_MAX : PASS_UNDOS;
						m_UndosDone = 0;
						SetState(UNDO);	// start undoing
					}
				} else {	// too many edits
					m_ErrorMsg = _T("Too many edits.");
					return(FAIL);
				}
				break;
			case DISABLED:	// too many retries
				m_ErrorMsg = _T("Too many retries.");
				return(FAIL);
			case ABORT:	// edit failed
				if (m_ErrorMsg.IsEmpty()) {
					CString	UndoTitle(LDS(m_View->GetUndoTitleID(UndoCode)));
					m_ErrorMsg.Format(_T("Error during %s."), UndoTitle);
				}
				return(FAIL);
			}
		}
		break;
	case UNDO:
		if (m_UndoMgr->CanUndo() && m_UndosDone < m_UndosToDo) {
			PRINTF(_T("%d: Undo %s\n"), m_UndosDone,
				m_UndoMgr->GetUndoTitle());
			m_UndoMgr->Undo();
			m_UndosDone++;
			if (MAKE_SNAPSHOTS) {
				LONGLONG	snap = GetSnapshot();
				int	iSnap = m_EditsDone - m_UndosDone;
				if (snap != m_Snapshot[iSnap]) {
					m_ErrorMsg.Format(_T("Current state doesn't match snapshot %d."), iSnap);
					return(FAIL);
				}
			}
		} else {	// undos completed
			if (LastPass()) {
				m_PassesDone++;
				return(DONE);	// all passes completed
			} else {	// not final pass
				SetState(REDO);	// start redoing
			}
		}
		break;
	case REDO:
		if (m_UndoMgr->CanRedo()) {
			PRINTF(_T("%d: Redo %s\n"), m_UndosDone,
				m_UndoMgr->GetRedoTitle());
			m_UndoMgr->Redo();
			m_UndosDone--;
		} else {	// redos completed
			m_PassesDone++;
			SetState(EDIT);	// resume editing
		}
		break;
	}
	if (m_State != STOP) {
		m_StepsDone++;
		// access progress control directly to avoid pumping message
		CWnd	*pProgCtrl = m_ProgressDlg.GetDlgItem(IDC_PROGRESS);
		pProgCtrl->SendMessage(PBM_SETPOS, m_StepsDone);
	}
	return(PASS);	// continue running
}

#endif
