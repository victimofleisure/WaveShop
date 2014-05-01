// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26mar13	initial version

		device context for double-buffering

*/

#if !defined(AFX_DOUBLEBUFDC_H__8DA580FF_35B7_464A_BC56_4AF82D60B1FE__INCLUDED_)
#define AFX_DOUBLEBUFDC_H__8DA580FF_35B7_464A_BC56_4AF82D60B1FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DoubleBufDC.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDoubleBufDC window

class CDoubleBufDC : public CDC
{
// Construction
public:
	CDoubleBufDC();
	BOOL	Create(CWnd *pWnd);

// Operations
public:
	BOOL	CreateBackBuffer(int Width, int Height);
	void	DestroyBackBuffer();

// Implementation
public:
	virtual ~CDoubleBufDC();

protected:
// Member data
	CBitmap	m_BackBuf;			// back buffer
	HGDIOBJ	m_PrevBmp;			// previously selected bitmap
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOUBLEBUFDC_H__8DA580FF_35B7_464A_BC56_4AF82D60B1FE__INCLUDED_)
