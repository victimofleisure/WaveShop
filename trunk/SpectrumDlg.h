// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		19mar13	initial version
		01		14jul13	add option to specify frequency range

		spectrum dialog

*/

#if !defined(AFX_SPECTRUMDLG_H__777467B5_20C4_40B7_9B6D_25C9A8262825__INCLUDED_)
#define AFX_SPECTRUMDLG_H__777467B5_20C4_40B7_9B6D_25C9A8262825__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpectrumDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpectrumDlg dialog

#include "PersistDlg.h"
#include "CtrlResize.h"
#include "PlotCtrl.h"

class CWaveShopView;

class CSpectrumDlg : public CPersistDlg
{
// Construction
public:
	CSpectrumDlg(CWaveShopView *View, CWnd* pParent = NULL);
	~CSpectrumDlg();

// Operations
	static	void	InitWindowFunctionCombo(CComboBox& Combo, int WindowFunction);
	static	void	InitWindowSizeCombo(CComboBox& Combo, int WindowSize);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpectrumDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CSpectrumDlg)
	enum { IDD = IDD_SPECTRUM };
	CComboBoxEx	m_ViewChannelCombo;
	CComboBox	m_ChannelModeCombo;
	CComboBox	m_FreqAxisCombo;
	CComboBox	m_WindowSizeCombo;
	CComboBox	m_WindowFuncCombo;
	BOOL	m_FreqRangeAuto;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CSpectrumDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnRefresh();
	afx_msg void OnExport();
	afx_msg void OnReplot();
	afx_msg void OnFreqRangeAuto();
	afx_msg void OnKillfocusFreqEdit();
	virtual void OnCancel();
	//}}AFX_MSG
	afx_msg LRESULT	OnKickIdle(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateFreqEdit(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// frequency axis types
		FAT_LINEAR,
		FAT_LOG,
		FREQ_AXIS_TYPES
	};
	enum {	// channel modes
		CM_COMBINE,		// combine channels pre-analysis
		CM_SEPARATE,	// analyze each channel separately
		CHANNEL_MODES
	};
	enum {
		FIRST_WINDOW_SIZE = 7,	// first window size, as an exponent of 2
		WINDOW_SIZES = 8,		// number of window sizes
	};
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];

// Member data
	CWaveShopView	*m_View;	// pointer to view
	CSize	m_InitSize;			// dialog's initial size in window coords
	CCtrlResize	m_Resize;		// control resizer
	CReportCtrl	m_List;			// statistics list control
	CPlotCtrl	m_Plot;			// histogram plot control
	int		m_WindowFunction;	// window function index
	int		m_WindowSize;		// window size, in samples
	int		m_FreqAxisType;		// frequency axis type
	int		m_ChannelMode;		// multichannel audio mode; see CSpectrum
	CWaveProcess::CSpectrum	m_Spec;	// spectrum container
	CImageList m_ViewChanImgList;	// image list for view channel combo box
	CDblRange	m_FreqRange;	// user-specified frequency range, in Hertz

// Helpers
	void	InitViewChannelCombo();
	void	InitPlotCtrl();
	bool	Refresh();
	void	MakePlot();
	void	Export(LPCTSTR Path);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPECTRUMDLG_H__777467B5_20C4_40B7_9B6D_25C9A8262825__INCLUDED_)
