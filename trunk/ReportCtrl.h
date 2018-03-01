// Copyleft 2003 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21apr03 initial version
		01		07may03	add reset control and callback rows
		02		08may03	fix crappy page up/down scrolling
		03		28aug03	add optional wait cursor during sort
		04		02feb05	add sort arrows
		05		07feb05	add SetColumns overload for resource strings
		06		29nov07	support Unicode
		07		30nov07	make sort arrow icon instead of bitmap
		08		19jun08	overload SortRows to resort without changing sort type
		09		23jun09	add FindRow, AddRow, RemoveRow, RedrawRow
		10		23jun09	add SortRows overload with sort direction argument
		11		11dec09	add store/load header state
		12		28jan13	add GetReport
		13		04mar13	make SortRows virtual
		14		02sep13	replace header image list with sort format flags

        simplified report view list control
 
*/

#if !defined(AFX_REPORTCTRL_H__84A05433_DB70_434F_A8BB_7D0BC8C17B63__INCLUDED_)
#define AFX_REPORTCTRL_H__84A05433_DB70_434F_A8BB_7D0BC8C17B63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReportCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReportCtrl window

class CReportCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CReportCtrl);
// Types
public:
	enum DIR {
		DIR_ASC		= 1,
		DIR_DESC	= -1
	};
	enum STYLE {
		SORT_ARROWS	= 0x01
	};
	typedef struct {
		LPCTSTR	Title;
		int		Align;
		int		Width;
		DIR		InitSort;
	} COLUMN;
	typedef struct {
		int		TitleID;
		int		Align;
		int		Width;
		DIR		InitSort;
	} RES_COL;

// Construction
public:
	CReportCtrl();

// Attributes
	void	SetColumns(int Columns, const COLUMN *Column);
	void	SetColumns(int Columns, const RES_COL *Column);
	void	SetSortCallback(PFNLVCOMPARE Callback, PVOID Data);
	CString GetReport() const;

// Operations
	void	InitControl(int Rows, int Style = 0);
	void	ResetControl(int Rows);
	int		InsertRow(int Idx, const CStringArray& Row, int SortKey);
	int		InsertRow(int Idx, const LPCTSTR *Row, int SortKey);
	int		InsertCallbackRow(int Idx, int SortKey);
	int		FindRow(int SortKey);
	void	AddRow(int SortKey);
	bool	RemoveRow(int SortKey);
	bool	RedrawRow(int SortKey);
	void	SortRows(int SortCol);
	void	SortRows(int SortCol, int SortDir);
	virtual	void	SortRows();
	int		SortCol() const;
	int		SortDir() const;
	void	EnsureSelectionVisible();
	void	SortWaitCursor(bool Enable);
	// template class for sorting
	template<class T> int SortCmp(const T& a, const T& b) const
		{ return(a == b ? 0 : (a < b ? -m_SortDir : m_SortDir)); }
	int SortCmp(LPCTSTR a, LPCTSTR b) const
		{ return(_tcscmp(a, b) * m_SortDir); }
	bool	StoreHeaderState(LPCTSTR Key, LPCTSTR SubKey);
	bool	LoadHeaderState(LPCTSTR Key, LPCTSTR SubKey);
	void	ResetHeaderState();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReportCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CReportCtrl();

protected:
// Generated message map functions
	//{{AFX_MSG(CReportCtrl)
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	struct tagHEADER_STATE;
	typedef struct tagHEADER_STATE {	// variable size
		int		Columns;			// # of columns
		int		SortCol;			// sort column
		int		SortDir;			// 0 = ascending, 1 = descending
		int		ColInfo[1];			// column order and column width arrays
	} HEADER_STATE;

// Member data
	const	COLUMN	*m_Column;
	int		m_Columns;
	int		m_SortCol;
	int		m_SortDir;
	PFNLVCOMPARE	m_SortCallback;
	PVOID	m_SortCallbackData;
	bool	m_SortWaitCursor;
	int		m_Style;
	CStringArray	m_Title;
	COLUMN	*m_ColHeap;
#if _MFC_VER < 0x0700	// if MFC 6
	CImageList	m_HdrImgList;
#endif

// Helpers
	static int CALLBACK TextSort(LPARAM p1, LPARAM p2, LPARAM This);
	int		TextSort(int p1, int p2);
	void	DrawSortArrow(int Col, int Desc);
	static	int		CalcHeaderStateSize(int Columns);
};

inline	int		CReportCtrl::SortCol() const
{
	return(m_SortCol);
}

inline	int		CReportCtrl::SortDir() const
{
	return(m_SortDir);
}

inline	void	CReportCtrl::SortWaitCursor(bool Enable)
{
	m_SortWaitCursor = Enable;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REPORTCTRL_H__84A05433_DB70_434F_A8BB_7D0BC8C17B63__INCLUDED_)
