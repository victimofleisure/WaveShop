// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
		01		16apr13	handle command help

		wave editor MDI child frame
 
*/

// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "WaveShop.h"
#include "WaveShopDoc.h"
#include "WaveShopView.h"

#include "ChildFrm.h"
#include "Persist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RK_CHILD_FRAME	_T("ChildFrame")

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	m_View = NULL;
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
    ASSERT(cs.lpszClass);

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

void CChildFrame::ShowChannelBar(bool Enable)
{
	ShowControlBar(&m_ChannelBar, Enable, 0);
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message map

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_MDIACTIVATE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
//printf("CChildFrame::OnCreate\n");
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create time bar
	UINT	dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP;
	if (!m_TimeBar.Create(this, dwStyle, 0))
		return -1;

	// create channel bar
	dwStyle = WS_CHILD | CBRS_LEFT;
	if (theApp.GetMain()->IsChannelBarVisible())
		dwStyle |= WS_VISIBLE;
	if (!m_ChannelBar.Create(this, dwStyle, 0))
		return -1;

	// find our view in child window list
	CWaveShopView	*pView = NULL;
	CWnd	*pWnd = GetWindow(GW_CHILD);
	while (pWnd != NULL) {
		pView = DYNAMIC_DOWNCAST(CWaveShopView, pWnd);
		if (pView != NULL)
			break;
		pWnd = GetNextWindow();
	}
	ASSERT(pView != NULL);
	if (pView == NULL)
		return -1;
	m_View = pView;	// store view pointer
	m_TimeBar.GetRuler()->SetTarget(pView);
	pView->SetTimeRuler(m_TimeBar.GetRuler());
	m_ChannelBar.SetView(pView);
	pView->SetChannelBar(&m_ChannelBar);

	return 0;
}

BOOL CChildFrame::DestroyWindow() 
{
	theApp.GetMain()->OnDestroyView(m_View);	// notify main frame beforehand
	return CMDIChildWnd::DestroyWindow();
}

void CChildFrame::OnDestroy() 
{
	// save maximize setting in registry
	CPersist::SaveWnd(REG_SETTINGS, this, RK_CHILD_FRAME);
	CMDIChildWnd::OnDestroy();
}

void CChildFrame::ActivateFrame(int nCmdShow) 
{
	if (GetMDIFrame()->MDIGetActive())
		CMDIChildWnd::ActivateFrame(nCmdShow); 
	else {
		int	RegShow = CPersist::GetWndShow(REG_SETTINGS, RK_CHILD_FRAME);
		if (RegShow == SW_SHOWMAXIMIZED)
			nCmdShow = SW_SHOWMAXIMIZED;
		CMDIChildWnd::ActivateFrame(nCmdShow);
	}
}

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
	if (bActivate) {	// if activating
		theApp.GetMain()->OnActivateView(m_View);	// notify main frame
		CUndoManager&	UndoMgr = m_View->GetDocument()->m_UndoMgr;
		UndoMgr.SetRoot(m_View);
		m_View->SetUndoManager(&UndoMgr);
	} else {	// deactivating
		if (pActivateWnd == NULL)	// if no document
			theApp.GetMain()->OnActivateView(NULL);	// notify main frame
	}
}

void CChildFrame::RecalcLayout(BOOL bNotify) 
{
	CMDIChildWnd::RecalcLayout(bNotify);
	if (m_View != NULL)
		m_View->OnRecalcLayout();
}

LRESULT CChildFrame::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
	// default handler returns view's ID and stops dispatching, 
	// which breaks context-sensitive help for tool bar buttons
	return 0;	// don't call base class, just continue dispatching
}
