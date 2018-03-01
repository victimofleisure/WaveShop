// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30aug05	initial version
		01		10aug07	add template resource ID to ctor
		02		27dec09	add ShowPercent
		03		10jan13	make PumpMessages public
		04		28feb13	add OnOK
		05		01mar13	add dual progress

        progress dialog
 
*/

#if !defined(AFX_PROGRESSDLG_H__371804AF_56CF_433C_BCFD_C543787BC927__INCLUDED_)
#define AFX_PROGRESSDLG_H__371804AF_56CF_433C_BCFD_C543787BC927__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

class CProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CProgressDlg);
// Construction
public:
	CProgressDlg(UINT nIDTemplate = IDD_PROGRESS, CWnd* pParent = NULL);
	~CProgressDlg();
	bool	Create(CWnd* pParent = NULL);

// Attributes
	void	SetPos(int Pos);
	void	SetRange(int Lower, int Upper);
	bool	Canceled() const;
	void	ShowPercent(bool Enable);
	void	SetWindowText(LPCTSTR Text);

// Operations
	void	PumpMessages();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CProgressDlg)
	enum { IDD = IDD_PROGRESS };
	CProgressCtrl	m_Progress;
	CStatic	m_Percent;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CProgressDlg)
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	UINT	m_IDTemplate;		// template resource ID
    CWnd	*m_ParentWnd;		// pointer to parent window
	bool	m_ParentDisabled;	// true if parent window is disabled
	bool	m_Canceled;			// true if cancel button was pressed
	bool	m_ShowPercent;		// true if showing percentage
	int		m_PrevPercent;		// previous percentage
	static	CProgressDlg	*m_Master;	// dual progress master if any

// Helpers
	void	ReenableParent();
	bool	IsSlave() const;
	bool	IsMaster() const;
};

inline bool CProgressDlg::Canceled() const
{
	return(m_Canceled);
}

inline void CProgressDlg::ShowPercent(bool Enable)
{
	m_ShowPercent = Enable;
}

inline bool CProgressDlg::IsSlave() const
{
	return(m_Master != NULL && this != m_Master);
}

inline bool CProgressDlg::IsMaster() const
{
	return(this == m_Master);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSDLG_H__371804AF_56CF_433C_BCFD_C543787BC927__INCLUDED_)
