// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30aug05	initial version
		01		10aug07	add template resource ID to ctor
		02		27dec09	add ShowPercent
		03		14jan13	dialog must have popup style, NOT overlapped
		04		28feb13	add OnOK
		05		01mar13	add dual progress

        progress dialog
 
*/

// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CDialog);

CProgressDlg	*CProgressDlg::m_Master;	// pointer to one and only master

CProgressDlg::CProgressDlg(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
	//{{AFX_DATA_INIT(CProgressDlg)
	//}}AFX_DATA_INIT
	m_IDTemplate = nIDTemplate;
	m_ParentWnd = NULL;
	m_ParentDisabled = FALSE;
	m_Canceled = FALSE;
	m_ShowPercent = FALSE;
	m_PrevPercent = 0;
}

CProgressDlg::~CProgressDlg()
{
    if (m_hWnd != NULL)
		DestroyWindow();
	if (IsMaster())	// if we're master
		m_Master = NULL;	// relinquish mastery
}

bool CProgressDlg::Create(CWnd* pParent)
{
	if (m_IDTemplate == IDD_PROGRESS_DUAL) {	// if dual progress template
		ASSERT(m_Master == NULL);	// only one master at a time
		m_Master = this;	// become master
	}
	bool	slave = IsSlave();
	if (slave) {	// if we're slave
		m_IDTemplate = IDD_PROGRESS_SLAVE;	// use slave child template
		pParent = m_Master;	// slave dialog is child of master
	} else {	// we're not slave
		m_ParentWnd = GetSafeOwner(pParent);
		if (m_ParentWnd != NULL && m_ParentWnd->IsWindowEnabled()) {
			m_ParentWnd->EnableWindow(FALSE);
			m_ParentDisabled = TRUE;
	    }
	}
	if (!CDialog::Create(m_IDTemplate, pParent)) {
		ReenableParent();
		return(FALSE);
	}
	if (slave) {	// if we're slave
		// get pointer to master's subprogress bar
		CWnd	*Subprogress = m_Master->GetDlgItem(IDC_PROGRESS_SUBPROGRESS);
		ASSERT(Subprogress != NULL);
		m_Progress.DestroyWindow();	// destroy our progress control
		// attach our progress control to master's subprogress bar
		VERIFY(m_Progress.Attach(Subprogress->m_hWnd));
		m_Progress.SetRange(0, 100);	// reset progress range to default
	} else {	// we're not slave
		// NOTE: dialog resource must be WS_POPUP and WS_VISIBLE; previous practice
		// of using WS_OVERLAPPED causes unexpected window deactivation if progress
		// dialogs are nested, or if a message box is shown during progress dialog
		ASSERT(GetStyle() & WS_POPUP);
	}
	return(TRUE);
}

void CProgressDlg::ReenableParent()
{
    if (m_ParentDisabled && m_ParentWnd != NULL)
		m_ParentWnd->EnableWindow(TRUE);
    m_ParentDisabled = FALSE;
}

void CProgressDlg::SetPos(int Pos)
{
	PumpMessages();
	m_Progress.SetPos(Pos);
	if (m_ShowPercent) {
		int	Lower, Upper;
		m_Progress.GetRange(Lower, Upper);
		int	Range = Upper - Lower;
		if (Range) {	// avoid potential divide by zero
			int	percent = round((Pos - Lower) * 100.0 / Range);
			if (percent != m_PrevPercent) {	// if percentage changed
				m_PrevPercent = percent;
				CString	s;
				s.Format(_T("%d%%"), percent);
				m_Percent.SetWindowText(s);
			}
		}
	}
}

void CProgressDlg::SetRange(int Lower, int Upper)
{
	m_Progress.SetRange32(Lower, Upper);
}

void CProgressDlg::SetWindowText(LPCTSTR Text)
{
	if (IsSlave()) {	// if we're slave
		CWnd	*subcaption = m_Master->GetDlgItem(IDC_PROGRESS_SUBCAPTION);
		ASSERT(subcaption != NULL);
		subcaption->SetWindowText(Text);	// display text in subcaption
	} else	// we're not slave
		CDialog::SetWindowText(Text);
}

void CProgressDlg::PumpMessages()
{
	ASSERT(m_hWnd != NULL);
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (!IsDialogMessage(&msg)) {
	        TranslateMessage(&msg);
		    DispatchMessage(&msg);
		}
	}
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_PROGRESS_PERCENT, m_Percent);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CProgressDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg message handlers

BOOL CProgressDlg::DestroyWindow() 
{
	if (IsSlave())	// if we're slave
		m_Progress.Detach();	// detach our progress control from master
    ReenableParent();
	return CDialog::DestroyWindow();
}

void CProgressDlg::OnCancel() 
{
	m_Canceled = TRUE;
	if (m_Master != NULL) {	// if master exists
		// assume master's only child control with ID of zero is slave dialog
		CProgressDlg	*SlaveDlg = 
			STATIC_DOWNCAST(CProgressDlg, m_Master->GetDlgItem(0));
		if (SlaveDlg != NULL)	// if slave dialog found
			SlaveDlg->m_Canceled = TRUE;	// cancel it too
	}
	CDialog::OnCancel();
}

void CProgressDlg::OnOK() 
{
	OnCancel();	// so pressing Enter cancels
}
