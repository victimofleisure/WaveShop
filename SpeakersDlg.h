// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10jan13	initial version

		speaker assignment dialog

*/

#if !defined(AFX_SPEAKERSDLG_H__82821E4C_2477_4877_8439_EC5FB93D553C__INCLUDED_)
#define AFX_SPEAKERSDLG_H__82821E4C_2477_4877_8439_EC5FB93D553C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpeakersDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpeakersDlg dialog

class CSpeakersDlg : public CDialog
{
// Construction
public:
	CSpeakersDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes
	void	SetChannelMask(UINT Channels, UINT Mask);
	UINT	GetChannelMask() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpeakersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CSpeakersDlg)
	enum { IDD = IDD_SPEAKERS };
	CCheckListBox	m_List;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CSpeakersDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	UINT	m_ChannelCount;		// number of channels
	UINT	m_ChannelMask;		// bitmask of channel speaker assignments
};

inline UINT CSpeakersDlg::GetChannelMask() const
{
	return(m_ChannelMask);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPEAKERSDLG_H__82821E4C_2477_4877_8439_EC5FB93D553C__INCLUDED_)
