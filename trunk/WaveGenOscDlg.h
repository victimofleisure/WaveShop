// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30oct12	initial version
		01		07may13	add InitWaveformCombo

		wave generator oscillator child dialog
 
*/

#if !defined(AFX_WAVEGENOSCDLG_H__15FBEE38_6875_4EEF_A8B8_83C5A7731993__INCLUDED_)
#define AFX_WAVEGENOSCDLG_H__15FBEE38_6875_4EEF_A8B8_83C5A7731993__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WaveGenOscDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWaveGenOscDlg dialog

#include "WaveGenParms.h"

class CWaveGenOscDlg : public CDialog, public WAVEGEN_OSC_PARMS
{
// Construction
public:
	CWaveGenOscDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWaveGenOscDlg)
	enum { IDD = IDD_WAVE_GEN_OSC };
	CComboBox	m_WaveformCombo;
	//}}AFX_DATA

// Constants
	static const WAVEGEN_OSC_PARMS m_Defaults;	// default parameters

// Operations
	static	void	InitWaveformCombo(CComboBox& Combo, int SelIdx);
	static	void	DDX_Combo(CDataExchange *pDX, CComboBox& Combo, int& SelIdx);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveGenOscDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CWaveGenOscDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const int m_WaveformName[];	// waveform name string resource IDs
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVEGENOSCDLG_H__15FBEE38_6875_4EEF_A8B8_83C5A7731993__INCLUDED_)
