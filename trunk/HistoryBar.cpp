// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		11feb13	initial version
		01		01mar13	in SetUndoPos, add dual progress
		02		29mar13	in OnSize, test for valid size
		03		28jun13	get undo title from view's undoable instance
		04		02sep13	replace position icon with bitmap

        history bar
 
*/

// HistoryBar.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "HistoryBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHistoryBar

IMPLEMENT_DYNAMIC(CHistoryBar, CMySizingControlBar);

CHistoryBar::CHistoryBar()
{
	m_szVert = CSize(DEF_VERT_DOCK_WIDTH, 0);	// default width when vertically docked
	m_ProgressDlg = NULL;
	m_ProgressPos = 0;
}

CHistoryBar::~CHistoryBar()
{
}

void CHistoryBar::UpdateList()
{
	int	items;
	CUndoManager	*UndoMgr = theApp.GetMain()->GetUndoManager();
	if (UndoMgr != NULL) {	// if active document exists
		items = UndoMgr->GetSize() + 1;	// add one extra item for initial state
		if (m_ProgressDlg != NULL) {	// if we're undoing or redoing
			m_ProgressPos++;
			m_ProgressDlg->SetPos(m_ProgressPos);	// update progress
			if (m_ProgressDlg->Canceled())	// if progress was canceled
				AfxThrowUserException();
		}
	} else	// no documents
		items = 0;
	m_List.SetItemCountEx(items, 0);
}

bool CHistoryBar::SetUndoPos(int Pos)
{
	CUndoManager	*UndoMgr = theApp.GetMain()->GetUndoManager();
	if (UndoMgr == NULL)	// if no documents
		return(FALSE);
	int	CurPos = UndoMgr->GetPos();
	ASSERT(Pos >= 0 && Pos <= UndoMgr->GetSize());
	if (Pos == CurPos)	// if undo position unchanged
		return(TRUE);	// nothing to do
	CProgressDlgEx	ProgDlg(IDD_PROGRESS_DUAL);	// allow nested progress
	if (!ProgDlg.Create())	// create progress dialog
		AfxThrowResourceException();
	int	CaptionID = Pos < CurPos ? IDS_HIST_UNDOING : IDS_HIST_REDOING;
	ProgDlg.SetWindowText(LDS(CaptionID));	// set appropriate caption
	ProgDlg.SetRange(0, abs(Pos - CurPos));	// set upper range to position delta
	m_ProgressPos = 0;
	m_ProgressDlg = &ProgDlg;	// derived progress dialog's dtor resets pointer
	UndoMgr->SetPos(Pos);	// undo or redo to selected history state
	return(TRUE);
}

LRESULT CHistoryBar::CListCtrlEx::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_GETDLGCODE)
		return(DLGC_WANTALLKEYS);	// enable NM_RETURN notification
	return(CListCtrl::WindowProc(message, wParam, lParam));	// delegate to base class
}

CHistoryBar::CProgressDlgEx::~CProgressDlgEx()
{
	// reset history bar's progress dialog pointer, even if SetUndoPos threw
	theApp.GetMain()->GetHistoryBar().m_ProgressDlg = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CHistoryBar message map

BEGIN_MESSAGE_MAP(CHistoryBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CHistoryBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_HISTORY_LIST, OnGetdispinfoList)
	ON_NOTIFY(NM_DBLCLK, IDC_HISTORY_LIST, OnDblclkList)
	ON_NOTIFY(NM_RETURN, IDC_HISTORY_LIST, OnDblclkList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHistoryBar message handlers

int CHistoryBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	UINT	style = WS_CHILD | WS_VISIBLE | LVS_REPORT 
		| LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_OWNERDATA;
	if (!m_List.Create(style, CRect(0, 0, 0, 0), this, IDC_HISTORY_LIST))
		return -1;
	int	width = DEF_VERT_DOCK_WIDTH - ((GetSystemMetrics(SM_CXFRAME) << 1) + 2);
	m_List.InsertColumn(0, LDS(IDS_HIST_COL_ACTION), LVCFMT_LEFT, width);
	if (!m_StateImgList.Create(IDB_HISTORY_POS, 6, 0, RGB(0, 128, 128)))
		return -1;
	m_List.SetImageList(&m_StateImgList, LVSIL_STATE);

	return 0;
}

void CHistoryBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_IsSizeValid)	// if size is valid
		m_List.MoveWindow(0, 0, cx, cy);
}

void CHistoryBar::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	LVITEM&	item = pDispInfo->item;
	CUndoManager	*UndoMgr = theApp.GetMain()->GetUndoManager();
	if (UndoMgr == NULL)	// if no documents
		return;
	*pResult = 0;
	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
		case COL_ACTION:
			{
				CString	s;
				if (item.iItem) {
					CWaveShopView	*View = theApp.GetMain()->GetView();
					// items are offset by one to make room for initial state
					s = View->GetUndoTitle(UndoMgr->GetState(item.iItem - 1));
				} else	// item zero is initial state
					s.LoadString(IDS_HIST_INITIAL_STATE);
				_tcsncpy(item.pszText, s, item.cchTextMax);
			}
			break;
		}
	}
	if (item.mask & LVIF_IMAGE) {
		switch (item.iSubItem) {
		case COL_ACTION:
			item.mask |= LVIF_STATE;
			item.state = INDEXTOSTATEIMAGEMASK(item.iItem == UndoMgr->GetPos());
			item.stateMask = LVIS_STATEIMAGEMASK;
			break;
		}
	}
}

void CHistoryBar::OnDblclkList(NMHDR* /* pNMHDR */, LRESULT* /* pResult */)
{
	int	iItem = m_List.GetSelectionMark();
	if (iItem >= 0)	// if valid item
		SetUndoPos(iItem);
}
