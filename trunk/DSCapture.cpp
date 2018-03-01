// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25apr13	initial version
		01		17may13	in Open, report file exception instead of throwing it

		DirectSound capture
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "DSCapture.h"
#include "Wave.h"
#include "CoInitializer.h"
#include "Benchmark.h"

#define DXChk(f) { HRESULT hr = f; if (FAILED(hr)) { OnDXError(hr); return(FALSE); }}

CDSCapture::CDSCapture()
{
	m_hWnd = NULL;
	m_BufDuration = 1000;
	m_BufSize = 0;
	m_BufIdx = FALSE;
	m_Sessions = 0;
	m_OutDataSize = 0;
	ZeroMemory(&m_WaveFormat, sizeof(m_WaveFormat));
}

CDSCapture::~CDSCapture()
{
	Destroy();
}

bool CDSCapture::EnumDevices(CDSDeviceInfoArray& DevInfo)
{
	DevInfo.RemoveAll();
	return(SUCCEEDED(DirectSoundCaptureEnumerate(
		CDSDeviceInfo::DSEnumCallback, &DevInfo)));
}

bool CDSCapture::Create(HWND hWnd, LPGUID Guid)
{
	Destroy();	// in case this instance is being reused
	if (!m_NotifyEvent.Create(NULL, FALSE, FALSE, NULL))
		return(FALSE);
	int	priority = THREAD_PRIORITY_TIME_CRITICAL;
	UINT	timeout = 5000;	// thread run/stop timeout, in milliseconds
	if (!m_Consumer.Create(ConsumerFunc, this, priority, 0, timeout))
		return(FALSE);
	SetThreadName(m_Consumer.GetID(), _T("Consumer"));
	// create DirectSound
	DXChk(DirectSoundCaptureCreate(Guid, &m_DSC, NULL));
	m_hWnd = hWnd;
	return(TRUE);
}

void CDSCapture::Destroy()
{
	Close();
	m_Consumer.Destroy();
	// explicitly release all interfaces, allowing reuse of this instance
	m_DSCBuf.Release();
	m_DSC.Release();
}

void CDSCapture::OnDXError(HRESULT hr) const
{
	CDSPlayer::ReportDXError(hr);	// display DirectX error message
	OnError(0);	// if we're worker thread, post error notification
}

void CDSCapture::OnError(int StrResID) const
{
	if (StrResID)	// if string resource ID specified
		AfxMessageBox(StrResID);	// display error message
	if (AmConsumer()) {	// if we're worker thread
		if (m_hWnd != NULL)	// if notification window exists
			PostMessage(m_hWnd, UWM_CAPTURE_ERROR, StrResID, 0);	// post error
	}
}

bool CDSCapture::IsCapturing() const
{
	if (!IsOpen())
		return(FALSE);
	DWORD	status;
	DXChk(m_DSCBuf->GetStatus(&status));
	return(status & DSCBSTATUS_CAPTURING);
}

bool CDSCapture::SetBufferDuration(UINT Millis)
{
	if (IsOpen())
		return(FALSE);	// can't set buffer size
	m_BufDuration = Millis;
	return(TRUE);
}

bool CDSCapture::CreateCaptureBuffer(const CWave& Wave)
{
	if (!IsCreated())
		return(FALSE);
	m_DSCBuf.Release();
	ULONGLONG	BufSamps = ULONGLONG(Wave.GetSampleRate()) * m_BufDuration / 1000;
	m_BufSize = int(min(BufSamps * Wave.GetFrameSize(), DSBSIZE_MAX));
	WAVEFORMATEXTENSIBLE	fmt;	// extensible required for hi-res formats
	Wave.GetFormat(fmt);
	DSCBUFFERDESC	desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwBufferBytes = m_BufSize * BUFFERS;	// set capture buffer size
	desc.lpwfxFormat = &fmt.Format;
	DXChk(m_DSC->CreateCaptureBuffer(&desc, &m_DSCBuf, NULL));
	Wave.GetFormat(m_WaveFormat);	// store audio format in member
	return(TRUE);
}

bool CDSCapture::Open(LPCTSTR Path, UINT Channels, UINT SampleRate, UINT SampleBits)
{
	ASSERT(IsCreated());
	if (!IsCreated())
		return(FALSE);
	Close();
	m_BufIdx = 0;
	m_OutDataSize = 0;
	CWave wave;
	wave.SetFormat(Channels, SampleRate, SampleBits);
	if (!CreateCaptureBuffer(wave))
		return(FALSE);
	// request buffer notifications
	CComPtr<IDirectSoundNotify>	pINotify;
	DXChk(m_DSCBuf->QueryInterface(IID_IDirectSoundNotify, (void **)&pINotify));
	DSBPOSITIONNOTIFY	Notify[BUFFERS];
	Notify[0].dwOffset = m_BufSize - 1;
	Notify[1].dwOffset = m_BufSize * 2 - 1;
	Notify[0].hEventNotify = m_NotifyEvent;
	Notify[1].hEventNotify = m_NotifyEvent;
	DXChk(pINotify->SetNotificationPositions(BUFFERS, Notify));
	CFileException	e;
	// create output file
	if (!m_OutFile.Open(Path, CFile::modeWrite | CFile::modeCreate, &e)) {
		e.ReportError();
		Close();	// avoid partially open state
		return(FALSE);
	}
	wave.WriteRF64CompatibleHeader(m_OutFile, 0);	// write placeholder header
	m_NotifyEvent.Reset();	// reset notification event
	return(m_Consumer.Run(TRUE));	// start worker thread
}

bool CDSCapture::Close()
{
	Stop();	// stop capturing
	m_Consumer.Stop();	// notify worker thread that we're stopping
	m_NotifyEvent.Set();	// unblock worker thread
	m_Consumer.Run(FALSE);	// stop worker thread
	if (IsOutputFileOpen()) {	// if output file is open
		Flush();
		if (m_OutDataSize & 1) {	// if odd data size
			BYTE	pad = 0;
			m_OutFile.Write(&pad, 1);	// chunks must be word-aligned; add pad byte
		}
		CWave	wave(m_WaveFormat.nChannels, m_WaveFormat.nSamplesPerSec, 
			m_WaveFormat.wBitsPerSample);
		ULONGLONG	frames = m_OutDataSize / wave.GetFrameSize();
		m_OutFile.SeekToBegin();	// rewind output file
		wave.WriteRF64CompatibleHeader(m_OutFile, frames);	// write actual header
		m_OutFile.Close();	// close output file
	}
	m_DSCBuf.Release();	// destroy capture buffer interface
	return(TRUE);
}

bool CDSCapture::Start()
{
	if (!IsOpen())	// if capture buffer not created
		return(FALSE);
	if (!m_Consumer.IsRunning())	// if consumer thread not running
		return(FALSE);
	m_Sessions++;	// increment session count before starting; order matters
	DXChk(m_DSCBuf->Start(DSCBSTART_LOOPING));	// start capturing
	return(TRUE);
}

bool CDSCapture::Stop()
{
	if (!IsOpen())	// if capture buffer not created
		return(FALSE);
	DXChk(m_DSCBuf->Stop());	// stop capturing
	return(TRUE);
}

inline bool CDSCapture::Flush()
{
	if (!IsOpen())
		return(FALSE);
	DWORD	ReadCursor;
	DXChk(m_DSCBuf->GetCurrentPosition(NULL, &ReadCursor));
	DWORD	ReadOfs = 0;
	DWORD	ReadLen = 0;
	if (m_BufIdx) {
		if (ReadCursor >= m_BufSize) {	// should always be true
			ReadOfs = m_BufSize;
			ReadLen = ReadCursor - m_BufSize;
		}
	} else {
		if (ReadCursor < m_BufSize)	// should always be true
			ReadLen = ReadCursor;
	}
	if (ReadLen) {	// if unread data pending in buffer
		PVOID	pBuf;
		DWORD	Len;
		DXChk(m_DSCBuf->Lock(ReadOfs, ReadLen, &pBuf, &Len, NULL, NULL, 0));
		m_OutFile.Write(pBuf, Len);
		m_OutDataSize += Len;
		DXChk(m_DSCBuf->Unlock(pBuf, Len, NULL, 0));
	}
	return(TRUE);
}

inline bool CDSCapture::Write()
{
	ASSERT(AmConsumer());	// should be called from worker thread only
	CBenchmark	bench;	// start timer
	DWORD	ofs;
	if (m_BufIdx)	// if second buffer
		ofs = m_BufSize;
	else	// first buffer
		ofs = 0;
	m_BufIdx ^= 1;	// switch buffers
	PVOID	pBuf;
	DWORD	Len;
	DXChk(m_DSCBuf->Lock(ofs, m_BufSize, &pBuf, &Len, NULL, NULL, 0));
	bool	retc;
	TRY {
		m_OutFile.Write(pBuf, Len);	// write captured audio to output file
		m_OutDataSize += Len;	// increment size of output data
		retc = TRUE;	// success
	}
	CATCH (CException, e) {
		e->ReportError();	// modal error message dialog; blocks this thread
		retc = FALSE;	// failure
	}
	END_CATCH
	DXChk(m_DSCBuf->Unlock(pBuf, Len, NULL, 0));
	if (!retc) {	// if exception occurred during write
		OnError(0);	// post notification; error message already displayed above
		return(FALSE);
	}
	UINT	ElapsedMillis = round(bench.Elapsed() * 1000);	// elapsed milliseconds
	if (ElapsedMillis > m_BufDuration) {	// if write took too long
		OnError(IDS_DS_CAPTURE_OVERRUN);
		return(FALSE);
	}
	if (m_hWnd != NULL)	// if notification window exists
		PostMessage(m_hWnd, UWM_CAPTURE_WRITE, 0, 0);	// post write notification
	return(TRUE);
}

bool CDSCapture::ConsumerMain()
{
	try {
		// initialize COM for multithreading for this worker thread
		CCoInitializer	coninit(COINIT_MULTITHREADED);
		while (m_Consumer.WaitForStart()) {	// wait for start signal
			DWORD	Timeout = m_BufDuration * BUFFERS;
			UINT	Sessions = 0;
			while (1) {	// main inner loop
				// wait for capture buffer position notification
				DWORD	retc = WaitForSingleObject(m_NotifyEvent, Timeout);
				if (m_Consumer.GetStopFlag())	// if thread stop requested
					break;	// wait result is irrelevant
				switch (retc) {	// check wait result
				case WAIT_OBJECT_0:	// wait succeeded
					if (!Write())	// if write failed
						goto MainInnerLoopExit;
					break;
				case WAIT_TIMEOUT:	// wait timed out
					// if capturing, and not in start/stop transition
					if (IsCapturing() && Sessions == m_Sessions) {
						OnError(IDS_REC_WAIT_TIMEOUT);
						goto MainInnerLoopExit;
					}
					break;
				default:	// wait failed
					OnError(IDS_REC_WAIT_ERROR);
					goto MainInnerLoopExit;
				}
				Sessions = m_Sessions;	// update cached session count
			}
MainInnerLoopExit:;
		}
	}
	catch (HRESULT) {
		return(FALSE);	// COM init failed
	}
	return(TRUE);
}

UINT CDSCapture::ConsumerFunc(LPVOID pParam)
{
	CDSCapture	*pPlayer = (CDSCapture *)pParam;
	pPlayer->ConsumerMain();
	return(0);
}
