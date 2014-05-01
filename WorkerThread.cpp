// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
        01      12sep10	remove stop event
        02      12nov12	add missing ATL header
        03      15jan13	reset m_Start in WaitForStart; reset m_Started in Run

		worker thread with run/stop support
 
*/

#include "stdafx.h"
#include "WorkerThread.h"
#include "atlconv.h"	// needed for SetThreadName Unicode

CWorkerThread::CWorkerThread()
{
	m_Thread = NULL;
	m_Timeout = 0;
	m_StopFlag = FALSE;
	m_KillFlag = FALSE;
	m_IsRunning = FALSE;
}

CWorkerThread::~CWorkerThread()
{
	Destroy();
}

bool CWorkerThread::Create(PTHREADFUNC ThreadFunc, LPVOID ThreadArg, int Priority, UINT StackSize, UINT Timeout)
{
	m_Timeout = Timeout;
	if (!(m_Start.Create(NULL, TRUE, FALSE, NULL)	// all manual
	&& m_Started.Create(NULL, TRUE, FALSE, NULL)
	&& m_Stopped.Create(NULL, TRUE, FALSE, NULL))) {
		return(FALSE);
	}
	m_StopFlag = TRUE;
	m_KillFlag = FALSE;
	m_Thread = AfxBeginThread(ThreadFunc, ThreadArg, Priority, StackSize, CREATE_SUSPENDED, NULL);
	if (m_Thread == NULL)
		return(FALSE);
	m_Thread->m_bAutoDelete = FALSE;
	m_Thread->ResumeThread();
	if (WaitForSingleObject(m_Stopped, Timeout) != WAIT_OBJECT_0)
		return(FALSE);
	return(TRUE);
}

bool CWorkerThread::Destroy()
{
	if (m_Thread == NULL)
		return(TRUE);
	Run(FALSE);
	m_KillFlag = TRUE;
	Run(TRUE);
	if (WaitForSingleObject(m_Thread->m_hThread, m_Timeout) != WAIT_OBJECT_0)
		return(FALSE);
	delete m_Thread;
	m_Thread = NULL;
	m_IsRunning = FALSE;
	return(TRUE);
}

bool CWorkerThread::Run(bool Enable)
{
	if (Enable == m_IsRunning)	// if state unchanged
		return(TRUE);	// nothing to do
	if (Enable) {	// if starting
		m_StopFlag = FALSE;	// reset stop
		m_Started.Reset();	// reset started
		m_Start.Set();	// signal start; wait for started
		if (WaitForSingleObject(m_Started, m_Timeout) != WAIT_OBJECT_0)
			return(FALSE);
	} else {	// stopping
		m_StopFlag = TRUE;	// request stop; wait for stopped
		if (WaitForSingleObject(m_Stopped, m_Timeout) != WAIT_OBJECT_0)
			return(FALSE);
	}
	m_IsRunning = Enable;	// update state
	return(TRUE);
}

bool CWorkerThread::WaitForStart()
{
	m_Stopped.Set();	// signal stopped
	WaitForSingleObject(m_Start, INFINITE);	// wait for start
	m_Stopped.Reset();	// reset stopped
	m_Start.Reset();	// reset start
	m_Started.Set();	// signal started
	return(!m_KillFlag);	// success if not killed
}

#define MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD	dwType;		// Must be 0x1000.
	LPCSTR	szName;		// Pointer to name (in user addr space).
	DWORD	dwThreadID; // Thread ID (-1=caller thread).
	DWORD	dwFlags;	// Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, LPCTSTR threadName)
{
#ifdef UNICODE
	USES_CONVERSION;
	LPCSTR	ANSIName = W2A(threadName);	// convert name from Unicode to ANSI
#else
	LPCSTR	ANSIName = threadName;
#endif
	Sleep(10);
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = ANSIName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;
	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(W64ULONG *), (const W64ULONG *)&info);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
}
