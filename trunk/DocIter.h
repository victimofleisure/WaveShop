// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version

		iterate documents, views, etc.
 
*/

#pragma once

class CDocTemplateIter : public WObject {
public:
	CDocTemplateIter();
	CDocTemplate	*GetNextDocTemplate();

protected:
	POSITION	m_TplPos;	// template position
};

class CDocIter : public WObject {
public:
	CDocIter(CDocTemplate *pTpl);
	CDocument	*GetNextDoc();

protected:
	CDocTemplate	*m_pTpl;	// template pointer
	POSITION	m_DocPos;		// document position
};

class CViewIter : public WObject {
public:
	CViewIter(CDocument *pDoc);
	CView	*GetNextView();

protected:
	CDocument	*m_pDoc;		// document pointer
	POSITION	m_ViewPos;		// view position
};

class CAllDocIter : public WObject {
public:
	CAllDocIter();
	CDocument	*GetNextDoc();

protected:
	CDocTemplateIter	m_TplIter;	// template iterator
	CDocTemplate	*m_pTpl;		// template pointer
	POSITION	m_DocPos;			// document position
};

class CAllViewIter : public WObject {
public:
	CAllViewIter();
	CView	*GetNextView();

protected:
	CAllDocIter	m_DocIter;		// document iterator
	CDocument	*m_pDoc;		// document pointer
	POSITION	m_ViewPos;		// view position
};

class CMDIChildIter : public WObject {
public:
	CMDIChildWnd	*GetNextChild();

protected:
	CAllDocIter	m_DocIter;	// all document iterator
};
