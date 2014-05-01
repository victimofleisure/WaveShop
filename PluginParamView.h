// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25jun13	initial version

        plugin parameters view
 
*/

#if !defined(AFX_PLUGINPARAMVIEW_H__41F0C35F_CE2D_4B55_94C4_93D4A13D8E8F__INCLUDED_)
#define AFX_PLUGINPARAMVIEW_H__41F0C35F_CE2D_4B55_94C4_93D4A13D8E8F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PluginParamView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPluginParamView form view

#include "CtrlResize.h"
#include "Plugin.h"
#include "PluginParamRow.h"

class CPluginParamView : public CScrollView
{
public:
	CPluginParamView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPluginParamView)

// Attributes
public:
	int		GetRowCount() const;
	const _LADSPA_Descriptor	*GetDescriptor() const;
	void	GetParams(CPlugin::CParamArray& Param) const;
	bool	SetParams(const CPlugin::CParamArray& Param);

// Operations
public:
	bool	InitRows(LPCTSTR Path, UINT SampleRate);
	void	RestoreDefaults();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPluginParamView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPluginParamView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
	//{{AFX_MSG(CPluginParamView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	afx_msg void OnEditKillFocus(UINT nID);
	DECLARE_MESSAGE_MAP()

// Types
	typedef CArrayEx<CPluginParamRow, CPluginParamRow&> CParamRowArray;

// Data members
	CCtrlResize	m_Resize;		// control resizer
	CParamRowArray	m_Row;		// array of plugin parameter rows
	CDLLWrap	m_Dll;			// plugin DLL instance
	const _LADSPA_Descriptor	*m_Desc;	// LADSPA plugin descriptor
};

inline int CPluginParamView::GetRowCount() const
{
	return(m_Row.GetSize());
}

inline const _LADSPA_Descriptor *CPluginParamView::GetDescriptor() const
{
	return(m_Desc);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLUGINPARAMVIEW_H__41F0C35F_CE2D_4B55_94C4_93D4A13D8E8F__INCLUDED_)
