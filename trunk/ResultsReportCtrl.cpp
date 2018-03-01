// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14jan13	initial version
        01      29jan13	defer updating items to OnUpdateItems
		02		04mar13 refactor for owner data

        derived report control for displaying audio processing results
 
*/

// ResultsReportCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "ResultsReportCtrl.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResultsReportCtrl

IMPLEMENT_DYNAMIC(CResultsReportCtrl, CReportCtrl);

const CResultsReportCtrl::RES_COL	CResultsReportCtrl::m_ClipColInfo[CLIP_COLS] = {
	{IDS_CLIP_COL_START,	LVCFMT_RIGHT,	100,	DIR_ASC},
	{IDS_CLIP_COL_LENGTH,	LVCFMT_RIGHT,	60,		DIR_ASC},
	{IDS_CLIP_COL_CHANNEL,	LVCFMT_RIGHT,	60,		DIR_ASC},
};

CResultsReportCtrl	*CResultsReportCtrl::m_This;

CResultsReportCtrl::CResultsReportCtrl()
{
	m_View = NULL;
	m_SortWaitCursor = TRUE;
}

void CResultsReportCtrl::Update(CWaveProcess::CClipSpanArray& ClipSpan, CWaveShopView *View)
{
	m_ClipSpan.Swap(ClipSpan);	// take ownership of clip spans via pointer swap
	m_View = View;	// set our owner view
	// defer updating report items until after bar layout is recalculated, 
	// to avoid calling view's EnsureVisible before view's size is updated;
	// also avoids report unexpectedly scrolling vertically by a few items
	PostMessage(UWM_UPDATEITEMS);
}

BOOL CResultsReportCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.dwExStyle |= WS_EX_CLIENTEDGE;	// add 3D edge to blend with host bar
	if (!CReportCtrl::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}

void CResultsReportCtrl::OnItemSelection(int ItemIdx)
{
	if (m_View != NULL) {	// should always be true but check anyway
		CChildFrame	*Frame = STATIC_DOWNCAST(CChildFrame, m_View->GetParentFrame());
		Frame->MDIActivate();	// activate view
		int	iSortedItem = m_SortIdx[ItemIdx];
		const CWaveProcess::CLIP_SPAN&	span = m_ClipSpan[iSortedItem];
		double	fStart = double(span.Start);
		theApp.GetMain()->SetNow(fStart, TRUE);
	}
}

#define SORT_CMP(x) retc = SortCmp(srv1.x, srv2.x);

int CResultsReportCtrl::SortCompare(const int *p1, const int *p2) const
{
	const CWaveProcess::CLIP_SPAN&	srv1 = m_ClipSpan[*p1];
	const CWaveProcess::CLIP_SPAN&	srv2 = m_ClipSpan[*p2];
	int	retc;
	switch (SortCol()) {
	case CLIP_COL_START:
		SORT_CMP(Start);
		if (!retc)
			SORT_CMP(Channel);
		break;
	case CLIP_COL_LENGTH:
		SORT_CMP(Length);
		if (!retc)
			SORT_CMP(Start);
		break;
	case CLIP_COL_CHANNEL:
		SORT_CMP(Channel);
		if (!retc)
			SORT_CMP(Start);
		break;
	default:
		NODEFAULTCASE;	// logic error
		retc = 0;
	}
	return(retc);
}

int CResultsReportCtrl::SortCompare(const void *p1, const void *p2)
{
	return(m_This->SortCompare((const int *)p1, (const int *)p2));
}

void CResultsReportCtrl::SortRows()
{
	if (m_SortIdx.GetSize() <= 0)
		return;	// nothing to do
	CWaitCursor	wc;	// sort can be slow
	int	sel = GetSelectionMark();
	if (sel >= 0)	// if an item is selected
		sel = m_SortIdx[sel];	// save its sorted index
	m_This = this;	// pass our instance to SortCompare
	qsort(m_SortIdx.GetData(), m_SortIdx.GetSize(), sizeof(int), SortCompare);
	Invalidate();
	if (sel >= 0)	// if an item was selected
		sel = FindItem(sel);	// find its sorted index
	else
		sel = 0;
	SelectItem(sel);	// reselect item
}

int CResultsReportCtrl::FindItem(int ItemIdx) const
{
	int	items = m_ClipSpan.GetSize();
	for (int iItem = 0; iItem < items; iItem++) {
		if (m_SortIdx[iItem] == ItemIdx)
			return(iItem);
	}
	return(-1);
}

void CResultsReportCtrl::SelectItem(int ItemIdx)
{
	SetSelectionMark(ItemIdx);
	int	mask = LVIS_FOCUSED | LVIS_SELECTED;
	SetItemState(ItemIdx, mask, mask);
	EnsureVisible(ItemIdx, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CResultsReportCtrl message map

BEGIN_MESSAGE_MAP(CResultsReportCtrl, CReportCtrl)
	//{{AFX_MSG_MAP(CResultsReportCtrl)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemchanged)
	ON_WM_SETFOCUS()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_UPDATEITEMS, OnUpdateItems)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResultsReportCtrl message handlers

int CResultsReportCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CReportCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(GetStyle() & LVS_OWNERDATA);	// must be owner data
	// set columns and init control
	SetColumns(CLIP_COLS, m_ClipColInfo);
	InitControl(0, CReportCtrl::SORT_ARROWS);
	SetExtendedStyle(LVS_EX_FULLROWSELECT);
	CReportCtrl::SortRows(CLIP_COL_START);	// set initial sort column

	return 0;
}

LRESULT	CResultsReportCtrl::OnUpdateItems(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor	wc;
	int	items = m_ClipSpan.GetSize();
	m_SortIdx.SetSize(items);
	for (int iItem = 0; iItem < items; iItem++)
		m_SortIdx[iItem] = iItem;
	SetItemCountEx(items, 0);	// set virtual item count
	SortRows();
	if (items)
		SelectItem(0);	// select first item
	return(0);
}

void CResultsReportCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	LVITEM&	item = pDispInfo->item;
	int	iSortedItem = m_SortIdx[item.iItem];
	const CWaveProcess::CLIP_SPAN&	span = m_ClipSpan[iSortedItem];
	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
		case CLIP_COL_START:
			_tcsncpy(item.pszText, 
				theApp.GetMain()->GetNavBar().FrameToStr(double(span.Start)),
				item.cchTextMax);
			break;
		case CLIP_COL_LENGTH:
			_sntprintf(item.pszText, item.cchTextMax, _T("%I64d"), LONGLONG(span.Length));
			break;
		case CLIP_COL_CHANNEL:
			_sntprintf(item.pszText, item.cchTextMax, _T("%d"), span.Channel);
			break;
		default:
			NODEFAULTCASE;
		}
	}
	*pResult = 0;
}

void CResultsReportCtrl::OnItemchanged(NMHDR* pNMHDR, LRESULT* result)
{
	NMLISTVIEW	*lpnmlv = (LPNMLISTVIEW)pNMHDR;
	if (!lpnmlv->iSubItem && (lpnmlv->uChanged & LVIF_STATE) 
	&& (lpnmlv->uNewState & LVIS_SELECTED)) {
		OnItemSelection(lpnmlv->iItem);
	}
}

void CResultsReportCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CReportCtrl::OnSetFocus(pOldWnd);
	int	iItem = GetSelectionMark();
	if (iItem >= 0)
		OnItemSelection(iItem);
}

BOOL CResultsReportCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (nFlags & MK_CONTROL && m_View != NULL) {
		m_View->StepZoom(m_View->GetNowX(), zDelta > 0);	// delegate to view
		return 0;
	}
	return CReportCtrl::OnMouseWheel(nFlags, zDelta, pt);
}
