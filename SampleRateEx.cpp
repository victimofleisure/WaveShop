// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25feb13	initial version
		01		13mar13	in Resample, copy channel mask
		
		wrapper for Erik de Castro Lopo's samplerate library
 
*/

#include "stdafx.h"
#include "WaveShop.h"
#include "SampleRateEx.h"
#include "Wave.h"
#include "ArrayEx.h"
#include "SndFileEx.h"	// for Convert
#include "PathStr.h"
#include "ProgressDlg.h"

#define SAMPLERATE_DEF(name, ordinal) ordinal,
const int CSampleRateEx::m_FuncOrd[] = {
	#include "samplerateDefs.h"
};

CSampleRateEx::CSampleRateEx()
{
	m_State = NULL;
	m_LastError = 0;
}

CSampleRateEx::~CSampleRateEx()
{
	Destroy();
}

inline bool CSampleRateEx::IsCreated() const
{
	return(m_State != NULL);
}

CString CSampleRateEx::GetError() const
{
	return(CString(m_src->src_strerror(m_LastError)));
}

bool CSampleRateEx::Create(int Quality, int Channels)
{
	Destroy();
	CPathStr	LibPath(theApp.GetAppFolder());
	LibPath.Append(_T("libsamplerate-0.dll"));
	int	funcs = _countof(m_FuncOrd);
	if (!CWaveShopApp::GetDLLFunctions(m_Lib, LibPath, m_FuncOrd, funcs, m_FuncPtr))
		return(FALSE);
	m_src = (ISampleRate *)m_FuncPtr.GetData();	// cast function pointer array to interface
	m_State = m_src->src_new(Quality, Channels, &m_LastError);
	return(IsCreated());
}

void CSampleRateEx::Destroy()
{
	if (IsCreated())
		m_src->src_delete(m_State);
}

bool CSampleRateEx::Resample(const CWave& SrcWave, CWave& DstWave, int NewSampleRate)
{
	ASSERT(IsCreated());
	UINT	chans = SrcWave.GetChannels();
	UINT	SampleSize = SrcWave.GetSampleSize();
	UINT	FrameSize = SrcWave.GetFrameSize();
	W64INT	SrcFrames = SrcWave.GetFrameCount();
	DstWave.SetFormat(chans, NewSampleRate, SrcWave.GetSampleBits(), 
		SrcWave.GetChannelMask());	// copy channel mask
	double	SrcRatio = double(NewSampleRate) / SrcWave.GetSampleRate();
	W64INT	DstFrames = roundW64INT(SrcFrames * SrcRatio);
	DstWave.SetFrameCount(DstFrames);
	int	BufFrames = BUF_SIZE / FrameSize;
	int	BufSamps = BufFrames * chans;
	CArrayEx<float, float&>	SrcFloat, DstFloat;
	SrcFloat.SetSize(BufSamps);
	DstFloat.SetSize(BufSamps);
	const BYTE	*pSrc = SrcWave.GetData();
	BYTE	*pDst = DstWave.GetData();
	W64INT	RemainSrcFrames = SrcFrames;
	W64INT	RemainDstFrames = DstFrames;
	CWave	tmp;	// temporary buffer for conversion
	if (SampleSize == 1 || SampleSize == 3) {	// if wave is 8 or 24 bit PCM
		// libsamplerate doesn't support operations on 8 or 24 bit PCM;
		// must read shorts or ints to a temporary buffer and convert
		UINT	TmpSampleBits;
		if (SampleSize == 1)	// if 8-bit
			TmpSampleBits = 16;	// read shorts
		else	// 24-bit
			TmpSampleBits = 32;	// read ints
		tmp.SetFormat(chans, SrcWave.GetSampleRate(), TmpSampleBits);
		tmp.SetFrameCount(BufFrames);
	}
	CProgressDlg	ProgDlg;
	if (!ProgDlg.Create())	// create progress dialog
		AfxThrowResourceException();
	ProgDlg.SetWindowText(LDS(IDS_WPRO_RESAMPLE));
	SRC_DATA	sd;
	do {
		int	SrcChunkFrames = W64INT_CAST32(min(RemainSrcFrames, BufFrames));
		int	SrcChunkSamps = SrcChunkFrames * chans;
		// convert input frames from source wave into float buffer
		switch (SampleSize) {
		case 1:	// 8-bit: convert 8-bit to short, then convert short to float
			CSndFileEx::Convert(SrcWave, tmp, pSrc - SrcWave.GetData(), 0, SrcChunkFrames);
			m_src->src_short_to_float_array(
				(short *)tmp.GetData(), SrcFloat.GetData(), SrcChunkSamps);
			break;
		case 2:	// 16-bit: convert short to float
			m_src->src_short_to_float_array(
				(short *)pSrc, SrcFloat.GetData(), SrcChunkSamps);
			break;
		case 3:	// 24-bit: convert 24-bit to int, then convert int to float
			CSndFileEx::Convert(SrcWave, tmp, pSrc - SrcWave.GetData(), 0, SrcChunkFrames);
			m_src->src_int_to_float_array(
				(int *)tmp.GetData(), SrcFloat.GetData(), SrcChunkSamps);
			break;
		case 4:	// 32-bit: convert int to float
			m_src->src_int_to_float_array(
				(int *)pSrc, SrcFloat.GetData(), SrcChunkSamps);
			break;
		default:
			NODEFAULTCASE;
		}
		int	SrcChunkRemainFrames = SrcChunkFrames;
		float	*pSrcFloat = SrcFloat.GetData();
		do {
			if (ProgDlg.Canceled())
				return(FALSE);
			sd.data_in = pSrcFloat;
			sd.data_out = DstFloat.GetData();
			sd.input_frames = SrcChunkRemainFrames;
			sd.output_frames = BufFrames;
			sd.src_ratio = SrcRatio;
			sd.end_of_input = !RemainSrcFrames;
			int	retc = m_src->src_process(m_State, &sd);
			if (retc) {
				CString	msg(m_src->src_strerror(retc));
				AfxMessageBox(msg);
				return(FALSE);
			}
			pSrcFloat += sd.input_frames_used * chans;
			SrcChunkRemainFrames -= sd.input_frames_used;
			int	DstChunkFrames = W64INT_CAST32(min(sd.output_frames_gen, RemainDstFrames));
			int	DstChunkSamps = DstChunkFrames * chans;
			// convert output frames from float buffer into destination wave
			switch (SampleSize) {
			case 1:	// 8-bit: convert float to short, then convert short to 8-bit
				m_src->src_float_to_short_array(
					DstFloat.GetData(), (short *)tmp.GetData(), DstChunkSamps);
				CSndFileEx::Convert(tmp, DstWave, 0, pDst - DstWave.GetData(), DstChunkFrames);
				break;
			case 2:	// 16-bit: convert float to short
				m_src->src_float_to_short_array(
					DstFloat.GetData(), (short *)pDst, DstChunkSamps);
				break;
			case 3:	// 24-bit: convert float to int, then convert int to 24-bit
				m_src->src_float_to_int_array(
					DstFloat.GetData(), (int *)tmp.GetData(), DstChunkSamps);
				CSndFileEx::Convert(tmp, DstWave, 0, pDst - DstWave.GetData(), DstChunkFrames);
				break;
			case 4:	// 32-bit: convert float to int
				m_src->src_float_to_int_array(
					DstFloat.GetData(), (int *)pDst, DstChunkSamps);
				break;
			default:
				NODEFAULTCASE;
			}
			pDst += DstChunkFrames * FrameSize;	// bump destination wave pointer
			RemainDstFrames -= DstChunkFrames;	// decrement destination frames remaining
			double	fPctDone = double(DstFrames - RemainDstFrames) / DstFrames * 100;
			ProgDlg.SetPos(round(fPctDone));
		} while (SrcChunkRemainFrames > 0);	// while this chunk has frames to process
		pSrc += SrcChunkFrames * FrameSize;	// bump source wave pointer
		RemainSrcFrames -= SrcChunkFrames;	// decrement source frames remaining
	} while (!sd.end_of_input);	// while there's more input to convert
	DstWave.SetFrameCount(DstFrames - RemainDstFrames);
	return(TRUE);
}
