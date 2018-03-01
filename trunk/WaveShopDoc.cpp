// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
		01		11feb13	override undo manager's OnUpdateTitles
        02      18feb13	handle non-native file formats
		03		28jul13	add metadata
		04		03sep13	in DoSave, force Save As for AAC/MP4 files

		wave editor document
 
*/

// WaveShopDoc.cpp : implementation of the CWaveShopDoc class
//

#include "stdafx.h"
#include "WaveShop.h"

#include "WaveShopDoc.h"
#include "Benchmark.h"
#include "sndfile.h"
#include "DocManagerEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaveShopDoc

IMPLEMENT_DYNCREATE(CWaveShopDoc, CDocument)

/////////////////////////////////////////////////////////////////////////////
// CWaveShopDoc construction/destruction

CWaveShopDoc::CWaveShopDoc() : m_UndoMgr(this)
{
	m_UndoMgr.SetLevels(theApp.GetMain()->GetOptions().m_UndoLevels);
	m_AudioFormat = 0;
}

CWaveShopDoc::~CWaveShopDoc()
{
}

inline CWaveShopDoc::CMyUndoManager::CMyUndoManager(CDocument *Doc)
{
	m_Document = Doc;
}

void CWaveShopDoc::CMyUndoManager::OnModify(bool Modified)
{
	m_Document->SetModifiedFlag(Modified);
}

void CWaveShopDoc::CMyUndoManager::OnUpdateTitles()
{
	theApp.GetMain()->OnUpdateUndoTitles();
}

/////////////////////////////////////////////////////////////////////////////
// CWaveShopDoc serialization

void CWaveShopDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWaveShopDoc diagnostics

#ifdef _DEBUG
void CWaveShopDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWaveShopDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWaveShopDoc message map

BEGIN_MESSAGE_MAP(CWaveShopDoc, CDocument)
	//{{AFX_MSG_MAP(CWaveShopDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaveShopDoc commands

BOOL CWaveShopDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

BOOL CWaveShopDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	int	AudioFormat;
	if (!m_Wave.Import(lpszPathName, AudioFormat, &m_Metadata))
		return(FALSE);
	// success; update our audio format
	m_AudioFormat = AudioFormat & SF_FORMAT_TYPEMASK;	// preserve format's type only

	return TRUE;
}

BOOL CWaveShopDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnSaveDocument(lpszPathName))
		return FALSE;

	CDocManagerEx	*pDocMgr = STATIC_DOWNCAST(CDocManagerEx, theApp.m_pDocManager);
	int	AudioFormat = pDocMgr->GetAudioFormat();	// get selected audio format
	if (!m_Wave.Export(lpszPathName, AudioFormat, &m_Metadata))
		return(FALSE);
	// success, update our audio format
	m_AudioFormat = AudioFormat;

	return TRUE;
}

BOOL CWaveShopDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
{
	CDocManagerEx	*pDocMgr = STATIC_DOWNCAST(CDocManagerEx, theApp.m_pDocManager);
	pDocMgr->SetAudioFormat(m_AudioFormat);	// pass audio format to doc manager

	CString	ext(PathFindExtension(lpszPathName));
	if (!ext.IsEmpty()) {
		ext.MakeLower();
		if (CString(MP4_FILTER).Find(ext) >= 0)	// if path has AAC/MP4 file extension
			lpszPathName = NULL;	// writing AAC/MP4 isn't supported; force Save As
	}

	return CDocument::DoSave(lpszPathName, bReplace);
}
