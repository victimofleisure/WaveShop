// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      20nov12	initial version

		toolbar-style flat icon button

*/

#if !defined(AFX_FLATICONBUTTON_H__2E60D189_D877_492E_B079_F2134D7B6D1A__INCLUDED_)
#define AFX_FLATICONBUTTON_H__2E60D189_D877_492E_B079_F2134D7B6D1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FlatIconButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFlatIconButton window

class CFlatIconButton : public CButton
{
// Construction
public:
	CFlatIconButton();

// Attributes
public:
	void	SetIcons(int ResUp, int ResDown);
	bool	GetAutoCheck() const;
	void	SetAutoCheck(bool Enable);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFlatIconButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFlatIconButton();

protected:
// Generated message map functions
	//{{AFX_MSG(CFlatIconButton)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetCheck(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetCheck(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Data members
	bool	m_Hovering;			// true if cursor is over button
	bool	m_Checked;			// true if button is checked
	bool	m_AutoCheck;		// true if emulating BS_AUTOCHECKBOX
	HICON	m_IconUp;			// icon to show when unchecked
	HICON	m_IconDown;			// icon to show when checked

// Helpers
	static	HICON	SimpleLoadIcon(int ResID);
	bool	HitTest(CPoint Point);
};

inline bool CFlatIconButton::GetAutoCheck() const
{
	return(m_AutoCheck);
}

inline void CFlatIconButton::SetAutoCheck(bool Enable)
{
	m_AutoCheck = Enable;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLATICONBUTTON_H__2E60D189_D877_492E_B079_F2134D7B6D1A__INCLUDED_)
