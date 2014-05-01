// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb04	initial version
		01		07mar04	add limit
		02		08mar04	in SwapState, set busy before finding undoable
		03		08mar04	in NotifyEdit, ctor isn't called when array shrinks
		04		12mar04	if coalescing, remove states above current position
		05		29sep04	cancel edit must update titles
		06		19mar05	bump m_Edits regardless of number of undo levels
        07      22nov06 rename strings to start with IDS_
        08      25nov06 use CArrayishList instead of CList
		09		23nov07	support Unicode
		10		03jan08	replace CSmartBuf with CRefPtr
		11		18mar08	remove key support
		12		19feb09	in NotifyEdit, fix stack size limit case
		13		28may10	support insignificant edits
		14		05dec12	add UndoNoRedo
		15		11feb13	add OnUpdateTitles and SetPos

        undoable edit interface
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "UndoManager.h"
#include "Undoable.h"

#if UNDO_NATTER
#define	UNDO_DUMP_STATE(Tag, Pos) DumpState(Tag, Pos);
#else
#define	UNDO_DUMP_STATE(Tag, Pos)
#endif

CUndoManager::CUndoManager(CUndoable *Root) :
	m_Root(Root)
{
	m_CanUndo = FALSE;
	m_CanRedo = FALSE;
	m_Pos = 0;
	m_Levels = INT_MAX;
	m_Edits = 0;
	m_Action = UA_NONE;
}

CUndoManager::~CUndoManager()
{
}

inline CString CUndoManager::GetTitle(int Pos)
{
	return(m_Root->GetUndoTitle(m_List[Pos]));
}

int CUndoManager::FindUndoable() const
{
	int	pos = m_Pos;
	while (pos > 0) {
		pos--;
		if (m_List[pos].IsSignificant())
			return(pos);
	}
	return(-1);
}

int CUndoManager::FindRedoable() const
{
	int	pos = m_Pos;
	while (pos < GetSize()) {
		if (m_List[pos].IsSignificant())
			return(pos);
		pos++;
	}
	return(-1);
}

void CUndoManager::Undo()
{
	int	PrevUndo = FindUndoable();
	if (PrevUndo >= 0) {
		// undo previous significant edit, and all insignificant edits back to it
		m_Action = UA_UNDO;
		for (int i = m_Pos - 1; i >= PrevUndo; i--) {
			UNDO_DUMP_STATE(_T("Undo"), i);
			SwapState(i);
			if (IsIdle())
				return;	// edits discarded
		}
		m_Pos = PrevUndo;
		UpdateTitles();
		m_Action = UA_NONE;
	}
}

void CUndoManager::Redo()
{
	int	NextRedo = FindRedoable();
	if (NextRedo >= 0) {
		// redo next significant edit, and all insignificant edits up to it
		m_Action = UA_REDO;
		for (int i = m_Pos; i <= NextRedo; i++) {
			UNDO_DUMP_STATE(_T("Redo"), i);
			SwapState(i);
			if (IsIdle())
				return;	// edits discarded
		}
		m_Pos = NextRedo + 1;
		UpdateTitles();
		m_Action = UA_NONE;
	}
}

void CUndoManager::UndoNoRedo()
{
	Undo();
	m_List.SetSize(m_Pos);
	UpdateTitles();
}

void CUndoManager::SwapState(int Pos)
{
	ASSERT(Pos >= 0 && Pos < GetSize());
	CUndoState	PrevState = m_List[Pos];
	CUndoable	*uap = m_Root;
	uap->SaveUndoState(m_List[Pos]);
	uap->RestoreUndoState(PrevState);
#if UNDO_NATTER
	if (uap == NULL)
		_tprintf(_T("Can't find instance.\n"));
#endif
}

void CUndoManager::NotifyEdit(WORD CtrlID, WORD Code, UINT Flags)
{
	ASSERT(CtrlID != UNDO_CTRL_ID_INSIGNIFICANT);	// reserved control ID
	if (IsIdle()) {
		// do insignificance test first; can modify CtrlID
		if (Flags & CUndoable::UE_INSIGNIFICANT)
			CtrlID = UNDO_CTRL_ID_INSIGNIFICANT;
		else {
			if (!m_Edits++)			// if first modification
				OnModify(TRUE);		// call derived handler
		}
		// if coalesce requested and notifier's key matches top of stack
		if ((Flags & CUndoable::UE_COALESCE) && m_Pos 
		&& m_List[m_Pos - 1].IsMatch(CtrlID, Code)) {
			if (GetSize() > m_Pos) {	// don't resize array needlessly
				m_List.SetSize(m_Pos);	// remove states above current position
				UpdateTitles();
			}
			return;
		}
		if (m_Levels <= 0)
			return;
		if (m_Pos >= m_Levels) {	// if stack size at limit
			m_List.RemoveAt(0);	// remove bottom state
			m_Pos--;
		}
		m_List.SetSize(m_Pos);	// array shrinks if we've undone
		CUndoState	us;
		us.m_Val.i64 = 0;
		us.m_CtrlID = CtrlID;
		us.m_Code = Code;
		m_List.Add(us);
		CUndoState	*usp = &m_List[m_Pos];
		m_Root->SaveUndoState(*usp);
		UNDO_DUMP_STATE(_T("Notify"), m_Pos);
		m_Pos++;
		UpdateTitles();
	} else {
#if UNDO_NATTER
		_tprintf(_T("Ignoring notify.\n"));
#endif
	}
}

void CUndoManager::CancelEdit(WORD CtrlID, WORD Code)
{
#if UNDO_NATTER
	_tprintf(_T("CancelEdit CtrlID=%d Code=%d\n"), CtrlID, Code);
#endif
	int	i;
	for (i = m_Pos - 1; i >= 0; i--) {
		if (m_List[i].IsMatch(CtrlID, Code))
			break;
	}
	if (i >= 0) {
		m_List.RemoveAt(i);
		m_Pos--;
		if (!--m_Edits)			// if last modification
			OnModify(FALSE);	// call derived handler
		UpdateTitles();
	}
#if UNDO_NATTER
	if (i < 0)
		_tprintf(_T("Can't cancel edit.\n"));
#endif
}

void CUndoManager::DiscardAllEdits()
{
#if UNDO_NATTER
	_tprintf(_T("DiscardAllEdits\n"));
#endif
	m_List.SetSize(0);
	m_CanUndo = FALSE;
	m_CanRedo = FALSE;
	m_Pos = 0;
	m_Edits = 0;
	m_Action = UA_NONE;
	OnModify(FALSE);	// call derived handler
	UpdateTitles();
}

void CUndoManager::DumpState(LPCTSTR Tag, int Pos)
{
	_tprintf(_T("%s '%s' Pos=%d %s Obj=0x%x\n"), Tag, GetTitle(Pos), Pos, 
		m_List[Pos].DumpState(), m_List[Pos].GetObj());
}

void CUndoManager::UpdateTitles()
{
	int	PrevUndo = FindUndoable();
	m_CanUndo = PrevUndo >= 0;
	if (m_CanUndo)
		m_UndoTitle = GetTitle(PrevUndo);
	else
		m_UndoTitle.Empty();
	int	NextRedo = FindRedoable();
	m_CanRedo = NextRedo >= 0;
	if (m_CanRedo)
		m_RedoTitle = GetTitle(NextRedo);
	else
		m_RedoTitle.Empty();
	OnUpdateTitles();
}

void CUndoManager::SetLevels(int Levels)
{
	if (Levels < 0)
		Levels = INT_MAX;
	if (Levels < GetSize()) {	// if shrinking history
		if (m_Pos < Levels)	// if undo depth is below new size
			DiscardAllEdits();	// history can't be saved
		else {	// trim excess history from the bottom
			int	Excess = GetSize() - Levels;
			m_List.RemoveAt(0, Excess);
			m_Pos = max(m_Pos - Excess, 0);
		}
	}
	m_Levels = Levels;
}

void CUndoManager::SetPos(int Pos)
{
	ASSERT(Pos >= 0 && Pos <= GetSize());
	int	Start = m_Pos;
	if (Pos == Start)
		return;	// nothing to do
	int	steps = abs(Pos - Start);
	bool	undoing = Pos < Start;
	for (int iStep = 0; iStep < steps; iStep++) {
		if (undoing)
			Undo();
		else
			Redo();
	}
}

void CUndoManager::OnModify(bool Modified)
{
}

void CUndoManager::OnUpdateTitles()
{
}
