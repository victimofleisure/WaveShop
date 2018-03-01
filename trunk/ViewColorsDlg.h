// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      12feb13	initial version
        01      16jun13	add excluded channel colors

		colors dialog

*/

#if !defined(AFX_VIEWCOLORSDLG_H__57ADC2FD_25DE_4A8D_AD9C_5F5727671FD1__INCLUDED_)
#define AFX_VIEWCOLORSDLG_H__57ADC2FD_25DE_4A8D_AD9C_5F5727671FD1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewColorsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewColorsDlg dialog

#include "SwatchButton.h"

class CViewColorsDlg : public CDialog
{
// Construction
public:
	CViewColorsDlg(COptionsInfo& Info, CWnd* pParent = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewColorsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CViewColorsDlg)
	enum { IDD = IDD_VIEW_COLORS };
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CViewColorsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	afx_msg void OnColorBtn(UINT nID);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		COLORS = VIEW_PALETTE::COLORS
	};
	static const int	m_BtnResID[COLORS];	// button resource IDs, in palette order

// Member data
	COptionsInfo&	m_oi;		// reference to options info
	CSwatchButton	m_SwatchBtn[COLORS];	// swatch buttons, in palette order

// Helpers
	static	int		FindColor(int ResID);
	void	EditColor(int ColorIdx);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWCOLORSDLG_H__57ADC2FD_25DE_4A8D_AD9C_5F5727671FD1__INCLUDED_)
