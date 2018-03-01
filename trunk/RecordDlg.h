// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28apr13	initial version
		01		17may13	add activation types
		02		18jun13	add open recording checkbox

		record dialog
 
*/

#if !defined(AFX_RECORDDLG_H__C4563F72_D33B_404D_8331_2A1DE732DBAB__INCLUDED_)
#define AFX_RECORDDLG_H__C4563F72_D33B_404D_8331_2A1DE732DBAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecordDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg dialog

#include "PersistDlg.h"
#include "CtrlResize.h"
#include "DSCapture.h"
#include "MeterView.h"
#include "NumFormat.h"

class CRecordDlg : public CPersistDlg
{
// Construction
public:
	CRecordDlg(CString OutPath, CWnd* pParent = NULL);   // standard constructor
	~CRecordDlg();
	static	CRecordDlg	*Record();

// Constants
	enum {	// statuses
		ST_STOP,
		ST_RECORD,
		ST_PAUSE,
		STATUSES
	};
	enum {	// stop flags
		SF_CONFIRM			= 0x01,	// get user confirmation before stopping
		SF_OPEN_RECORDING	= 0x02,	// open recorded audio after stopping
		SF_CLOSE_DIALOG		= 0x04,	// close record dialog after stopping
		SF_DEFAULT			= SF_CONFIRM | SF_OPEN_RECORDING | SF_CLOSE_DIALOG,
	};
	enum {	// activation types
		ACT_PROMPT,			// prompt for output path and all parameters
		ACT_ONE_TOUCH,		// one-touch recording; no prompts
		ACT_SOUND,			// sound-activated recording
		ACTIVATION_TYPES
	};

// Attributes
	bool	IsRecording() const;
	bool	IsPaused() const;
	int		GetStatus() const;
	static	UINT	GetTimerFrequency();

// Operations
	bool	Start();
	bool	Stop(UINT Flags = SF_DEFAULT);
	bool	StopCheck();
	bool	Pause(bool Enable);
	void	Show();
	bool	OnDeviceChange();
	static	void	InitSampleRateCombo(CComboBox& Combo, UINT SampleRate);
	static	UINT	GetSampleRate(CComboBox& Combo);
	static	void	InitSampleSizeCombo(CComboBox& Combo, UINT SampleSize);
	static	UINT	GetSampleSize(CComboBox& Combo);
	static	void	ValidateSampleRate(CDataExchange *pDX, int nIDC, CComboBox& Combo, UINT& SampleRate);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CRecordDlg)
	enum { IDD = IDD_RECORD };
	CComboBox	m_SampleSizeCombo;
	CComboBox	m_SampleRateCombo;
	CStatic	m_Status;
	CStatic	m_FreeDiskSpace;
	CStatic	m_RecordingSize;
	CStatic	m_Elapsed;
	UINT	m_Channels;
	BOOL	m_SyncPlayback;
	BOOL	m_OpenRecording;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CRecordDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(W64UINT nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnKillfocusChannels();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPause();
	//}}AFX_MSG
	afx_msg void OnUpdateFormat(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatus(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStop(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePause(CCmdUI* pCmdUI);
	afx_msg LRESULT OnCaptureWrite(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCaptureError(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Types
	class CWaveDSCapture : public CDSCapture {
	public:
		bool	Start(UINT Channels, UINT SampleRate, UINT SampleBits);
		bool	Read();
		CWaveEdit	m_Wave;
	};
	struct STATUS_INFO {
		int		Name;			// string resource ID of status name
		int		PauseBtnText;	// string resource ID of pause button text
		COLORREF	BkColor;	// status pane background color
	};

// Constants
	enum {
		TIMER_ID = 1792,		// timer identifier
		TIMER_FREQUENCY = 10,	// timer frequency, in Hz
		SAMPLE_SIZE_PRESETS = 4,	// number of sample size presets
	};
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];
	static const STATUS_INFO m_StatusInfo[STATUSES];
	static const UINT	m_SampleRatePreset[];

// Data members
	CCtrlResize	m_Resize;		// control resizer
	CSize	m_InitSize;			// dialog's initial size
	CString	m_InitCaption;		// dialog's initial caption
	CMeterView	*m_MeterView;	// pointer to meter view
	bool	m_Canceled;			// true if dialog was canceled
	UINT	m_SampleRate;		// sampling rate, in Hz
	UINT	m_SampleSize;		// sample size, in bits
	CDSCapture	m_RecordCapture;	// capture instance for recording
	CWaveDSCapture	m_MeterCapture;	// capture instance for meter view
	CString	m_OutPath;			// output file path
	CString	m_OutDir;			// output directory
	CString	m_PrevFreeDiskStr;	// previous free disk space string
	CNumFormat	m_NumFormat;	// numeric formatter
	UINT	m_TriggerLen;		// trigger level duration, in milliseconds

// Helpers
	bool	OnFormatChange();
	bool	CreateDevices();
	void	LoadSettings();
	void	StoreSettings();
	void	ResizeMeterView();
	bool	UpdateMeterCapture();
	void	UpdateFreeDiskSpace();
	void	UpdateUI();
	void	OnOutputPathChange();
	static	bool	CloseDoc(LPCTSTR Path);
	static	bool	DoFileDlg(CString& Path);
	static	const RECORD_PARMS&	GetRecParms();
	static	int		GetActivationType();
};

inline bool	CRecordDlg::IsRecording() const
{
	return(m_RecordCapture.IsOpen());
}

inline bool	CRecordDlg::IsPaused() const
{
	return(!m_RecordCapture.IsCapturing());
}

inline UINT CRecordDlg::GetTimerFrequency()
{
	return(TIMER_FREQUENCY);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDDLG_H__C4563F72_D33B_404D_8331_2A1DE732DBAB__INCLUDED_)
