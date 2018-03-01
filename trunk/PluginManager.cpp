// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27jun13	initial version

        plugin manager
 
*/

#include "stdafx.h"
#include "WaveShop.h"
#include "PluginManager.h"
#include "PathStr.h"
#include "DocIter.h"
#include "ladspa.h"
#include "DLLWrap.h"

inline CPluginManager::CPluginMenuItem::CPluginMenuItem()
{
}

inline CPluginManager::CPluginMenuItem::CPluginMenuItem(CString Name, int CmdID) : CString(Name)
{
	m_CmdID = CmdID;
}

inline CPluginManager::CPluginMenuItem& CPluginManager::CPluginMenuItem::operator=(const CPluginMenuItem& Item)
{
	CString::operator=(Item);	// copy plugin name (CString base class)
	m_CmdID = Item.m_CmdID;
	return(*this);
}

CPluginManager::CPluginInfo::CPluginInfo()
{
	m_Param = NULL;
	m_NumParams = 0;
}

CPluginManager::CPluginInfo::~CPluginInfo()
{
	delete [] m_Param;
}

void CPluginManager::GetParams(int PluginIdx, CPlugin::CParamArray& Param) const
{
	const CPluginInfo&	info = m_PluginInfo[PluginIdx];
	int	params = info.m_NumParams;
	Param.SetSize(params);	// size caller's array
	memcpy(Param.GetData(), info.m_Param, params * sizeof(float));	// copy parameters
}

void CPluginManager::SetParams(int PluginIdx, const CPlugin::CParamArray& Param)
{
	CPluginInfo&	info = m_PluginInfo[PluginIdx];
	int	params = Param.GetSize();
	info.m_Param = new float[params];	// allocate parameter array
	memcpy(info.m_Param, Param.GetData(), params * sizeof(float));	// copy parameters
	info.m_NumParams = params;	// set parameter count last in case allocator throws
}

int CPluginManager::GetPluginName(int PluginIdx, CString& Name) const
{
	return(m_PluginMenu.GetMenuString(CMD_ID_FIRST + PluginIdx, Name, MF_BYCOMMAND));
}

CString CPluginManager::GetPluginFolder() const
{
	CPathStr	folder(theApp.GetAppFolder());
	folder.Append(_T("Plugins"));	// default to Plugins subfolder of app folder
	theApp.RdReg2String(_T("PluginFolder"), folder);	// allow registry override
	return(folder);
}

bool CPluginManager::IteratePlugins()
{
	CPluginMenuItemArray	MenuItem;
	CFileFind	ff;
	CPathStr	FindPath(GetPluginFolder());
	FindPath.Append(_T("*.dll"));
	BOOL	Working = ff.FindFile(FindPath);
	int	CmdID = CMD_ID_FIRST;
	while (Working) {
		Working = ff.FindNextFile();
		CDLLWrap	dll;
		const LADSPA_Descriptor	*desc = CPlugin::Load(dll, ff.GetFilePath());
		if (desc != NULL) {	// if LADSPA plugin was loaded
			CPluginMenuItem	item(desc->Name, CmdID);
			MenuItem.Add(item);
			CPathStr	name(ff.GetFileName());
			name.RemoveExtension();
			int	plugins = m_PluginInfo.GetSize();
			m_PluginInfo.SetSize(plugins + 1);	// avoid needing assignment operator
			m_PluginInfo[plugins].m_FileName = name;
			CmdID++;
		}
	}
	MenuItem.Sort();
	int	plugins = MenuItem.GetSize();
	if (plugins) {	// if any plugins were found
		if (!m_PluginMenu.CreateMenu())	// create plugin menu
			AfxThrowResourceException();
		CString	sRepeat(LPCTSTR(IDS_PLUGIN_REPEAT));
		if (!m_PluginMenu.AppendMenu(MF_STRING, ID_PLUGIN_REPEAT, sRepeat))
			AfxThrowResourceException();
		if (!m_PluginMenu.AppendMenu(MF_SEPARATOR, 0, LPCTSTR(NULL)))
			AfxThrowResourceException();
		for (int iPlugin = 0; iPlugin < plugins; iPlugin++) {	// for each plugin
			// append to plugin menu
			const CPluginMenuItem& item = MenuItem[iPlugin];
			if (!m_PluginMenu.AppendMenu(MF_STRING, item.m_CmdID, item))
				AfxThrowResourceException();
		}
	}
	return(plugins > 0);
}
