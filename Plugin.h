// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25jun13	initial version

        plugin instance
 
*/

#ifndef CPLUGIN_INCLUDED
#define	CPLUGIN_INCLUDED

#include "DLLWrap.h"
#include "ArrayEx.h"

struct _LADSPA_Descriptor;
class CWave;

class CPlugin : public WObject {
public:
// Construction
	CPlugin();
	~CPlugin();
	bool	Create(LPCTSTR Path, UINT SampleRate);

// Types
	class CPortStats : public WObject {	// port statistics
	public:
		CDWordArrayEx	m_ControlIn;	// array of control input port indices
		CDWordArrayEx	m_ControlOut;	// array of control output port indices
		CDWordArrayEx	m_AudioIn;		// array of audio input port indices
		CDWordArrayEx	m_AudioOut;		// array of audio output port indices
	};
	typedef CArrayEx<float, float> CParamArray;

// Attributes
	bool	IsCreated() const;
	CString	GetName() const;
	bool	IsChannelCountCompatible(const CByteArray& ChanSel) const;
	static	bool	GetPortStats(const _LADSPA_Descriptor *Desc, CPortStats& Stats);
	static	void	GetSelectedChannelIndices(const CByteArray& ChanSel, CDWordArrayEx& SelChanIdx);

// Operations
	bool	Run(CWave& Wave, const CW64IntRange& Selection, const CByteArray& ChanSel, const CParamArray& Param);
	static const _LADSPA_Descriptor	*Load(CDLLWrap& Dll, LPCTSTR Path);

protected:
// Types
	typedef CArrayEx<float, float> CSampleBuffer;
	typedef CArrayEx<CSampleBuffer, CSampleBuffer&> CSampleBufferArray;

// Data members
	CDLLWrap	m_Dll;			// DLL instance
	const	_LADSPA_Descriptor	*m_Desc;	// plugin descriptor
	void	*m_Plugin;			// plugin instance handle

// Helpers
	bool	Instantiate(UINT SampleRate);
	void	Cleanup();
	void	ConnectControlPorts(const CPortStats& PortStats, CParamArray& Input, CParamArray& Output);
};

inline bool CPlugin::IsCreated() const
{
	return(m_Plugin != NULL);
}

#endif
