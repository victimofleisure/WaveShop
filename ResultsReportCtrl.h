// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14jan13	initial version
		01		04mar13 refactor for owner data

        derived report control for displaying audio processing results
 
*/

#if !defined(AFX_RESULTSREPORTCTRL_H__84A05433_DB70_434F_A8BB_7D0BC8C17B63__INCLUDED_)
#define AFX_RESULTSREPORTCTRL_H__84A05433_DB70_434F_A8BB_7D0BC8C17B63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResultsReportCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResultsReportCtrl window

#include "ReportCtrl.h"
#include "RefPtr.h"
#include "WaveProcess.h"

class CWaveShopView;

class CResultsReportCtrl : public CReportCtrl, public CRefObj
{
	DECLARE_DYNAMIC(CResultsReportCtrl);
// Construction
public:
	CResultsReportCtrl();

// Attributes
	void	Update(CWaveProcess::CClipSpanArray& ClipSpan, CWaveShopView *View);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResultsReportCtrl)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:
// Generated message map functions
	//{{AFX_MSG(CResultsReportCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* result);	
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	LRESULT	OnUpdateItems(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// clipping report columns
		CLIP_COL_START,
		CLIP_COL_LENGTH,
		CLIP_COL_CHANNEL,
		CLIP_COLS
	};
	static const RES_COL	m_ClipColInfo[CLIP_COLS];
	enum {
		UWM_UPDATEITEMS = WM_USER + 100
	};

// Member data
	CWaveProcess::CClipSpanArray	m_ClipSpan;	// array of clipping spans
	CIntArrayEx	m_SortIdx;	// array of clipping span indices for sorting
	CWaveShopView	*m_View;	// pointer to owner view
	static	CResultsReportCtrl	*m_This;	// pointer to our instance for sorting

// Overrides
	virtual	void	SortRows();

// Helpers
	void	OnItemSelection(int ItemIdx);
	int		SortCompare(const int *p1, const int *p2) const;
	static	int		SortCompare(const void *p1, const void *p2);
	int		FindItem(int ItemIdx) const;
	void	SelectItem(int ItemIdx);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESULTSREPORTCTRL_H__84A05433_DB70_434F_A8BB_7D0BC8C17B63__INCLUDED_)
