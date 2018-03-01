// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      12jan13	initial version

		swap channels dialog

*/

#if !defined(AFX_SWAPCHANNELSDLG_H__F3AD9D14_253D_427E_BD16_2553E287ADC5__INCLUDED_)
#define AFX_SWAPCHANNELSDLG_H__F3AD9D14_253D_427E_BD16_2553E287ADC5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SwapChannelsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSwapChannelsDlg dialog

class CWaveProcess;

class CSwapChannelsDlg : public CDialog
{
// Construction
public:
	CSwapChannelsDlg(const CWaveProcess& Wave, CWnd* pParent = NULL);

// Attributes
	CUIntRange	GetPair() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSwapChannelsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CSwapChannelsDlg)
	enum { IDD = IDD_SWAP_CHANNELS };
	CComboBox	m_ChannelB;
	CComboBox	m_ChannelA;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CSwapChannelsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeChannel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	const CWaveProcess&	m_Wave;	// reference to wave
	CUIntRange	m_Pair;			// pair of channels to be swapped, as indices

// Helper
	CUIntRange	RetrievePair() const;
};

inline CUIntRange CSwapChannelsDlg::GetPair() const
{
	return(m_Pair);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SWAPCHANNELSDLG_H__F3AD9D14_253D_427E_BD16_2553E287ADC5__INCLUDED_)
