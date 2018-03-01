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

#include "stdafx.h"
#include "SndFileFormat.h"

CSndFileFormat::CSndFileFormat()
{
	m_Format = 0;
}

CSndFileFormat& CSndFileFormat::operator=(const CSndFileFormat& Info)
{
	if (this == &Info)
		return(*this);	// self-assignment
	m_Format	= Info.m_Format;
	m_Name		= Info.m_Name;
	m_Extension	= Info.m_Extension;
	return(*this);
}

int	CSndFileFormatArray::FindFormat(int Format) const
{
	int	formats = GetSize();
	for (int iFormat = 0; iFormat < formats; iFormat++) {	// for each format
		if (ElementAt(iFormat).m_Format == Format)	// if format matches caller's
			return(iFormat);
	}
	return(-1);	// format not found
}
