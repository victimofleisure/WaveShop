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
		12		06jan10	W64: cast item data to 32-bit
		13		28jan13	add GetReport
		14		02sep13	replace header image list with sort format flags

        simplified report view list control
 
*/

// ReportCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ReportCtrl.h"
#include "Persist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReportCtrl

IMPLEMENT_DYNAMIC(CReportCtrl, CListCtrl);

CReportCtrl::CReportCtrl()
{
	m_Columns = 0;
	m_Column = NULL;
	m_SortCol = -1;
	m_SortDir = 0;
	m_SortCallback = NULL;
	m_SortWaitCursor = FALSE;
	m_Style = 0;
	m_ColHeap = NULL;
}

CReportCtrl::~CReportCtrl()
{
	if (m_ColHeap != NULL)
		delete [] m_ColHeap;
}

void	CReportCtrl::SetColumns(int Columns, const COLUMN *Column)
{
	m_Columns = Columns;
	m_Column = Column;
}

void	CReportCtrl::SetColumns(int Columns, const RES_COL *Column)
{
	m_Columns = Columns;
	m_ColHeap = new COLUMN[Columns];
	m_Title.SetSize(Columns);
	for (int i = 0; i < Columns; i++) {
		m_Title[i] = LDS(Column[i].TitleID);
		m_ColHeap[i].Title		= m_Title[i];
		m_ColHeap[i].Align		= Column[i].Align;
		m_ColHeap[i].Width		= Column[i].Width;
		m_ColHeap[i].InitSort	= Column[i].InitSort;
	}
	m_Column = m_ColHeap;
}

void	CReportCtrl::SetSortCallback(PFNLVCOMPARE Callback, PVOID Data)
{
	m_SortCallback = Callback;
	m_SortCallbackData = Data;
}

void	CReportCtrl::InitControl(int Rows, int Style)
{
	while (DeleteColumn(0));	// delete any existing columns
	for (int i = 0; i < m_Columns; i++) {
		InsertColumn(i, m_Column[i].Title, 
			m_Column[i].Align, m_Column[i].Width, i);
	}
#if _MFC_VER < 0x0700	// if MFC 6
	if (Style & SORT_ARROWS) {
		// create header image list containing sort arrows
		m_HdrImgList.Create(IDB_HEADER_SORT, 8, 0, RGB(0, 128, 128));
		GetHeaderCtrl()->SetImageList(&m_HdrImgList);
	}
#endif
	m_Style = Style;
	ModifyStyle(0, LVS_REPORT);	// force report view
	ResetControl(Rows);
}

void	CReportCtrl::ResetControl(int Rows)
{
	m_SortCol = -1;
	m_SortDir = 0;
	DeleteAllItems();
	SetItemCount(Rows);	// allocate memory in advance
}

int		CReportCtrl::InsertRow(int Idx, const CStringArray& Row, int SortKey)
{
	int	pos = InsertItem(Idx, Row[0]);
	for (int i = 1; i < m_Columns; i++)
		SetItemText(pos, i, Row[i]);
	SetItemData(pos, SortKey);
	return(pos);
}

int		CReportCtrl::InsertRow(int Idx, const LPCTSTR *Row, int SortKey)
{
	int	pos = InsertItem(Idx, Row[0]);
	for (int i = 1; i < m_Columns; i++)
		SetItemText(pos, i, Row[i]);
	SetItemData(pos, SortKey);
	return(pos);
}

int		CReportCtrl::InsertCallbackRow(int Idx, int SortKey)
{
	int	pos = InsertItem(Idx, LPSTR_TEXTCALLBACK);
	for (int i = 1; i < m_Columns; i++)
		SetItemText(pos, i, LPSTR_TEXTCALLBACK);
	SetItemData(pos, SortKey);
	return(pos);
}

int	CReportCtrl::FindRow(int SortKey)
{
	LVFINDINFO	fi;
	ZeroMemory(&fi, sizeof(fi));
	fi.flags = LVFI_PARAM;
	fi.lParam = SortKey;
	return(FindItem(&fi));
}

void	CReportCtrl::AddRow(int SortKey)
{
	int	row = GetItemCount();
	InsertCallbackRow(row, SortKey);
	SortRows();
}

bool	CReportCtrl::RemoveRow(int SortKey)
{
	int	row = FindRow(SortKey);
	if (row < 0)
		return(FALSE);
	DeleteItem(row);
	int	items = GetItemCount();
	// items with sort keys >= the deleted item's need updating
	for (int i = 0; i < items; i++) { // for each item
		int	sk = INT64TO32(GetItemData(i));	// get item's sort key
		if (sk >= SortKey)	// if sort key is invalid
			SetItemData(i, sk - 1);	// update it
	}
	return(TRUE);
}

bool	CReportCtrl::RedrawRow(int SortKey)
{
	int	row = FindRow(SortKey);
	if (row < 0)
		return(FALSE);
	RedrawItems(row, row);
	return(TRUE);
}

int	CReportCtrl::TextSort(int p1, int p2)
{
	LVFINDINFO info;
	info.flags = LVFI_PARAM;
	info.lParam = p1;
	int nItem1 = FindItem(&info);
	info.lParam = p2;
	int nItem2 = FindItem(&info);
	return(SortCmp(GetItemText(nItem1, m_SortCol), GetItemText(nItem2, m_SortCol)));
}

int CALLBACK CReportCtrl::TextSort(LPARAM p1, LPARAM p2, LPARAM This)
{
	return(((CReportCtrl *)This)->TextSort(INT64TO32(p1), INT64TO32(p2)));
}

void	CReportCtrl::DrawSortArrow(int Col, int Desc)
{
	if (Col < 0)
		return;
#if _MFC_VER < 0x0700	// if MFC 6
	HDITEM	hdi;
	hdi.mask = HDI_IMAGE | HDI_FORMAT;
	GetHeaderCtrl()->GetItem(Col, &hdi);
	if (Desc < 0) {	// erase
		hdi.mask = HDI_FORMAT;
		hdi.fmt &= ~HDF_IMAGE;
	} else {
		hdi.mask = HDI_FORMAT | HDI_IMAGE;
		hdi.fmt |= HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
		hdi.iImage = Desc;
	}
#else	// .NET; use sort header format flags instead of image list
	HDITEM	hdi;
	hdi.mask = HDI_FORMAT;
	GetHeaderCtrl()->GetItem(Col, &hdi);
	hdi.mask = HDI_FORMAT;
	if (Desc < 0)	// erase
		hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
	else {
		if (Desc)
			hdi.fmt |= HDF_SORTDOWN;
		else
			hdi.fmt |= HDF_SORTUP;
	}
#endif
	GetHeaderCtrl()->SetItem(Col, &hdi);
}

void	CReportCtrl::SortRows()
{
	if (m_SortWaitCursor)
		CCmdTarget::BeginWaitCursor();
	if (m_SortCallback != NULL)		// is there a sort callback?
		SortItems(m_SortCallback, (DWORD)m_SortCallbackData);	// y, use it
	else
		SortItems(TextSort, (DWORD)this);	// n, use text sort
	if (m_SortWaitCursor)
		CCmdTarget::EndWaitCursor();
	// if there's a selection, keep it visible, otherwise go to top
	EnsureVisible(GetSelectedCount() ? GetSelectionMark() : 0, FALSE);
}

void	CReportCtrl::SortRows(int SortCol, int SortDir)
{
	if (m_Style & SORT_ARROWS)
		DrawSortArrow(m_SortCol, -1);	// erase previous column's arrow
	if (m_Style & SORT_ARROWS)
		DrawSortArrow(SortCol, SortDir < 0);	// draw new arrow
	m_SortDir = SortDir;
	m_SortCol = SortCol;
	SortRows();
}

void	CReportCtrl::SortRows(int SortCol)
{
	if (SortCol == m_SortCol)		// same column as last time?
		m_SortDir = -m_SortDir;					// y, reverse sort order
	else
		m_SortDir = m_Column[SortCol].InitSort;	// n, set default sort order
	SortRows(SortCol, m_SortDir);
}

void	CReportCtrl::EnsureSelectionVisible()
{
	if (GetSelectedCount())
		EnsureVisible(GetSelectionMark(), FALSE);
}

inline int CReportCtrl::CalcHeaderStateSize(int Columns)
{
	return(sizeof(HEADER_STATE) + ((Columns * 2) - 1) * sizeof(int));
}

bool CReportCtrl::StoreHeaderState(LPCTSTR Key, LPCTSTR SubKey)
{
	DWORD	hsz = CalcHeaderStateSize(m_Columns);
	HEADER_STATE	*ph = (HEADER_STATE *)new BYTE[hsz];
	ph->Columns = m_Columns;
	ph->SortCol = SortCol();
	ph->SortDir = SortDir();
	int	cols = m_Columns;
	int	*pColWidth = &ph->ColInfo[cols];
	for (int i = 0; i < cols; i++)
		pColWidth[i] = GetColumnWidth(i);
	GetColumnOrderArray(ph->ColInfo, cols);
	bool	retc = CPersist::WriteBinary(Key, SubKey, ph, hsz) != 0;
	delete ph;
	return(retc);
}

bool CReportCtrl::LoadHeaderState(LPCTSTR Key, LPCTSTR SubKey)
{
	DWORD	hsz = CalcHeaderStateSize(m_Columns);
	HEADER_STATE	*ph = (HEADER_STATE *)new BYTE[hsz];
	ZeroMemory(ph, hsz);
	bool	retc = CPersist::GetBinary(Key, SubKey, ph, &hsz) != 0;
	if (retc) {
		SortRows(ph->SortCol, ph->SortDir);
		int	cols = ph->Columns;
		int	*pColWidth = &ph->ColInfo[cols];
		for (int i = 0; i < cols; i++)
			SetColumnWidth(i, pColWidth[i]);
		SetColumnOrderArray(cols, ph->ColInfo);
	}
	delete ph;
	return(retc);
}

void CReportCtrl::ResetHeaderState()
{
	int	cols = m_Columns;
	int	*pColOrder = new int[cols];
	for (int i = 0; i < cols; i++) {
		SetColumnWidth(i, m_Column[i].Width);
		pColOrder[i] = i;
	}
	SetColumnOrderArray(cols, pColOrder);
	delete pColOrder;
	Invalidate();
}

CString CReportCtrl::GetReport() const
{
	int	nRows = GetItemCount();
	int	nCols = m_Columns;
	CString	report;
	for (int iRow = 0; iRow < nRows; iRow++) {
		for (int iCol = 0; iCol < nCols; iCol++) {
			if (iCol)
				report += '\t';
			report += GetItemText(iRow, iCol);
		}
		report += '\n';
	}
	return(report);
}

BEGIN_MESSAGE_MAP(CReportCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CReportCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_WM_VSCROLL()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReportCtrl message handlers

void CReportCtrl::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int	Col = pNMListView->iSubItem;
	SortRows(Col);
	*pResult = 0;
}

void CReportCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	switch (nSBCode) {
	case SB_PAGEDOWN:
	case SB_PAGEUP:
	case SB_LINEDOWN:
	case SB_LINEUP:
		// Don't update window if paging up or down. It looks terrible!
		LockWindowUpdate();
		CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
		UnlockWindowUpdate();
		break;
	default:
		CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
		break;
	}
}

void CReportCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar) {
	case VK_NEXT:
	case VK_PRIOR:
		// Don't update window if paging up or down. It looks terrible!
		LockWindowUpdate();
		CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
		UnlockWindowUpdate();
		break;
	default:
		CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}
