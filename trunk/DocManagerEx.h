// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      18feb13	initial version
        01      27feb13	make extension aliases a member

		derived document manager
 
*/

#if !defined(AFX_DOCMANAGEREX_H__ED62E7B1_7F4E_4DC3_92D2_75B32CDFBBBB__INCLUDED_)
#define AFX_DOCMANAGEREX_H__ED62E7B1_7F4E_4DC3_92D2_75B32CDFBBBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DocManagerEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDocManagerEx

#include "SndFileFormat.h"

class CDocManagerEx : public CDocManager {
	DECLARE_DYNAMIC(CDocManagerEx);
public:
// Construction
	CDocManagerEx();

// Types
	class CFileDialogEx : public CFileDialog {
	public:
		CFileDialogEx(BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL, DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL);
		virtual void OnTypeChange();
		bool	SetFileName(CString FileName);
		CSndFileFormatArray	*m_SndFileFormat;
	};

// Attributes
	int		GetAudioFormat() const;
	void	SetAudioFormat(int Format);
	static	bool	GetFileFilter(CString& Filter, BOOL bOpenFileDialog, CSndFileFormatArray *pSndFileFormat = NULL);
	static	CString	GetAliasedExtension(int Format, CString Extension);
	static	CString	GetAllFilter();

// Overrides
	virtual	BOOL	DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);

protected:
// Types
	struct EXT_ALIAS {	// extension alias
		int		Format;			// sndfile major format
		int		Exts;			// number of extensions
		const LPCTSTR	*Ext;	// array of pointers to extensions
	};

// Constants
	static const EXT_ALIAS	m_ExtAlias[];	// array of extension aliases

// Data members
	int		m_AudioFormat;			// libsndfile audio format
};

inline int CDocManagerEx::GetAudioFormat() const
{
	return(m_AudioFormat);
}

inline void CDocManagerEx::SetAudioFormat(int Format)
{
	m_AudioFormat = Format;
}

inline CDocManagerEx::CFileDialogEx::CFileDialogEx(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd)
	: CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOCMANAGEREX_H__ED62E7B1_7F4E_4DC3_92D2_75B32CDFBBBB__INCLUDED_)
