// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30jan13	initial version

        wrap transient DLL instance to avoid leaks
 
*/

#include "stdafx.h"
#include "DLLWrap.h"
#include "atlconv.h"	// for ATL string conversion macros

CDLLWrap::CDLLWrap()
{
	m_hInst = NULL;
}

CDLLWrap::~CDLLWrap()
{
	FreeLibrary();
}

bool CDLLWrap::LoadLibrary(LPCTSTR lpLibFileName)
{
	FreeLibrary();	// allow reuse of instance
	m_hInst = ::LoadLibrary(lpLibFileName);
	return(IsLoaded());
}

bool CDLLWrap::LoadLibraryEx(LPCTSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	FreeLibrary();	// allow reuse of instance
	m_hInst = ::LoadLibraryEx(lpLibFileName, hFile, dwFlags);
	return(IsLoaded());
}

bool CDLLWrap::FreeLibrary()
{
	if (!IsLoaded())	// if library not loaded
		return(FALSE);
	if (!::FreeLibrary(m_hInst))	// if free fails
		return(FALSE);
	m_hInst = NULL;	// invalidate handle
	return(TRUE);
}

FARPROC	CDLLWrap::GetProcAddress(LPCTSTR lpProcName)
{
	USES_CONVERSION;	// for ATL string conversion macros
	return(::GetProcAddress(m_hInst, T2CA(lpProcName)));
}
