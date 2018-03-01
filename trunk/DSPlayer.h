// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      12oct12	initial version
		01		25apr13	move enum callback to device info
		02		26apr13	add DX error reporting
		03		05may13	in ReportDXError, avoid reentrance
		04		12sep13	add GetChannelMask

		DirectSound player
 
*/

#ifndef CDSPLAYER_INCLUDED
#define CDSPLAYER_INCLUDED

#include "mmsystem.h"
#include "dsound.h"
#include "atlbase.h"
#include "ArrayEx.h"
#include "WorkerThread.h"

class CWave;

class CDSPlayer : public WObject {
public:
// Construction
	CDSPlayer();
	virtual	~CDSPlayer();
	bool	Create(LPGUID Guid, HWND hWnd);
	void	Destroy();

// Types
	class CDSDeviceInfo : public WObject {
	public:
		CDSDeviceInfo();
		CDSDeviceInfo& operator=(const CDSDeviceInfo& Info);
		GUID	m_Guid;			// globally unique identifier of device
		CString	m_Description;	// textual description of the device
		static	BOOL	__stdcall DSEnumCallback(LPGUID lpGuid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext);
	};
	typedef CArrayEx<CDSDeviceInfo, CDSDeviceInfo&> CDSDeviceInfoArray;

// Attributes
	bool	IsCreated() const;
	bool	IsOpen() const;
	bool	IsPlaying() const;
	bool	SetPlaying(bool Enable);
	bool	GetVolume(double& Volume) const;
	bool	SetVolume(double Volume) const;
	bool	GetPan(double& Pan) const;
	bool	SetPan(double Pan) const;
	bool	GetFrequency(double& Freq) const;
	bool	SetFrequency(double Freq) const;
	bool	GetRepeat() const;
	bool	SetRepeat(bool Enable);
	bool	GetPosition(W64INT& Frame) const;
	bool	GetWritePosition(W64INT& Frame) const;
	bool	SetPosition(W64INT Frame);
	bool	GetLoopPoints(W64INT& StartFrame, W64INT& EndFrame) const;
	bool	SetLoopPoints(W64INT StartFrame, W64INT EndFrame);
	UINT	GetBufferDuration() const;
	UINT	GetBufferSize() const;
	bool	SetBufferDuration(UINT Millis);
	const CWave	*GetWave() const;

// Operations
	static	bool	EnumDevices(CDSDeviceInfoArray& DevInfo);
	static	int		FindDeviceGuid(const CDSDeviceInfoArray& DevInfo, const GUID& Guid);
	bool	Open(CWave& Wave);
	bool	Close();
	bool	Play();
	bool	Stop();
	bool	Rewind();
	static	double	LinearToDecibels(double Linear);
	static	double	DecibelsToLinear(double Decibels);
	static	void	ReportDXError(HRESULT hr);

protected:
// Constants
	enum {
		BUFFERS = 2				// number of buffers; don't change, see GetPosition
	};

// Data members
	CComPtr<IDirectSound>	m_DS;	// DirectSound interface
	CComPtr<IDirectSoundBuffer>	m_DSBuf;	// DirectSound buffer interface
	CWorkerThread	m_Feeder;	// buffer-filling worker thread
	CWave	*m_Wave;			// pointer to wave
	WEvent	m_NotifyEvent;		// notification event
	UINT	m_BufSize;			// size of each buffer, in bytes
	UINT	m_BufDuration;		// duration of each buffer, in milliseconds
	UINT	m_WriteBufIdx;		// index of buffer to be written
	W64INT	m_WaveOffset;		// offset within wave, in bytes
	W64INT	m_PrevWaveOffset[BUFFERS];	// previous wave offset for each buffer
	W64INT	m_LoopStart;		// start of loop, as wave byte offset
	W64INT	m_LoopEnd;			// end of loop, as wave byte offset
	bool	m_Playing;			// true if playing audio
	bool	m_Repeat;			// true if looping audio
	BYTE	m_SilenceVal;		// value to fill with for silence
	static	bool	m_InError;	// true if in ReportDXError

// Overridables
	virtual	void	OnDXError(HRESULT hr) const;

// Helpers
	W64INT	FrameToOffset(W64INT Frame) const;
	W64INT	OffsetToFrame(W64INT Offset) const;
	bool	SilentFill();
	void	NormalFill(PVOID pBuf, DWORD Len);
	void	RepeatFill(PVOID pBuf, DWORD Len);
	bool	FillBuffer(UINT BufIdx);
	bool	FeederMain();
	static	UINT	FeederFunc(LPVOID pParam);
	static	DWORD	GetChannelMask(WORD Channels);
};

inline bool CDSPlayer::IsCreated() const
{
	return(m_DS != NULL);
}

inline bool CDSPlayer::IsOpen() const
{
	return(m_DSBuf != NULL);
}

inline bool CDSPlayer::IsPlaying() const
{
	return(m_Playing);
}

inline bool CDSPlayer::GetRepeat() const
{
	return(m_Repeat);
}

inline UINT CDSPlayer::GetBufferDuration() const
{
	return(m_BufDuration);
}

inline UINT CDSPlayer::GetBufferSize() const
{
	return(m_BufSize);
}

inline const CWave *CDSPlayer::GetWave() const
{
	return(m_Wave);
}

#endif
