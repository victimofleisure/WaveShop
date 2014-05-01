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

#ifndef CDLLWRAP_INCLUDED
#define CDLLWRAP_INCLUDED

class CDLLWrap : public WObject {
public:
// Construction
	CDLLWrap();
	~CDLLWrap();

// Attributes
	bool	IsLoaded() const;
	FARPROC	GetProcAddress(LPCTSTR lpProcName);

// Operations
	bool	LoadLibrary(LPCTSTR lpLibFileName);
	bool	LoadLibraryEx(LPCTSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
	bool	FreeLibrary();

protected:
// Data members
	HINSTANCE	m_hInst;	// handle to DLL instance
};

inline bool CDLLWrap::IsLoaded() const
{
	return(m_hInst != NULL);
}

#endif
