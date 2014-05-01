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
        02      25nov06 use CArrayishList instead of CList
		03		23nov07	support Unicode
		04		18mar08	remove key support
		05		03feb09	add GetSize, GetPos, GetState
		06		28may10	support insignificant edits
		07		05dec12	add UndoNoRedo
		08		11feb13	add OnUpdateTitles and SetPos

        undoable edit interface
 
*/

#ifndef CUNDOMANAGER_INCLUDED
#define CUNDOMANAGER_INCLUDED

#include "ArrayEx.h"
#include "UndoState.h"

class CUndoable;

class CUndoManager : public WObject {
public:
// Construction
	CUndoManager(CUndoable *Root = NULL);
	virtual	~CUndoManager();

// Constants
	enum {	// undo actions
		UA_NONE,
		UA_UNDO,
		UA_REDO,
	};

// Attributes
	bool	CanUndo() const;
	bool	CanRedo() const;
	int		GetAction() const;
	bool	IsIdle() const;
	bool	IsUndoing() const;
	bool	IsRedoing() const;
	bool	IsModified() const;
	void	ResetModifiedFlag();
	LPCTSTR	GetUndoTitle();
	LPCTSTR	GetRedoTitle();
	int		GetLevels() const;
	void	SetLevels(int Levels);
	void	SetRoot(CUndoable *Root);
	int		GetSize() const;
	int		GetPos() const;
	void	SetPos(int Pos);
	const	CUndoState& GetState(int Pos) const;

// Operations
	void	Undo();
	void	Redo();
	void	UndoNoRedo();
	void	NotifyEdit(WORD CtrlID, WORD Code, UINT Flags = 0);
	void	CancelEdit(WORD CtrlID, WORD Code);
	void	DiscardAllEdits();

protected:
// Overridables
	virtual	void	OnModify(bool Modified);
	virtual	void	OnUpdateTitles();

private:
// Types
	typedef	CArrayEx<CUndoState, CUndoState&> CUndoStateArray;

// Constants
	enum {
		BLOCK_SIZE = 100	// list grows in blocks of this many elements
	};

// Member data
	CUndoable	*m_Root;	// owner of this undo stack
	CUndoStateArray	m_List;	// undo stack; array of undo states
	bool	m_CanUndo;		// true if edit can be undone
	bool	m_CanRedo;		// true if edit can be redone
	int		m_Pos;			// current position in undo stack
	int		m_Levels;		// number of undo levels, or -1 for unlimited
	int		m_Edits;		// total number of edits made so far
	int		m_Action;		// undo action; see enum above
	CString	m_UndoTitle;	// current undo title for edit menu
	CString	m_RedoTitle;	// current redo title for edit menu

// Helpers
	void	SwapState(int Pos);
	void	DumpState(LPCTSTR Tag, int Pos);
	CString	GetTitle(int Pos);
	void	UpdateTitles();
	int		FindUndoable() const;
	int		FindRedoable() const;
};

inline bool CUndoManager::CanUndo() const
{
	return(m_CanUndo);
}

inline bool CUndoManager::CanRedo() const
{
	return(m_CanRedo);
}

inline int CUndoManager::GetAction() const
{
	return(m_Action);
}

inline bool CUndoManager::IsIdle() const
{
	return(m_Action == UA_NONE);
}

inline bool CUndoManager::IsUndoing() const
{
	return(m_Action == UA_UNDO);
}

inline bool CUndoManager::IsRedoing() const
{
	return(m_Action == UA_REDO);
}

inline bool CUndoManager::IsModified() const
{
	return(m_Edits > 0);
}

inline void CUndoManager::ResetModifiedFlag()
{
	m_Edits = 0;
}

inline int CUndoManager::GetLevels() const
{
	return(m_Levels);
}

inline LPCTSTR CUndoManager::GetUndoTitle()
{
	return(m_UndoTitle);
}

inline LPCTSTR CUndoManager::GetRedoTitle()
{
	return(m_RedoTitle);
}

inline void CUndoManager::SetRoot(CUndoable *Root)
{
	m_Root = Root;
}

inline int CUndoManager::GetSize() const
{
	return(m_List.GetSize());
}

inline int CUndoManager::GetPos() const
{
	return(m_Pos);
}

inline const CUndoState& CUndoManager::GetState(int Pos) const
{
	return(m_List[Pos]);
}

#endif
