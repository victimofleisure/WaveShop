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
		02		23nov07	support Unicode
		03		03jan08	replace CSmartBuf with CRefPtr
		04		18mar08	make CRefPtr a template

        undo state container
 
*/

#include "stdafx.h"
#include "UndoState.h"

CUndoState::CUndoState(const CUndoState& State) :
	CRefPtr<CRefObj>(State)	// call base class copy constructor
{
	Copy(State);
}

CUndoState& CUndoState::operator=(const CUndoState& State)
{
	CRefPtr<CRefObj>::operator=(State);	// call base class assignment operator
    Copy(State);
	return(*this);
}

void CUndoState::Copy(const CUndoState& State)
{
	m_Val = State.m_Val;
	m_CtrlID = State.m_CtrlID;
	m_Code = State.m_Code;
}

CString CUndoState::DumpState() const
{
	CString	s;
	s.Format(_T("CtrlID=%d Code=%d a=0x%x b=0x%x"), 
		m_CtrlID, m_Code, m_Val.p.x.i, m_Val.p.y.i);
	return(s);
}
