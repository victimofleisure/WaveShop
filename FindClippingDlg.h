// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14jan13	initial version
		01		04mar13	add clipping level

		find clipping dialog

*/

#if !defined(AFX_FINDCLIPPINGDLG_H__FBA0CA0B_6D57_4371_BEF3_AF14C3BBC59B__INCLUDED_)
#define AFX_FINDCLIPPINGDLG_H__FBA0CA0B_6D57_4371_BEF3_AF14C3BBC59B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindClippingDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindClippingDlg dialog

class CFindClippingDlg : public CDialog
{
// Construction
public:
	CFindClippingDlg(CWnd* pParent = NULL);   // standard constructor
	~CFindClippingDlg();

// Dialog Data
	//{{AFX_DATA(CFindClippingDlg)
	enum { IDD = IDD_FIND_CLIPPING };
	UINT	m_StopThreshold;
	UINT	m_StartThreshold;
	double	m_ClippingLevel;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindClippingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CFindClippingDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDCLIPPINGDLG_H__FBA0CA0B_6D57_4371_BEF3_AF14C3BBC59B__INCLUDED_)
