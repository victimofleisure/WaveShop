// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05dec12	initial version
        01      28jan13	add sort header
        02      20mar13	pass view to ctor instead of title

		peak statistics dialog
 
*/

#if !defined(AFX_PEAKSTATSDLG_H__9F98DAD8_98F2_43BD_8C09_0C821BDFE7DB__INCLUDED_)
#define AFX_PEAKSTATSDLG_H__9F98DAD8_98F2_43BD_8C09_0C821BDFE7DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PeakStatsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPeakStatsDlg dialog

#include "PersistDlg.h"
#include "WaveProcess.h"
#include "ReportCtrl.h"
#include "CtrlResize.h"

class CWaveShopView;

class CPeakStatsDlg : public CPersistDlg
{
// Construction
public:
	CPeakStatsDlg(CWaveProcess::CPeakStats& Stats, CWaveShopView *View, CWnd* pParent = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPeakStatsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CPeakStatsDlg)
	enum { IDD = IDD_PEAK_STATS };
	CReportCtrl	m_List;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CPeakStatsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnGoto();
	afx_msg void OnUpdateGoto(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// columns
		COL_CHANNEL,
		COL_MIN_SAMPLE,
		COL_MIN_PCT,
		COL_MAX_SAMPLE,
		COL_MAX_PCT,
		COL_PEAK_DB,
		COL_DC_BIAS,
		COL_BIAS_PCT,
		COLS
	};
	enum {	// goto targets
		GOTO_NONE,
		GOTO_MIN,
		GOTO_MAX,
	};
	static const CReportCtrl::RES_COL m_ColInfo[COLS];
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];

// Member data
	CWaveShopView	*m_View;	// pointer to view
	CWaveProcess::CPeakStats&	m_Stats;	// reference to peak statistics
	CCtrlResize	m_Resize;		// control resizer
	CPoint	m_ContextPt;		// context menu point

// Helpers
	int		GetGotoTarget(CPoint Point, int& ChannelIdx);
	static int CALLBACK SortCompare(LPARAM p1, LPARAM p2, LPARAM This);
	int		SortCompare(int p1, int p2) const;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PEAKSTATSDLG_H__9F98DAD8_98F2_43BD_8C09_0C821BDFE7DB__INCLUDED_)
