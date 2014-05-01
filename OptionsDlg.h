// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version
        01      01apr13	add real-time spectrum analyzer
		02		28apr13	refactor to make options a reference
        03      17may13	add record page

        options dialog
 
*/

#if !defined(AFX_OPTIONSDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
#define AFX_OPTIONSDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog

#include "OptionsInfo.h"
#include "OptsViewDlg.h"
#include "OptsAudioDlg.h"
#include "OptsEditDlg.h"
#include "OptsRTSADlg.h"
#include "OptsRecordDlg.h"

class COptionsDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(COptionsDlg)
// Construction
public:
	COptionsDlg(UINT nIDCaption, COptionsInfo& Options, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Constants
	#define OPTIONSPAGEDEF(name) PAGE_##name,
	enum {
		#include "OptionsPages.h"	// enumerate property pages
		OPTIONS_PAGES
	};

// Attributes
	int		GetCurPage() const;

// Operations
	void	RestoreDefaults();
	const COptionsInfo&	GetDefaults() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsDlg)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(COptionsDlg)
	afx_msg void OnDestroy();
	afx_msg void OnResetAll();
	//}}AFX_MSG
	afx_msg LRESULT	OnKickIdle(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	static const int	m_PageOffset[OPTIONS_PAGES];	// page offsets

// Member data
	COptionsInfo&	m_oi;			// reference to current options
	#define OPTIONSPAGEDEF(name) COpts##name##Dlg m_##name##Dlg;
	#include "OptionsPages.h"		// allocate property pages
	CButton	m_ResetAll;				// reset all button
	int		m_CurPage;				// index of current page

// Helpers
	void	CreateResetAllButton();
};

inline int COptionsDlg::GetCurPage() const
{
	return(m_CurPage);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
