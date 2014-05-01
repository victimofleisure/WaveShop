// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		05aug06	initial version
		01		20oct07	move GetEdit into .cpp

		handlers for focused edit control

*/

#ifndef CFOCUSEDIT_INCLUDED
#define CFOCUSEDIT_INCLUDED

class CFocusEdit {
public:
	static	bool	Undo();
	static	bool	Cut();
	static	bool	Copy();
	static	bool	Paste();
	static	bool	Insert();
	static	bool	Delete();
	static	bool	SelectAll();
	static	bool	UpdateUndo(CCmdUI* pCmdUI);
	static	bool	UpdateCut(CCmdUI* pCmdUI);
	static	bool	UpdateCopy(CCmdUI* pCmdUI);
	static	bool	UpdatePaste(CCmdUI* pCmdUI);
	static	bool	UpdateInsert(CCmdUI* pCmdUI);
	static	bool	UpdateDelete(CCmdUI* pCmdUI);
	static	bool	UpdateSelectAll(CCmdUI* pCmdUI);

// protected
	static	CEdit	*GetEdit();
	static	bool	IsReadOnly(CEdit *ep);
	static	bool	HasSelection(CEdit *ep);
};

inline bool CFocusEdit::IsReadOnly(CEdit *ep)
{
	return((ep->GetStyle() & ES_READONLY) != 0);
}

inline bool CFocusEdit::HasSelection(CEdit *ep)
{
	int	nBeg, nEnd;
	ep->GetSel(nBeg, nEnd);
	return(nBeg != nEnd);
}

#endif
