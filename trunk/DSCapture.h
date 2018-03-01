// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25apr13	initial version

		DirectSound capture
 
*/

#ifndef CDSCAPTURE_INCLUDED
#define CDSCAPTURE_INCLUDED

#include "DSPlayer.h"

class CDSCapture : public WObject {
public:
// Construction
	CDSCapture();
	virtual	~CDSCapture();
	bool	Create(HWND hWnd, LPGUID Guid);
	void	Destroy();

// Types
	typedef CDSPlayer::CDSDeviceInfo CDSDeviceInfo;
	typedef CDSPlayer::CDSDeviceInfoArray CDSDeviceInfoArray;

// Attributes
	bool	IsCreated() const;
	bool	IsOpen() const;
	bool	IsCapturing() const;
	bool	IsOutputFileOpen() const;
	UINT	GetBufferDuration() const;
	UINT	GetBufferSize() const;
	bool	SetBufferDuration(UINT Millis);
	ULONGLONG	GetOutputDataSize() const;
	const WAVEFORMATEX&	GetWaveFormat() const;

// Operations
	bool	Open(LPCTSTR Path, UINT Channels, UINT SampleRate, UINT SampleBits);
	bool	Close();
	bool	Start();
	bool	Stop();
	static	bool	EnumDevices(CDSPlayer::CDSDeviceInfoArray& DevInfo);

protected:
// Constants
	enum {
		BUFFERS = 2				// number of buffers; don't change
	};

// Data members
	CComPtr<IDirectSoundCapture>	m_DSC;	// DirectSound capture interface
	CComPtr<IDirectSoundCaptureBuffer>	m_DSCBuf;	// DirectSound capture buffer
	CWorkerThread	m_Consumer;	// buffer-reading worker thread
	WEvent	m_NotifyEvent;		// notification event
	HWND	m_hWnd;				// window to receive notifications
	UINT	m_BufDuration;		// duration of each buffer, in milliseconds
	UINT	m_BufSize;			// size of each buffer, in bytes
	bool	m_BufIdx;			// which buffer to use next
	volatile	UINT	m_Sessions;	// session counter, for detecting timeouts
	CFile	m_OutFile;			// output file for captured audio
	ULONGLONG	m_OutDataSize;	// size of output data, in bytes
	WAVEFORMATEX	m_WaveFormat;	// audio format being captured

// Overridables
	virtual	void	OnDXError(HRESULT hr) const;

// Helpers
	bool	CreateCaptureBuffer(const CWave& Wave);
	bool	Write();
	bool	Flush();
	bool	ConsumerMain();
	bool	AmConsumer() const;
	void	OnError(int StrResID) const;
	static	UINT	ConsumerFunc(LPVOID pParam);
};

inline bool CDSCapture::IsCreated() const
{
	return(m_DSC != NULL);
}

inline bool CDSCapture::IsOpen() const
{
	return(m_DSCBuf != NULL);
}

inline bool CDSCapture::IsOutputFileOpen() const
{
	return(m_OutFile.m_hFile != CFile::hFileNull);
}

inline UINT CDSCapture::GetBufferDuration() const
{
	return(m_BufDuration);
}

inline UINT CDSCapture::GetBufferSize() const
{
	return(m_BufSize);
}

inline ULONGLONG CDSCapture::GetOutputDataSize() const
{
	return(m_OutDataSize);
}

inline const WAVEFORMATEX& CDSCapture::GetWaveFormat() const
{
	return(m_WaveFormat);
}

inline bool CDSCapture::AmConsumer() const
{
	return(GetCurrentThreadId() == m_Consumer.GetID());	// true if worker thread
}

#endif
