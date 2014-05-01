// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25jan13	initial version

		find dialog

*/

#if !defined(AFX_FINDDLG_H__1C442729_89BB_414E_A7EA_9FE11945D043__INCLUDED_)
#define AFX_FINDDLG_H__1C442729_89BB_414E_A7EA_9FE11945D043__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog

#include "WaveProcess.h"

class CFindDlg : public CDialog
{
// Construction
public:
	CFindDlg(CWnd* pParent = NULL);   // standard constructor
	~CFindDlg();

// Dialog Data
	//{{AFX_DATA(CFindDlg)
	enum { IDD = IDD_FIND };
	double	m_TargetStart;
	double	m_TargetEnd;
	int		m_Unit;
	int		m_Match;
	int		m_Dir;
	BOOL	m_Wrap;
	CComboBox	m_ChannelCombo;
	//}}AFX_DATA
	int		m_Channel;	// index of channel to search, or -1 for all channels

// Types
	typedef CWaveProcess::CSampleRange CSampleRange;

// Attributes
	CSampleRange	GetTargetRange() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CFindDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	afx_msg void OnUnit(UINT nID);
	DECLARE_MESSAGE_MAP()

// Helpers
	CDblRange	GetTargetRange(int Unit) const;
	void	SetTargetRange(CDblRange Target, int Unit);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDDLG_H__1C442729_89BB_414E_A7EA_9FE11945D043__INCLUDED_)
