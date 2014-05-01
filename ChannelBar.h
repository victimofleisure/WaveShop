// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version
        01      23mar13	add minimum minor tick gap

		channel control bar

*/

#if !defined(AFX_CHANNELBAR_H__4BA66D6B_9B2D_4D4F_B604_F9B08D2CD5A9__INCLUDED_)
#define AFX_CHANNELBAR_H__4BA66D6B_9B2D_4D4F_B604_F9B08D2CD5A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChannelBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChannelBar window

#include "ChannelRulerCtrl.h"
#include "ArrayEx.h"

class CWaveShopView;

class CChannelBar : public CControlBar
{
	DECLARE_DYNAMIC(CChannelBar);
// Construction
public:
	CChannelBar();
	BOOL	Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);

// Attributes
public:
	void	SetView(CWaveShopView *View);
	int		GetChannelCount() const;
	bool	SetChannelCount(int Channels);
	void	SetChannelPos(int ChannelIdx, int y, int Height);
	void	SetAmplitude(int ChannelIdx, double Origin, double Amplitude);
	bool	WidthPending() const;
	int		FindRuler(const CRulerCtrl *Ruler) const;
	static	double	GetNominalZoom();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChannelBar)
	//}}AFX_VIRTUAL
	virtual void	OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual CSize	CalcFixedLayout(BOOL bStretch, BOOL bHorz);

// Implementation
public:
	virtual ~CChannelBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CChannelBar)
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT	OnUpdateWidth(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		NOMINAL_ZOOM = -2		// reverse ruler (bottom to top in client coords)
	};

// Types
	typedef CArrayEx<CChannelRulerCtrl*, CChannelRulerCtrl*&> CRulerPtrArray;

// Data members
	CRulerPtrArray	m_Ruler;	// array of pointers to ruler windows
	CWaveShopView	*m_View;	// pointer to wave view
	int		m_Width;			// bar width in client coords
	int		m_ViewOffset;		// vertical offset of view in bar coords
	int		m_MinMajorTickGap;	// minimum gap between major ticks, in pixels
	int		m_MinMinorTickGap;	// minimum gap between minor ticks, in pixels
	bool	m_WidthPending;		// true if update width message is pending in queue

// Helpers
	void	UpdateWidth();
	int		CalcTextWidth() const;
	void	OnChannelPosChange();
};

inline void CChannelBar::SetView(CWaveShopView *View)
{
	m_View = View;
}

inline int CChannelBar::GetChannelCount() const
{
	return(m_Ruler.GetSize());
}

inline bool CChannelBar::WidthPending() const
{
	return(m_WidthPending);
}

inline double CChannelBar::GetNominalZoom()
{
	return(NOMINAL_ZOOM);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANNELBAR_H__4BA66D6B_9B2D_4D4F_B604_F9B08D2CD5A9__INCLUDED_)
