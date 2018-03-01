// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      16nov12	initial version
        01      29jan13	check for null view pointer in all cases

        navigation dialog bar
 
*/

// NavBar.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "NavBar.h"
#include "RulerCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNavBar dialog

IMPLEMENT_DYNAMIC(CNavBar, CDialogBarEx);

const CCtrlResize::CTRL_LIST CNavBar::m_CtrlList[] = {
	{IDC_NAV_NOW,			BIND_LEFT},
	{IDC_NAV_SEL_START,		BIND_LEFT},
	{IDC_NAV_SEL_END,		BIND_LEFT},
	{IDC_NAV_SHOW_LENGTH,	BIND_LEFT},
	{0}
};

#define RK_NAV_SHOW_LENGTH _T("NavShowLength")

CNavBar::CNavBar(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(CNavBar)
	//}}AFX_DATA_INIT
	m_Main = NULL;
	m_ShowLength = theApp.RdRegBool(RK_NAV_SHOW_LENGTH, FALSE);
}

CNavBar::~CNavBar()
{
	theApp.WrRegBool(RK_NAV_SHOW_LENGTH, m_ShowLength);
}

void CNavBar::SetNow(double Now)
{
	SetStr(m_NowEdit, m_sNow, Now, !m_Main->IsPlaying());
}

void CNavBar::SetSelection(const CDblRange& Sel)
{
	SetStr(m_SelStartEdit, m_sSelStart, Sel.Start);
	double	EndFrame = m_ShowLength ? Sel.Length() : Sel.End;
	SetStr(m_SelEndEdit, m_sSelEnd, EndFrame);
}

void CNavBar::SetStr(CEdit& Edit, CString& Str, double Frame, bool Select) const
{
	CString	s(FrameToStr(Frame));
	if (s != Str) {	// if text changed
		Edit.SetWindowText(s);
		Str = s;
	}
	if (Select && GetFocus() == &Edit)	// if select was requested and edit has focus
		Edit.SetSel(0, -1);	// select entire text
}

bool CNavBar::GetStr(CEdit& Edit, CString& Str, double& Frame) const
{
	CString	s;
	Edit.GetWindowText(s);
	if (s == Str)	// if text unchanged
		return(FALSE);
	if (!StrToFrame(s, Frame))	// if conversion failed
		return(FALSE);
	Str = s;
	return(TRUE);
}

void CNavBar::OnActivateView(CWaveShopView *View)
{
	m_NowEdit.EnableWindow(View != NULL);
	m_SelStartEdit.EnableWindow(View != NULL);
	m_SelEndEdit.EnableWindow(View != NULL);
	m_ShowLengthBtn.EnableWindow(View != NULL);
	if (View != NULL) {	// if activating a view
		SetNow(View->GetNow());
		CDblRange	sel = View->GetSelection();
		SetSelection(sel);
	} else {	// no active view
		m_sNow.Empty();
		m_sSelStart.Empty();
		m_sSelEnd.Empty();
		LPCTSTR	Empty = _T("");
		m_NowEdit.SetWindowText(Empty);
		m_SelStartEdit.SetWindowText(Empty);
		m_SelEndEdit.SetWindowText(Empty);
	}
}

void CNavBar::OnTimeFormatChange()
{
	CWaveShopView	*View = m_Main->GetView();
	if (View != NULL) {
		SetNow(View->GetNow());
		SetSelection(View->GetSelection());
	}
}

UINT CNavBar::GetSampleRate() const
{
	CWaveShopView	*View = m_Main->GetView();
	if (View != NULL)
		return(View->GetWave().GetSampleRate());
	return(0);
}

CString CNavBar::FrameToStr(double Frame) const
{
	CString	s;
	if (m_Main->GetOptions().m_TimeInFrames) {
		s.Format(_T("%.0f"), Frame);
	} else {
		UINT	SampleRate = GetSampleRate();
		if (SampleRate)	// avoid potential divide by zero
			s = CRulerCtrl::FormatTime(Frame / SampleRate);
	}
	return(s);
}

bool CNavBar::StrToFrame(LPCTSTR Time, double& Frame) const
{
	if (m_Main->GetOptions().m_TimeInFrames) {
		if (_stscanf(Time, _T("%lf"), &Frame) != 1)
			return(FALSE);
	} else {
		int	hours = 0, mins = 0, secs = 0;
		double	ticks = 0;
		if (_stscanf(Time, _T("%d:%d:%d%lf"), &hours, &mins, &secs, &ticks) < 1)
			return(FALSE);
		Frame = (hours * 3600 + mins * 60 + secs + ticks) * GetSampleRate();
	}
	return(TRUE);
}

void CNavBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBarEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNavBar)
	DDX_Control(pDX, IDC_NAV_SHOW_LENGTH, m_ShowLengthBtn);
	DDX_Control(pDX, IDC_NAV_SEL_END, m_SelEndEdit);
	DDX_Control(pDX, IDC_NAV_SEL_START, m_SelStartEdit);
	DDX_Control(pDX, IDC_NAV_NOW, m_NowEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNavBar, CDialogBarEx)
	//{{AFX_MSG_MAP(CNavBar)
	ON_EN_KILLFOCUS(IDC_NAV_NOW, OnKillfocusNow)
	ON_EN_KILLFOCUS(IDC_NAV_SEL_START, OnKillfocusSelStart)
	ON_EN_KILLFOCUS(IDC_NAV_SEL_END, OnKillfocusSelEnd)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_NAV_SHOW_LENGTH, OnShowLength)
	ON_BN_DOUBLECLICKED(IDC_NAV_SHOW_LENGTH, OnShowLength)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
	ON_UPDATE_COMMAND_UI(IDC_NAV_SHOW_LENGTH, OnUpdateShowLength)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNavBar message handlers

LRESULT CNavBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	CDialogBarEx::OnInitDialog(wParam, lParam);
	m_Main = theApp.GetMain();
	OnActivateView(NULL);
	m_Resize.AddControlList(this, m_CtrlList);
	m_ShowLengthBtn.SetIcons(IDI_NAV_SHOW_LENGTH, IDI_NAV_SHOW_LENGTH);
	m_ShowLengthBtn.SetAutoCheck(TRUE);
	return 0;
}

void CNavBar::OnKillfocusNow() 
{
	double	now;
	if (GetStr(m_NowEdit, m_sNow, now)) {
		if (!m_Main->IsPlaying())	// if stopped
			m_Main->SetNow(now);	// update position; center
	}
}

void CNavBar::OnKillfocusSelStart() 
{
	CWaveShopView	*View = m_Main->GetView();
	if (View != NULL) {
		CDblRange	sel = View->GetSelection();
		double	len = sel.Length();
		if (GetStr(m_SelStartEdit, m_sSelStart, sel.Start)) {
			if (m_ShowLength)	// if showing selection length instead of end
				sel.End = sel.Start + len;	// update end to maintain length
			else {	// showing selection end
				if (!sel.IsNormalized())	// if range is out of order
					sel.End = sel.Start;		// create empty selection at start
			}
			m_Main->SetSelection(sel);	// update selection
		}
	}
}

void CNavBar::OnKillfocusSelEnd() 
{
	CWaveShopView	*View = m_Main->GetView();
	if (View != NULL) {
		CDblRange	sel = View->GetSelection();
		if (GetStr(m_SelEndEdit, m_sSelEnd, sel.End)) {
			if (m_ShowLength)	// if showing selection length instead of end
				sel.End += sel.Start;	// convert length to end
			else {	// showing selection end
				if (!sel.IsNormalized())	// if range is out of order
					sel.Start = sel.End;		// create empty selection at end
			}
			m_Main->SetSelection(sel);	// update selection
		}
	}
}

BOOL CNavBar::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN) {
			CEdit	*pEdit = DYNAMIC_DOWNCAST(CEdit, GetFocus());
			if (pEdit != NULL) {
				if (pEdit == &m_NowEdit)
					OnKillfocusNow();
				else if (pEdit == &m_SelStartEdit)
					OnKillfocusSelStart();
				else if (pEdit == &m_SelEndEdit)
					OnKillfocusSelEnd();
			}
		}
	}
	return CDialogBarEx::PreTranslateMessage(pMsg);
}

void CNavBar::OnSize(UINT nType, int cx, int cy) 
{
	CDialogBarEx::OnSize(nType, cx, cy);
	int	xshift = IsFloating() ? 0 : GRIPPER_SIZE;
	m_Resize.SetOriginShift(CSize(xshift, 0));
	m_Resize.OnSize();
}

void CNavBar::OnShowLength() 
{
	m_ShowLength ^= 1;
	CWaveShopView	*View = m_Main->GetView();
	if (View != NULL) {
		CDblRange	sel = m_Main->GetView()->GetSelection();
		SetSelection(sel);
	}
}

void CNavBar::OnUpdateShowLength(CCmdUI *pCmdUI)
{
	// this handler is mandatory, else button is automatically disabled
	pCmdUI->SetCheck(m_ShowLength);
}
