// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      29jul13	initial version

		wrapper for libid3tag ID3 tag manipulation library
 
*/

#ifndef CID3TAGWRAP_INCLUDED
#define CID3TAGWRAP_INCLUDED

#include "DLLWrap.h"
#include "id3tag.h"
#include "id3tagFuncs.h"

struct id3_file;

class CID3TagWrap : public WObject {
public:
// Construction
	CID3TagWrap();
	~CID3TagWrap();
	bool	Create();

// Operations
	bool	Read(LPCTSTR Path, CStringArray& Metadata);

protected:
// Constants
	#define METADATASTR(sndfile_str, id3v2_tag, wav_tag) STR_##sndfile_str,
	enum {
		#include "MetadataStr.h"	// generate metadata string enum
		STRINGS
	};
	static const LPCSTR m_StrTag[];

// Types
	struct IID3TAG {	// id3tag interface
		#define ID3TAG_DEF(name, ordinal) p##name name;
		#include "id3tagDefs.h"
	};
	static const int m_FuncOrd[];	// function ordinals

// Member data
	CDLLWrap	m_Lib;			// wrapped instance of library
	CPtrArray	m_FuncPtr;		// array of pointers to exported functions
	IID3TAG	*m_id3;				// pointer to id3tag interface struct
	struct id3_file	*m_File;	// id3tag file interface struct
};

#endif
