// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
        01      31mar13	add SetPosEx

		asynchronous processing
 
*/

#ifndef CASYNCJOB_INCLUDED
#define CASYNCJOB_INCLUDED

#include "WorkerThread.h"

class CProgressDlg;

class CAsyncJob : public WObject {
public:
// Construction
	CAsyncJob();

// Types
	typedef bool (*PJOBFUNC)(CAsyncJob& Job, LPVOID Data);

// Attributes
	W64INT	GetPos() const;
	void	SetPos(W64INT Pos);
	bool	SetPosEx(W64INT Pos);
	void	GetRange(W64INT& Start, W64INT& End) const;
	void	SetRange(W64INT Start, W64INT End);
	bool	GetStopFlag() const;
	bool	GetResult() const;
	bool	Canceled() const;

// Operations
	bool	StartJob(PJOBFUNC Func, LPVOID Data, LPCTSTR ProgressCaption = NULL);

protected:
// Types
	class CMyWorker : public CWorkerThread {
	public:
		bool	MsgWaitForStop(DWORD Timeout, CProgressDlg *ProgDlg);
	};

// Member data
	CMyWorker	m_Worker;			// worker thread
	PJOBFUNC	m_Func;				// pointer to job function
	LPVOID	m_Data;					// pointer to job data
	volatile	W64INT	m_CurPos;	// progress current position
	volatile	W64INT	m_StartPos;	// progress start position
	volatile	W64INT	m_EndPos;	// progress end position
	volatile	bool	m_Result;	// true if job succeeded
	bool	m_Canceled;				// true if job was canceled

// Helpers
	bool	WorkerMain();
	static	UINT	WorkerFunc(LPVOID Param);
};

inline W64INT CAsyncJob::GetPos() const
{
	return(m_CurPos);
}

inline void	CAsyncJob::SetPos(W64INT Pos)
{
	m_CurPos = Pos;
}

inline bool CAsyncJob::SetPosEx(W64INT Pos)
{
	m_CurPos = Pos;
	return(GetStopFlag());
}

inline void CAsyncJob::GetRange(W64INT& Start, W64INT& End) const
{
	Start = m_StartPos;
	End = m_EndPos;
}

inline void CAsyncJob::SetRange(W64INT Start, W64INT End)
{
	m_StartPos = Start;
	m_EndPos = End;
}

inline bool CAsyncJob::GetStopFlag() const
{
	return(m_Worker.GetStopFlag());
}

inline bool CAsyncJob::GetResult() const
{
	return(m_Result);
}

inline bool CAsyncJob::Canceled() const
{
	return(m_Canceled);
}

template<class T>
class CTypedAsyncJob : public CAsyncJob {
public:
	typedef bool (*PTYPEDJOBFUNC)(CAsyncJob& Job, T *Data);
	bool	StartJob(PTYPEDJOBFUNC Func, T *Data, LPCTSTR ProgressCaption = NULL);
};

template<class T>
inline bool CTypedAsyncJob<T>::StartJob(PTYPEDJOBFUNC Func, T *Data, LPCTSTR ProgressCaption)
{
	return(CAsyncJob::StartJob(PJOBFUNC(Func), Data, ProgressCaption));
}

#endif
