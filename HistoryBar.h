// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		11feb13	initial version
		01		01mar13	add progress ctor
		02		29mar13	derive from custom sizing bar

        history bar
 
*/

#if !defined(AFX_HISTORYBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_HISTORYBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistoryBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHistoryBar window

#include "MySizingControlBar.h"
#include "ResultsReportCtrl.h"
#include "ProgressDlg.h"

class CHistoryBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CHistoryBar);
// Construction
public:
	CHistoryBar();

// Attributes
public:
	bool	SetUndoPos(int Pos);

// Operations
public:
	void	UpdateList();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHistoryBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CHistoryBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg	void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg	void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	class CListCtrlEx : public CListCtrl {
	public:
		virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	};
	class CProgressDlgEx : public CProgressDlg {
	public:
		CProgressDlgEx(UINT nIDTemplate = IDD_PROGRESS, CWnd* pParent = NULL);
		~CProgressDlgEx();
	};

// Constants
	enum {	// columns
		COL_ACTION,
		COLUMNS
	};
	enum {
		DEF_VERT_DOCK_WIDTH = 120	// default width when vertically docked
	};

// Member data
	CListCtrlEx	m_List;			// undo history list control
	CImageList	m_StateImgList;	// state image list for undo position marker
	CProgressDlgEx	*m_ProgressDlg;	// pointer to progress dialog during undo/redo
	int		m_ProgressPos;		// progress position during undo/redo
	friend class CProgressDlgEx;
};

inline CHistoryBar::CProgressDlgEx::CProgressDlgEx(UINT nIDTemplate, CWnd* pParent) :
	CProgressDlg(nIDTemplate, pParent)
{
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTORYBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
