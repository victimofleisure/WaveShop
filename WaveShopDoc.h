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
        02      18feb13	add audio format, override DoSave
		03		28jul13	add metadata

		wave editor document
 
*/

// WaveShopDoc.h : interface of the CWaveShopDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVVIEWDOC_H__19BD38C6_0E43_4888_A08A_636088ADA052__INCLUDED_)
#define AFX_WAVVIEWDOC_H__19BD38C6_0E43_4888_A08A_636088ADA052__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WaveProcess.h"
#include "UndoManager.h"

class CWaveShopDoc : public CDocument
{
protected: // create from serialization only
	CWaveShopDoc();
	DECLARE_DYNCREATE(CWaveShopDoc)

// Types
	class CMyUndoManager : public CUndoManager {
	public:
		CMyUndoManager(CDocument *Doc);

	protected:
		virtual	void	OnModify(bool Modified);
		virtual	void	OnUpdateTitles();
		CDocument	*m_Document;
	};

// Attributes
public:
	CWaveProcess	m_Wave;			// wave
	CMyUndoManager	m_UndoMgr;		// undo manager
	int				m_AudioFormat;	// libsndfile audio format
	CStringArray	m_Metadata;		// audio file metadata strings

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveShopDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWaveShopDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWaveShopDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVVIEWDOC_H__19BD38C6_0E43_4888_A08A_636088ADA052__INCLUDED_)
