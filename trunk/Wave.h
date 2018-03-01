// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
		01		18feb13	make ThrowError public
		02		19feb13	use enhanced byte array to allow data sizes > 4GB
		03		13mar13	add channel mask to ctor
		04		30apr13	add RF64 header write
		05		01aug13	add metadata

		wave container
 
*/

#ifndef CWAVE_INCLUDED
#define CWAVE_INCLUDED

#include "ks.h"			// must precede ksmedia.h
#include "ksmedia.h"	// for KSDATAFORMAT_SUBTYPE_PCM
#include "mmsystem.h"	// for MAKEFOURCC
#include "mmreg.h"		// for WAVEFORMATEXTENSIBLE
#include "ByteArrayEx.h"

class CWave : public WObject {
public:
// Types
	typedef int SAMPLE;
	typedef bool (*IO_CALLBACK)(UINT iBlock, UINT nBlocks, WPARAM wParam, LPARAM lParam);
	struct IO_HOOK {
		IO_CALLBACK	Callback;	// I/O callback function
		WPARAM	wParam;			// user-defined parameter
		LPARAM	lParam;			// user-defined parameter
		UINT	BlockSize;		// callback period in bytes
	};

// Construction
	CWave();
	virtual	~CWave();
	CWave(UINT Channels, UINT SampleRate, UINT SampleBits, UINT ChannelMask = 0);
	CWave(const CWave& Src);
	CWave& operator=(const CWave& Src);

// Attributes
	bool	IsValid() const;
	bool	IsEmpty() const;
	void	GetFormat(WAVEFORMATEX& WaveFmt) const;
	void	GetFormat(WAVEFORMATEXTENSIBLE& WaveFmtExt) const;
	void	SetFormat(UINT Channels, UINT SampleRate, UINT SampleBits, UINT ChannelMask = 0);
	UINT	GetChannels() const;
	void	SetChannels(UINT Channels);
	UINT	GetSampleRate() const;
	void	SetSampleRate(UINT SampleRate);
	UINT	GetSampleBits() const;
	void	SetSampleBits(UINT SampleBits);
	void	GetSampleRails(SAMPLE& NegRail, SAMPLE& PosRail) const;
	UINT	GetChannelMask() const;
	void	SetChannelMask(UINT Mask);
	UINT	GetSampleSize() const;
	UINT	GetFrameSize() const;
	UINT	GetBytesPerSec() const;
	W64INT	GetFrameCount() const;
	void	SetFrameCount(W64INT FrameCount);
	W64INT	GetDataSize() const;
	SAMPLE	GetSample(UINT ChannelIdx, W64INT FrameIdx) const;
	void	SetSample(UINT ChannelIdx, W64INT FrameIdx, SAMPLE Value);
	W64INT	GetByteOffset(UINT ChannelIdx, W64INT FrameIdx) const;
	SAMPLE	GetSampleAt(W64INT ByteOffset) const;
	void	SetSampleAt(W64INT ByteOffset, SAMPLE Value);
	const BYTE	*GetData() const;
	BYTE	*GetData();

// Operations
	void	Init();
	void	Empty();
	void	Read(LPCTSTR Path, const IO_HOOK *Hook = NULL, CStringArray *Metadata = NULL);
	void	Write(LPCTSTR Path, const IO_HOOK *Hook = NULL, const CStringArray *Metadata = NULL) const;
	bool	SafeRead(LPCTSTR Path, const IO_HOOK *Hook = NULL, CStringArray *Metadata = NULL);
	bool	SafeWrite(LPCTSTR Path, const IO_HOOK *Hook = NULL, const CStringArray *Metadata = NULL) const;
	void	WriteRF64Header(CFile& fp, ULONGLONG FrameCount);
	void	WriteRF64CompatibleHeader(CFile& fp, ULONGLONG FrameCount);
	static	void	ThrowError(int ErrorID);

protected:
// Constants
	static const FOURCC	m_MetadataTag[];	// metadata string tags

// Data members
	UINT	m_Channels;		// channel count
	UINT	m_SampleRate;	// sampling frequency in Hz
	UINT	m_SampleBits;	// bits per sample
	UINT	m_ChannelMask;	// speaker configuration
	UINT	m_SampleSize;	// bytes per sample point
	UINT	m_FrameSize;	// bytes per sample frame
	W64INT	m_FrameCount;	// number of sample frames
	CByteArrayEx	m_Data;		// sample data

// Helpers
	void	Copy(const CWave& Src);
	void	OnFormatChange();
	static	void	Read(CFile& fp, void *lpBuf, UINT nCount);
	void	ReadData(CFile& fp, BYTE *pData, W64UINT Length, const IO_HOOK *Hook);
	void	WriteData(CFile& fp, const BYTE *pData, W64UINT Length, const IO_HOOK *Hook) const;
	void	ReadMetadata(CFile& fp, UINT ListSize, CStringArray *Metadata);
	void	WriteMetadata(CFile& fp, const CStringArray *Metadata) const;
	UINT	GetMetadataSize(const CStringArray *Metadata) const;
};

class CWaveFileException : public CFileException {
public:
	DECLARE_DYNAMIC(CWaveFileException);
	CWaveFileException(UINT ResID);
	virtual	BOOL	GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext = NULL);
	UINT	m_ResID;		// string resource ID of error message
};

inline CWave::CWave(const CWave& Src)
{
	Copy(Src);
}

inline CWave& CWave::operator=(const CWave& Src)
{
	if (&Src != this)
		Copy(Src);
	return(*this);
}

inline bool CWave::IsValid() const
{
	return(GetFrameSize() != 0);
}

inline bool CWave::IsEmpty() const
{
	return(!GetFrameCount());
}

inline UINT CWave::GetChannels() const
{
	return(m_Channels);
}

inline UINT CWave::GetSampleRate() const
{
	return(m_SampleRate);
}

inline UINT CWave::GetSampleBits() const
{
	return(m_SampleBits);
}

inline UINT CWave::GetChannelMask() const
{
	return(m_ChannelMask);
}

inline void CWave::SetChannelMask(UINT Mask)
{
	m_ChannelMask = Mask;
}

inline UINT CWave::GetSampleSize() const
{
	return(m_SampleSize);
}

inline UINT CWave::GetFrameSize() const
{
	return(m_FrameSize);
}

inline UINT CWave::GetBytesPerSec() const
{
	return(m_FrameSize * m_SampleRate);
}

inline W64INT CWave::GetDataSize() const
{
	return(m_Data.GetSize());
}

inline W64INT CWave::GetFrameCount() const
{
	return(m_FrameCount);
}

inline const BYTE *CWave::GetData() const
{
	return(m_Data.GetData());
}

inline BYTE *CWave::GetData()
{
	return(m_Data.GetData());
}

#endif
