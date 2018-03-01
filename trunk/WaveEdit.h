// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
        01      24jan13	add GetFileSize
        02      18feb13	add import and export
        03      02mar13	add MatchFormat
		04		09mar13	add AbbreviateChannelName
        05		04jun13	in Replace, add channel selection
		06		28jul13	add metadata

		wave editing
 
*/

#ifndef CWAVEEDIT_INCLUDED
#define CWAVEEDIT_INCLUDED

#include "Wave.h"
#include "ArrayEx.h"
#include "RefPtr.h"

class CWaveEdit : public CWave {
public:
// Construction
	CWaveEdit();
	virtual	~CWaveEdit();
	CWaveEdit(const CWaveEdit&);
	CWaveEdit& operator=(const CWaveEdit&);

// Constants
	enum {	// define speakers
		#define SPEAKERDEF(name) SPKR_##name,
		#include "SpeakerDef.h"
		SPEAKERS
	};
	enum {	// match format flags
		MF_COMPATIBLE		= 0x01,	// format must be compatible
		MF_SAMPLE_SIZE		= 0x02,	// sample size must match
		MF_SAMPLE_RATE		= 0x04,	// sample rate must match
	};

// Types
	struct CHAN_STATS {
		SAMPLE	Min;		// minimum sample
		SAMPLE	Max;		// maximum sample
		double	Sum;		// sum of samples
	};
	typedef CArrayEx<CHAN_STATS, CHAN_STATS&> CChanStatsArray;

// Attributes
	bool	IsWithinData(LPBYTE pData, W64INT Len) const;
	bool	IsValidRange(const CW64IntRange& Sel) const;
	bool	IsCompatibleFormat(const CWaveEdit& Wave) const;
	bool	IsFileOpen() const;
	ULONGLONG	GetFileSize() const;
	static	CString	GetSpeakerName(int SpeakerIdx);
	void	GetChannelNames(CStringArray& ChannelName) const;
	static	CString AbbreviateChannelName(LPCTSTR Name);
	static	CString	GetChannelCountString(UINT Channels);
	CString	GetChannelCountString() const;
	static	CString	GetSampleBitsString(UINT SampleBits);
	CString	GetSampleBitsString() const;
	static	CString	GetSampleRateString(UINT SampleRate);
	CString	GetSampleRateString() const;
	CString	GetAudioFormatString() const;

// Operations
	bool	Cut(CWaveEdit& Wave, const CW64IntRange& Sel);
	bool	Copy(CWaveEdit& Wave, const CW64IntRange& Sel) const;
	bool	Insert(const CWaveEdit& Wave, W64INT Frame);
	bool	Replace(const CWaveEdit& Wave, W64INT Frame, const BYTE *ChanSel = NULL);
	bool	Delete(const CW64IntRange& Sel);
	bool	InsertSilence(W64INT Frame, W64INT FrameCount);
	bool	ProgressRead(LPCTSTR Path, CStringArray *Metadata = NULL);
	bool	ProgressWrite(LPCTSTR Path, const CStringArray *Metadata = NULL) const;
	bool	FindZeroCrossing(W64INT& Frame, bool Reverse) const;
	bool	FindZeroCrossing(W64INT& Frame) const;
	bool	Import(LPCTSTR Path, int& Format, CStringArray *Metadata = NULL);
	bool	Export(LPCTSTR Path, int Format, const CStringArray *Metadata = NULL);
	void	MatchFormat(const CWaveEdit& Wave, UINT Flags) const;

protected:
// Types
	class CFileEx : public CFile, public CRefObj {
	public:
		void	SetCloseOnDelete(bool Enable);
	};

// Constants
	enum {
		IO_BLOCK_SIZE = 0x2000000	// I/O callback period in bytes (32 MB)
	};
	static const int m_SpeakerNameID[];	// array of speaker name string resource IDs

// Data members
	CRefPtr<CFileEx>	m_TmpFile;	// alternate data storage, or null if none

// Helpers
	void	Copy(const CWaveEdit& Src);
	int		GetHdrSize() const;
	W64INT	ReplaceChannels(const CWave& Wave, W64INT DstOffset, W64INT Frames, const BYTE *ChanSel);
	static	bool	IOCallback(UINT iBlock, UINT nBlocks, WPARAM wParam, LPARAM lParam);
};

inline CWaveEdit::CWaveEdit()
{
}

inline CWaveEdit::CWaveEdit(const CWaveEdit& Src)
{
	Copy(Src);
}

inline CWaveEdit& CWaveEdit::operator=(const CWaveEdit& Src)
{
	if (&Src != this)
		Copy(Src);
	return(*this);
}

inline bool CWaveEdit::IsFileOpen() const
{
	return(m_TmpFile != NULL && m_TmpFile->m_hFile != CFile::hFileNull);
}

inline void CWaveEdit::CFileEx::SetCloseOnDelete(bool Enable)
{
	m_bCloseOnDelete = Enable;
}

inline ULONGLONG CWaveEdit::GetFileSize() const
{
	return(m_TmpFile->GetLength());
}

#endif
