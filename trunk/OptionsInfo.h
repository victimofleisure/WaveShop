// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version
		01		12feb13	add view palette
		02		02mar13	add MP3 import quality
		03		04mar13	add VBR encoding quality
		04		12mar13	add show channel names
        05      01apr13	add real-time spectrum analyzer
        06      17apr13	add temporary files folder
        07      20apr13	add MP4 import
		08		27apr13	add record device
		09		28apr13	add persistence and defaults
		10		05may13	use GUID instead of description for device persistence
		11		08may13	upcast via assignment operator
		12		10may13	add meter clip threshold
		13		17may13	add record parameters
		14		21may13	move GetTempFolderPath to .cpp

		container for options information
 
*/

#pragma once

#include <afxtempl.h>
#include "DSPlayer.h"

struct VIEW_PALETTE {
	#define VIEW_COLOR_DEF(name, tag, R, G, B) tag,
	enum {	// palette as enum
		#include "ViewColors.h"
		COLORS
	};
	union {
		#define VIEW_COLOR_DEF(name, tag, R, G, B) COLORREF name;
		struct {	// palette as struct
			#include "ViewColors.h"
		};
		COLORREF	Color[COLORS];	// palette as color array
	};
};

struct RTSA_PARMS {	// real-time spectrum analyzer parameters
	int		WindowFunction;		// index of window function
	int		WindowSize;			// window size, in frames; must be a power of two
	int		Averaging;			// number of additional windows to analyze per clock
	int		PlotStyle;			// plot style; see enum in SpectrumBar.h
	int		FreqAxisType;		// if non-zero, frequency axis is logarithmic
	int		ChannelMode;		// if non-zero, separate channels, else combine them
	int		ShowPeaks;			// show peaks style; see enum in SpectrumBar.h
	int		PeakHoldTime;		// peak hold duration, in milliseconds
	int		PeakDecay;			// peak decay, in decibels per second
	COLORREF	PlotBkgndColor;	// plot background color
	COLORREF	PlotGridColor;	// plot grid color
};

struct RECORD_PARMS {	// record parameters
	enum {	// hot key functions; append only, don't reorder
		HK_START,				// start recording
		HK_STOP,				// stop recording
		HOT_KEYS
	};
	struct HOT_KEY_DEFS {
		DWORD	Def[HOT_KEYS];	// array of system-wide hot key definitions
	};
	int		ActivationType;		// activation type; see enum in RecordDlg.h
	UINT	Channels;			// number of channels
	UINT	SampleRate;			// sample rate, in Hertz
	UINT	SampleSize;			// sample size, in bits
	float	StartLevel;			// level at which recording starts, in decibels
	float	StartDuration;		// minimum duration of start level, in seconds
	float	StopLevel;			// level at which recording stops, in decibels
	float	StopDuration;		// minimum duration of stop level, in seconds
	HOT_KEY_DEFS	HotKeys;	// hot key definitions
};

struct CUSTOM_COLORS {
	COLORREF	Color[16];	// for color dialog
};

struct OPTIONS_INFO {
	int		m_ZoomStepHorz;		// horizontal zoom step as a percentage
	int		m_ZoomStepVert;		// vertical zoom step as a percentage
	bool	m_TimeInFrames;		// if true, show time in sample frames
	bool	m_VertSyncChans;	// if true, vertically synchronize channels
	bool	m_VertZoomCursor;	// if true, vertical zoom origin is cursor
	bool	m_ShowInterpolation;	// if true, highlight interpolation
	bool	m_CheckForUpdates;	// if true, automatically check for updates
	bool	m_ShowChannelNames;	// if true, display channel names in view
	bool	m_CustomTempFolder;	// if true, use custom temporary files folder
	bool	m_MP4Downmix;		// if true, mix MP4 surround down to stereo
	int		m_UndoLevels;		// undo levels, or -1 for unlimited
	UINT	m_PlayBufferSize;	// size of playback buffer, in milliseconds
	UINT	m_RecordBufferSize;	// size of recording buffer, in milliseconds
	int		m_DiskThreshold;	// size above which to use disk storage, in MB
	int		m_MaxDensity;		// view maximum density, in samples per pixel
	int		m_MP3ImportQuality;	// MP3 import quality: 0 == 16-bit, 1 == 24-bit
	int		m_VBREncodingQuality;	// VBR encoding quality, from 0 to 100
	int		m_MP4ImportQuality;	// MP4 import quality: 0 == 16-bit, 1 == 24-bit
	int		m_MeterClipThreshold;	// number of consecutive samples on the rails
	VIEW_PALETTE	m_ViewPalette;	// view palette
	CUSTOM_COLORS	m_CustomColors;	// custom colors for color dialog
	RTSA_PARMS	m_RTSA;			// real-time spectrum analyzer parameters
	RECORD_PARMS	m_Record;	// record parameters
};

class COptionsInfo : public CObject, public OPTIONS_INFO {
public:
	DECLARE_SERIAL(COptionsInfo);

// Construction
	COptionsInfo();
	COptionsInfo(const COptionsInfo& Info);
	COptionsInfo(const OPTIONS_INFO& OptionsInfo);
	COptionsInfo& operator=(const COptionsInfo& Info);

// Attributes
	void	SetOptionsInfo(const OPTIONS_INFO& OptionsInfo);
	CString	GetTempFolderPath() const;

// Operations
	void	Load();
	void	Store();

// Constants
	enum {
		ARCHIVE_VERSION = 1			// archive version number
	};
	static const OPTIONS_INFO	m_DefaultOptionsInfo;	// default scalar options
	static const COptionsInfo	m_Defaults;				// default options

// Public data; members MUST be included in Copy
	GUID	m_PlayDeviceGuid;	// identifier of selected playback device
	GUID	m_RecordDeviceGuid;	// identifier of selected recording device
	CString	m_TempFolderPath;	// path to temporary files folder
	CString	m_RecordFolderPath;	// path to folder for recordings

// Operations
	OPTIONS_INFO&	GetBaseInfo();

protected:
// Helpers
	void	Copy(const COptionsInfo& Info);
};

inline COptionsInfo::COptionsInfo(const COptionsInfo& Info)
{
	Copy(Info);
}

inline COptionsInfo& COptionsInfo::operator=(const COptionsInfo& Info)
{
	Copy(Info);
	return(*this);
}

inline void COptionsInfo::SetOptionsInfo(const OPTIONS_INFO& OptionsInfo)
{
	OPTIONS_INFO::operator=(OptionsInfo);
}

inline COptionsInfo::COptionsInfo(const OPTIONS_INFO& OptionsInfo)
{
	SetOptionsInfo(OptionsInfo);
}

inline OPTIONS_INFO& COptionsInfo::GetBaseInfo()
{
	return(*this);	// automatic upcast to base struct
}
