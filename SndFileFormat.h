// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      18feb13	initial version

		container for sndfile formats
 
*/

#ifndef CSNDFILEFORMAT_INCLUDED
#define CSNDFILEFORMAT_INCLUDED

#include "ArrayEx.h"

class CSndFileFormat : public WObject {
public:
// Construction
	CSndFileFormat();
	CSndFileFormat& operator=(const CSndFileFormat& Info);

// Data members
	int		m_Format;		// identifier
	CString	m_Name;			// descriptive title
	CString	m_Extension;	// file extension
};

class CSndFileFormatArray : public CArrayEx<CSndFileFormat, CSndFileFormat&> {
public:
	int		FindFormat(int Format) const;
};

#endif
