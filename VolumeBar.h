// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16nov12	initial version
		01		12jul13	remove main frame member pointer

        volume dialog bar
 
*/

#if !defined(AFX_VOLUMEBAR_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_)
#define AFX_VOLUMEBAR_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VolumeBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVolumeBar dialog

#include "DialogBarEx.h"
#include "ClickSliderCtrl.h"
#include "FlatIconButton.h"
#include "CtrlResize.h"

class CVolumeBar : public CDialogBarEx
{
	DECLARE_DYNAMIC(CVolumeBar);
// Construction
public:
	CVolumeBar(CWnd* pParent = NULL);   // standard constructor
	~CVolumeBar();

// Attributes
	double	GetVolume() const;
	void	SetVolume(double Volume);

// Operations

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVolumeBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CVolumeBar)
	enum { IDD = IDD_VOLUME_BAR };
	CFlatIconButton	m_MuteBtn;
	CClickSliderCtrl	m_VolumeSlider;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CVolumeBar)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVolumeMute();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateMute(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		VOLUME_STEPS = 100
	};
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];

// Member data
	double	m_Volume;			// current volume
	bool	m_Mute;				// true if muted
	CCtrlResize	m_Resize;

// Helpers
	void	UpdatePlayer();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VOLUMEBAR_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_)
