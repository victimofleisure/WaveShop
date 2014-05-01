// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      18feb13	initial version
        01      27feb13	in GetFileFilter, add MPEG filter
		02		10apr13	in GetFileFilter, add foreign formats
        03      20apr13	in GetFileFilter, add AAC/MP4 filter
		04		03sep13	fix misleading Save As file extension

		derived document manager
 
*/

// DocManagerEx.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "DocManagerEx.h"
#include "PathStr.h"
#include "SndFileEx.h"
#include "Dlgs.h"	// for file dialog control IDs

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDocManagerEx dialog

IMPLEMENT_DYNAMIC(CDocManagerEx, CDocManager);

static const LPCTSTR	AIFF_EXT[]	= {_T("aifc"), _T("aif")};
static const LPCTSTR	OGG_EXT[]	= {_T("ogg")};

const CDocManagerEx::EXT_ALIAS CDocManagerEx::m_ExtAlias[] = {
	{SF_FORMAT_AIFF,	_countof(AIFF_EXT),	AIFF_EXT},
	{SF_FORMAT_OGG,		_countof(OGG_EXT),	OGG_EXT},
};

CDocManagerEx::CDocManagerEx()
{
	m_AudioFormat = 0;
}

BOOL CDocManagerEx::DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate)
{
	CString	filter;
	CSndFileFormatArray	SndFileFormat;
	// if unable to get filter, delegate to base class
	if (!GetFileFilter(filter, bOpenFileDialog, &SndFileFormat)) {
		return CDocManager::DoPromptFileName(fileName, nIDSTitle, 
			lFlags, bOpenFileDialog, pTemplate);
	}
	lFlags |= OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CFileDialogEx	dlgFile(bOpenFileDialog, NULL, NULL, lFlags, filter);
	dlgFile.m_SndFileFormat = &SndFileFormat;
	CString title;
	VERIFY(title.LoadString(nIDSTitle));
	dlgFile.m_ofn.lpstrTitle = title;
	CString	DefExt(PathFindExtension(fileName));	// get default extension from file name
	if (DefExt.IsEmpty())	// if no extension found
		DefExt = WAV_EXT;	// default to wave extension
	if (!bOpenFileDialog) {	// if save as dialog
		int iFmt = SndFileFormat.FindFormat(m_AudioFormat);	// find audio format
		if (iFmt >= 0) {	// if audio format was found
			// set filter index from format index; filter index is one-origin
			dlgFile.m_ofn.nFilterIndex = iFmt + 1;
			DefExt = '.' + SndFileFormat[iFmt].m_Extension;	// set default extension
			if (!fileName.IsEmpty()) {	// if filename isn't empty
				CPathStr	name(fileName);	// rename file name's extension to default
				name.RenameExtension(DefExt);
				fileName = name;
			}
		}
	}
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(MAX_PATH);
	dlgFile.m_ofn.lpstrDefExt = DefExt;	// set default extension pointer to string
	W64INT	retc = dlgFile.DoModal();	// do file dialog
	fileName.ReleaseBuffer();	// release caller's file name before returning
	if (retc != IDOK)
		return(FALSE);
	if (!bOpenFileDialog) {	// if save as dialog
		// get format index from filter index; filter index is one-origin
		int	iFmt = dlgFile.m_ofn.nFilterIndex - 1;
		m_AudioFormat = SndFileFormat[iFmt].m_Format;	// set audio format
	}
	return(TRUE);
}

CString CDocManagerEx::GetAliasedExtension(int Format, CString Extension)
{
	CString	result(_T("*.") + Extension);	// init result to caller's extension
	int	aliases = _countof(m_ExtAlias);
	for (int iAlias = 0; iAlias < aliases; iAlias++) {	// for each alias 
		const EXT_ALIAS&	alias = m_ExtAlias[iAlias];
		if (alias.Format == Format) {	// if alias format matches caller's format
			int	exts = alias.Exts;
			for (int iExt = 0; iExt < exts; iExt++) {	// for each extension
				result += _T(";*.");	// append separator and wildcard to result
				result += alias.Ext[iExt];	// append alternate extension to result
			}
			break;	// early out
		}
	}
	return(result);
}

CString CDocManagerEx::GetAllFilter()
{
	CString	filter;
	VERIFY(filter.LoadString(AFX_IDS_ALLFILTER));
	return(filter + _T("|*.*||"));
}

bool CDocManagerEx::GetFileFilter(CString& Filter, BOOL bOpenFileDialog, CSndFileFormatArray *pSndFileFormat)
{
	if (!PathFileExists(CSndFileEx::GetLibPath()))
		return(FALSE);	// sndfile library not found
	CSndFileEx	sf;
	if (!sf.Create())	// create sndfile wrapper
		return(FALSE);	// can't load sndfile library
	CSndFileFormatArray SndFileFormat;
	UINT	MajorFmtFlags = CSndFileEx::GMF_NATIVE_WAVE;	// handling wave natively
	if (!bOpenFileDialog)	// if save file dialog
		MajorFmtFlags |= CSndFileEx::GMF_INCLUDE_FOREIGN;	// include foreign formats
	if (!sf.GetMajorFormats(SndFileFormat, MajorFmtFlags))
		return(FALSE);	// can't get major formats
	if (bOpenFileDialog) {	// if open file dialog
		CString	ext;
		int	fmts = SndFileFormat.GetSize();
		for (int iFmt = 0; iFmt < fmts; iFmt++) {	// for each sndfile format
			const CSndFileFormat&	fmt = SndFileFormat[iFmt];
			if (iFmt)	// if not first format
				ext += ';';	// add separator
			ext += GetAliasedExtension(fmt.m_Format, fmt.m_Extension);
		}
		ext += MPEG_FILTER;	// add MPEG filter
		ext += MP4_FILTER;	// add MP4 filter
		CString	desc(ext);	// copy extension list to description
		desc.Replace(_T(";"), _T("; "));	// in description, add space after semicolon
		CString	AudioFiles;
		VERIFY(AudioFiles.LoadString(IDS_FILT_AUDIO_FILES));
		Filter = AudioFiles + _T(" (") + desc + _T(")|") + ext + '|' + GetAllFilter();
	} else {	// save file dialog
		CString	filt;
		int	fmts = SndFileFormat.GetSize();
		for (int iFmt = 0; iFmt < fmts; iFmt++) {
			const CSndFileFormat&	fmt = SndFileFormat[iFmt];
			CString	ext(GetAliasedExtension(fmt.m_Format, fmt.m_Extension));
			CString	desc(ext);
			desc.Replace(_T(";"), _T("; "));	// add space after semicolon
			filt += fmt.m_Name + _T(" (") + desc + _T(")|") + ext + '|';
		}
		Filter = filt + '|';	// add final terminator
	}
	if (pSndFileFormat != NULL)	// if caller passed a sndfile format array
		pSndFileFormat->Swap(SndFileFormat);	// exchange data with caller's array
	return(TRUE);
}

bool CDocManagerEx::CFileDialogEx::SetFileName(CString FileName)
{
#if _MFC_VER >= 0x0800	// Vista style requires NET2008
	if (m_bVistaStyle) {
		IFileDialog	*pIFileDialog = static_cast<IFileDialog*>(m_pIFileDialog);
		USES_CONVERSION;
		HRESULT	hr = pIFileDialog->SetFileName(T2CW(FileName));	// set file name
		return(SUCCEEDED(hr));
	}
#endif
	// not Vista style, so use ye olden method: hunt for file name control
	CWnd	*pParent = GetParent();
	CWnd	*pWnd = pParent->GetDlgItem(cmb13);	// try combo box first
	if (pWnd == NULL)	// if combo box not found
		pWnd = pParent->GetDlgItem(edt1);	// try edit control
	if (pWnd == NULL)	// if neither found
		return(FALSE);
	pWnd->SetWindowText(FileName);	// set file name
	return(TRUE);
}

void CDocManagerEx::CFileDialogEx::OnTypeChange()
{
	if (!m_bOpenFileDialog) {	// if save as dialog
		if (m_ofn.nFilterIndex > 0) {
			int	iExt = m_ofn.nFilterIndex - 1;	// filter index is one-origin
			ASSERT(m_SndFileFormat != NULL);
			CString	ext = (*m_SndFileFormat)[iExt].m_Extension;
			USES_CONVERSION;
			SetDefExt(T2CA(ext));	// set default extension
			CPathStr	FileName(GetFileName());	// get current filename
			if (!FileName.IsEmpty()) {	// if filename isn't empty
				FileName.RenameExtension('.' + ext);
				SetFileName(FileName);	// update filename with new extension
			}
		}
	}
	CFileDialog::OnTypeChange();
}
