// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27feb13	initial version
        01      09apr13	remove unused constant, fix comments
		02      20apr13	move OpenTempStream to app
		03      20apr13	in Create, add Quality argument

		wrapper for libmad MPEG audio decoder DLL
 
*/

#ifndef CMADWRAP_INCLUDED
#define CMADWRAP_INCLUDED

#include "mad.h"
#include "madFuncs.h"
#include "DllWrap.h"
#include "ProgressDlg.h"

class CWave;

class CMadWrap : public WObject {
public:
// Construction
	CMadWrap();
	~CMadWrap();
	bool	Create(int Quality);

// Constants
	enum {	// decode quality
		QUAL_16_BIT,
		QUAL_24_BIT,
		QUALITIES
	};

// Operations
	bool	Read(LPCTSTR Path, CWave& Wave);

protected:
// Types
	struct IMad {	// mad interface
		#define MAD_DEF(name, ordinal) p##name name;
		#include "madDefs.h"
	};
	static const int m_FuncOrd[];	// function ordinals
	class CFileEx : public CFile {
	public:
		void	SetCloseOnDelete(bool Enable);
	};

// Constants
	enum {
		READ_BUF_SIZE = 0x10000,	// read buffer size, in bytes
	};

// Member data
	CDLLWrap	m_Lib;			// wrapped instance of library
	CPtrArray	m_FuncPtr;		// array of pointers to exported functions
	IMad	*m_mad;				// pointer to mad interface struct
#if _MFC_VER < 0x0700
	UINT	m_InFileSize;		// size of input file, in bytes
#else
	ULONGLONG	m_InFileSize;	// size of input file, in bytes
#endif
	CByteArray	m_ReadBuf;		// read buffer for input file
	CFile	m_InFile;			// input file containing source stream
	FILE	*m_OutFile;			// temporary file to receive samples
	UINT	m_Channels;			// number of channels in stream
	UINT	m_SampleRate;		// stream's sample rate in Hz
	UINT	m_SampleSize;		// sample size in bits
	UINT	m_SampleBits;		// sample size in bytes
	UINT	m_TotalFrames;		// estimated total number of frames
	UINT	m_RcvdFrames;		// actual number of frames received
	CProgressDlg	m_ProgDlg;	// progress dialog

// Static callback wrappers
	static enum mad_flow input(void *data, struct mad_stream *stream);
	static enum mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm);
	static enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame);

// Callbacks
	enum mad_flow OnInput(struct mad_stream *stream);
	enum mad_flow OnOutput(struct mad_header const *header, struct mad_pcm *pcm);
	enum mad_flow OnError(struct mad_stream *stream, struct mad_frame *frame);

// Helpers
	int		scale(mad_fixed_t sample);
};

inline void CMadWrap::CFileEx::SetCloseOnDelete(bool Enable)
{
	m_bCloseOnDelete = Enable;
}

#endif
