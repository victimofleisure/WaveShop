// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb04	initial version
		01		22nov06	add support for double and int64
		02		26nov06	make undo key DWORD, max keys = 2
		03		18dec06	add support for pair of shorts
		04		03jan08	replace CSmartBuf with CRefPtr
		05		18mar08	make CRefPtr a template
		06		28may10	add IsInsignificant

        undo state container
 
*/

#ifndef CUNDOSTATE_INCLUDED
#define CUNDOSTATE_INCLUDED

#pragma pack(push, 1)	// 1-byte packing alignment

#include "RefPtr.h"

class CUndoManager;

// create accessors for an undo state value member
#define CUNDOSTATE_VAL(alias, type, member)						\
	inline static const type& alias(const CUndoState& State)	\
		{ return(State.m_Val.member); }							\
	inline static type& alias(CUndoState& State)				\
		{ return(State.m_Val.member); }

// reserved control ID for insignificant edits
#define UNDO_CTRL_ID_INSIGNIFICANT 0x8000

class CUndoState : public CRefPtr<CRefObj> {
public:
// Types
	typedef union tagELEM {
		int		i;
		UINT	u;
		float	f;
		bool	b;
		struct {
			WORD	lo;
			WORD	hi;
		} w;
		struct {
			short	lo;
			short	hi;
		} s;
		struct {
			BYTE	al;
			BYTE	ah;
			BYTE	bl;
			BYTE	bh;
		} c;
	} ELEM;
	typedef struct tagPAIR {
		ELEM	x;
		ELEM	y;
	} PAIR;
	typedef union tagVALUE {
		PAIR	p;
		double	d;
		__int64	i64;
	} VALUE;

// Member data
	VALUE	m_Val;

// Attributes
	WORD	GetCtrlID() const;
	WORD	GetCode() const;
	bool	IsMatch(WORD CtrlID, WORD Code) const;
	bool	IsSignificant() const;

// Construction
	CUndoState();
	CUndoState(const CUndoState& State);
	CUndoState& operator=(const CUndoState& State);

// Operations
	CString	DumpState() const;
	void	Empty();

private:
// Member data
	WORD	m_CtrlID;	// ID of notifying control; UNDO_ID_INSIGNIFICANT
						// is reserved for flagging insignificant edits
	WORD	m_Code;		// edit function code

// Helpers
	void	Copy(const CUndoState& State);

	friend CUndoManager;
};

#pragma pack(pop)	// restore default packing

inline CUndoState::CUndoState()
{
}

inline WORD CUndoState::GetCtrlID() const
{
	return(m_CtrlID);
}

inline WORD CUndoState::GetCode() const
{
	return(m_Code);
}

inline bool CUndoState::IsMatch(WORD CtrlID, WORD Code) const
{
	return(CtrlID == m_CtrlID && Code == m_Code);
}

inline bool CUndoState::IsSignificant() const
{
	return(m_CtrlID != UNDO_CTRL_ID_INSIGNIFICANT);
}

inline void CUndoState::Empty()
{
	CRefPtr<CRefObj>::SetEmpty();
	memset(&m_Val, 0, sizeof(VALUE));
}

#endif
