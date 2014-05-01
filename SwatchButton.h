// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      15feb13	initial version
		01		02apr13	add EditColor

		swatch button

*/

#if !defined(AFX_SWATCHBUTTON_H__E2FC8C22_2FF5_4CD2_B5E4_A912A9EAB77F__INCLUDED_)
#define AFX_SWATCHBUTTON_H__E2FC8C22_2FF5_4CD2_B5E4_A912A9EAB77F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SwatchButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSwatchButton window

class CSwatchButton : public CButton
{
	DECLARE_DYNAMIC(CSwatchButton)
// Construction
public:
	CSwatchButton();

// Attributes
public:
	COLORREF	GetColor() const;
	void	SetColor(COLORREF Color);

// Operations
public:
	bool	EditColor(COLORREF *CustomColors);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSwatchButton)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSwatchButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSwatchButton)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnSetState(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		SWATCH_MARGIN = 5,		// swatch margin in pixels
		PUSHED_OFFSET = 1,		// pushed offset in pixels
	};

// Member data
	CStatic	m_Swatch;			// swatch overlay
	COLORREF	m_Color;		// swatch color

// Helpers
	void	GetSwatchRect(CRect& Rect) const;
};

inline COLORREF CSwatchButton::GetColor() const
{
	return(m_Color);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SWATCHBUTTON_H__E2FC8C22_2FF5_4CD2_B5E4_A912A9EAB77F__INCLUDED_)
