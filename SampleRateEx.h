// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25feb13	initial version
		
		wrapper for Erik de Castro Lopo's samplerate library
 
*/

#ifndef CSAMPLERATEEX_INCLUDED
#define CSAMPLERATEEX_INCLUDED

#include "samplerate.h"
#include "DllWrap.h"
#include "samplerateFuncs.h"

class CWave;

class CSampleRateEx : public WObject {
public:
// Construction
	CSampleRateEx();
	~CSampleRateEx();

// Attributes
	bool	IsCreated() const;
	CString	GetError() const;

// Operations
	bool	Create(int Quality, int Channels);
	void	Destroy();
	bool	Resample(const CWave& SrcWave, CWave& DstWave, int NewSampleRate);

protected:
// Types
	struct ISampleRate {	// sample rate converter interface
		#define SAMPLERATE_DEF(name, ordinal) p##name name;
		#include "samplerateDefs.h"
	};

// Constants
	enum {
		BUF_SIZE = 0x100000,	// buffer size in bytes
	};
	static const int m_FuncOrd[];	// function ordinals

// Data members
	CDLLWrap	m_Lib;			// DLL wrapper
	ISampleRate	*m_src;			// pointer to sample rate converter interface
	CPtrArray	m_FuncPtr;		// array of pointers to exported functions
	SRC_STATE	*m_State;		// pointer to converter state
	int		m_LastError;		// most recent error code
};

#endif
