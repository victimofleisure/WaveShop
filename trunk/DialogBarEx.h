// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28sep04	initial version
		01		15aug06	remove undo, fix vertical docking
		02		23nov07	support Unicode
		03		21nov12	add gripper size
		04		22nov12	fix OnInitDialog prototype
		05		23nov12	in ctor, default accelerator handle to zero
		06		23nov12	remove DockBar method
		07		23nov12	in Create, replace bar index with bar ID
		08      16apr13	handle command help

		dialog bar base class
 
*/

#if !defined(AFX_DIALOGBAREX_H__9D725823_F782_4C58_BBB8_01B240B648CA__INCLUDED_)
#define AFX_DIALOGBAREX_H__9D725823_F782_4C58_BBB8_01B240B648CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogBarEx.h : header file
//

#include "CtrlResize.h"

/////////////////////////////////////////////////////////////////////////////
// CDialogBarEx dialog

class CDialogBarEx : public CDialogBar
{
	DECLARE_DYNAMIC(CDialogBarEx);
public:
// Construction
	CDialogBarEx(UINT nIDAccel = 0);
	virtual ~CDialogBarEx();
	BOOL	Create(CWnd *pParentWnd, UINT nIDTemplate, UINT nStyle, UINT nID);
	BOOL	Create(CFrameWnd *pFrameWnd, UINT nIDTemplate, UINT nStyle,
				   DWORD dwDockStyle, UINT nBarID, CControlBar *LeftOf);

// Constants
	enum {	// option flags
		OPT_HORZ_RESIZE = 0x01,
		OPT_VERT_RESIZE = 0x02
	};
	enum {
		GRIPPER_SIZE = 6
	};

// Attributes
	CSize	GetMargin() const;
	int		GetLeftMargin() const;
	int		GetTopMargin() const;
	void	SetBarCaption(LPCTSTR Text);
	void	SetWindowText(LPCTSTR lpszString);
	int		GetBarOptions() const;
	void	SetBarOptions(int Options);

// Operations
	virtual	BOOL	EnableToolTips(BOOL bEnable);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogBarEx)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CDialogBarEx)
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CDialogBarEx)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnToolTipText(UINT id, NMHDR* pTTTStruct, LRESULT* pResult);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	static const int	CAP_MARGIN;

// Member data
	CSize	m_DockedSize;
	CSize	m_FloatingSize;
	BOOL	m_bChangeDockedSize;
	HACCEL	m_Accel;
	CStatic	m_Caption;
	CRect	m_CapRect;
	CSize	m_CapSize;
	CSize	m_Margin;
	CCtrlResize	m_Resize;
	int		m_PrevFloating;
	bool	m_GotMRUWidth;
	int		m_Options;
};

inline CSize CDialogBarEx::GetMargin() const
{
	return(m_Margin);
}

inline int CDialogBarEx::GetLeftMargin() const
{
	return(m_Margin.cx);
}

inline int CDialogBarEx::GetTopMargin() const
{
	return(m_Margin.cy);
}

inline int CDialogBarEx::GetBarOptions() const
{
	return(m_Options);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGBAREX_H__9D725823_F782_4C58_BBB8_01B240B648CA__INCLUDED_)
