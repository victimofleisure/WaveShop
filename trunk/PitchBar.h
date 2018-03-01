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

        pitch dialog bar
 
*/

#if !defined(AFX_PITCHBAR_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_)
#define AFX_PITCHBAR_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PitchBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPitchBar dialog

#include "DialogBarEx.h"
#include "EditSliderCtrl.h"
#include "CtrlResize.h"

class CPitchBar : public CDialogBarEx
{
	DECLARE_DYNAMIC(CPitchBar);
// Construction
public:
	CPitchBar(CWnd* pParent = NULL);   // standard constructor
	~CPitchBar();

// Attributes
	double	GetPitch() const;
	void	SetPitch(double Pitch);

// Operations

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPitchBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CPitchBar)
	enum { IDD = IDD_PITCH_BAR };
	CEditSliderCtrl	m_PitchSlider;
	CNumEdit	m_PitchEdit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CPitchBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChangedPitch(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateMute(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()

// Constants
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];
	static const CEditSliderCtrl::INFO	m_SliderInfo;

// Member data
	CCtrlResize	m_Resize;

// Helpers
};

inline double CPitchBar::GetPitch() const
{
	return(m_PitchEdit.GetVal());
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PITCHBAR_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_)
