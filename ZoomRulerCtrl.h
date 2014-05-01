// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      24oct12	initial version

		zoom ruler control

*/

#if !defined(AFX_ZOOMRULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_)
#define AFX_ZOOMRULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ZoomRulerCtrl.h : header file
//

#include "DragRulerCtrl.h"
#include "RefPtr.h"

/////////////////////////////////////////////////////////////////////////////
// CZoomRulerCtrl window

class CZoomRulerCtrl : public CDragRulerCtrl
{
	DECLARE_DYNAMIC(CZoomRulerCtrl)
// Construction
public:
	CZoomRulerCtrl();

// Constants
	enum {	// marker styles
		MS_VERTICAL	= 0x01
	};

// Attributes
public:
	void	SetTarget(CWnd *Target);
	CWnd	*GetTarget();
	const CWnd	*GetTarget() const;

// Operations
public:
	virtual	void	EndDrag();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZoomRulerCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CZoomRulerCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CZoomRulerCtrl)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	class CMarker : public CWnd, public CRefObj {
	public:
		virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	};
	typedef CRefPtr<CMarker> CMarkerPtr;	// reference-counted marker pointer

// Constants
	enum {	// marker indices
		MARKER_START,
		MARKER_END,
		MARKERS
	};

// Data members
	CWnd	*m_Target;	// pointer to target window
	CMarkerPtr	m_Marker[MARKERS];	// array of marker pointers

// Overrides
	virtual	void	OnDragBegin(UINT Flags, CPoint Point);

// Overridables
	virtual	void	GetTargetRect(CRect& Rect) const;

// Helpers
	bool	CreateMarker(const CRect& Rect, CMarkerPtr& Marker);
	void	GetMarkerRect(CPoint Point, CRect& Rect);
	void	UpdateCursor(bool ZoomIn);
};

inline void CZoomRulerCtrl::SetTarget(CWnd *Target)
{
	m_Target = Target;
}

inline CWnd *CZoomRulerCtrl::GetTarget()
{
	return(m_Target);
}

inline const CWnd *CZoomRulerCtrl::GetTarget() const
{
	return(m_Target);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZOOMRULERCTRL_H__490CA5D7_29D1_4381_9849_416A9904451A__INCLUDED_)
