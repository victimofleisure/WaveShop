// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26jul13	initial version

		metadata editing dialog
 
*/

#if !defined(AFX_METADATADLG_H__207C23D0_78D6_4AAB_867C_1DFA5E4C7817__INCLUDED_)
#define AFX_METADATADLG_H__207C23D0_78D6_4AAB_867C_1DFA5E4C7817__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MetadataDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMetadataDlg dialog

#include "PersistDlg.h"
#include "CtrlResize.h"
#include "EditSubitemListCtrl.h"

class CMetadataDlg : public CPersistDlg
{
// Construction
public:
	CMetadataDlg(CStringArray& Metadata, CWnd* pParent = NULL);

// Constants
	#define METADATASTR(sndfile_str, id3v2_tag, wav_tag) STR_##sndfile_str,
	enum {
		#include "MetadataStr.h"	// generate metadata string enum
		STRINGS
	};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMetadataDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMetadataDlg)
	enum { IDD = IDD_METADATA };
	CEditSubitemListCtrl	m_List;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMetadataDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	virtual void OnOK();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnClear();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	struct COL_INFO {	// column info
		int		NameID;			// column's name as a string resource ID
		int		Width;			// column's default width
	};

// Constants
	enum {
		COL_NAME,
		COL_VALUE,
		COLUMNS
	};
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];
	static const COL_INFO	m_ColInfo[COLUMNS];
	static const int	m_StrNameID[STRINGS];

// Member data
	CSize	m_InitSize;			// dialog's initial size in window coords
	CCtrlResize	m_Resize;		// control resizer
	CStringArray&	m_Metadata;	// array of metadata strings

// Helpers
	static	bool	StoreColumnWidths(CListCtrl& List, LPCTSTR Key);
	static	bool	LoadColumnWidths(CListCtrl& List, LPCTSTR Key);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_METADATADLG_H__207C23D0_78D6_4AAB_867C_1DFA5E4C7817__INCLUDED_)
