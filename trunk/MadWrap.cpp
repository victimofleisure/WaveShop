// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27feb13	initial version
        01      08apr13	in OnInput, preserve unused data from previous pass
		02      20apr13	move OpenTempStream to app
		03      20apr13	in Create, add Quality argument

		wrapper for libmad MPEG audio decoder DLL
		adapted from demo code given in minimad.c
 
*/

#include "stdafx.h"
#include "WaveShop.h"
#include "MadWrap.h"
#include "Wave.h"
#include "PathStr.h"

#define MAD_DEF(name, ordinal) ordinal,
const int CMadWrap::m_FuncOrd[] = {
	#include "madDefs.h"
};

CMadWrap::CMadWrap()
{
	m_mad = NULL;
	m_InFileSize = 0;
	m_OutFile = NULL;
	m_Channels = 0;
	m_SampleRate = 0;
	m_SampleSize = 0;
	m_SampleBits = 0;
	m_TotalFrames = 0;
	m_RcvdFrames = 0;
}

CMadWrap::~CMadWrap()
{
	if (m_OutFile != NULL)
		fclose(m_OutFile);
}

bool CMadWrap::Create(int Quality)
{
	ASSERT(m_mad == NULL);	// reuse of instance not supported
	LPCTSTR	LibName = _T("libmad.dll");
	CPathStr	LibPath(theApp.GetAppFolder());
	LibPath.Append(LibName);
	int	funcs = _countof(m_FuncOrd);
	if (!CWaveShopApp::GetDLLFunctions(m_Lib, LibPath, m_FuncOrd, funcs, m_FuncPtr))
		return(FALSE);
	m_mad = (IMad *)m_FuncPtr.GetData();	// cast function pointer array to interface
	ASSERT(Quality >= 0 && Quality < QUALITIES);
	m_SampleSize = Quality + 2;	// convert quality to sample size in bytes
	return(TRUE);
}

enum mad_flow CMadWrap::input(void *data, struct mad_stream *stream)
{
	ASSERT(data != NULL);
	CMadWrap	*pThis = (CMadWrap *)data;
	return(pThis->OnInput(stream));
}

enum mad_flow CMadWrap::output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
	ASSERT(data != NULL);
	CMadWrap	*pThis = (CMadWrap *)data;
	return(pThis->OnOutput(header, pcm));
}

enum mad_flow CMadWrap::error(void *data, struct mad_stream *stream, struct mad_frame *frame)
{
	ASSERT(data != NULL);
	CMadWrap	*pThis = (CMadWrap *)data;
	return(pThis->OnError(stream, frame));
}

enum mad_flow CMadWrap::OnInput(struct mad_stream *stream)
{
	// calculate size of unused data from previous pass
	UINT	unused = UINT64TO32(stream->bufend - stream->next_frame);
	// move unused portion of data to start of buffer
	memmove(const_cast<BYTE *>(stream->buffer), stream->next_frame, unused);
	// read more data into buffer, after unused data from previous pass
	UINT	BytesRead = m_InFile.Read(
		m_ReadBuf.GetData() + unused, READ_BUF_SIZE - unused);
	if (!BytesRead)	// if no more input data available
		return MAD_FLOW_STOP;	// stop decoding
	m_mad->mad_stream_buffer(stream, m_ReadBuf.GetData(), BytesRead + unused);
	return MAD_FLOW_CONTINUE;
}

enum mad_flow CMadWrap::OnOutput(struct mad_header const *header, struct mad_pcm *pcm)
{
	if (!m_RcvdFrames) {
		m_Channels = pcm->channels;
		m_SampleRate = header->samplerate;
		m_SampleBits = m_SampleSize << 3;
		if (header->bitrate) {	// if valid bitrate
			double	OutputSecs = double(m_InFileSize) / header->bitrate * 8;
			m_TotalFrames = round(OutputSecs * header->samplerate);
			if (!m_ProgDlg.Create())	// create progress dialog
				AfxThrowResourceException();
			m_ProgDlg.SetWindowText(LDS(IDS_DOC_READING));
			m_ProgDlg.SetRange(0, m_TotalFrames);
		}
	}
	// iterate over output samples, converting them to integer PCM
	// and writing them to output file stream in interleaved order
	UINT	frames = pcm->length;
	UINT	chans = m_Channels;
	for (UINT iFrame = 0; iFrame < frames; iFrame++) {	// for each frame
		for (UINT iChan = 0; iChan < chans; iChan++) {	// for each channel
			int	samp = scale(pcm->samples[iChan][iFrame]);	// convert to integer PCM
			fwrite(&samp, 1, m_SampleSize, m_OutFile);	// write sample to temp file
		}
	}
	m_RcvdFrames += frames;	// increment count of frames received
	if (m_TotalFrames)	// if size was estimated
		m_ProgDlg.SetPos(m_RcvdFrames);	// update progress
	if (m_ProgDlg.Canceled())	// if user canceled
		return(MAD_FLOW_BREAK);
	return(MAD_FLOW_CONTINUE);
}

enum mad_flow CMadWrap::OnError(struct mad_stream *stream, struct mad_frame *frame)
{
#ifdef _DEBUG
	printf("decoding error 0x%04x (%s) at frame %u\n",
		stream->error, m_mad->mad_stream_errorstr(stream),
		stream->this_frame);
#endif
	if (!MAD_RECOVERABLE(stream->error)) {	// if unrecoverable error
		CString	ErrorMsg(m_mad->mad_stream_errorstr(stream));
		if (ErrorMsg.IsEmpty())
			AfxMessageBox(ErrorMsg);
		return(MAD_FLOW_BREAK);
	}
	return(MAD_FLOW_CONTINUE);
}

inline signed int CMadWrap::scale(mad_fixed_t sample)
{
	// convert sample from fixed-point to integer, as shown in minimad.c
	sample += (1L << (MAD_F_FRACBITS - m_SampleBits));
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;
	return(sample >> (MAD_F_FRACBITS + 1 - m_SampleBits));
}

bool CMadWrap::Read(LPCTSTR Path, CWave& Wave)
{
	ASSERT(m_OutFile == NULL);	// reuse not allowed; one decode per instance
	TRY {
		CFileException	e;
		if (!m_InFile.Open(Path, CFile::modeRead, &e)) {
			e.ReportError();
			return(FALSE);
		}
		CString	TmpPath;
		if (!theApp.GetTempFileName(TmpPath))
			CWave::ThrowError(IDS_WEDT_NO_TEMP_FILE_NAME);
		// computing MPEG output size is tricky and unreliable, so output samples
		// to temporary stream instead of array; it's nearly as fast due to stream
		// buffering, and avoids risk of running out of memory while growing array
		m_OutFile = CWaveShopApp::OpenTempStream(TmpPath);
		if (m_OutFile == NULL)
			CFileException::ThrowOsError((LONG)::GetLastError(), TmpPath);
		m_ReadBuf.SetSize(READ_BUF_SIZE);
		m_InFileSize = m_InFile.GetLength();
		struct mad_decoder	decoder;
		m_mad->mad_decoder_init(&decoder, this, input, 0, 0, output, error, 0);
		int	result = m_mad->mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
		m_mad->mad_decoder_finish(&decoder);
		if (result < 0)	// if unrecoverable error
			return(FALSE);	// error message already displayed
		// success
		rewind(m_OutFile);	// rewind temp file
		Wave.Empty();	// destroy any existing wave before setting format
		Wave.SetFormat(m_Channels, m_SampleRate, m_SampleBits);	// set format
		Wave.SetFrameCount(m_RcvdFrames);	// resize data array
		W64UINT	DataSize = Wave.GetDataSize();
		// read samples from temp file into wave
		if (fread(Wave.GetData(), 1, DataSize, m_OutFile) != DataSize)
			CFileException::ThrowOsError(ERROR_HANDLE_EOF, TmpPath);
	}
	CATCH (CException, e) {
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}
