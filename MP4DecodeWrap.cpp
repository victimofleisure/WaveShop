// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      19apr13	initial version
		01		24may13	in Read, add Unicode conversion

		wrapper for libmp4ad (libfaad2) AAC/MP4 audio decoder DLL
 
*/

#include "stdafx.h"
#include "WaveShop.h"
#include "MP4DecodeWrap.h"
#include "Wave.h"
#include "PathStr.h"

#define MP4_DEF(name, ordinal) ordinal,
const int CMP4DecodeWrap::m_FuncOrd[] = {
	#include "MP4DecodeDefs.h"
};

CMP4DecodeWrap::CMP4DecodeWrap()
{
	m_mp4 = NULL;
	m_InFileSize = 0;
	m_OutFile = NULL;
	m_Channels = 0;
	m_SampleRate = 0;
	m_SampleSize = 0;
	m_RcvdSamples = 0;
	m_Downmix = FALSE;
	m_PctDone = 0;
	m_Quality = 0;
	m_ErrorCount = 0;
}

CMP4DecodeWrap::~CMP4DecodeWrap()
{
	if (m_OutFile != NULL)
		fclose(m_OutFile);
}

bool CMP4DecodeWrap::Create(int Quality, bool Downmix)
{
	ASSERT(m_mp4 == NULL);	// reuse of instance not supported
	LPCTSTR	LibName = _T("libmp4ad.dll");
	CPathStr	LibPath(theApp.GetAppFolder());
	LibPath.Append(LibName);
	int	funcs = _countof(m_FuncOrd);
	if (!CWaveShopApp::GetDLLFunctions(m_Lib, LibPath, m_FuncOrd, funcs, m_FuncPtr))
		return(FALSE);
	m_mp4 = (IMP4 *)m_FuncPtr.GetData();	// cast function pointer array to interface
	ASSERT(Quality >= 0 && Quality < QUALITIES);
	m_SampleSize = Quality + 2;	// convert quality to sample size in bytes
	m_Downmix = Downmix;
	m_Quality = Quality;
	return(TRUE);
}

int CMP4DecodeWrap::OnOutput(void *param, long total_samples, long samples, const NeAACDecFrameInfo *frame_info, const void *sample_buffer)
{
	CMP4DecodeWrap	*This = (CMP4DecodeWrap *)param;
	return(This->OnOutput(total_samples, samples, frame_info, sample_buffer));
}

void CMP4DecodeWrap::OnError(void *param, const char *error_message)
{
	CMP4DecodeWrap	*This = (CMP4DecodeWrap *)param;
	This->OnError(error_message);
}

int CMP4DecodeWrap::OnOutput(long total_samples, long samples, const NeAACDecFrameInfo *frame_info, const void *sample_buffer)
{
	if (m_OutFile == NULL) {
		m_OutFile = CWaveShopApp::OpenTempStream(m_TmpPath);
		if (m_OutFile == NULL)
			CFileException::ThrowOsError((LONG)::GetLastError(), m_TmpPath);
		m_Channels = frame_info->channels;
		m_SampleRate = frame_info->samplerate;
		if (m_SampleSize == 3)
			m_ConvBuf.SetSize(frame_info->samples * 3);
		m_ProgDlg.Create();
		m_ProgDlg.SetWindowText(LDS(IDS_DOC_READING));
	}
	int	PctDone = round(samples / double(total_samples) * 100);
	if (PctDone != m_PctDone) {
		m_ProgDlg.SetPos(PctDone);
		m_PctDone = PctDone;
	}
	if (m_ProgDlg.Canceled())
		return(1);
	const void	*outbuf;
	if (m_ConvBuf.GetSize()) {
		const BYTE	*pIn = (const BYTE *)sample_buffer;
		BYTE	*pOut = m_ConvBuf.GetData();
		UINT	samps = frame_info->samples;
		for (UINT iSamp = 0; iSamp < samps; iSamp++) {
			memcpy(pOut, pIn, 3);
			pIn += 4;
			pOut += 3;
		}
		outbuf = m_ConvBuf.GetData();
	} else
		outbuf = sample_buffer;
	size_t	wrbytes = fwrite(outbuf, m_SampleSize, frame_info->samples, m_OutFile);
	if (wrbytes != frame_info->samples)
		AfxThrowFileException(CFileException::diskFull, _doserrno, m_TmpPath);
	m_RcvdSamples += frame_info->samples;
	return(0);	// non-zero to abort decoding
}

void CMP4DecodeWrap::OnError(const char *error_message)
{
	CString	sErrMsg(error_message);
	if (m_ErrorCount < MAX_REPORT_ERRORS)
		m_ErrorReport += sErrMsg;
	m_LastError = sErrMsg;
	m_ErrorCount++;
}

CString CMP4DecodeWrap::MakeErrorReport() const
{
	CString	sHdr, sErrCnt;
	if (m_ErrorCount > MAX_REPORT_ERRORS) {
		sErrCnt.Format(IDS_MP4AD_EXCESS_ERRORS, MAX_REPORT_ERRORS, m_ErrorCount);
		sHdr = m_LastError + '\n';
	} else
		sErrCnt.Format(IDS_MP4AD_ERRORS, m_ErrorCount);
	return(sHdr + sErrCnt + '\n' + m_ErrorReport);
}

bool CMP4DecodeWrap::Read(LPCTSTR Path, CWave& Wave)
{
	ASSERT(m_OutFile == NULL);	// reuse not allowed; one decode per instance
	TRY {
		if (!theApp.GetTempFileName(m_TmpPath))
			CWave::ThrowError(IDS_WEDT_NO_TEMP_FILE_NAME);
		aac_dec_opt opt;
		memset(&opt, 0, sizeof(opt));
		opt.on_output = OnOutput;
		opt.on_error = OnError;
		opt.output_format = m_Quality + 1;	// convert quality to output format
		opt.down_matrix = m_Downmix;
		opt.param = this;
		m_ErrorCount = 0;
		m_ErrorReport.Empty();
		m_LastError.Empty();
		USES_CONVERSION;
		int	result = m_mp4->aac_decode(T2CA(Path), &opt);
		if (!result || !m_Channels) {	// if unrecoverable error
			if (!m_ProgDlg.Canceled())
				AfxMessageBox(MakeErrorReport());
			return(FALSE);
		}
		if (m_ErrorCount)
			AfxMessageBox(MakeErrorReport());
		// success
		rewind(m_OutFile);	// rewind temp file
		Wave.Empty();	// destroy any existing wave before setting format
		UINT	SampleBits = m_SampleSize << 3;
		Wave.SetFormat(m_Channels, m_SampleRate, SampleBits);	// set format
		Wave.SetFrameCount(m_RcvdSamples / m_Channels);	// resize data array
		W64UINT	DataSize = Wave.GetDataSize();
		// read samples from temp file into wave
		if (fread(Wave.GetData(), 1, DataSize, m_OutFile) != DataSize)
			CFileException::ThrowOsError(ERROR_HANDLE_EOF, m_TmpPath);
	}
	CATCH (CException, e) {
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}
