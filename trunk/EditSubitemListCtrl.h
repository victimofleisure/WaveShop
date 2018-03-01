// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26feb13	initial version

		list control with subitem editing
 
*/

#if !defined(AFX_EDITSUBITEMLISTCTRL_H__80FB53E2_FA82_4D1F_A010_1D430E50700E__INCLUDED_)
#define AFX_EDITSUBITEMLISTCTRL_H__80FB53E2_FA82_4D1F_A010_1D430E50700E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditSubitemListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditSubitemListCtrl window

class CEditSubitemListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CEditSubitemListCtrl);
// Construction
public:
	CEditSubitemListCtrl();

// Attributes
public:
	bool	IsEditing() const;

// Operations
public:
	bool	EditSubitem(int Row, int Col);
	void	EndEdit();
	void	CancelEdit();

// Constants
	enum {
		UWM_END_EDIT = WM_USER + 400,
		UWM_UPDATE_POS,
	};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditSubitemListCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditSubitemListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditSubitemListCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	afx_msg LRESULT OnEndEdit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdatePos(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Types
	class CPopupEdit : public CEdit {
	public:
		virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	};

// Data members
	CPopupEdit	m_SubEdit;		// popup edit control for subitem editing
	int		m_SubRow;			// row index of subitem being edited
	int		m_SubCol;			// column index of subitem being edited

// Helpers
	bool	GotoSubitem(int DeltaRow, int DeltaCol);
};

inline bool CEditSubitemListCtrl::IsEditing() const
{
	return(m_SubEdit.m_hWnd != NULL);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITSUBITEMLISTCTRL_H__80FB53E2_FA82_4D1F_A010_1D430E50700E__INCLUDED_)
