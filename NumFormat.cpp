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

#include "stdafx.h"
#include "NumFormat.h"
#include "shlwapi.h"

CNumFormat::CNumFormat()
{
	LCID	lcid = LOCALE_USER_DEFAULT;
	TCHAR	buf[MAX_BUF];
	GetLocaleInfo(lcid, LOCALE_IDIGITS, buf, MAX_BUF);
	m_DefFmt.NumDigits = _ttoi(buf);
	GetLocaleInfo(lcid, LOCALE_ILZERO, buf, MAX_BUF);
    m_DefFmt.LeadingZero = _ttoi(buf);
	GetLocaleInfo(lcid, LOCALE_SGROUPING, buf, MAX_BUF);
    m_DefFmt.Grouping = _ttoi(buf);
	m_DefFmt.lpDecimalSep = m_DecimalSep;
	GetLocaleInfo(lcid, LOCALE_SDECIMAL, m_DefFmt.lpDecimalSep, MAX_BUF);
	m_DefFmt.lpThousandSep = m_ThousandSep;
	GetLocaleInfo(lcid, LOCALE_STHOUSAND, m_DefFmt.lpThousandSep, MAX_BUF);
	GetLocaleInfo(lcid, LOCALE_INEGNUMBER, buf, MAX_BUF);
    m_DefFmt.NegativeOrder = _ttoi(buf);
}

CString	CNumFormat::Format(LONGLONG Val) const
{
	NUMBERFMT	fmt = m_DefFmt;
	fmt.NumDigits = 0;	// no decimals
	CString	sVal, sResult;
	sVal.Format(_T("%I64d"), Val);
	int	MAXLEN = 32;
	LPTSTR	pResult = sResult.GetBuffer(MAXLEN);
	GetNumberFormat(0, 0, sVal, &fmt, pResult, MAXLEN);
	sResult.ReleaseBuffer();
	return(sResult);
}

CString	CNumFormat::Format(double Val, int Decimals) const
{
	NUMBERFMT	fmt = m_DefFmt;
	fmt.NumDigits = Decimals;
	CString	sVal, sResult;
	sVal.Format(_T("%.*lf"), Decimals, Val);
	int	MAXLEN = 32;
	LPTSTR	pResult = sResult.GetBuffer(MAXLEN);
	GetNumberFormat(0, 0, sVal, &fmt, pResult, MAXLEN);
	sResult.ReleaseBuffer();
	return(sResult);
}

CString	CNumFormat::FormatByteSize(LONGLONG Val)
{
	WCHAR	buf[32];
	StrFormatByteSizeW(Val, buf, _countof(buf));
	CString	s = buf;
	return(s);
}

