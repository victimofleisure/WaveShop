// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
		01		13mar13	add channel mask to ctor
		02		31mar13	assert on negative offset
		03		24apr13	in Write, fix incorrect file size calculation
		04		30apr13	add RF64 header write
		05		01aug13	add metadata

		wave container
 
*/

#include "stdafx.h"
#include "Wave.h"
#include "resource.h"	// for error message strings
#include "aviriff.h"	// for RIFFLIST and RIFFCHUNK
#include "atlconv.h"	// for ATL string conversion macros

#define WAVE_FOURCC_WAVE	FCC('WAVE')
#define WAVE_FOURCC_FORMAT	FCC('fmt ')
#define WAVE_FOURCC_DATA	FCC('data')
#define WAVE_FOURCC_LIST	FCC('LIST')
#define WAVE_FOURCC_INFO	FCC('INFO')

#define METADATASTR(sndfile_str, id3v2_tag, wav_tag) FCC(wav_tag),
const FOURCC CWave::m_MetadataTag[] = {
	#include "MetadataStr.h"	// generate metadata string tags
};

#pragma pack(push, 2)
struct DATASIZE64_CHUNK : RIFFCHUNK {
	ULARGE_INTEGER	RIFFSize;		// size of file in bytes, minus 8
	ULARGE_INTEGER	DataSize;		// size of audio data, in bytes
	ULARGE_INTEGER	SampleCount;	// number of frames of audio
	UINT	TableLength;			// number of entries in table
};
#pragma pack(pop)

CWave::CWave()
{
	Init();
}

CWave::~CWave()
{
}

CWave::CWave(UINT Channels, UINT SampleRate, UINT SampleBits, UINT ChannelMask)
{
	Init();
	SetFormat(Channels, SampleRate, SampleBits, ChannelMask);
}

void CWave::Init()
{
	m_Channels = 0;
	m_SampleRate = 0;
	m_SampleBits = 0;
	m_ChannelMask = 0;
	m_SampleSize = 0;
	m_FrameSize = 0;
	m_FrameCount = 0;
	if (GetDataSize())
		m_Data.RemoveAll();
}

void CWave::Copy(const CWave& Src)
{
	m_Channels		= Src.m_Channels;
	m_SampleRate	= Src.m_SampleRate;
	m_SampleBits	= Src.m_SampleBits;
	m_SampleSize	= Src.m_SampleSize;
	m_ChannelMask	= Src.m_ChannelMask;
	m_FrameSize		= Src.m_FrameSize;
	m_FrameCount	= Src.m_FrameCount;
	m_Data.Copy(Src.m_Data);
}

void CWave::GetFormat(WAVEFORMATEX& WaveFmt) const
{
	WaveFmt.wFormatTag		= WAVE_FORMAT_PCM;
	WaveFmt.nChannels		= WORD(m_Channels);
	WaveFmt.nSamplesPerSec	= m_SampleRate;
	WaveFmt.nAvgBytesPerSec	= GetBytesPerSec();
	WaveFmt.nBlockAlign		= WORD(m_FrameSize);
	WaveFmt.wBitsPerSample	= WORD(m_SampleBits);
	WaveFmt.cbSize			= 0;
}

void CWave::GetFormat(WAVEFORMATEXTENSIBLE& WaveFmtExt) const
{
	WAVEFORMATEX&	WaveFmt = WaveFmtExt.Format;
	WaveFmt.wFormatTag		= WAVE_FORMAT_EXTENSIBLE;
	WaveFmt.nChannels		= WORD(m_Channels);
	WaveFmt.nSamplesPerSec	= m_SampleRate;
	WaveFmt.nAvgBytesPerSec	= GetBytesPerSec();
	WaveFmt.nBlockAlign		= WORD(m_FrameSize);
	WaveFmt.wBitsPerSample	= WORD(m_SampleSize << 3);	// container size
	WaveFmt.cbSize			= sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
	WaveFmtExt.Samples.wValidBitsPerSample	= WORD(m_SampleBits);	// valid bits
	WaveFmtExt.dwChannelMask	= m_ChannelMask;
	WaveFmtExt.SubFormat		= KSDATAFORMAT_SUBTYPE_PCM;
}

void CWave::SetFormat(UINT Channels, UINT SampleRate, UINT SampleBits, UINT ChannelMask)
{
	ASSERT(Channels > 0);
	ASSERT(SampleRate > 0);
	ASSERT(SampleBits > 0);
	m_Channels = Channels;
	m_SampleRate = SampleRate;
	m_SampleBits = SampleBits;
	m_ChannelMask = ChannelMask;
	OnFormatChange();
}

void CWave::OnFormatChange()
{
	m_SampleSize = ((m_SampleBits - 1) / 8) + 1;
	ASSERT(m_SampleSize <= sizeof(SAMPLE));	// must fit within SAMPLE type
	m_FrameSize = m_SampleSize * m_Channels;
	m_Data.SetSize(m_FrameCount * m_FrameSize);
}

void CWave::SetChannels(UINT Channels)
{
	ASSERT(Channels > 0);
	m_Channels = Channels;
	OnFormatChange();
}

void CWave::SetSampleRate(UINT SampleRate)
{
	ASSERT(SampleRate > 0);
	m_SampleRate = SampleRate;
}

void CWave::SetSampleBits(UINT SampleBits)
{
	ASSERT(SampleBits > 0);
	m_SampleBits = SampleBits;
	OnFormatChange();
}

void CWave::GetSampleRails(SAMPLE& NegRail, SAMPLE& PosRail) const
{
	ASSERT(m_SampleSize > 0);
	PosRail = (1 << ((m_SampleSize << 3) - 1)) - 1;
	NegRail = -PosRail - 1;
}

void CWave::SetFrameCount(W64INT FrameCount)
{
	m_Data.SetSize(FrameCount * m_FrameSize);
	m_FrameCount = FrameCount;
}

void CWave::Empty()
{
	m_Data.RemoveAll();
	m_FrameCount = 0;
}

W64INT CWave::GetByteOffset(UINT ChannelIdx, W64INT FrameIdx) const
{
	return(FrameIdx * m_FrameSize + ChannelIdx * m_SampleSize);
}

CWave::SAMPLE CWave::GetSampleAt(W64INT ByteOffset) const
{
	ASSERT(ByteOffset >= 0 && ByteOffset + int(m_SampleSize) <= GetDataSize());
	const BYTE	*pData = m_Data.GetData() + ByteOffset;
	switch (m_SampleSize) {
	case 1:	// 8-bit
		return(*pData - 0x80);
	case 2:	// 16-bit
		return(*((short *)pData));
	case 3:	// 24-bit
		{
			int	samp;
			memcpy(&samp, pData, 3);
			((short *)&samp)[1] = ((char *)&samp)[2];	// extend sign into high byte
			return(samp);
		}
	case 4:	// 32-bit
		return(*((int *)pData));
	default:
		SAMPLE	samp = 0;
		memcpy(&samp, pData, m_SampleSize);
		if (((BYTE *)&samp)[m_SampleSize - 1] & 0x80)	// if negative value, extend sign
			memset(((BYTE *)&samp) + m_SampleSize, 0xff, sizeof(SAMPLE) - m_SampleSize);
		return(samp);
	}
}

void CWave::SetSampleAt(W64INT ByteOffset, SAMPLE Value)
{
	ASSERT(ByteOffset >= 0 && ByteOffset + int(m_SampleSize) <= GetDataSize());
	BYTE	*pData = m_Data.GetData() + ByteOffset;
	switch (m_SampleSize) {
	case 1:	// 8-bit
		*pData = BYTE(Value + 0x80);
		break;
	case 2:	// 16-bit
		*((short *)pData) = short(Value);
		break;
	case 3:	// 24-bit
		memcpy(pData, &Value, 3);
		break;
	case 4:	// 32-bit
		*((int *)pData) = int(Value);
		break;
	default:
		memcpy(pData, &Value, m_SampleSize);
	}
}

CWave::SAMPLE CWave::GetSample(UINT ChannelIdx, W64INT FrameIdx) const
{
	W64INT	ByteOffset = GetByteOffset(ChannelIdx, FrameIdx);
	return(GetSampleAt(ByteOffset));
}

void CWave::SetSample(UINT ChannelIdx, W64INT FrameIdx, SAMPLE Value)
{
	W64INT	ByteOffset = GetByteOffset(ChannelIdx, FrameIdx);
	SetSampleAt(ByteOffset, Value);
}

void CWave::ThrowError(int ErrorID)
{
	THROW(new CWaveFileException(ErrorID));
}

inline void CWave::Read(CFile& fp, void *lpBuf, UINT nCount)
{
	if (fp.Read(lpBuf, nCount) != nCount)	// if partial read
		ThrowError(IDS_WAVE_ERR_END_OF_FILE);
}

void CWave::Read(LPCTSTR Path, const IO_HOOK *Hook, CStringArray *Metadata)
{
	enum {
		MIN_BIT_COUNT = 8,
		MAX_BIT_COUNT = sizeof(SAMPLE) * 8,
	};
	CFile	fp(Path, CFile::modeRead | CFile::shareDenyWrite);
	RIFFLIST	list;
	fp.Read(&list, sizeof(list));
	if (list.fcc != FOURCC_RIFF)
		ThrowError(IDS_WAVE_ERR_NOT_RIFF);
	if (list.fccListType != WAVE_FOURCC_WAVE)
		ThrowError(IDS_WAVE_ERR_NOT_WAVE);
	bool	bGotData = FALSE;
	WAVEFORMATEXTENSIBLE	WaveFmtExt;
	WAVEFORMATEX&	WaveFmt = WaveFmtExt.Format;
	ZeroMemory(&WaveFmtExt, sizeof(WaveFmtExt));
	RIFFCHUNK	chunk;
	while (fp.Read(&chunk, sizeof(chunk)) == sizeof(chunk)) {	// read chunk header
		switch (chunk.fcc) {
		case WAVE_FOURCC_FORMAT:	// format chunk
			if (chunk.cb > sizeof(WAVEFORMATEXTENSIBLE))	// if unexpected chunk size
				ThrowError(IDS_WAVE_ERR_BAD_FORMAT_SIZE);
			Read(fp, &WaveFmt, chunk.cb);	// read format
			if (WaveFmt.wFormatTag != WAVE_FORMAT_PCM	// if format isn't PCM
			&& !(WaveFmt.wFormatTag == WAVE_FORMAT_EXTENSIBLE	// or extensible PCM
			&& WaveFmtExt.SubFormat == KSDATAFORMAT_SUBTYPE_PCM))
				ThrowError(IDS_WAVE_ERR_UNKNOWN_FORMAT);
			if (!WaveFmt.nChannels)	// if channel count is zero
				ThrowError(IDS_WAVE_ERR_BAD_CHANNEL_COUNT);
			if (!WaveFmt.nSamplesPerSec)	// if sample rate is zero
				ThrowError(IDS_WAVE_ERR_BAD_SAMPLE_RATE);
			// if bit count is out of range
			if (WaveFmt.wBitsPerSample < MIN_BIT_COUNT 
			|| WaveFmt.wBitsPerSample > MAX_BIT_COUNT)
				ThrowError(IDS_WAVE_ERR_BAD_BIT_COUNT);
			break;
		case WAVE_FOURCC_DATA:	// data chunk
			if (!WaveFmt.wFormatTag)	// if format header wasn't read
				ThrowError(IDS_WAVE_ERR_NO_FORMAT_CHUNK);
			m_FrameCount = 0;	// so SetFormat deletes any preexisting data
			// set wave format; calls OnFormatChange to set frame size
			SetFormat(WaveFmt.nChannels, WaveFmt.nSamplesPerSec, 
				WaveFmt.wBitsPerSample, WaveFmtExt.dwChannelMask);
			ASSERT(m_FrameSize);	// frame size should be non-zero, else logic error
			if (WaveFmt.nBlockAlign != m_FrameSize)	// if unexpected block align
				ThrowError(IDS_WAVE_ERR_BAD_BLOCK_ALIGN);
			m_Data.SetSize(chunk.cb);	// allocate sample data buffer
			ReadData(fp, GetData(), chunk.cb, Hook);	// read sample data
			m_FrameCount = chunk.cb / m_FrameSize;	// set frame count last
			bGotData = TRUE;	// success
			break;
		case WAVE_FOURCC_LIST:	// list chunk
			{
				FOURCC	ListType;
				Read(fp, &ListType, sizeof(ListType));	// read list type
				// list chunk data size includes list type, complicating things
				UINT	ListSize = chunk.cb - sizeof(ListType);	// exclude list type
				// if list chunk is info list, and caller wants metadata
				if (ListType == WAVE_FOURCC_INFO && Metadata != NULL)
					ReadMetadata(fp, ListSize, Metadata);	// read metadata
				else	// not info list, or info not wanted
					fp.Seek(ListSize, CFile::current);	// skip list chunk
			}
			break;
		default:	// unknown chunk
			fp.Seek(chunk.cb, CFile::current);	// skip chunk
		}
		if (chunk.cb & 1)	// if odd chunk size
			fp.Seek(1, CFile::current);	// chunks are word-aligned, so skip pad byte
	}
	if (!bGotData)	// if data chunk wasn't read
		ThrowError(IDS_WAVE_ERR_NO_DATA_CHUNK);
}

void CWave::Write(LPCTSTR Path, const IO_HOOK *Hook, const CStringArray *Metadata) const
{
	CFile	fp(Path, CFile::modeCreate | CFile::modeWrite);
	UINT	FormatSize;	// format size is variable
	WAVEFORMATEXTENSIBLE	WaveFmtExt;
	// if speakers were assigned, or sample bits don't fill container
	if (m_ChannelMask || (m_SampleBits & 7)) {
		GetFormat(WaveFmtExt);	// use extensible format
		FormatSize = sizeof(WAVEFORMATEXTENSIBLE);	
	} else {	// no speaker assignments and standard sample size
		GetFormat(WaveFmtExt.Format);	// use standard format
		FormatSize = sizeof(WAVEFORMATEX);
	}
	// write file header, format chunk, optional metadata chunk, and data chunk
	UINT	MetadataSize = GetMetadataSize(Metadata);	// compute metadata size
	UINT	HdrSize = 4 + sizeof(RIFFCHUNK) * 2 + FormatSize + MetadataSize;
	RIFFLIST	FileHdr;
	FileHdr.fcc = FOURCC_RIFF;
	FileHdr.cb = W64UINT_CAST32(GetDataSize() + HdrSize);	// potential 32-bit wrap
	FileHdr.fccListType = WAVE_FOURCC_WAVE;
	fp.Write(&FileHdr, sizeof(FileHdr));
	RIFFCHUNK	FmtHdr;
	FmtHdr.fcc = WAVE_FOURCC_FORMAT;
	FmtHdr.cb = FormatSize;
	fp.Write(&FmtHdr, sizeof(FmtHdr));
	fp.Write(&WaveFmtExt, FormatSize);
	if (MetadataSize)	// if metadata chunk needed
		WriteMetadata(fp, Metadata);	// write metadata
	RIFFCHUNK	DataHdr;
	DataHdr.fcc = WAVE_FOURCC_DATA;
	DataHdr.cb = W64UINT_CAST32(GetDataSize());	// potential 32-bit wrap
	fp.Write(&DataHdr, sizeof(DataHdr));
	WriteData(fp, GetData(), DataHdr.cb, Hook);	// write sample data
	if (DataHdr.cb & 1) {	// if odd data chunk size
		BYTE	pad = 0;
		fp.Write(&pad, 1);	// chunks must be word-aligned, so add pad byte
	}
}

void CWave::ReadData(CFile& fp, BYTE *pData, W64UINT Length, const IO_HOOK *Hook)
{
	if (Hook != NULL) {	// if callbacks requested
		if (Length) {	// if at least one block
			UINT	blocks = W64UINT_CAST32((Length - 1) / Hook->BlockSize + 1);
			for (UINT iBlock = 0; iBlock < blocks; iBlock++) {
				if (!Hook->Callback(iBlock, blocks, Hook->wParam, Hook->lParam))
					AfxThrowUserException();
				UINT	BlockSize = static_cast<UINT>(min(Length, Hook->BlockSize));
				Read(fp, pData, BlockSize);	// read sample frames
				pData += BlockSize;
				Length -= BlockSize;
			}
		}
	} else	// no callbacks
		Read(fp, pData, W64UINT_CAST32(Length));	// read sample frames
}

void CWave::WriteData(CFile& fp, const BYTE *pData, W64UINT Length, const IO_HOOK *Hook) const
{
	if (Hook != NULL) {	// if callbacks requested
		if (Length) {	// if at least one block
			UINT	blocks = W64UINT_CAST32((Length - 1) / Hook->BlockSize + 1);
			for (UINT iBlock = 0; iBlock < blocks; iBlock++) {
				if (!Hook->Callback(iBlock, blocks, Hook->wParam, Hook->lParam))
					AfxThrowUserException();
				UINT	BlockSize = static_cast<UINT>(min(Length, Hook->BlockSize));
				fp.Write(pData, BlockSize);	// write sample frames
				pData += BlockSize;
				Length -= BlockSize;
			}
		}
	} else	// no callbacks
		fp.Write(pData, W64UINT_CAST32(Length));	// write sample frames
}

inline void CWave::ReadMetadata(CFile& fp, UINT ListSize, CStringArray *Metadata)
{
	ASSERT(Metadata != NULL);
	int	nStrs = _countof(m_MetadataTag);
	Metadata->SetSize(nStrs);	// allocate metadata string array
	UINT	BytesRead = 0;
	int	nStrsFound = 0;
	// ListSize should be the sum of the sizes of the list's info subchunks,
	// including any pad bytes they may have, but excluding the list's type
	while (BytesRead < ListSize) {	// while within list size
		RIFFCHUNK	info;
		Read(fp, &info, sizeof(info));	// read info subchunk
		CByteArray	buf;
		buf.SetSize(info.cb);	// allocate char buffer for info string
		Read(fp, buf.GetData(), info.cb);	// read info string into buffer
		// if string is non-empty and null terminated, as it should be
		if (info.cb && !buf[info.cb - 1]) {
			for (int iStr = 0; iStr < nStrs; iStr++) {	// for each metadata string
				if (info.fcc == m_MetadataTag[iStr]) {	// if tag matches
					// copy string to metadata array, converting to Unicode if needed
					(*Metadata)[iStr] = CString((LPCSTR)buf.GetData());
					nStrsFound++;	// increment string found count
					break;	// success: stop iterating
				}
			}
		}
		BytesRead += sizeof(info) + info.cb;	// increment bytes read
		if (info.cb & 1) {	// if odd subchunk size
			fp.Seek(1, CFile::current);	// skip pad byte
			BytesRead++;	// increment bytes read to account for padding
		}
	}
	if (!nStrsFound)	// if no metadata strings were found
		Metadata->RemoveAll();	// delete metadata array
}

inline void CWave::WriteMetadata(CFile& fp, const CStringArray *Metadata) const
{
	ASSERT(Metadata != NULL);
	USES_CONVERSION;	// for ATL string conversion macros
	int	nStrs = _countof(m_MetadataTag);
	RIFFLIST	list = {WAVE_FOURCC_LIST, 0, WAVE_FOURCC_INFO};
	fp.Write(&list, sizeof(list));	// write list header with dummy data size
	int	ListSize = 0;
	for (int iStr = 0; iStr < nStrs; iStr++) {	// for each metadata string
		RIFFCHUNK	info;
		info.fcc = m_MetadataTag[iStr];	// set info subchunk tag value
		if (info.fcc) {	// if metadata string has a corresponding tag value
			const CString&	str = (*Metadata)[iStr];	// dereference string
			if (!str.IsEmpty()) {	// if metadata string isn't empty
				// chunk data size does NOT include pad byte; see Multimedia
				// Programming Interface and Data Specifications 1.0, page 11
				info.cb = str.GetLength() + 1;	// set info subchunk data size,
												// including null terminator
				fp.Write(&info, sizeof(info));	// write info subchunk header
				fp.Write(T2CA(str), info.cb);	// write info subchunk data
				ListSize += sizeof(info) + info.cb;	// increment list size
				if (info.cb & 1) {	// if info data size is odd
					BYTE	pad = 0;
					fp.Write(&pad, 1);	// add pad byte
					ListSize++;	// increment list size to account for padding
				}
			}
		}
	}
	ListSize += sizeof(FOURCC);	// include list type in list chunk data size
	// list header was previously written above, but with dummy data size
	// of zero; calculate byte offset of list data size within output file,
	// relative to current file position, which is assumed to be end of file
	LONG	offset = ListSize + sizeof(DWORD);
	fp.Seek(-offset, CFile::current);	// seek backward to list data size
	fp.Write(&ListSize, sizeof(DWORD));	// update list data size
	fp.SeekToEnd();	// seek forward to end of file
}

inline UINT CWave::GetMetadataSize(const CStringArray *Metadata) const
{
	if (Metadata == NULL || !Metadata->GetSize())	// if no metadata
		return(0);	// don't write list chunk
	UINT	ChunkSize = sizeof(RIFFLIST);	// include list header
	int	nStrs = _countof(m_MetadataTag);
	for (int iStr = 0; iStr < nStrs; iStr++) {	// for each metadata string
		if (m_MetadataTag[iStr]) {	// if metadata string has a tag value
			const CString&	str = (*Metadata)[iStr];	// dereference string
			if (!str.IsEmpty()) {	// if metadata string isn't empty
				UINT	len = str.GetLength() + 1;	// compute data size
				if (len & 1)	// if data size is odd
					len++;	// add pad byte
				ChunkSize += sizeof(RIFFCHUNK) + len;	// add data size to total
			}
		}
	}
	return(ChunkSize);	// total size of list chunk, including its header
}

bool CWave::SafeRead(LPCTSTR Path, const IO_HOOK *Hook, CStringArray *Metadata)
{
	TRY {
		Read(Path, Hook, Metadata);
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

bool CWave::SafeWrite(LPCTSTR Path, const IO_HOOK *Hook, const CStringArray *Metadata) const
{
	TRY {
		Write(Path, Hook, Metadata);
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

void CWave::WriteRF64Header(CFile& fp, ULONGLONG FrameCount)
{
	ASSERT(IsValid());
	ULONGLONG	DataSize = m_FrameSize * FrameCount; 
	ULONGLONG	FileLen = sizeof(RIFFLIST) + sizeof(DATASIZE64_CHUNK) 
		+ sizeof(RIFFCHUNK) * 2 + sizeof(WAVEFORMATEXTENSIBLE) + DataSize;
	RIFFLIST	FileHdr;
	FileHdr.fcc = MAKEFOURCC('R', 'F', '6', '4');
	FileHdr.cb = ULONG_MAX;
	FileHdr.fccListType = WAVE_FOURCC_WAVE;
	fp.Write(&FileHdr, sizeof(FileHdr));
	DATASIZE64_CHUNK	DataSize64;
	DataSize64.fcc = MAKEFOURCC('d', 's', '6', '4');
	DataSize64.cb = sizeof(DATASIZE64_CHUNK) - sizeof(RIFFCHUNK);
	DataSize64.RIFFSize.QuadPart = FileLen - 8;
	DataSize64.DataSize.QuadPart = DataSize;
	DataSize64.SampleCount.QuadPart = FrameCount;
	DataSize64.TableLength = 0;
	fp.Write(&DataSize64, sizeof(DataSize64));
	RIFFCHUNK	ChunkHdr;
	ChunkHdr.fcc = WAVE_FOURCC_FORMAT;
	ChunkHdr.cb = sizeof(WAVEFORMATEXTENSIBLE);
	fp.Write(&ChunkHdr, sizeof(ChunkHdr));
	WAVEFORMATEXTENSIBLE	WaveFmtExt;
	GetFormat(WaveFmtExt);
	fp.Write(&WaveFmtExt, sizeof(WaveFmtExt));
	ChunkHdr.fcc = WAVE_FOURCC_DATA;
	ChunkHdr.cb = ULONG_MAX;
	fp.Write(&ChunkHdr, sizeof(ChunkHdr));
}

void CWave::WriteRF64CompatibleHeader(CFile& fp, ULONGLONG FrameCount)
{
	ASSERT(IsValid());
	ULONGLONG	DataSize = m_FrameSize * FrameCount; 
	ULONGLONG	FileLen = sizeof(RIFFLIST) + sizeof(DATASIZE64_CHUNK) 
		+ sizeof(RIFFCHUNK) * 2 + sizeof(WAVEFORMATEXTENSIBLE) + DataSize;
	if (FileLen >= UINT_MAX) {	// if file is too big for WAV format
		WriteRF64Header(fp, FrameCount);	// write RF64 header instead
		return;
	}
	RIFFLIST	FileHdr;
	FileHdr.fcc = FOURCC_RIFF;
	FileHdr.cb = UINT(FileLen) - 8;
	FileHdr.fccListType = WAVE_FOURCC_WAVE;
	fp.Write(&FileHdr, sizeof(FileHdr));
	DATASIZE64_CHUNK	DataSize64;
	ZeroMemory(&DataSize64, sizeof(DataSize64) );
	DataSize64.fcc = MAKEFOURCC('J', 'U', 'N', 'K');
	DataSize64.cb = sizeof(DATASIZE64_CHUNK) - sizeof(RIFFCHUNK);
	fp.Write(&DataSize64, sizeof(DataSize64));
	RIFFCHUNK	ChunkHdr;
	ChunkHdr.fcc = WAVE_FOURCC_FORMAT;
	ChunkHdr.cb = sizeof(WAVEFORMATEXTENSIBLE);
	fp.Write(&ChunkHdr, sizeof(ChunkHdr));
	WAVEFORMATEXTENSIBLE	WaveFmtExt;
	GetFormat(WaveFmtExt);
	fp.Write(&WaveFmtExt, sizeof(WaveFmtExt));
	ChunkHdr.fcc = WAVE_FOURCC_DATA;
	ChunkHdr.cb = UINT(DataSize);
	fp.Write(&ChunkHdr, sizeof(ChunkHdr));
}

IMPLEMENT_DYNAMIC(CWaveFileException, CFileException);

CWaveFileException::CWaveFileException(UINT ResID)
{
	m_ResID = ResID;
}

BOOL CWaveFileException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext)
{
	return(LoadString(AfxGetApp()->m_hInstance, m_ResID, lpszError, nMaxError));
}
