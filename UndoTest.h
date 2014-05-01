// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jul12	initial version
		01		07jun13	add channel selection

		automated undo test
 
*/

#pragma once

#include "ProgressDlg.h"

class CWaveShopView;
class CWaveEdit;
class CWaveProcess;

class CUndoTest : public WObject {
public:
// Construction
	CUndoTest(bool Start);
	~CUndoTest();

// Attributes
	bool	IsRunning() const;

// Operations
	bool	Run(bool InitRunning);

protected:
// Types
	typedef struct tagEDIT_INFO {
		int		UndoCode;		// undo code; see UndoCodes.h
		float	Probability;	// relative probability
		int		ColumnIdx;		// for view column edits
	} EDIT_INFO;

// Constants
	enum {	// command returns
		SUCCESS,
		DISABLED,
		ABORT,
	};
	enum {	// states
		STOP,
		EDIT,
		UNDO,
		REDO,
		STATES
	};
	enum {	// pass returns
		PASS,
		FAIL,
		DONE,
		CANCEL,
	};
	enum {
		CYCLES = 1,				// number of test cycles
		PASSES = 2,				// number of passes to do
		PASS_EDITS = 250,		// number of edits per pass
		PASS_UNDOS = 100,		// number of undos per pass
		MAX_EDITS = INT_MAX,	// maximum number of edits
		MAX_PLUGINS = 100,		// maximum number of plugins
		RAND_SEED = 666,		// random number generator seed
		TIMER_PERIOD = 0,		// timer period, in milliseconds
		MIN_CHANNELS = 1,		// minimum number of channels
		MAX_CHANNELS = 3,		// maximum number of channels
		SAMPLE_RATE = 44100,	// sample rate in Hz
		SAMPLE_BITS = 16,		// sample size in bits
		MAX_SAMPLE_SIZE = 2,	// maximum sample size, in bytes
		INIT_DURATION = 1200,	// initial duration, in seconds
		MAX_DURATION = 1800,	// maximum duration, in seconds
		MAX_INSERT = 10,		// maximum insert duration, in seconds
		GEN_PASTE_ODDS = 3,		// odds of generated paste; 0 = never, else 1/N
		MAX_GEN_PASTE = 300,	// maximum generated paste duration, in seconds
		DISK_THRESHOLD = 0,		// threshold for disk-based clipboard, in MB
		PLAY_DURING_TEST = 0,	// if true, play audio during test
		MAKE_SNAPSHOTS = 1,		// if true, create and test snapshots
		CHANNEL_SELECTION = 1,	// if true, generate channel selections
		MAX_FRAMES = MAX_DURATION * SAMPLE_RATE, 
	};
	static const EDIT_INFO	m_EditInfo[];
	static const int		m_EditInfoCount;
	static const LPCTSTR	m_StateName[STATES];

// Member data
	bool	m_InitRunning;		// true if initally running
	FILE	*m_LogFile;			// log file for test results
	CMainFrame	*m_Main;		// pointer to main frame
	CUndoManager	*m_UndoMgr;	// pointer to undo manager
	CWaveShopView	*m_View;	// pointer to view
	CWaveProcess	*m_Wave;	// pointer to wave
	W64UINT	m_Timer;			// timer instance
	int		m_State;			// current state
	int		m_CyclesDone;		// number of cycles completed
	int		m_PassesDone;		// number of passes completed
	int		m_EditsDone;		// number of edits completed
	int		m_UndosDone;		// number of undos completed
	int		m_UndosToDo;		// number of undos to do
	int		m_StepsDone;		// number of steps completed
	int		m_LastResult;		// most recent pass result
	bool	m_InTimer;			// true if we're in OnTimer
	CDWordArray	m_UndoCode;		// array of undo codes
	CProgressDlg	m_ProgressDlg;	// progress dialog
	CString	m_ErrorMsg;			// error message
	CArrayEx<LONGLONG, LONGLONG&> m_Snapshot;	// array of checksums
	COptionsInfo	m_PrevOpts;	// previous options

// Helpers
	void	Init();
	bool	Create();
	void	Destroy();
	int		LogPrintf(LPCTSTR Format, ...);
	static	int		Random(int Vals);
	static	W64INT	RandW64INT(W64INT Vals);
	static	double	RandomFloat(double Limit);
	int		GetRandomEdit() const;
	bool	GetRandomFrame(W64INT& Frame) const;
	bool	GetRandomFrameRange(CW64IntRange& Range, W64INT MinLength = 0) const;
	bool	RandomSelection(CW64IntRange& Sel, W64INT MinLength = 0) const;
	bool	RandomNow(W64INT& Now) const;
	bool	RandomChannelSelection(WORD UndoCode);
	bool	MakeWave(CWaveEdit *Wave, UINT Channels, UINT SampleBits, double Duration);
	void	NatterSelection(LPCTSTR Tag, CW64IntRange Sel);
	void	UpdateView();
	LONGLONG	GetSnapshot() const;
	int		ApplyEdit(int UndoCode);
	bool	LastPass() const;
	void	SetState(int State);
	void	OnTimer();
	int		DoPass();
	static	VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
};

inline bool CUndoTest::IsRunning() const
{
	return(m_State != STOP);
}

inline bool CUndoTest::LastPass() const
{
	return(m_PassesDone >= PASSES - 1);
}
