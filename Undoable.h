// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb04	initial version
        01      22nov06 derive from WObject
		02		18mar08	remove key support
		03		28may10	add CtrlID to notify and cancel

        undoable edit interface
 
*/

#ifndef CUNDOABLE_INCLUDED
#define CUNDOABLE_INCLUDED

#include "UndoState.h"
#include "UndoManager.h"

class CUndoable : public WObject {
public:
// Constants
	enum UE_FLAGS {
		UE_COALESCE			= 0x01,
		UE_INSIGNIFICANT	= 0x02
	};

// Construction
	CUndoable();

// Attributes
	CUndoManager	*GetUndoManager() const;
	void	SetUndoManager(CUndoManager *Mgr);
	CUndoable	*GetUndoHandler() const;
	void	SetUndoHandler(CUndoable *Handler);
	int		GetUndoAction() const;
	bool	UndoMgrIsIdle() const;
	bool	IsUndoing() const;
	bool	IsRedoing() const;

// Operations
	void	NotifyUndoableEdit(WORD CtrlID, WORD Code, UINT Flags = 0);
	void	CancelUndoableEdit(WORD CtrlID, WORD Code);
	void	ClearUndoHistory();

// Overridables
	virtual	void	SaveUndoState(CUndoState& State) = 0;
	virtual	void	RestoreUndoState(const CUndoState& State) = 0;
	virtual	CString	GetUndoTitle(const CUndoState& State) = 0;

private:
// Member data
	CUndoManager	*m_UndoManager;
};

inline CUndoable::CUndoable()
{
	m_UndoManager = NULL;
}

inline CUndoManager *CUndoable::GetUndoManager() const
{
	return(m_UndoManager);
}

inline void CUndoable::SetUndoManager(CUndoManager *Mgr)
{
	m_UndoManager = Mgr;
}

inline void CUndoable::NotifyUndoableEdit(WORD CtrlID, WORD Code, UINT Flags)
{
	m_UndoManager->NotifyEdit(CtrlID, Code, Flags);
}

inline void CUndoable::CancelUndoableEdit(WORD CtrlID, WORD Code)
{
	m_UndoManager->CancelEdit(CtrlID, Code);
}

inline int CUndoable::GetUndoAction() const
{
	return(m_UndoManager->GetAction());
}

inline bool CUndoable::UndoMgrIsIdle() const
{
	return(m_UndoManager->IsIdle());
}

inline bool CUndoable::IsUndoing() const
{
	return(m_UndoManager->IsUndoing());
}

inline bool CUndoable::IsRedoing() const
{
	return(m_UndoManager->IsRedoing());
}

inline void CUndoable::ClearUndoHistory()
{
	m_UndoManager->DiscardAllEdits();
}

#endif
