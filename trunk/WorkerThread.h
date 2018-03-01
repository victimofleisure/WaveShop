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
		02		14may11	add GetHandle/GetID
		03		07jan13	add default args to Create

		worker thread with run/stop support
 
*/

#pragma once

#include "Event.h"

class CWorkerThread : public WObject {
public:
	CWorkerThread();
	~CWorkerThread();

// Types
	typedef UINT (*PTHREADFUNC)(LPVOID Arg);

// Attributes
	bool	IsRunning() const;
	bool	IsCreated() const;
	bool	GetStopFlag() const;
	DWORD	GetThreadID() const;
	int		GetPriority() const;
	bool	SetPriority(int Priority);
	DWORD	GetTimeout() const;
	void	SetTimeout(DWORD Timeout);
	HANDLE	GetHandle() const;
	DWORD	GetID() const;

// Operations
	bool	Create(PTHREADFUNC ThreadFunc, LPVOID ThreadArg = NULL, int Priority = 0, UINT StackSize = 0, UINT Timeout = INFINITE);
	bool	Destroy();
	bool	Run(bool Enable);

// Thread operations (call from thread only)
	bool	WaitForStart();
	void	Stop();

protected:
	CWinThread	*m_Thread;			// thread instance
	UINT	m_Timeout;				// run/stop timeout in milliseconds
	volatile bool	m_StopFlag;		// true if thread should stop
	volatile bool	m_KillFlag;		// true if thread should exit
	volatile bool	m_IsRunning;	// true if thread is running
	WEvent	m_Start;				// set by caller to start thread
	WEvent	m_Started;				// set by thread when it starts
	WEvent	m_Stopped;				// set by thread when it stops
};

extern void SetThreadName(DWORD dwThreadID, LPCTSTR threadName);

inline bool CWorkerThread::IsRunning() const
{
	return(m_IsRunning);
}

inline bool CWorkerThread::IsCreated() const
{
	return(m_Thread != NULL);
}

inline bool CWorkerThread::GetStopFlag() const
{
	return(m_StopFlag);
}

inline void CWorkerThread::Stop()
{
	m_StopFlag = TRUE;
}

inline DWORD CWorkerThread::GetThreadID() const
{
	return(m_Thread->m_nThreadID);
}

inline int CWorkerThread::GetPriority() const
{
	return(GetThreadPriority(m_Thread->m_hThread));
}

inline bool CWorkerThread::SetPriority(int Priority)
{
	return(SetThreadPriority(m_Thread->m_hThread, Priority) != 0);
}

inline DWORD CWorkerThread::GetTimeout() const
{
	return(m_Timeout);
}

inline void CWorkerThread::SetTimeout(DWORD Timeout)
{
	m_Timeout = Timeout;
}

inline HANDLE CWorkerThread::GetHandle() const
{
	return(m_Thread->m_hThread);
}

inline DWORD CWorkerThread::GetID() const
{
	return(m_Thread->m_nThreadID);
}
