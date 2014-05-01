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
		04		12sep13	in Open, fix zero channel mask

		DirectSound player
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "DSPlayer.h"
#include "Wave.h"
#include <math.h>
#include "CoInitializer.h"
#include "dxerr8.h"	// for DX error strings

#define DXChk(f) { HRESULT hr = f; if (FAILED(hr)) { OnDXError(hr); return(FALSE); }}

bool	CDSPlayer::m_InError;

CDSPlayer::CDSPlayer()
{
	m_Wave = NULL;
	m_BufSize = 0;
	m_BufDuration = 100;	// optimal value in milliseconds
	m_WriteBufIdx = 0;
	m_WaveOffset = 0;
	ZeroMemory(m_PrevWaveOffset, sizeof(m_PrevWaveOffset));
	m_LoopStart = 0;
	m_LoopEnd = 0;
	m_Playing = FALSE;
	m_Repeat = FALSE;
	m_InError = FALSE;
	m_SilenceVal = 0;
}

CDSPlayer::~CDSPlayer()
{
	Destroy();
}

bool CDSPlayer::Create(LPGUID Guid, HWND hWnd)
{
	Destroy();	// in case this instance is being reused
	if (!m_NotifyEvent.Create(NULL, FALSE, FALSE, NULL))
		return(FALSE);
	int	priority = THREAD_PRIORITY_TIME_CRITICAL;
	UINT	timeout = 5000;	// thread run/stop timeout, in milliseconds
	if (!m_Feeder.Create(FeederFunc, this, priority, 0, timeout))
		return(FALSE);
	SetThreadName(m_Feeder.GetID(), _T("Feeder"));
	// create DirectSound
	DXChk(DirectSoundCreate(Guid, &m_DS, NULL));
	DXChk(m_DS->SetCooperativeLevel(hWnd, DSSCL_NORMAL));
	return(TRUE);
}

void CDSPlayer::Destroy()
{
	Close();
	m_Feeder.Destroy();
	// explicitly release all interfaces, allowing reuse of this instance
	m_DSBuf.Release();
	m_DS.Release();
}

void CDSPlayer::ReportDXError(HRESULT hr)
{
	if (m_InError)	// avoid reentrance while in AfxMessageBox
		return;
	m_InError = TRUE;
	CString	msg;
	AfxFormatString2(msg, IDS_DIRECTSOUND_ERROR, 
		DXGetErrorString8(hr), DXGetErrorDescription8(hr));
	AfxMessageBox(msg);
	m_InError = FALSE;
}

void CDSPlayer::OnDXError(HRESULT hr) const
{
	ReportDXError(hr);
}

CDSPlayer::CDSDeviceInfo::CDSDeviceInfo()
{
	ZeroMemory(&m_Guid, sizeof(m_Guid));
}

CDSPlayer::CDSDeviceInfo& CDSPlayer::CDSDeviceInfo::operator=(const CDSDeviceInfo& Info)
{
	m_Guid = Info.m_Guid;
	m_Description = Info.m_Description;
	return(*this);
}

BOOL __stdcall CDSPlayer::CDSDeviceInfo::DSEnumCallback(LPGUID lpGuid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext)
{
	CDSDeviceInfoArray	*DevInfo = (CDSDeviceInfoArray *)lpContext;
	CDSDeviceInfo	info;
	if (lpGuid != NULL)
		info.m_Guid = *lpGuid;
	info.m_Description = lpcstrDescription;
	DevInfo->Add(info);
	return(TRUE);
}

bool CDSPlayer::EnumDevices(CDSDeviceInfoArray& DevInfo)
{
	DevInfo.RemoveAll();
	return(SUCCEEDED(DirectSoundEnumerate(
		CDSDeviceInfo::DSEnumCallback, &DevInfo)));
}

int CDSPlayer::FindDeviceGuid(const CDSDeviceInfoArray& DevInfo, const GUID& Guid)
{
	int	devs = DevInfo.GetSize();
	for (int iDev = 0; iDev < devs; iDev++) {	// for each device
		const CDSPlayer::CDSDeviceInfo&	info = DevInfo[iDev];
		if (IsEqualGUID(info.m_Guid, Guid))
			return(iDev);	// return index of device
	}
	return(-1);
}

double CDSPlayer::LinearToDecibels(double Linear)
{
	ASSERT(Linear > 0);
	return(20.0 * log10(Linear));
}

double CDSPlayer::DecibelsToLinear(double Decibels)
{
	return(pow(10.0, Decibels / 20.0));
}

bool CDSPlayer::GetVolume(double& Volume) const
{
	if (!IsOpen())
		return(FALSE);
	LONG	DSVol;
	DXChk(m_DSBuf->GetVolume(&DSVol));	// get attenuation, in hundredths of a dB
	double	r = DecibelsToLinear(DSVol / 100.0);	// convert to volume
	if (r <= -1.0 / DSBVOLUME_MIN)	// if below minimum volume
		r = 0;	// substitute zero
	Volume = r;
	return(TRUE);
}

bool CDSPlayer::SetVolume(double Volume) const
{
	if (!IsOpen())
		return(FALSE);
	double	r = CLAMP(Volume, -1.0 / DSBVOLUME_MIN, 1);	// enforce volume range
	LONG	DSVol = round(LinearToDecibels(r) * 100);	// convert volume to attenuation
	DXChk(m_DSBuf->SetVolume(DSVol));	// set attenuation, in hundredths of a dB
	return(TRUE);
}

bool CDSPlayer::GetPan(double& Pan) const
{
	if (!IsOpen())
		return(FALSE);
	LONG	DSPan;
	DXChk(m_DSBuf->GetPan(&DSPan));	// get attenuation, in signed hundredths of a dB
	double	r = DecibelsToLinear(abs(DSPan) / -100.0);	// convert to absolute volume
	if (r <= 1.0 / DSBPAN_RIGHT)	// if below minimum pan
		r = 0;	// substitute zero
	if (DSPan < 0)	// if pan is negative
		Pan = r - 1;	// attenuating right channel
	else	// pan is positive
		Pan = 1 - r;	// attenuating left channel
	return(TRUE);
}

bool CDSPlayer::SetPan(double Pan) const
{
	if (!IsOpen())
		return(FALSE);
	double	r = 1 - fabs(Pan);	// convert pan to absolute volume
	r = CLAMP(r, 1.0 / DSBPAN_RIGHT, 1);	// enforce volume range
	LONG	DSPan = round(LinearToDecibels(r) * 100);	// convert volume to attenuation
	if (Pan > 0)	// if pan is positive
		DSPan = -DSPan;	// make attenuation positive (attenuate left channel)
	DXChk(m_DSBuf->SetPan(DSPan));	// set signed attenuation, in hundredths of a dB
	return(TRUE);
}

bool CDSPlayer::GetFrequency(double& Freq) const
{
	if (!IsOpen())
		return(FALSE);
	DWORD	DSFreq;	// frequency in Hertz or zero for normal playback
	DXChk(m_DSBuf->GetFrequency(&DSFreq));
	if (DSFreq)	// if frequency is non-zero
		Freq = log(double(DSFreq) / m_Wave->GetSampleRate()) / log(2.0);
	else	// frequency is zero
		Freq = 0;	// normal playback
	return(TRUE);
}

bool CDSPlayer::SetFrequency(double Freq) const
{
	if (!IsOpen())
		return(FALSE);
	LONG	DSFreq;	// frequency in Hertz or zero for normal playback
	if (Freq)	// if frequency is non-zero
		DSFreq = round(m_Wave->GetSampleRate() * pow(2.0, Freq));
	else	// frequency is zero
		DSFreq = 0;	// normal playback
	DXChk(m_DSBuf->SetFrequency(DSFreq));
	return(TRUE);
}

bool CDSPlayer::SetPlaying(bool Enable)
{
	if (Enable)
		return(Play());
	return(Stop());
}

bool CDSPlayer::Play()
{
	if (!IsOpen())
		return(FALSE);
	if (m_Playing)	// if already playing
		return(TRUE);	// nothing to do
	DXChk(m_DSBuf->SetCurrentPosition(m_BufSize / 2));	// reset start position
	if (m_Repeat) {	// if repeating 
		if (m_WaveOffset < m_LoopStart)	// if before start of loop
			m_WaveOffset = m_LoopStart;	// jump to start of loop
	}
	m_WaveOffset = CLAMP(m_WaveOffset, 0, m_Wave->GetDataSize());
	m_WriteBufIdx = 0;	// reset write buffer index
	m_PrevWaveOffset[0] = m_WaveOffset;	// set first previous wave offset
	m_NotifyEvent.Reset();	// reset notification event
	SilentFill();
	if (!FillBuffer(0))	// prime first buffer
		return(FALSE);
	if (!m_Feeder.Run(TRUE))	// start feeder
		return(FALSE);
	DXChk(m_DSBuf->Play(0, 0, DSBPLAY_LOOPING));
	m_Playing = TRUE;
	return(TRUE);
}

bool CDSPlayer::Stop()
{
	if (!IsOpen())
		return(FALSE);
	if (!m_Playing)	// if already stopped
		return(TRUE);	// nothing to do
	DXChk(m_DSBuf->Stop());
	m_Feeder.Stop();	// inform feeder that we're stopping it
	m_NotifyEvent.Set();	// set notification event, unblocking feeder
	if (!m_Feeder.Run(FALSE))	// wait for feeder to stop
		return(FALSE);
	W64INT	Frame;
	if (!GetPosition(Frame))
		return(FALSE);
	m_WaveOffset = FrameToOffset(Frame);
	m_Playing = FALSE;
	return(TRUE);
}

inline W64INT CDSPlayer::FrameToOffset(W64INT Frame) const
{
	return(Frame * m_Wave->GetFrameSize());
}

inline W64INT CDSPlayer::OffsetToFrame(W64INT Offset) const
{
	return(Offset / m_Wave->GetFrameSize());
}

bool CDSPlayer::GetPosition(W64INT& Frame) const
{
	if (!IsOpen())
		return(FALSE);
	W64INT	Offset;
	// if playing, and either repeating or not at end of wave
	if (m_Playing) {
//
//		feed phases
//
//						play(0)							play(1)
//						feed(1)							feed(0)
//						V								V
//		0								bs
//		\______________________________/\______________________________/
//  	pwo[0] - bs * 0.5 + cursor		pwo[1] - bs * 0.75 + cursor
//
		ASSERT(BUFFERS == 2);	// phase logic supports dual-buffer only
		DWORD	cursor;
		DXChk(m_DSBuf->GetCurrentPosition(&cursor, NULL));
		if (cursor < m_BufSize)
			Offset = m_PrevWaveOffset[0] - m_BufSize / 2 + cursor;
		else
			Offset = m_PrevWaveOffset[1] - (m_BufSize + m_BufSize / 2) + cursor;
		if (m_Repeat) {	// if repeating
			W64INT	LoopLen = m_LoopEnd - m_LoopStart;	// get loop length
			W64INT	ModOfs = (Offset - m_LoopStart) % LoopLen;	// modulo offset
			if (ModOfs < 0)	// if negative offset
				ModOfs += LoopLen;	// wrap around
			Offset = m_LoopStart + ModOfs;
		}
	} else
		Offset = m_WaveOffset;
	Frame = OffsetToFrame(Offset);	// convert offset to frame
	return(TRUE);
}

bool CDSPlayer::GetWritePosition(W64INT& Frame) const
{
	if (!IsOpen())
		return(FALSE);
	Frame = OffsetToFrame(m_WaveOffset);
	return(TRUE);
}

bool CDSPlayer::SetPosition(W64INT Frame)
{
	if (!IsOpen())
		return(FALSE);
	Frame = CLAMP(Frame, 0, m_Wave->GetFrameCount());
	bool	WasPlaying = m_Playing;	// save play state
	if (!Stop())	// stop audio stream
		return(FALSE);
	m_WaveOffset = FrameToOffset(Frame);	// set wave offset
	if (!SetPlaying(WasPlaying))	// restore previous play state
		return(FALSE);
	return(TRUE);
}

bool CDSPlayer::Rewind()
{
	return(SetPosition(0));
}

bool CDSPlayer::SetRepeat(bool Enable)
{
	m_Repeat = Enable;
	return(TRUE);
}

bool CDSPlayer::GetLoopPoints(W64INT& StartFrame, W64INT& EndFrame) const
{
	if (!IsOpen())
		return(FALSE);
	StartFrame = OffsetToFrame(m_LoopStart);
	EndFrame = OffsetToFrame(m_LoopEnd);
	return(TRUE);
}

bool CDSPlayer::SetLoopPoints(W64INT StartFrame, W64INT EndFrame)
{
	if (!IsOpen())
		return(FALSE);
	if (StartFrame >= EndFrame)	// loop length must be greater than zero
		return(FALSE);
	if (StartFrame < 0 || EndFrame > m_Wave->GetFrameCount())
		return(FALSE);	// loop must be entirely within wave
	bool	WasPlaying = m_Playing;	// save play state
	if (m_Repeat && m_Playing) {
		if (!Stop())
			return(FALSE);
	}
	m_LoopStart = FrameToOffset(StartFrame);
	m_LoopEnd = FrameToOffset(EndFrame);
	if (!SetPlaying(WasPlaying))	// restore previous play state
		return(FALSE);
	return(TRUE);
}

bool CDSPlayer::Close()
{
	bool	retc = Stop();	// stop playing
	m_DSBuf.Release();	// explicitly release buffer interface
	m_Wave = NULL;	// detach from wave
	m_WaveOffset = 0;
	return(retc);
}

bool CDSPlayer::SetBufferDuration(UINT Millis)
{
	if (IsOpen())
		return(FALSE);	// can't set buffer size while open
	m_BufDuration = Millis;
	return(TRUE);
}

inline DWORD CDSPlayer::GetChannelMask(WORD Channels)
{
	DWORD	mask;
	switch (Channels) {
	case 1:	// mono
		mask = SPEAKER_FRONT_CENTER;
		break;
	case 2:	// stereo
		mask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
		break;
	case 3:	// 2.1
		mask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT 
			| SPEAKER_LOW_FREQUENCY;
		break;
	case 4:	// quad
		mask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT 
			| SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
		break;
	case 5:	// 4.1
		mask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT 
			| SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
		break;
	case 6:	// 5.1
		mask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT 
			| SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY 
			| SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
		break;
	case 7:	// 6.1
		mask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT 
			| SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY 
			| SPEAKER_BACK_CENTER | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		break;
	case 8:	// 7.1
		mask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT 
			| SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY 
			| SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT 
			| SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		break;
	default:
		mask = (1 << Channels) - 1;	// set one bit per channel
	}
	return(mask);
}

bool CDSPlayer::Open(CWave& Wave)
{
	if (!IsCreated())
		return(FALSE);
	bool	WasPlaying = m_Playing;	// save play state
	Close();
	m_Wave = &Wave;
	// get wave format
	WAVEFORMATEXTENSIBLE	WaveFmtExt;
	WAVEFORMATEX&	WaveFmt = WaveFmtExt.Format;
	m_Wave->GetFormat(WaveFmtExt);
	if (!WaveFmtExt.dwChannelMask)	// if zero channel mask
		WaveFmtExt.dwChannelMask = GetChannelMask(WaveFmt.nChannels);	// fix it
	// compute our buffer size, which DirectSound's buffer size is an integer
	// multiple of; our size must be evenly divisible by block align TIMES TWO
	// because we start filling in middle of buffer, see comment in GetPosition
	ULONGLONG	BufSamps = ULONGLONG(WaveFmt.nSamplesPerSec) * m_BufDuration / 2000;
	m_BufSize = int(min(BufSamps * WaveFmt.nBlockAlign * 2, DSBSIZE_MAX));
	// create sound buffer
	DSBUFFERDESC	desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY
		| DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
	desc.dwBufferBytes = m_BufSize * BUFFERS;	// set DirectSound buffer size
	desc.lpwfxFormat = &WaveFmt;
	DXChk(m_DS->CreateSoundBuffer(&desc, &m_DSBuf, NULL));
	// request buffer notifications
	CComPtr<IDirectSoundNotify>	pINotify;
	DXChk(m_DSBuf->QueryInterface(IID_IDirectSoundNotify, (void **)&pINotify));
	DSBPOSITIONNOTIFY	Notify[BUFFERS];
	for (int iBuf = 0; iBuf < BUFFERS; iBuf++) {
		Notify[iBuf].dwOffset = iBuf * m_BufSize + m_BufSize / 2;
		Notify[iBuf].hEventNotify = m_NotifyEvent;
	}
	DXChk(pINotify->SetNotificationPositions(BUFFERS, Notify));
	// initialize remaining members
	if (WaveFmt.wBitsPerSample <= 8)	// if 8-bit samples
		m_SilenceVal = 0x80;	// special silence value
	else	// not 8-bit samples
		m_SilenceVal = 0;	// normal silence value
	m_LoopStart = 0;	// reset loop points for entire wave
	m_LoopEnd = m_Wave->GetDataSize();
	// start playing wave if needed
	if (!SetPlaying(WasPlaying))	// restore previous play state
		return(FALSE);
	return(TRUE);
}

bool CDSPlayer::SilentFill()
{
	PVOID	pBuf;
	DWORD	Len;
	DXChk(m_DSBuf->Lock(0, 0, &pBuf, &Len, NULL, NULL, DSBLOCK_ENTIREBUFFER));
	memset(((BYTE *)pBuf), m_SilenceVal, Len);
	DXChk(m_DSBuf->Unlock(pBuf, Len, NULL, 0));
	return(TRUE);
}

inline void CDSPlayer::NormalFill(PVOID pBuf, DWORD Len)
{
	// compute chunk size
	W64INT	WaveRemain = m_Wave->GetDataSize() - m_WaveOffset;
	UINT	ChunkSize = UINT(CLAMP(WaveRemain, 0, int(Len)));
	// write chunk to buffer, filling remainder with silence
	BYTE	*pWaveData = m_Wave->GetData() + m_WaveOffset;
	memcpy(pBuf, pWaveData, ChunkSize);
	memset(((BYTE *)pBuf) + ChunkSize, m_SilenceVal, Len - ChunkSize);
	m_WaveOffset += Len;	// increment wave offset past buffer
}

inline void CDSPlayer::RepeatFill(PVOID pBuf, DWORD Len)
{
	if (m_Wave->IsEmpty()) {	// if wave is empty
		NormalFill(pBuf, Len);	// avoid infinite loop; fill with silence
	} else {	// wave isn't empty
		if (m_WaveOffset < m_LoopStart)	// if before start of loop
			m_WaveOffset = m_LoopStart;	// jump to start of loop
		int	BufRemain = Len;
		while (BufRemain > 0) {	// loop until we fill buffer
			if (m_WaveOffset >= m_LoopEnd)	// if past end of loop
				m_WaveOffset = m_LoopStart;	// wrap to start of loop
			// compute chunk size
			W64INT	WaveRemain = m_LoopEnd - m_WaveOffset;
			UINT	ChunkSize = UINT(min(WaveRemain, BufRemain));
			// write chunk to buffer
			BYTE	*pWaveData = m_Wave->GetData() + m_WaveOffset;
			memcpy(((BYTE *)pBuf) + Len - BufRemain, pWaveData, ChunkSize);
			m_WaveOffset += ChunkSize;	// increment wave offset past chunk
			BufRemain -= ChunkSize;	// subtract chunk from buffer remaining
		}
	}
}

bool CDSPlayer::FillBuffer(UINT BufIdx)
{
	m_PrevWaveOffset[BufIdx] = m_WaveOffset;
	DWORD	WriteOfs = BufIdx * m_BufSize + m_BufSize / 2;
	PVOID	pBuf1, pBuf2;
	DWORD	Len1, Len2;
	DXChk(m_DSBuf->Lock(WriteOfs, m_BufSize, &pBuf1, &Len1, &pBuf2, &Len2, 0));
	if (m_Repeat) {	// if repeating
		RepeatFill(pBuf1, Len1);
		if (pBuf2 != NULL)	// if second write needed
			RepeatFill(pBuf2, Len2);
	} else {	// not repeating
		NormalFill(pBuf1, Len1);
		if (pBuf2 != NULL)	// if second write needed
			NormalFill(pBuf2, Len2);
	}
	DXChk(m_DSBuf->Unlock(pBuf1, Len1, pBuf2, Len2));
	return(TRUE);
}

bool CDSPlayer::FeederMain()
{
	try {
		// initialize COM for multithreading for this worker thread
		CCoInitializer	coninit(COINIT_MULTITHREADED);
		while (m_Feeder.WaitForStart()) {
			while (1) {
				DWORD	retc = WaitForSingleObject(m_NotifyEvent, INFINITE);
				if (m_Feeder.GetStopFlag())
					break;
				switch (retc) {
				case WAIT_OBJECT_0:
					m_WriteBufIdx++;	// increment index to next buffer
					if (m_WriteBufIdx >= BUFFERS)	// if past last buffer
						m_WriteBufIdx = 0;	// wrap index
					FillBuffer(m_WriteBufIdx);	// fill buffer
					break;
				}
			}
		}
	}
	catch (HRESULT) {
		return(FALSE);	// COM init failed
	}
	return(TRUE);
}

UINT CDSPlayer::FeederFunc(LPVOID pParam)
{
	CDSPlayer	*pPlayer = (CDSPlayer *)pParam;
	pPlayer->FeederMain();
	return(0);
}
