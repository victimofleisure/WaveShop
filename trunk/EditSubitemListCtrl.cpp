// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26feb13	initial version

		list control with subitem editing
 
*/

// EditSubitemListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "EditSubitemListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditSubitemListCtrl

IMPLEMENT_DYNAMIC(CEditSubitemListCtrl, CListCtrl);

CEditSubitemListCtrl::CEditSubitemListCtrl()
{
	m_SubRow = 0;
	m_SubCol = 0;
}

CEditSubitemListCtrl::~CEditSubitemListCtrl()
{
}

bool CEditSubitemListCtrl::EditSubitem(int Row, int Col)
{
	ASSERT(Row >= 0 && Row < GetItemCount());	// validate row
	ASSERT(Col >= 1 && Col < GetHeaderCtrl()->GetItemCount());	// subitems only
	EndEdit();	// end previous edit if any
	EnsureVisible(Row, FALSE);	// make sure specified row is fully visible
	POSITION	pos = GetFirstSelectedItemPosition();
	int	StateMask = LVIS_FOCUSED | LVIS_SELECTED;
	while (pos != NULL)	// deselect all items
		SetItemState(GetNextSelectedItem(pos), 0, StateMask);
	SetItemState(Row, StateMask, StateMask);	// select specified row
	SetSelectionMark(Row);	// set selection mark too
	// clip siblings is mandatory, else edit control overwrites header control
	UINT	style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS 
		| ES_AUTOHSCROLL;	// allow text entry to exceed column width
	CRect	rSubitem;
	GetSubItemRect(Row, Col, LVIR_BOUNDS, rSubitem);	// get subitem rect
	if (!m_SubEdit.Create(style, rSubitem, this, 0))	// create edit control
		return(FALSE);
	m_SubEdit.SetFont(GetFont());	// set edit control's font same as ours
	CString	s(GetItemText(Row, Col));	// get subitem text
	m_SubEdit.SetWindowText(s);	// set edit control to subitem text
	m_SubEdit.SetSel(0, -1);	// select entire text
	m_SubEdit.SetFocus();	// focus edit control
	m_SubRow = Row;	// update our members
	m_SubCol = Col;
	return(TRUE);
}

void CEditSubitemListCtrl::EndEdit()
{
	if (IsEditing())
		SetFocus();	// call OnEndEdit indirectly by taking focus
}

void CEditSubitemListCtrl::CancelEdit()
{
	if (IsEditing()) {
		m_SubEdit.SetModify(FALSE);
		EndEdit();
	}
}

bool CEditSubitemListCtrl::GotoSubitem(int DeltaRow, int DeltaCol)
{
	ASSERT(abs(DeltaRow) <= 1);	// valid delta range is [-1..1]
	ASSERT(abs(DeltaCol) <= 1);
	ASSERT(IsEditing());
	int	cols = GetHeaderCtrl()->GetItemCount();
	int	col = m_SubCol + DeltaCol;
	if (col >= cols) {	// if after last column
		col = 1;	// wrap to first column
		DeltaRow = 1;	// next row
	} else if (col < 1) {	// if before first column
		col = cols - 1;	// wrap to last column
		DeltaRow = -1;	// previous row
	}
	int	rows = GetItemCount();
	int	row = m_SubRow + DeltaRow;
	if (row >= rows) {	// if after last row
		row = 0;	// wrap to first row
	} else if (row < 0) {	// if before first row
		row = rows - 1;	// wrap to last row
	}
	return(EditSubitem(row, col));
}

LRESULT CEditSubitemListCtrl::CPopupEdit::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_KILLFOCUS:
		GetParent()->SendMessage(UWM_END_EDIT);
		break;
	}
	return CEdit::WindowProc(message, wParam, lParam);
}

BEGIN_MESSAGE_MAP(CEditSubitemListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CEditSubitemListCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_PARENTNOTIFY()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_END_EDIT, OnEndEdit)
	ON_MESSAGE(UWM_UPDATE_POS, OnUpdatePos)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditSubitemListCtrl message handlers

void CEditSubitemListCtrl::PreSubclassWindow() 
{
	// clip children is mandatory, else edit control disappears during scrolling
	ModifyStyle(0, WS_CLIPCHILDREN);
	CListCtrl::PreSubclassWindow();
}

void CEditSubitemListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	LVHITTESTINFO	info;
	info.pt = point;
	info.flags = UINT_MAX;
	int	item = SubItemHitTest(&info);
	if (item >= 0 && info.iSubItem > 0) {	// if clicked on a subitem
		EditSubitem(item, info.iSubItem);	// edit subitem
	} else	// not on a subitem
		CListCtrl::OnLButtonDown(nFlags, point);	// delegate to base class
}

BOOL CEditSubitemListCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if (IsEditing()) {
		if (pMsg->message == WM_KEYDOWN) {
			switch (pMsg->wParam) {
			case VK_UP:
				GotoSubitem(-1, 0);
				return(TRUE);
			case VK_DOWN:
				GotoSubitem(1, 0);
				return(TRUE);
			case VK_TAB:
				{
					int	DeltaCol = (GetKeyState(VK_SHIFT) & GKS_DOWN) ? -1 : 1;
					GotoSubitem(0, DeltaCol);
				}
				return(TRUE);
			case VK_RETURN:
				EndEdit();
				return(TRUE);
			case VK_ESCAPE:
				CancelEdit();
				return(TRUE);
			}
		}
	}
	return CListCtrl::PreTranslateMessage(pMsg);
}

void CEditSubitemListCtrl::OnParentNotify(UINT message, LPARAM lParam) 
{
	CListCtrl::OnParentNotify(message, lParam);
	// the following ensures left-clicking in header control ends edit
	if (IsEditing() && message == WM_LBUTTONDOWN) {
		CPoint	pt;
		POINTSTOPOINT(pt, lParam);
		CRect	rEdit;
		m_SubEdit.GetWindowRect(rEdit);
		ScreenToClient(rEdit);
		if (!rEdit.PtInRect(pt))	// if clicked outside of edit rect
			EndEdit();
	}
}

LRESULT CEditSubitemListCtrl::OnEndEdit(WPARAM wParam, LPARAM lParam)
{
	if (IsEditing()) {
		if (m_SubEdit.GetModify()) {	// if text was modified
			CString	s;
			m_SubEdit.GetWindowText(s);
			SetItemText(m_SubRow, m_SubCol, s);	// update subitem
		}
		m_SubEdit.DestroyWindow();
	}
	return(0);
}

LRESULT CEditSubitemListCtrl::OnUpdatePos(WPARAM wParam, LPARAM lParam)
{
	if (IsEditing()) {
		CRect	rSubitem;
		GetSubItemRect(m_SubRow, m_SubCol, LVIR_BOUNDS, rSubitem);
		m_SubEdit.MoveWindow(rSubitem);
	}
	return(0);
}

void CEditSubitemListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
	if (IsEditing())
		PostMessage(UWM_UPDATE_POS);	// post lets us finish updating first
}

void CEditSubitemListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
	if (IsEditing())
		PostMessage(UWM_UPDATE_POS);	// post lets us finish updating first
}
