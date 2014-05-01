// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		12jan13	initial version
		01		29mar13	derive from custom sizing bar

        results bar
 
*/

#if !defined(AFX_RESULTSBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_RESULTSBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResultsBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResultsBar window

#include "MySizingControlBar.h"
#include "ResultsReportCtrl.h"

class CResultsBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CResultsBar);
// Construction
public:
	CResultsBar();

// Attributes
public:
	bool	ReportClipping(CWaveProcess::CClipSpanArray& ClipSpan, CWaveShopView *View);

// Operations
public:
	void	OnDestroyView(CWaveShopView *View);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResultsBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CResultsBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CResultsBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef CRefPtr<CResultsReportCtrl> CResultsReportCtrlPtr;

// Member data
	CResultsReportCtrlPtr	m_Report;	// reference-counted pointer to report control
	CWaveShopView	*m_View;	// pointer to owner view
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESULTSBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
