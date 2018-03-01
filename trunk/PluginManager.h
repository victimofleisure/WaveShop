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

#ifndef CPLUGINMANAGER_INCLUDED
#define	CPLUGINMANAGER_INCLUDED

#include "Plugin.h"
#include "SortArray.h"

class CPluginManager : public WObject {
public:
// Constants
	enum {
		CMD_ID_FIRST	= 0xD000,
		CMD_ID_LAST		= 0xDFFF,
	};

// Attributes
	HMENU	GetMenu();
	int		GetPluginCount() const;
	int		GetPluginIndex(int CommandID) const;
	CString	GetFileName(int PluginIdx) const;
	int		GetPluginName(int PluginIdx, CString& Name) const;
	void	GetParams(int PluginIdx, CPlugin::CParamArray& Param) const;
	void	SetParams(int PluginIdx, const CPlugin::CParamArray& Param);
	CString	GetPluginFolder() const;

// Operations
	bool	IteratePlugins();

protected:
// Types
	class CPluginMenuItem : public CString {
	public:
		CPluginMenuItem();
		CPluginMenuItem(CString Name, int CmdID);
		CPluginMenuItem& operator=(const CPluginMenuItem& Item);
		int		m_CmdID;		// plugin command ID
	};
	class CPluginInfo : public WObject {
	public:
		CPluginInfo();
		~CPluginInfo();
		CString	m_FileName;		// plugin file name, without extension
		float	*m_Param;		// array of plugin parameter values
		int		m_NumParams;	// number of parameters in array
	};
	typedef CSortArray<CPluginMenuItem, CPluginMenuItem&> CPluginMenuItemArray;
	typedef CArrayEx<CPluginInfo, CPluginInfo&> CPluginInfoArray;

// Data members
	CPluginInfoArray	m_PluginInfo;	// array of plugin info
	CMenu	m_PluginMenu;		// plugin popup menu
};

inline HMENU CPluginManager::GetMenu()
{
	return(m_PluginMenu.m_hMenu);
}

inline int CPluginManager::GetPluginCount() const
{
	return(m_PluginInfo.GetSize());
}

inline CString CPluginManager::GetFileName(int PluginIdx) const
{
	return(m_PluginInfo[PluginIdx].m_FileName);
}

inline int CPluginManager::GetPluginIndex(int CommandID) const
{
	int	PluginIdx = CommandID - CPluginManager::CMD_ID_FIRST;
	ASSERT(PluginIdx >= 0 && PluginIdx < GetPluginCount());
	return(PluginIdx);
}

#endif
