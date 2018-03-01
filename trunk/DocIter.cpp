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

#include "stdafx.h"
#include "DocIter.h"

CDocTemplateIter::CDocTemplateIter()
{
	m_TplPos = AfxGetApp()->GetFirstDocTemplatePosition();
}

CDocTemplate *CDocTemplateIter::GetNextDocTemplate()
{
	if (m_TplPos == NULL)
		return(NULL);
	return(AfxGetApp()->GetNextDocTemplate(m_TplPos));
}

CDocIter::CDocIter(CDocTemplate *pTpl)
{
	m_pTpl = pTpl;
	m_DocPos = pTpl != NULL ? pTpl->GetFirstDocPosition() : NULL;
}

CDocument *CDocIter::GetNextDoc()
{
	if (m_DocPos == NULL)
		return(NULL);
	return(m_pTpl->GetNextDoc(m_DocPos));
}

CViewIter::CViewIter(CDocument *pDoc)
{
	m_pDoc = pDoc;
	m_ViewPos = pDoc != NULL ? pDoc->GetFirstViewPosition() : NULL;
}

CView *CViewIter::GetNextView()
{
	if (m_ViewPos == NULL)
		return(NULL);
	return(m_pDoc->GetNextView(m_ViewPos));
}

CAllDocIter::CAllDocIter()
{
	m_DocPos = NULL;
	m_pTpl = NULL;
}

CDocument *CAllDocIter::GetNextDoc()
{
	while (m_DocPos == NULL) {	// while no document position
		if ((m_pTpl = m_TplIter.GetNextDocTemplate()) == NULL)	// if no more templates
			return(NULL);
		m_DocPos = m_pTpl->GetFirstDocPosition();
	}
	return(m_pTpl->GetNextDoc(m_DocPos));
}

CAllViewIter::CAllViewIter()
{
	m_pDoc = NULL;
	m_ViewPos = NULL;
}

CView *CAllViewIter::GetNextView()
{
	while (m_ViewPos == NULL) {	// while no view position
		if ((m_pDoc = m_DocIter.GetNextDoc()) == NULL)	// if no more documents
			return(NULL);
		m_ViewPos = m_pDoc->GetFirstViewPosition();
	}
	return(m_pDoc->GetNextView(m_ViewPos));
}

CMDIChildWnd *CMDIChildIter::GetNextChild()
{
	CDocument	*pDoc = m_DocIter.GetNextDoc();
	if (pDoc != NULL) {
		POSITION	pos = pDoc->GetFirstViewPosition();
		if (pos != NULL) {
			CView	*pView = pDoc->GetNextView(pos);
			if (pView != NULL)
				return(DYNAMIC_DOWNCAST(CMDIChildWnd, pView->GetParentFrame()));
		}
	}
	return(NULL);
}
