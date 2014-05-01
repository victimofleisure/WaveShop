// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17dec12	initial version
        01      28jan13	add sort header

		RMS statistics dialog

*/

#if !defined(AFX_RMSSTATSDLG_H__E62FB83A_D80C_496D_A772_0B3766C3A3DC__INCLUDED_)
#define AFX_RMSSTATSDLG_H__E62FB83A_D80C_496D_A772_0B3766C3A3DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RMSStatsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRMSStatsDlg dialog

#include "CtrlResize.h"
#include "ReportCtrl.h"
#include "PlotCtrl.h"
#include "WaveProcess.h"
#include "PersistDlg.h"

class CWaveShopView;

class CRMSStatsDlg : public CPersistDlg
{
// Construction
public:
	CRMSStatsDlg(CWaveShopView *View, CWnd* pParent = NULL);
	~CRMSStatsDlg();

// Operations
	bool	CalcStats();
	bool	PlotSeries();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRMSStatsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CRMSStatsDlg)
	enum { IDD = IDD_RMS_STATS };
	CComboBox	m_TopChannelCombo;
	CStatic	m_TopChannelCap;
	CTabCtrl	m_Tab;
	BOOL	m_AccountForDC;
	int		m_WindowSize;
	int		m_ZeroWave;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CRMSStatsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRefresh();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSelchangeTabCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeTopChannel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// columns
		COL_CHANNEL,
		COL_MIN,
		COL_MAX,
		COL_AVG,
		COL_TOTAL,
		COLS
	};
	enum {	// tabs
		TAB_STATS,
		TAB_HISTOGRAM,
		TABS
	};
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];
	static const CReportCtrl::RES_COL m_ColInfo[COLS];

// Member data
	CWaveShopView	*m_View;	// pointer to view
	CSize	m_InitSize;			// dialog's initial size in window coords
	CCtrlResize	m_Resize;		// control resizer
	CReportCtrl	m_List;			// statistics list control
	CPlotCtrl	m_Plot;			// histogram plot control
	CWnd	*m_TabChild[TABS];	// array of pointers to tab's child controls
	CWaveProcess::CPeakStats	m_PeakStats;	// peak statistics
	CWaveProcess::CRMSStats	m_RMSStats;	// RMS statistics
	CWaveProcess::RMS_STATS_PARMS	m_Parms;	// RMS statistics parameters
	int		m_TabSel;			// index of selected tab

// Helpers
	bool	DumpHistogram();
	void	ResizeTabChild();
	void	OnNewStats();
	void	ShowTabChild(int TabIdx);
	static int CALLBACK SortCompare(LPARAM p1, LPARAM p2, LPARAM This);
	int		SortCompare(int p1, int p2) const;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RMSSTATSDLG_H__E62FB83A_D80C_496D_A772_0B3766C3A3DC__INCLUDED_)
