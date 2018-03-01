// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      19apr13	initial version

		wrapper for libmp4ad (libfaad2) AAC/MP4 audio decoder DLL
 
*/

#ifndef CMP4DECODEWRAP_INCLUDED
#define CMP4DECODEWRAP_INCLUDED

#include "libmp4ad.h"
#include "MP4DecodeFuncs.h"
#include "DllWrap.h"
#include "ProgressDlg.h"

class CWave;

class CMP4DecodeWrap : public WObject {
public:
// Construction
	CMP4DecodeWrap();
	~CMP4DecodeWrap();
	bool	Create(int Quality, bool Downmix);

// Constants
	enum {	// decode quality
		QUAL_16_BIT,
		QUAL_24_BIT,
		QUAL_32_BIT,
		QUALITIES
	};

// Operations
	bool	Read(LPCTSTR Path, CWave& Wave);

protected:
// Types
	struct IMP4 {	// mad interface
		#define MP4_DEF(name, ordinal) p##name name;
		#include "MP4DecodeDefs.h"
	};
	static const int m_FuncOrd[];	// function ordinals
	class CFileEx : public CFile {
	public:
		void	SetCloseOnDelete(bool Enable);
	};

// Constants
	enum {
		MAX_REPORT_ERRORS = 10	// maximum number of errors to report
	};

// Member data
	CDLLWrap	m_Lib;			// wrapped instance of library
	CPtrArray	m_FuncPtr;		// array of pointers to exported functions
	IMP4	*m_mp4;				// pointer to mp4 interface struct
#if _MFC_VER < 0x0700
	UINT	m_InFileSize;		// size of input file, in bytes
#else
	ULONGLONG	m_InFileSize;	// size of input file, in bytes
#endif
	CString	m_TmpPath;			// path of temporary output file
	FILE	*m_OutFile;			// temporary file to receive samples
	UINT	m_Channels;			// number of channels in stream
	UINT	m_SampleRate;		// stream's sample rate in Hz
	UINT	m_SampleSize;		// sample size in bits
	UINT	m_RcvdSamples;		// number of samples received
	bool	m_Downmix;			// if true, mix surround down to stereo
	CByteArray	m_ConvBuf;		// sample conversion buffer
	CProgressDlg	m_ProgDlg;	// progress dialog
	int		m_PctDone;			// percentage completed
	int		m_Quality;			// decode quality
	UINT	m_ErrorCount;		// number of decoding errors
	CString	m_ErrorReport;		// concatenated error messages
	CString	m_LastError;		// most recent error message

// Helpers
	int		OnOutput(long total_samples, long samples, const NeAACDecFrameInfo *frame_info, const void *sample_buffer);
	void	OnError(const char *error_message);
	CString	MakeErrorReport() const;
	static	int		OnOutput(void *param, long total_samples, long samples, const NeAACDecFrameInfo *frame_info, const void *sample_buffer);
	static	void	OnError(void *param, const char *error_message);
};

inline void CMP4DecodeWrap::CFileEx::SetCloseOnDelete(bool Enable)
{
	m_bCloseOnDelete = Enable;
}

#endif
