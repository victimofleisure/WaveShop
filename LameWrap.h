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

#ifndef CLAMEWRAP_INCLUDED
#define CLAMEWRAP_INCLUDED

#include "lame.h"
#include "lameFuncs.h"
#include "DllWrap.h"

class CWave;

class CLameWrap : public WObject {
public:
// Construction
	CLameWrap();
	~CLameWrap();
	bool	Create(LPCTSTR LibPath);

// Constants
	enum {	// bit rate modes
		BRM_CONSTANT,
		BRM_AVERAGE,
		BRM_VARIABLE,
		BIT_RATE_MODES
	};

// Types
	struct ENCODING_PARAMS {
		int		AlgorithmQuality;	// encoder algorithm quality [0..9]
									// 0=slowest/best, 9=fastest/worst
		int		BitRateMode;		// bit rate mode; see enum above
		int		TargetBitRate;		// target bitrate in kbits/s; applicable
									// in constant and average modes only
		int		TargetQuality;		// target quality [0..9] 0=slowest/best, 
									// 9=fastest/worst; for variable mode only
	};

// Operations
	bool	Write(LPCTSTR Path, const CWave& Wave, const ENCODING_PARAMS& Params, const CStringArray *Metadata = NULL);

protected:
// Types
	struct ILame {	// lame interface
		#define LAME_DEF(name, ordinal) p##name name;
		#include "lameDefs.h"
	};

// Constants
	#define METADATASTR(sndfile_str, id3v2_tag, wav_tag) STR_##sndfile_str,
	enum {
		#include "MetadataStr.h"	// generate metadata string enum
		STRINGS
	};
	enum {
		CHUNK_FRAMES = 0x10000,	// size of chunk to process at once, in frames 
	};
	static const int m_FuncOrd[];	// function ordinals
	static const int m_LameEncodeErr[];	// lame encode errors

// Member data
	CDLLWrap	m_Lib;			// wrapped instance of library
	CPtrArray	m_FuncPtr;		// array of pointers to exported functions
	ILame	*m_lame;			// pointer to lame interface struct
	lame_global_flags	*m_gfp;	// pointer to lame encoder instance

// Helpers
	void	OnEncodingError(int RetVal);
};

#endif
