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

// DoubleBufDC.cpp : implementation file
//

#include "stdafx.h"
#include "DoubleBufDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDoubleBufDC

CDoubleBufDC::CDoubleBufDC()
{
	m_PrevBmp = NULL;
}

CDoubleBufDC::~CDoubleBufDC()
{
	DestroyBackBuffer();
}

BOOL CDoubleBufDC::Create(CWnd *pWnd)
{
	CClientDC	dc(pWnd);
	return(CreateCompatibleDC(&dc));
}

BOOL CDoubleBufDC::CreateBackBuffer(int Width, int Height)
{
	DestroyBackBuffer();
	CClientDC	dc(GetWindow());
	if (!m_BackBuf.CreateCompatibleBitmap(&dc, Width, Height))
		return(FALSE);	// can't create back buffer
	m_PrevBmp = SelectObject(m_BackBuf);
	return(TRUE);
}

void CDoubleBufDC::DestroyBackBuffer()
{
	if (m_PrevBmp != NULL) {	// if previous back buffer exists
		SelectObject(m_PrevBmp);
		m_BackBuf.DeleteObject();	// delete previous back buffer
		m_PrevBmp = NULL;
	}
}
