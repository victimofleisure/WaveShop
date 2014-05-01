// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10apr13	initial version
        01      10jul13	use click slider
        02      03aug13	add metadata

		MP3 encoder configuration dialog
 
*/

#if !defined(AFX_MP3ENCODERDLG_H__5B0BA7CC_5869_4247_B498_7B32EDAC8F75__INCLUDED_)
#define AFX_MP3ENCODERDLG_H__5B0BA7CC_5869_4247_B498_7B32EDAC8F75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MP3EncoderDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMP3EncoderDlg dialog

#include "LameWrap.h"

class CMP3EncoderDlg : public CDialog
{
// Construction
public:
	CMP3EncoderDlg(CWnd* pParent = NULL);   // standard constructor
	~CMP3EncoderDlg();

// Attributes
	void	GetParams(CLameWrap::ENCODING_PARAMS& Params) const;

// Operations
	static	bool	Encode(LPCTSTR Path, const CWave& Wave, const CStringArray *Metadata = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMP3EncoderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMP3EncoderDlg)
	enum { IDD = IDD_MP3_ENCODER };
	CClickSliderCtrl	m_TargetQualitySlider;
	CComboBox	m_TargetBitRateCombo;
	int		m_BitRateMode;
	int		m_AlgorithmQualityPresetIdx;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMP3EncoderDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	afx_msg	LRESULT OnKickIdle(WPARAM wParam, LPARAM lParam);
	afx_msg	void OnUpdateTargetBitRate(CCmdUI* pCmdUI);
	afx_msg	void OnUpdateTargetQuality(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {	// algorithm quality presets
		AQP_FAST,
		AQP_STANDARD,
		AQP_HIGH_QUALITY,
		ALG_QUALITY_PRESETS
	};
	enum {
		MAX_VBR_QUALITY = 9
	};
	static const int	m_AlgorithmQualityPreset[ALG_QUALITY_PRESETS];
	static const int	m_BitRatePreset[];

// Member data
	int		m_AlgorithmQuality;	// encoder algorithm quality [0..9]
								// 0=slowest/best, 9=fastest/worst
	int		m_TargetBitRate;	// target bitrate in kbits/s; applicable
								// in constant and average modes only
	int		m_TargetQuality;	// target quality [0..9] 0=slowest/best, 
								// 9=fastest/worst; for variable mode only
// Helpers
	bool	LimitQuality();
	int		FindAlgorithmQuality(int Quality) const;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MP3ENCODERDLG_H__5B0BA7CC_5869_4247_B498_7B32EDAC8F75__INCLUDED_)
