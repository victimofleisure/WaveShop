// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      20feb13	initial version

		export dialog
 
*/

// ExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ExportDlg.h"
#include "DocManagerEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

const CExportDlg::ENDIANNESS_INFO CExportDlg::m_EndiannessInfo[] = {
	{SF_ENDIAN_FILE,	IDS_EXPO_ENDIAN_DEFAULT},
	{SF_ENDIAN_LITTLE,	IDS_EXPO_ENDIAN_FORCE_LITTLE},
	{SF_ENDIAN_BIG,		IDS_EXPO_ENDIAN_FORCE_BIG},
	{SF_ENDIAN_CPU,		IDS_EXPO_ENDIAN_FORCE_CPU},
};

CExportDlg::CExportDlg(const CWave& Wave, CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent), m_Wave(Wave)
{
	//{{AFX_DATA_INIT(CExportDlg)
	//}}AFX_DATA_INIT
	m_InitSubtype = 0;
	m_SelFormat = 0;
}

int CExportDlg::GetFormatFromCombos() const
{
	int	format = 0;
	int	sel = m_MajorFormatCombo.GetCurSel();	// get major format combo selection
	if (sel < 0)	// if invalid selection
		return(0);
	format = m_MajorFormat[sel].m_Format;	// set format to selected major format
	sel = m_SubtypeCombo.GetCurSel();	// get subtype combo selection
	if (sel < 0)	// if invalid selection
		return(0);
	int	subtype = INT64TO32(m_SubtypeCombo.GetItemData(sel));	// get subtype from item data
	format |= subtype;	// add selected subtype to format
	sel = m_EndiannessCombo.GetCurSel();	// get endianness combo selection
	if (sel < 0)	// if invalid selection
		return(0);
	format |= m_EndiannessInfo[sel].Option;	// add selected endianness option to format
	return(format);
}

bool CExportDlg::UpdateSubtypeCombo(int MajorFormatIdx)
{
	int	PrevSelSubtype = GetFormatFromCombos() & SF_FORMAT_SUBMASK;
	m_SubtypeCombo.ResetContent();	// reset combo
	int	MajorFormat = m_MajorFormat[MajorFormatIdx].m_Format;	// get major format
	int	sel = -1;	// assume previous selection not applicable
	int	InitSel = 0;
	int	subtypes = m_Subtype.GetSize();
	for (int iSubtype = 0; iSubtype < subtypes; iSubtype++) {	// for each subtype
		const CSndFileFormat&	sff = m_Subtype[iSubtype];
		int	subtype = sff.m_Format;
		int	format = MajorFormat | subtype;	// make composite format
		SF_INFO	info;
		info.format = format;
		info.channels = m_Wave.GetChannels();
		info.samplerate = m_Wave.GetSampleRate();
		if (m_SndFile.FormatCheck(info)) {	// if sndfile supports format
			int	pos = m_SubtypeCombo.AddString(sff.m_Name);	// add subtype name to combo
			m_SubtypeCombo.SetItemData(pos, subtype);	// store subtype in item data
			if (subtype == PrevSelSubtype)	// if subtype was previously selected
				sel = pos;	// reselect previously selected subtype
			if (subtype == m_InitSubtype)	// if subtype matches initial subtype
				InitSel = pos;	// store selection index of initial subtype
		}
	}
	if (sel < 0)	// if previous selection not applicable
		sel = InitSel;	// default to initial subtype if any
	m_SubtypeCombo.SetCurSel(sel);	// set combo selection
	bool	HaveSubtypes = m_SubtypeCombo.GetCount() > 0;	// if any subtypes available
	m_SubtypeCombo.EnableWindow(HaveSubtypes);	// enable subtype combo
	GetDlgItem(IDOK)->EnableWindow(HaveSubtypes);	// enable OK button
	return(TRUE);
}

void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportDlg)
	DDX_Control(pDX, IDC_EXPO_ENDIANNESS, m_EndiannessCombo);
	DDX_Control(pDX, IDC_EXPO_SUBTYPE, m_SubtypeCombo);
	DDX_Control(pDX, IDC_EXPO_MAJOR_FORMAT, m_MajorFormatCombo);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message map

BEGIN_MESSAGE_MAP(CExportDlg, CDialog)
	//{{AFX_MSG_MAP(CExportDlg)
	ON_WM_CREATE()
	ON_CBN_SELCHANGE(IDC_EXPO_MAJOR_FORMAT, OnSelchangeMajorFormat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

int CExportDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_SndFile.Create())	// if can't create sndfile wrapper
		return -1;	// wrapper already displayed error message
	
	return 0;
}

BOOL CExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// retrieve all major formats; no special handling for wave
	m_SndFile.GetMajorFormats(m_MajorFormat, FALSE);
	m_SndFile.GetSubtypes(m_Subtype);	// retrieve subtypes too
	int	MajorFormats = m_MajorFormat.GetSize();
	for (int iFmt = 0; iFmt < MajorFormats; iFmt++) {	// for each major format
		m_MajorFormatCombo.AddString(m_MajorFormat[iFmt].m_Name);	// add to combo
	}
	m_MajorFormatCombo.SetCurSel(0);	// select first major format
	int	Endians = _countof(m_EndiannessInfo);
	for (int iEnd = 0; iEnd < Endians; iEnd++) {	// for each endianness
		m_EndiannessCombo.AddString(LDS(m_EndiannessInfo[iEnd].Name));	// add to combo
	}
	m_EndiannessCombo.SetCurSel(0);	// select first endianness
	if (m_Wave.GetSampleSize() <= SF_FORMAT_PCM_32)	// if sample size is valid subtype
		m_InitSubtype = m_Wave.GetSampleSize();	// use sample size as initial subtype
	OnSelchangeMajorFormat();	// update subtype combo

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExportDlg::OnOK() 
{
	m_SelFormat = GetFormatFromCombos();
	int	sel = m_MajorFormatCombo.GetCurSel();	// get major format combo selection
	if (sel >= 0) {
		const CSndFileFormat&	SelFormat = m_MajorFormat[sel];
		CString	ext = SelFormat.m_Extension;
		m_SelExtension = '.' + ext;
		CString	AliasedExt = CDocManagerEx::GetAliasedExtension(SelFormat.m_Format, ext);
		CString	filter = SelFormat.m_Name + _T(" (") + AliasedExt 
			+ _T(")|") + AliasedExt + '|' + CDocManagerEx::GetAllFilter();
		m_SelFilter = filter;
	}
	CDialog::OnOK();
}

void CExportDlg::OnSelchangeMajorFormat() 
{
	int	sel = m_MajorFormatCombo.GetCurSel();	// get combo selection
	if (sel >= 0) {	// if valid selection
		UpdateSubtypeCombo(sel);
	}
}
