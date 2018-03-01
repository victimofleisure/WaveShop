// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21nov12	initial version

        fancy number formatting
 
*/

#pragma once

class CNumFormat : public WObject {
public:
	CNumFormat();
	CString	Format(LONGLONG Val) const;
	CString	Format(double Val, int Decimals) const;
	static	CString	FormatByteSize(LONGLONG Val);

protected:
	enum {
		MAX_BUF = 5
	};
	NUMBERFMT	m_DefFmt;
	TCHAR	m_DecimalSep[MAX_BUF];
	TCHAR	m_ThousandSep[MAX_BUF];
};
