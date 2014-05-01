// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
		01		01mar13	in StartJob, check progress dialog create
		02		04apr13	in StartJob, pass focus window to progress dialog create

		asynchronous processing
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "AsyncJob.h"
#include "ProgressDlg.h"

CAsyncJob::CAsyncJob()
{
	m_Func = NULL;
	m_Data = NULL;
	m_CurPos = 0;
	m_StartPos = 0;
	m_EndPos = 100;
	m_Result = FALSE;
	m_Canceled = FALSE;
}

UINT CAsyncJob::WorkerFunc(LPVOID Param)
{
	CAsyncJob	*pJob = static_cast<CAsyncJob *>(Param);
	pJob->WorkerMain();
	return(0);
}

bool CAsyncJob::WorkerMain()
{
	while (m_Worker.WaitForStart()) {
		TRY {
			m_Result = m_Func(*this, m_Data);
		}
		CATCH (CException, e) {
			e->ReportError();
			m_Result = FALSE;
		}
		END_CATCH
	}
	return(m_Result);
}

bool CAsyncJob::StartJob(PJOBFUNC Func, LPVOID Data, LPCTSTR ProgressCaption)
{
	const UINT NAP_PERIOD = 100;	// progress granularity, in milliseconds
	m_Func = Func;
	m_Data = Data;
	m_Result = TRUE;	// assume success
	m_Canceled = FALSE;
	if (!m_Worker.Create(CAsyncJob::WorkerFunc, this))	// create worker thread
		AfxThrowResourceException();
	if (!m_Worker.Run(TRUE))	// start worker thread
		AfxThrowResourceException();
	{
		CProgressDlg	ProgDlg(IDD_PROGRESS);
		// progress dialog will disable/reenable safe parent of focus window
		CWnd	*FocusWnd = CWnd::FromHandle(GetFocus());
		if (!ProgDlg.Create(FocusWnd))	// create progress dialog
			AfxThrowResourceException();
		if (ProgressCaption != NULL)	// if progress caption specified
			ProgDlg.SetWindowText(ProgressCaption);	// set progress caption
		int	PrevPos = 0;
		// loop until worker thread stops or cancel or error occurs
		while (!m_Worker.MsgWaitForStop(NAP_PERIOD, &ProgDlg)) {
			if (ProgDlg.Canceled()) {		// if canceled or error
				m_Canceled = TRUE;	// set canceled flag
				break;	// early out
			}
			if (!m_Result)	// if error
				break;	// early out
			W64INT	delta = m_EndPos - m_StartPos;
			if (delta) {	// avoid divide by zero
				int	pos = round(double(m_CurPos - m_StartPos) / delta * 100);
				if (pos != PrevPos) {	// if progress position changed
					ProgDlg.SetPos(pos);	// update progress
					PrevPos = pos;
				}
			}
		}
	}	// destroy progress dialog
	if (!m_Worker.Destroy())	// destroy worker thread
		return(FALSE);
	return(m_Result);
}

bool CAsyncJob::CMyWorker::MsgWaitForStop(DWORD Timeout, CProgressDlg *ProgDlg)
{
	ASSERT(ProgDlg != NULL);
	DWORD	start = GetTickCount();
	DWORD	elapsed = 0;
	while (elapsed < Timeout) {	// until timeout expires
		HANDLE	ha[] = {m_Stopped};	// worker thread stopped event
		DWORD	retc = MsgWaitForMultipleObjects(1, ha, FALSE, 
			Timeout - elapsed, QS_ALLINPUT);
		switch (retc) {
		case WAIT_OBJECT_0:	// worker thread stopped
			return(TRUE);
		case WAIT_OBJECT_0 + 1:	// message(s) in queue
			{
				ProgDlg->PumpMessages();	// dispatch messages
				if (ProgDlg->Canceled())	// if user canceled
					return(FALSE);
				DWORD	now = GetTickCount();
				if (now < start)	// if tick count wrapped
					elapsed = UINT_MAX - start + now;
				else	// normal case
					elapsed = now - start;
			}
			break;
		default:	// wait timeout or error
			return(FALSE);
		}
	}
	return(FALSE);	// timeout expired
}
