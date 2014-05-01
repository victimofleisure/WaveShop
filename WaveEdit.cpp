// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
        01      24jan13	in Copy, if storing in memory, destroy temp file
        02      11feb13	in Insert, fix data array bloat from throwing
        03      18feb13	add import and export
        04      27feb13	in Import, add MPEG audio decoder
		05		27feb13	in Copy's CreateFile, remove erroneous CREATE_NEW
		06		28feb13	in IOCallback, check progress dialog create
		07		01mar13	throw errors instead of returning false
        08      02mar13	add MatchFormat
		09		09mar13	add AbbreviateChannelName
		10		10apr13	in Export, add MP3 encoder
		11		20apr13	in Export, add AAC/MP4 encoder
        12		04jun13	in Replace, add channel selection
		13		28jul13	add metadata
		14		13oct13	in IOCallback, check for progress dialog already created

		wave editing
 
*/

#include "stdafx.h"
#include "WaveShop.h"
#include "WaveEdit.h"
#include "ProgressDlg.h"
#include "SndFileEx.h"
#include "MadWrap.h"
#include "ID3TagWrap.h"
#include "LameWrap.h"
#include "MP3EncoderDlg.h"
#include "MP4DecodeWrap.h"

const int CWaveEdit::m_SpeakerNameID[] = {
	#define SPEAKERDEF(name) IDS_SPKR_##name,
	#include "SpeakerDef.h"
};

CWaveEdit::~CWaveEdit()
{
}

void CWaveEdit::Copy(const CWaveEdit& Src)
{
	CWave::Copy(Src);
	m_TmpFile = Src.m_TmpFile;
}

bool CWaveEdit::IsWithinData(LPBYTE pData, W64INT Len) const
{
	return(pData >= GetData() && pData + Len <= GetData() + GetDataSize());
}

bool CWaveEdit::IsValidRange(const CW64IntRange& Sel) const
{
	return(Sel.Start >= 0 && Sel.End <= m_FrameCount);
}

bool CWaveEdit::IsCompatibleFormat(const CWaveEdit& Wave) const
{
	return(Wave.m_Channels == m_Channels && Wave.m_SampleBits == m_SampleBits);
}

bool CWaveEdit::IOCallback(UINT iBlock, UINT nBlocks, WPARAM wParam, LPARAM lParam)
{
	CProgressDlg	*ProgDlg = (CProgressDlg *)lParam;
	ASSERT(ProgDlg != NULL);	// progress dialog is required
	if (!iBlock && nBlocks > 1) {	// if first of multiple blocks
		if (ProgDlg->m_hWnd == NULL) {	// if not already created
			if (!ProgDlg->Create())	// create progress dialog
				AfxThrowResourceException();
		}
		ProgDlg->SetWindowText(LDS(wParam));
		ProgDlg->SetRange(0, nBlocks);	// set progress range
	}
	if (ProgDlg->m_hWnd)	// if progress dialog was created
		ProgDlg->SetPos(iBlock);	// set progress position
	return(!ProgDlg->Canceled());	// if user canceled, return false to abort
}

bool CWaveEdit::ProgressRead(LPCTSTR Path, CStringArray *Metadata)
{
	CProgressDlg	ProgDlg;
	IO_HOOK	Hook = {IOCallback, IDS_DOC_READING, LPARAM(&ProgDlg), IO_BLOCK_SIZE};
	return(SafeRead(Path, &Hook, Metadata));
}

bool CWaveEdit::ProgressWrite(LPCTSTR Path, const CStringArray *Metadata) const
{
	CProgressDlg	ProgDlg;
	IO_HOOK	Hook = {IOCallback, IDS_DOC_WRITING, LPARAM(&ProgDlg), IO_BLOCK_SIZE};
	return(SafeWrite(Path, &Hook, Metadata));
}

bool CWaveEdit::Cut(CWaveEdit& Wave, const CW64IntRange& Sel)
{
	if (!Copy(Wave, Sel))
		return(FALSE);
	return(Delete(Sel));
}

inline int CWaveEdit::GetHdrSize() const
{
	return(offsetof(CWaveEdit, m_Data));
}

bool CWaveEdit::Copy(CWaveEdit& Wave, const CW64IntRange& Sel) const
{
	if (!IsValid())	// if invalid source
		ThrowError(IDS_WEDT_INVALID_WAVE);
	if (!IsValidRange(Sel))	// if invalid selection range
		ThrowError(IDS_WEDT_RANGE_ERROR);
	if (IsFileOpen())	// if source is temp file
		ThrowError(IDS_WEDT_NOT_IN_MEMORY);
	W64INT	Frames = Sel.Length();
	W64INT	DataOffset = Sel.Start * m_FrameSize;
	W64INT	DataSize = Frames * m_FrameSize;
	const BYTE	*pSrcData = GetData() + DataOffset;
	CopyMemory(&Wave, this, GetHdrSize());	// copy header
	LONGLONG	DiskThreshold =	LONGLONG(theApp.GetMain()->GetOptions().m_DiskThreshold);
	DiskThreshold <<= 20;	// convert disk threshold from MB to bytes
	if (DataSize < DiskThreshold) {	// if storing data in memory
		Wave.m_TmpFile.SetEmpty();	// destroy destination temp file if any
		Wave.m_FrameCount = 0;	// in case allocator throws
		Wave.m_Data.SetSize(DataSize);	// allocate destination data array
		CopyMemory(Wave.GetData(), pSrcData, DataSize);	// copy data
	} else {	// store data in temp file
		Wave.Empty();	// empty destination data array first in case we throw
		CString	TmpPath;
		if (!theApp.GetTempFileName(TmpPath))	// get temp file path
			ThrowError(IDS_WEDT_NO_TEMP_FILE_NAME);
		Wave.m_TmpFile.CreateObj();	// create file instance
		Wave.m_TmpFile->SetCloseOnDelete(TRUE);	// ensure destructor closes file
		Wave.m_TmpFile->SetFilePath(TmpPath);	// set path for error reporting
		HANDLE	hFile = CreateFile(TmpPath,	// create temp file
			GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
		if (hFile == INVALID_HANDLE_VALUE)	// if create failed, throw error
			CFileException::ThrowOsError((LONG)::GetLastError(), TmpPath);
#if _MFC_VER < 0x0700
		Wave.m_TmpFile->m_hFile = (UINT)hFile;	// attach file object to handle
#else
		Wave.m_TmpFile->m_hFile = hFile;	// attach file object to handle
#endif
		CProgressDlg	ProgDlg;
		IO_HOOK	Hook = {IOCallback, IDS_DOC_WRITING, LPARAM(&ProgDlg), IO_BLOCK_SIZE};
		Wave.WriteData(*Wave.m_TmpFile, pSrcData, DataSize, &Hook);
	}
	Wave.m_FrameCount = Frames;	// set frame count last for safety
	return(TRUE);
}

bool CWaveEdit::Insert(const CWaveEdit& Wave, W64INT Frame)
{
	if (!Wave.IsValid())	// if invalid source wave
		ThrowError(IDS_WEDT_INVALID_WAVE);
	if (IsFileOpen())	// if destination is temp file
		ThrowError(IDS_WEDT_NOT_IN_MEMORY);
	if (Frame < 0 || Frame > m_FrameCount)	// if frame range error
		ThrowError(IDS_WEDT_RANGE_ERROR);
	if (IsValid()) {	// if we're valid
		if (!IsCompatibleFormat(Wave))	// if incompatible formats
			ThrowError(IDS_WEDT_INCOMPATIBLE_FORMATS);
	} else {	// we're invalid
		CopyMemory(this, &Wave, GetHdrSize());	// copy header
		Empty();	// empty data array and zero frame count
	}
	W64INT	SrcLen;
	if (!Wave.IsFileOpen())	// if source stores data in memory
		SrcLen = Wave.GetDataSize();	// get source length from data array
	else	// source stores data in temp file
		SrcLen = static_cast<W64INT>(Wave.m_TmpFile->GetLength());	// get source length from temp file
	W64INT	DstOffset = Frame * m_FrameSize;
	// calculate destination length from frame count (which gets updated post-read)
	// instead of GetDataSize; otherwise data size rachets up if we throw repeatedly
	W64INT	DstLen = m_FrameCount * m_FrameSize;
	W64INT	ShiftLen = DstLen - DstOffset;
	m_Data.SetSize(DstLen + SrcLen);	// grow our data array as needed
	BYTE	*pDstData = GetData() + DstOffset;	// order matters; data may have moved
	ASSERT(IsWithinData(pDstData + SrcLen, ShiftLen));	// check move destination
	MoveMemory(pDstData + SrcLen, pDstData, ShiftLen);	// make room for source data
	ASSERT(IsWithinData(pDstData, SrcLen));		// check copy destination
	if (!Wave.IsFileOpen()) {	// if source stores data in memory
		CopyMemory(pDstData, Wave.GetData(), SrcLen);	// insert source data
	} else {	// source stores data in temp file
		CProgressDlg	ProgDlg;
		IO_HOOK	Hook = {IOCallback, IDS_DOC_READING, LPARAM(&ProgDlg), IO_BLOCK_SIZE};
		CWaveEdit&	MutableWave = const_cast<CWaveEdit&>(Wave);	// cast away const
		MutableWave.m_TmpFile->SeekToBegin();	// rewind temp file
		MutableWave.ReadData(*MutableWave.m_TmpFile, pDstData, SrcLen, &Hook);
	}
	m_FrameCount += Wave.m_FrameCount;	// update frame count last
	return(TRUE);
}

bool CWaveEdit::Replace(const CWaveEdit& Wave, W64INT Frame, const BYTE *ChanSel)
{
	if (!Wave.IsValid() || !IsValid())	// if invalid source or destination
		ThrowError(IDS_WEDT_INVALID_WAVE);
	if (IsFileOpen())	// if destination is temp file
		ThrowError(IDS_WEDT_NOT_IN_MEMORY);
	if (Frame < 0 || Frame + Wave.m_FrameCount > m_FrameCount)	// if frame range error
		ThrowError(IDS_WEDT_RANGE_ERROR);
	if (!IsCompatibleFormat(Wave))	// if incompatible formats
		ThrowError(IDS_WEDT_INCOMPATIBLE_FORMATS);
	W64INT	DstOffset = Frame * m_FrameSize;
	if (ChanSel != NULL) {	// if channel selection specified
		if (!Wave.IsFileOpen()) {	// if source stores data in memory
			ReplaceChannels(Wave, DstOffset, Wave.m_FrameCount, ChanSel);
		} else {	// source stores data in temp file
			CWaveEdit&	MutableWave = const_cast<CWaveEdit&>(Wave);	// cast away const
			MutableWave.m_TmpFile->SeekToBegin();	// rewind temp file
			W64INT	RemainFrames = Wave.GetFrameCount();
			const int	BUFFER_SIZE = 0x100000;	// read buffer size, in bytes
			int	BufferFrames = BUFFER_SIZE / m_FrameSize;
			CWave	buf(m_Channels, m_SampleRate, m_SampleBits);
			buf.SetFrameCount(BufferFrames);
			W64INT	Chunks = (RemainFrames - 1) / BufferFrames + 1;
			CProgressDlg	ProgDlg;
			ProgDlg.Create();
			ProgDlg.SetWindowText(LDS(IDS_DOC_READING));
			ProgDlg.SetRange(0, INT64TO32(Chunks));
			for (int iChunk = 0; iChunk < Chunks; iChunk++) {	// for each chunk
				ProgDlg.SetPos(iChunk);
				if (ProgDlg.Canceled())	// if user canceled
					AfxThrowUserException();
				W64INT	ChunkFrames = min(RemainFrames, BufferFrames);
				W64INT	ChunkBytes = ChunkFrames * m_FrameSize;
				MutableWave.ReadData(*MutableWave.m_TmpFile, buf.GetData(), ChunkBytes, NULL);
				DstOffset = ReplaceChannels(buf, DstOffset, ChunkFrames, ChanSel);
				RemainFrames -= ChunkFrames;
			}
		}
	} else {	// null channel selection
		BYTE	*pDstData = GetData() + DstOffset;
		W64INT	SrcLen;
		if (!Wave.IsFileOpen())	// if source stores data in memory
			SrcLen = Wave.GetDataSize();	// get source length from data array
		else	// source stores data in temp file
			SrcLen = static_cast<W64INT>(Wave.m_TmpFile->GetLength());	// get source length from temp file
		ASSERT(IsWithinData(pDstData, SrcLen));		// check copy destination
		if (!Wave.IsFileOpen()) {	// if source stores data in memory
			CopyMemory(pDstData, Wave.GetData(), SrcLen);	// insert source data
		} else {	// source stores data in temp file
			CProgressDlg	ProgDlg;
			IO_HOOK	Hook = {IOCallback, IDS_DOC_READING, LPARAM(&ProgDlg), IO_BLOCK_SIZE};
			CWaveEdit&	MutableWave = const_cast<CWaveEdit&>(Wave);	// cast away const
			MutableWave.m_TmpFile->SeekToBegin();	// rewind temp file
			MutableWave.ReadData(*MutableWave.m_TmpFile, pDstData, SrcLen, &Hook);
		}
	}
	return(TRUE);
}

W64INT CWaveEdit::ReplaceChannels(const CWave& Wave, W64INT DstOffset, W64INT Frames, const BYTE *ChanSel)
{
	W64INT	SrcOffset = 0;
	for (W64INT iFrame = 0; iFrame < Frames; iFrame++) {	// for each frame
		for (UINT iChan = 0; iChan < m_Channels; iChan++) {	// for each channel
			if (ChanSel[iChan])	// if channel is selected
				SetSampleAt(DstOffset, Wave.GetSampleAt(SrcOffset));	// copy it
			SrcOffset += m_SampleSize;
			DstOffset += m_SampleSize;
		}
	}
	return(DstOffset);
}

bool CWaveEdit::Delete(const CW64IntRange& Sel)
{
	if (!IsValid())	// if invalid target
		ThrowError(IDS_WEDT_INVALID_WAVE);
	if (!IsValidRange(Sel))	// if invalid selection range
		ThrowError(IDS_WEDT_RANGE_ERROR);
	if (IsFileOpen())	// if target is temp file
		ThrowError(IDS_WEDT_NOT_IN_MEMORY);
	W64INT	Offset = GetByteOffset(0, Sel.Start);
	W64INT	Length = Sel.Length();
	m_Data.RemoveAt(Offset, Length * m_FrameSize);
	m_FrameCount -= Length;
	return(TRUE);
}

bool CWaveEdit::InsertSilence(W64INT Frame, W64INT FrameCount)
{
	if (!IsValid())	// if invalid target
		ThrowError(IDS_WEDT_INVALID_WAVE);
	if (Frame < 0 || Frame > m_FrameCount || FrameCount < 0)	// if frame range error
		ThrowError(IDS_WEDT_RANGE_ERROR);
	if (!FrameCount)
		return(TRUE);	// nothing to do; otherwise InsertAt asserts
	if (IsFileOpen())	// if target is temp file
		ThrowError(IDS_WEDT_NOT_IN_MEMORY);
	W64INT	DstOffset = Frame * m_FrameSize;
	W64INT	ShiftLen = FrameCount * m_FrameSize;
	BYTE	SilenceVal;
	if (m_SampleBits <= 8)	// if 8-bit samples
		SilenceVal = 0x80;	// special silence value
	else	// not 8-bit samples
		SilenceVal = 0;	// normal silence value
	m_Data.InsertAt(DstOffset, SilenceVal, ShiftLen);	// shift data array as needed
	m_FrameCount += FrameCount;	// update frame count last
	return(TRUE);
}

bool CWaveEdit::FindZeroCrossing(W64INT& Frame, bool Reverse) const
{
	W64INT	frames = m_FrameCount;
	W64INT	start = CLAMP(Frame, 0, frames - 1);	// avoid range errors
	int	chans = m_Channels;
	if (Reverse) {	// if searching backward
		for (W64INT iFrame = start - 1; iFrame >= 0; iFrame--) {	// for each frame
			for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
				if (GetSample(iChan, iFrame) < 0
				&& GetSample(iChan, iFrame + 1) >= 0) {	// if span crosses zero
					Frame = iFrame;
					return(TRUE);
				}
			}
		}
	} else {	// searching forward
		for (W64INT iFrame = start; iFrame < frames - 1; iFrame++) {	// for each frame
			for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
				if (GetSample(iChan, iFrame) < 0
				&& GetSample(iChan, iFrame + 1) >= 0) {	// if span crosses zero
					Frame = iFrame;
					return(TRUE);
				}
			}
		}
	}
	return(FALSE);
}

bool CWaveEdit::FindZeroCrossing(W64INT& Frame) const
{
	W64INT	prev = Frame;
	W64INT	next = Frame;
	bool	GotPrev = FindZeroCrossing(prev, TRUE);	// find previous crossing
	bool	GotNext = FindZeroCrossing(next, FALSE);	// find next crossing
	if (GotPrev) {	// if previous found
		if (GotNext) {	// if next found 
			if ((next - Frame) < (Frame - prev))	// if next is closer
				Frame = next;
			else	// previous is closer, or equidistant
				Frame = prev;
		} else	// next not found
			Frame = prev;
	} else {	// previous not found
		if (GotNext)	// if next found 
			Frame = next;
		else	// neither found
			return(FALSE);
	}
	return(TRUE);
}

bool CWaveEdit::Import(LPCTSTR Path, int& Format, CStringArray *Metadata)
{
	Format = 0;	// initialize format first
	CString	ext(PathFindExtension(Path));
	ext.MakeLower();
	CString	filter(MPEG_FILTER);
	if (filter.Find(ext) >= 0) {	// if extension is associated with MPEG audio
		if (Metadata != NULL) {
			CID3TagWrap	id3;
			if (id3.Create())
				id3.Read(Path, *Metadata);
		}
		CMadWrap	mad;
		int	Quality = theApp.GetMain()->GetOptions().m_MP3ImportQuality;
		if (!mad.Create(Quality))	// create MPEG audio decoder
			return(FALSE);
		if (!mad.Read(Path, *this))	// read MPEG audio
			return(FALSE);
		Format = CSndFileEx::FORMAT_MP3;	// set format last
		return(TRUE);	// success
	}
	filter = MP4_FILTER;
	if (filter.Find(ext) >= 0) {	// if extension is associated with MPEG audio
		CMP4DecodeWrap	mp4;
		const COptionsInfo	opts = theApp.GetMain()->GetOptions();
		int	Quality = opts.m_MP4ImportQuality;
		bool	Downmix = opts.m_MP4Downmix;
		if (!mp4.Create(Quality, Downmix))	// create MP4 audio decoder
			return(FALSE);
		if (!mp4.Read(Path, *this))	// read MPEG audio
			return(FALSE);
		return(TRUE);	// success
	}
	CProgressDlg	ProgDlg;
	IO_HOOK	Hook = {IOCallback, IDS_DOC_READING, LPARAM(&ProgDlg), IO_BLOCK_SIZE};
	// try native read first
	TRY {
		Read(Path, &Hook, Metadata);
		return(TRUE);
	}
	CATCH (CUserException, e) {
		return(FALSE);
	}
	CATCH (CException, e) {
		if (!PathFileExists(CSndFileEx::GetLibPath())) {	// if libsndfile not found
			e->ReportError();
			return(FALSE);
		}
	}
	END_CATCH;
	// read failed but wasn't canceled, and libsndfile exists, so try libsndfile
	TRY {
//		if (IsWindow(ProgDlg))
//			ProgDlg.DestroyWindow();
		CSndFileEx	sf;
		if (!sf.Create())	// create sndfile wrapper
			return(FALSE);
		if (!sf.Read(Path, *this, Format, Hook, Metadata))
			return(FALSE);
	}
	CATCH (CUserException, e) {
		return(FALSE);
	}
	CATCH (CException, e) {
		e->ReportError();
		return(FALSE);
	}
	END_CATCH;
	return(TRUE);
}

bool CWaveEdit::Export(LPCTSTR Path, int Format, const CStringArray *Metadata)
{
	CString	ext(PathFindExtension(Path));
	ext.MakeLower();
	if (ext == _T(".mp3"))	// if extension is MP3
		return(CMP3EncoderDlg::Encode(Path, *this, Metadata));	// encode and write
	if (!Format)	// if format not specified
		return(ProgressWrite(Path, Metadata));	// do native write
	CProgressDlg	ProgDlg;
	IO_HOOK	Hook = {IOCallback, IDS_DOC_WRITING, LPARAM(&ProgDlg), IO_BLOCK_SIZE};
	TRY {
		CSndFileEx	sf;
		if (!sf.Create())	// create sndfile wrapper
			return(FALSE);
		if (!sf.Write(Path, *this, Format, Hook, Metadata))
			return(FALSE);
	}
	CATCH (CUserException, e) {
		return(FALSE);
	}
	CATCH (CException, e) {
		e->ReportError();
		return(FALSE);
	}
	END_CATCH;
	return(TRUE);
}

CString CWaveEdit::GetSpeakerName(int SpeakerIdx)
{
	ASSERT(SpeakerIdx >= 0 && SpeakerIdx < SPEAKERS);
	return(LDS(m_SpeakerNameID[SpeakerIdx]));
}

void CWaveEdit::GetChannelNames(CStringArray& ChannelName) const
{
	UINT	chans = m_Channels;
	ChannelName.SetSize(chans);	// allocate channel name string array
	UINT	ChanMask = m_ChannelMask;
	if (ChanMask) {	// if speakers are assigned
		int	iSpkr = 0;
		for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
			// find next assigned speaker in channel mask
			while (iSpkr < SPEAKERS && !(ChanMask & (1 << iSpkr)))
				iSpkr++;
			CString	s;
			if (iSpkr < SPEAKERS) {	// if channel has a speaker assignment
				s = GetSpeakerName(iSpkr);	// get speaker name
				iSpkr++;	// increment past speaker
			} else	// more channels than speaker assignments
				s.Format(_T("%d"), iChan);	// use channel index as speaker name
			ChannelName[iChan] = s;
		}
	} else {	// zero channel mask: no speakers assigned
		switch (chans) {
		case 1:	// mono
			ChannelName[0] = LDS(IDS_SPKR_MONO);
			break;
		case 2:	// stereo
			ChannelName[0] = LDS(IDS_SPKR_LEFT);
			ChannelName[1] = LDS(IDS_SPKR_RIGHT);
			break;
		default:
			// use channel indices as speaker names
			for (UINT iChan = 0; iChan < chans; iChan++)
				ChannelName[iChan].Format(_T("%d"), iChan);
		}
	}
}

CString CWaveEdit::AbbreviateChannelName(LPCTSTR Name)
{
	CString	abbr;
	if (isdigit(Name[0])) {	// if name is numeric
		abbr = Name;	// don't abbreviate
	} else {	// name isn't numeric
		CString	s;
		int	iSubStr = 0;
		while (AfxExtractSubString(s, Name, iSubStr, ' ')) {	// for each word
			if (!s.IsEmpty()) {
				TCHAR	c = s[0];
				if (isupper(c))	// if word is capitalized
					abbr += c;	// add word's first character to abbreviation
			}
			iSubStr++;
		}
	}
	return(abbr);
}

CString	CWaveEdit::GetChannelCountString(UINT Channels)
{
	static const int CONFIG_NAME[] = {
		0,	// invalid configuration
		IDS_SPKR_MONO,
		IDS_SPKR_STEREO,
	};
	CString	s;
	if (Channels < _countof(CONFIG_NAME))	// if named configuration
		s.LoadString(CONFIG_NAME[Channels]);	// look up name
	else	// unnamed configuration
		s.Format(IDS_WEDT_FMT_CHANNEL_COUNT, Channels);	// generate name
	return(s);
}

CString	CWaveEdit::GetChannelCountString() const
{
	return(GetChannelCountString(GetChannels()));
}

CString	CWaveEdit::GetSampleBitsString(UINT SampleBits)
{
	CString	s;
	s.Format(IDS_WEDT_FMT_SAMPLE_BITS, SampleBits);
	return(s);
}

CString	CWaveEdit::GetSampleBitsString() const
{
	return(GetSampleBitsString(GetSampleBits()));
}

CString	CWaveEdit::GetSampleRateString(UINT SampleRate)
{
	CString	s;
	s.Format(IDS_WEDT_FMT_SAMPLE_RATE, SampleRate);
	return(s);
}

CString	CWaveEdit::GetSampleRateString() const
{
	return(GetSampleRateString(GetSampleRate()));
}

CString CWaveEdit::GetAudioFormatString() const
{
	return(GetSampleBitsString() 
		+ ' ' + GetChannelCountString() 
		+ ' ' + GetSampleRateString());
}

void CWaveEdit::MatchFormat(const CWaveEdit& Wave, UINT Flags) const
{
	if (!IsValid())	// if we're invalid, everything matches
		return;
	if (Flags & MF_COMPATIBLE) {	// if format must be compatible
		if (!IsCompatibleFormat(Wave)) {	// if mismatch
			CString	msg;
			msg.Format(IDS_WEDT_FORMAT_MISMATCH,
				Wave.GetSampleBitsString(), Wave.GetChannelCountString(),
				GetSampleBitsString(), GetChannelCountString());
			AfxMessageBox(msg);
			AfxThrowUserException();
		}
	}
	if (Flags & MF_SAMPLE_SIZE) {	// if sample size must match
		if (Wave.GetSampleSize() != GetSampleSize()) {	// if mismatch
			CString	msg;
			msg.Format(IDS_WEDT_SAMPLE_SIZE_MISMATCH,
				Wave.GetSampleBitsString(), GetSampleBitsString());
			AfxMessageBox(msg);
			AfxThrowUserException();
		}
	}
	if (Flags & MF_SAMPLE_RATE) {	// if sample rate must match
		if (Wave.GetSampleRate() != GetSampleRate()) {	// if mismatch
			CString	msg;
			msg.Format(IDS_WEDT_SAMPLE_RATE_MISMATCH, 
				Wave.GetSampleRateString(), GetSampleRateString());
			if (AfxMessageBox(msg, MB_OKCANCEL) != IDOK)	// warning only
				AfxThrowUserException();
		}
	}
}
