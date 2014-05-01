// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28mar13	initial version

		data tooltip control

*/

#if !defined(AFX_DATATIPCTRL_H__8DA580FF_35B7_464A_BC56_4AF82D60B1FE__INCLUDED_)
#define AFX_DATATIPCTRL_H__8DA580FF_35B7_464A_BC56_4AF82D60B1FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DataTipCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDataTipCtrl window

class DPoint;

class CDataTipCtrl : public CToolTipCtrl {
	DECLARE_DYNAMIC(CDataTipCtrl)
// Construction
public:
	CDataTipCtrl();

// Attributes
	CPoint	m_Pos;				// tip position, in client coords
	CPoint	m_DataPos;			// data position, in user-defined coords
	int		m_Epsilon;			// show tip within this proximity to data point
	CString	m_Text;				// tip text

// Operations
	void	OnMouseMove(MSG *pMsg);
	bool	PointsNear(const CPoint& P1, const CPoint& P2) const;
	virtual	bool	FindPoint(CPoint Target, CPoint& DataPos) const = 0;
	static	bool	PointsNear(const CPoint& P1, const CPoint& P2, int Epsilon);
	static	double	DistancePointToLine(const DPoint& Pt, const DPoint& P1, const DPoint& P2);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataTipCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDataTipCtrl();

// Generated message map functions
protected:
	//{{AFX_MSG(CDataTipCtrl)
	afx_msg void OnTimer(W64UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

inline bool CDataTipCtrl::PointsNear(const CPoint& P1, const CPoint& P2, int Epsilon)
{
	return(abs(P1.x - P2.x) <= Epsilon && abs(P1.y - P2.y) <= Epsilon);
}

inline bool CDataTipCtrl::PointsNear(const CPoint& P1, const CPoint& P2) const
{
	return(PointsNear(P1, P2, m_Epsilon));
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATATIPCTRL_H__8DA580FF_35B7_464A_BC56_4AF82D60B1FE__INCLUDED_)
