// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25jun13	initial version

        plugin parameters dialog
 
*/

#if !defined(AFX_PLUGINPARAMDLG_H__BDEF99F6_639B_4E98_936E_C8CCD3D8CAC9__INCLUDED_)
#define AFX_PLUGINPARAMDLG_H__BDEF99F6_639B_4E98_936E_C8CCD3D8CAC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PluginParamDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPluginParamDlg dialog

#include "PersistDlg.h"
#include "CtrlResize.h"
#include "Plugin.h"

class CPluginParamView;

class CPluginParamDlg : public CPersistDlg
{
public:
// Construction
	CPluginParamDlg(LPCTSTR Path, UINT SampleRate, CPlugin::CParamArray& Param, CWnd* pParent = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPluginParamDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CPluginParamDlg)
	enum { IDD = IDD_PLUGIN_PARAM };
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CPluginParamDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	virtual void OnOK();
	afx_msg void OnReset();
	afx_msg void OnAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];

// Data members
	CSize	m_InitSize;			// dialog's initial size in window coords
	CCtrlResize	m_Resize;		// control resizer
	CString	m_PluginPath;		// plugin path
	UINT	m_SampleRate;		// audio sample rate in Hertz
	CPlugin::CParamArray&	m_Param;	// reference to caller's parameter array
	CPluginParamView	*m_View;	// pointer to parameter view

// Helpers
	bool	CreateView();
	void	GetViewHolderRect(CRect& Rect) const;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLUGINPARAMDLG_H__BDEF99F6_639B_4E98_936E_C8CCD3D8CAC9__INCLUDED_)
