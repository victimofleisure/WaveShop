// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		05aug06	initial version
		01		20oct07	change GetEdit to detect non-MFC edit controls

		handlers for focused edit control

*/

#include "stdafx.h"
#include "FocusEdit.h"

#define VERIFY_FOCUS_EDIT CEdit *ep = GetEdit(); if (ep == NULL) return(FALSE);

// We use the SDK GetClassName function because MFC IsKindOf fails if no CEdit
// variable has been created for the control we're trying to test. An example
// of this is the edit control created by a list control for label editing.
CEdit *CFocusEdit::GetEdit()
{
	HWND	hWnd = GetFocus();
	TCHAR	szClassName[6];
	if (GetClassName(hWnd, szClassName, 6) 
	&& _tcsicmp(szClassName, _T("Edit")) == 0)
		return((CEdit *)CWnd::FromHandle(hWnd));
	return(NULL);
}

bool CFocusEdit::Undo()
{
	VERIFY_FOCUS_EDIT;
	ep->Undo();
	return(TRUE);
}

bool CFocusEdit::Cut()
{
	VERIFY_FOCUS_EDIT;
	ep->Cut();
	return(TRUE);
}

bool CFocusEdit::Copy()
{
	VERIFY_FOCUS_EDIT;
	ep->Copy();
	return(TRUE);
}

bool CFocusEdit::Paste()
{
	VERIFY_FOCUS_EDIT;
	ep->Paste();
	return(TRUE);
}

bool CFocusEdit::Insert()
{
	VERIFY_FOCUS_EDIT;
	return(TRUE);
}

bool CFocusEdit::Delete()
{
	VERIFY_FOCUS_EDIT;
	if (HasSelection(ep))
		ep->Clear();
	else
		ep->SendMessage(WM_KEYDOWN, VK_DELETE);	// delete character at cursor
	return(TRUE);
}

bool CFocusEdit::SelectAll()
{
	VERIFY_FOCUS_EDIT;
	ep->SetSel(0, -1);
	return(TRUE);
}

bool CFocusEdit::UpdateUndo(CCmdUI* pCmdUI)
{
	VERIFY_FOCUS_EDIT;
	pCmdUI->Enable(ep->CanUndo());
	return(TRUE);
}

bool CFocusEdit::UpdateCut(CCmdUI* pCmdUI)
{
	VERIFY_FOCUS_EDIT;
	pCmdUI->Enable(HasSelection(ep) && !IsReadOnly(ep));
	return(TRUE);
}

bool CFocusEdit::UpdateCopy(CCmdUI* pCmdUI)
{
	VERIFY_FOCUS_EDIT;
	pCmdUI->Enable(HasSelection(ep));
	return(TRUE);
}

bool CFocusEdit::UpdatePaste(CCmdUI* pCmdUI)
{
	VERIFY_FOCUS_EDIT;
	pCmdUI->Enable(!IsReadOnly(ep) && IsClipboardFormatAvailable(CF_TEXT));
	return(TRUE);
}

bool CFocusEdit::UpdateInsert(CCmdUI* pCmdUI)
{
	VERIFY_FOCUS_EDIT;
	pCmdUI->Enable(FALSE);
	return(TRUE);
}

bool CFocusEdit::UpdateDelete(CCmdUI* pCmdUI)
{
	VERIFY_FOCUS_EDIT;
	bool	Enab = FALSE;
	if (!IsReadOnly(ep)) {
		int	nBeg, nEnd;
		ep->GetSel(nBeg, nEnd);
		if (nBeg != nEnd || nBeg < ep->LineLength())
			Enab = TRUE;	// has selection, or cursor is on a character
	}
	pCmdUI->Enable(Enab);
	return(TRUE);
}

bool CFocusEdit::UpdateSelectAll(CCmdUI* pCmdUI)
{
	VERIFY_FOCUS_EDIT;
	pCmdUI->Enable(ep->LineLength());
	return(TRUE);
}
