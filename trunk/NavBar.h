// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16nov12	initial version

        navigation dialog bar
 
*/

#if !defined(AFX_NAVBAR_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_)
#define AFX_NAVBAR_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NavBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNavBar dialog

#include "DialogBarEx.h"
#include "CtrlResize.h"
#include "FlatIconButton.h"

class CMainFrame;
class CWaveShopView;

class CNavBar : public CDialogBarEx
{
	DECLARE_DYNAMIC(CNavBar);
// Construction
public:
	CNavBar(CWnd* pParent = NULL);   // standard constructor
	~CNavBar();

// Attributes
	void	SetNow(double Now);
	void	SetSelection(const CDblRange& Sel);

// Operations
	void	OnActivateView(CWaveShopView *View);
	void	OnTimeFormatChange();
	CString	FrameToStr(double Frame) const;
	bool	StrToFrame(LPCTSTR Time, double& Frame) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNavBar)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CNavBar)
	enum { IDD = IDD_NAV_BAR };
	CFlatIconButton	m_ShowLengthBtn;
	CEdit	m_SelEndEdit;
	CEdit	m_SelStartEdit;
	CEdit	m_NowEdit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CNavBar)
	afx_msg void OnKillfocusNow();
	afx_msg void OnKillfocusSelStart();
	afx_msg void OnKillfocusSelEnd();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowLength();
	//}}AFX_MSG
	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateShowLength(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()

// Constants
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];

// Member data
	CMainFrame	*m_Main;		// pointer to main frame
	CString	m_sNow;				// now string
	CString	m_sSelStart;		// selection start string
	CString	m_sSelEnd;			// selection end string
	bool	m_ShowLength;		// if true, show selection length instead of end

// Helpers
	UINT	GetSampleRate() const;
	bool	GetStr(CEdit& Edit, CString& Str, double& Frame) const;
	void	SetStr(CEdit& Edit, CString& Str, double Frame, bool Select = TRUE) const;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NAVBAR_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_)
