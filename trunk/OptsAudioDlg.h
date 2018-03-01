// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25oct12	initial version
		01		04mar13	add VBR encoding quality
        02      20apr13	add MP4 import
        03      27apr13	add record device
		04		28apr13	remove persistence
		05		05may13	use GUID instead of description for device persistence

        audio options dialog
 
*/

#if !defined(AFX_OPTSAUDIODLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
#define AFX_OPTSAUDIODLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsAudioDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsAudioDlg dialog

#include "OptionsInfo.h"
#include "DSPlayer.h"
#include "EditSliderCtrl.h"

class COptsAudioDlg : public CPropertyPage
{
// Construction
public:
	COptsAudioDlg(COptionsInfo& Info);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptsAudioDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(COptsAudioDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	afx_msg LRESULT	OnKickIdle(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Dialog data
	//{{AFX_DATA(COptsAudioDlg)
	enum { IDD = IDD_OPT_AUDIO };
	CEditSliderCtrl	m_VBRQualitySlider;
	CNumEdit	m_VBRQualityEdit;
	CComboBox	m_PlayDeviceCombo;
	CComboBox	m_RecordDeviceCombo;
	//}}AFX_DATA

// Types
	typedef CDSPlayer::CDSDeviceInfoArray CDSDeviceInfoArray;

// Member data
	COptionsInfo&	m_oi;		// reference to parent's options info 
	CDSDeviceInfoArray	m_PlayDevInfo;	// playback device info array
	CDSDeviceInfoArray	m_RecordDevInfo;	// record device info array

// Helpers
	bool	PopulatePlayDeviceCombo();
	bool	PopulateRecordDeviceCombo();
	static	void	PopulateDeviceCombo(CComboBox& Combo, const CDSDeviceInfoArray& DevInfo, const GUID& DeviceGuid);
	static	CString	GetSelectedDeviceName(CComboBox& Combo, const CDSDeviceInfoArray& DevInfo);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSAUDIODLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
