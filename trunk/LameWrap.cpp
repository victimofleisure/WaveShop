// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10apr13	initial version
        01      03aug13	add metadata

		wrapper for libmp3lame MPEG audio encoder DLL
 
*/

#include "stdafx.h"
#include "WaveShop.h"
#include "LameWrap.h"
#include "PathStr.h"
#include "WaveProcess.h"
#include "ProgressDlg.h"
#include "atlconv.h"	// for ATL string conversion macros
#include "MetadataDlg.h"	// for metadata string enum

#define LAME_FAILED(x) ((x) < 0)

#define LAME_DEF(name, ordinal) ordinal,
const int CLameWrap::m_FuncOrd[] = {
	#include "lameDefs.h"
};

const int CLameWrap::m_LameEncodeErr[] = {	// order must match lame.h
	IDS_LAME_ERR_ENC_BUFFER_SIZE,	// -1: output buffer too small
	IDS_LAME_ERR_ENC_MALLOC,		// -2: malloc problem
	IDS_LAME_ERR_ENC_INIT_PARAMS,	// -3: lame_init_params not called
	IDS_LAME_ERR_ENC_PSYCHO,		// -4: psychoacoustic problems
};

CLameWrap::CLameWrap()
{
	m_lame = NULL;
	m_gfp = NULL;
}

CLameWrap::~CLameWrap()
{
	if (m_gfp != NULL)	// if encoder exists
		m_lame->lame_close(m_gfp);	// destroy it
}

bool CLameWrap::Create(LPCTSTR LibPath)
{
	ASSERT(m_lame == NULL);	// reuse of instance not supported
	int	funcs = _countof(m_FuncOrd);
	if (!CWaveShopApp::GetDLLFunctions(m_Lib, LibPath, m_FuncOrd, funcs, m_FuncPtr))
		return(FALSE);
	m_lame = (ILame *)m_FuncPtr.GetData();	// cast function pointer array to interface
	return(TRUE);
}

void CLameWrap::OnEncodingError(int RetVal)
{
	int	ErrID, iErr = -RetVal - 1;	// convert lame return value to error index
	if (iErr >= 0 && iErr < _countof(m_LameEncodeErr))	// if error index in range
		ErrID = m_LameEncodeErr[iErr];	// look up error
	else	// error index out of range
		ErrID = IDS_LAME_ERR_ENCODE;	// generic error; shouldn't happen
	CWave::ThrowError(ErrID);
}

bool CLameWrap::Write(LPCTSTR Path, const CWave& Wave, const ENCODING_PARAMS& Params, const CStringArray *Metadata)
{
	TRY {
		ASSERT(m_gfp == NULL);	// reuse of instance not allowed
		m_gfp = m_lame->lame_init();	// initialize lame
		if (m_gfp == NULL)	// if init failed
			CWave::ThrowError(IDS_LAME_ERR_INIT);
		if (Metadata != NULL && Metadata->GetSize()) {
			USES_CONVERSION;	// for ATL string conversion macros
			m_lame->id3tag_add_v2(m_gfp);	// add ID3V2 tags
			m_lame->id3tag_set_title(m_gfp,		T2CA((*Metadata)[STR_TITLE]));
			m_lame->id3tag_set_artist(m_gfp,	T2CA((*Metadata)[STR_ARTIST]));
			m_lame->id3tag_set_album(m_gfp,		T2CA((*Metadata)[STR_ALBUM]));
			m_lame->id3tag_set_year(m_gfp,		T2CA((*Metadata)[STR_DATE]));
			m_lame->id3tag_set_comment(m_gfp,	T2CA((*Metadata)[STR_COMMENT]));
			m_lame->id3tag_set_track(m_gfp,		T2CA((*Metadata)[STR_TRACKNUMBER]));
			m_lame->id3tag_set_genre(m_gfp,		T2CA((*Metadata)[STR_GENRE]));
		}
		int	chans = Wave.GetChannels();
		int	frames = W64INT_CAST32(Wave.GetFrameCount());	// 32-bit limit
		// set channel count
		if (LAME_FAILED(m_lame->lame_set_num_channels(m_gfp, chans)))
			CWave::ThrowError(IDS_LAME_ERR_SET_NUM_CHANNELS);
		// set frame count
		if (LAME_FAILED(m_lame->lame_set_num_samples(m_gfp, frames)))
			CWave::ThrowError(IDS_LAME_ERR_SET_NUM_SAMPLES);
		// set sample rate
		if (LAME_FAILED(m_lame->lame_set_in_samplerate(m_gfp, Wave.GetSampleRate())))
			CWave::ThrowError(IDS_LAME_ERR_SET_SAMPLE_RATE);
		// set encoder algorithm quality
		vbr_mode_e	VBRMode = vbr_off;	// avoid warning
		if (LAME_FAILED(m_lame->lame_set_quality(m_gfp, Params.AlgorithmQuality)))
			CWave::ThrowError(IDS_LAME_ERR_SET_ENC_QUALITY);
		switch (Params.BitRateMode) {
		case BRM_CONSTANT:
			// set target constant bit rate
			if (LAME_FAILED(m_lame->lame_set_brate(m_gfp, Params.TargetBitRate)))
				CWave::ThrowError(IDS_LAME_ERR_SET_BIT_RATE);
			VBRMode = vbr_off;	// disable VBR
			break;
		case BRM_AVERAGE:
			// set target average bit rate
			if (LAME_FAILED(m_lame->lame_set_VBR_mean_bitrate_kbps(m_gfp, Params.TargetBitRate)))
				CWave::ThrowError(IDS_LAME_ERR_SET_AVG_BIT_RATE);
			VBRMode = vbr_abr;	// enable ABR
			break;
		case BRM_VARIABLE:
			// for variable bit rate, set target quality
			if (LAME_FAILED(m_lame->lame_set_VBR_q(m_gfp, Params.TargetQuality)))
				CWave::ThrowError(IDS_LAME_ERR_SET_VBR_QUALITY);
			VBRMode = vbr_default;	// enable VBR
			break;
		default:
			NODEFAULTCASE;
		}
		// set VBR mode
		if (LAME_FAILED(m_lame->lame_set_VBR(m_gfp, VBRMode)))
			CWave::ThrowError(IDS_LAME_ERR_SET_VBR_MODE);
		// apply above parameters
		if (LAME_FAILED(m_lame->lame_init_params(m_gfp)))
			CWave::ThrowError(IDS_LAME_ERR_INIT_PARAMS);
		int	BufSize = round(CHUNK_FRAMES * 1.25) + 7200;	// worst case, from lame.h
		UINT	SampleSize = Wave.GetSampleSize();
		UINT	FrameSize = Wave.GetFrameSize();
		CByteArray	ba;
		ba.SetSize(BufSize);	// allocate output buffer
		const BYTE	*pWaveData = Wave.GetData();	// init input pointer
		CWaveProcess::CDblArray	NormSamp;
		CWaveProcess::CConvert	cvt;
		if (SampleSize != 2) {	// if not 16-bit, convert to normalized double
			NormSamp.SetSize(CHUNK_FRAMES * chans);	// allocate conversion buffer
			cvt.Create(Wave);	// init converter; used for normalizing
		}
		CFile	fp(Path, CFile::modeCreate | CFile::modeWrite);	// open output file
		CProgressDlg	ProgDlg;
		if (!ProgDlg.Create())	// create progress dialog
			AfxThrowResourceException();
		ProgDlg.SetWindowText(LDS(IDS_DOC_ENCODING));	// set progress caption
		int	chunks = (frames - 1) / CHUNK_FRAMES + 1;	// compute chunk count
		int	RemainFrames = frames;	// init remaining frames to total frames
		int	PrevPct = 0;
		W64INT	offset = 0;
		for (int iChunk = 0; iChunk < chunks; iChunk++) {	// for each chunk
			int	pct = round(double(iChunk) / chunks * 100);
			if (pct != PrevPct) {	// if percentage changed
				ProgDlg.SetPos(pct);	// update progress bar
				PrevPct = pct;
			}
			if (ProgDlg.Canceled())	// if user canceled
				return(FALSE);
			int	ChunkFrames = min(CHUNK_FRAMES, RemainFrames);
			int	BytesOut;
			if (SampleSize == 2) {	// if 16-bit
				// no conversion needed; encode directly from wave data
				if (chans > 1) {	// if stereo
					// use interleaved encoder
					BytesOut = m_lame->lame_encode_buffer_interleaved(
						m_gfp, (short *)pWaveData, ChunkFrames, 
						ba.GetData(), BufSize);
				} else {	// mono
					// use regular encoder with one channel NULL
					BytesOut = m_lame->lame_encode_buffer(
						m_gfp, (short *)pWaveData, NULL, ChunkFrames, 
						ba.GetData(), BufSize);
				}
				pWaveData += ChunkFrames * FrameSize;	// advance input pointer
			} else {	// not 16-bit
				// convert samples to normalized double, storing in conversion buffer
				double	*pNorm = NormSamp.GetData();
				for (int iFrame = 0; iFrame < ChunkFrames; iFrame++) {	// for each frame
					for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
						CWave::SAMPLE	samp = Wave.GetSampleAt(offset);
						*pNorm++ = cvt.SampleToNorm(samp);	// store in buffer
						offset += SampleSize;
					}
				}
				if (chans > 1) {	// if stereo
					BytesOut = m_lame->lame_encode_buffer_interleaved_ieee_double(
						m_gfp, NormSamp.GetData(), ChunkFrames, 
						ba.GetData(), BufSize);
				} else {
					BytesOut = m_lame->lame_encode_buffer_ieee_double(
						m_gfp, NormSamp.GetData(), NULL, ChunkFrames, 
						ba.GetData(), BufSize);
				}
			}
			if (LAME_FAILED(BytesOut))	// if encoding error
				OnEncodingError(BytesOut);
			fp.Write(ba.GetData(), BytesOut);	// write encoded chunk
			RemainFrames -= ChunkFrames;	// decrement remaining frames
		}
		// all input processed; flush any remaining frames out of encoder
		int	BytesOut = m_lame->lame_encode_flush(m_gfp, ba.GetData(), BufSize);
		if (LAME_FAILED(BytesOut))	// if encoding error
			OnEncodingError(BytesOut);
		fp.Write(ba.GetData(), BytesOut);	// write remaining frames if any
	}
	CATCH (CException, e) {
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}
