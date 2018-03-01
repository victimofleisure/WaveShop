// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		08mar10	initial version
        01		04oct12	move rounding to its own file
        02		13oct12	add W64ULONG
		03		01dec12	add W64 dynamic casts
		04		29dec12	move x64 wrappers to their own file
		05		24jan13	disable undo natter
		06		25feb13	move app-specific stuff to end
        07      27feb13	add MPEG filter
        08      20apr13	add AAC/MP4 filter
        09      29apr13	add capture notifications
        10      24may13	add initial record message
        11      25jun13	add float range
		12		28jun13	add plugins found message

		global definitions and inlines

*/

#pragma once

#pragma warning(disable : 4100)	// unreferenced formal parameter

// minimal base for non-CObject classes
#include "WObject.h"

// registry strings
#define REG_SETTINGS		_T("Settings")

// key status bits for GetKeyState and GetAsyncKeyState
#define GKS_TOGGLED			0x0001
#define GKS_DOWN			0x8000

// trig macros
#define PI 3.141592653589793
#define DTR(x) (x * PI / 180)	// degrees to radians
#define RTD(x) (x * 180 / PI)	// radians to degrees

// clamp a value to a range
#define CLAMP(x, lo, hi) (min(max((x), (lo)), (hi)))

// trap bogus default case in switch statement
#define NODEFAULTCASE	ASSERT(0)

// load string from resource via temporary object
#define LDS(x) CString((LPCTSTR)x)

#if _MFC_VER < 0x0800
// calculate number of elements in a fixed-size array
#define _countof(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

// atof's generic-text wrapper is missing in MFC 6
#ifndef _tstof
#ifdef UNICODE
#define _tstof(x) _tcstod(x, NULL)
#else
#define _tstof(x) atof(x)
#endif
#endif

#if _MFC_VER < 0x0800
#define genericException generic	// generic was deprecated in .NET 2005
#endif

#if _MFC_VER < 0x0700
#define TBS_DOWNISLEFT          0x0400  // Down=Left and Up=Right (default is Down=Right and Up=Left)
#endif

inline void StoreBool(CArchive& ar, bool flag)
{
#if _MFC_VER >= 0x0700
	ar << flag;
#else	// MFC 6 CArchive doesn't support bool
	BYTE	byte = flag;
	ar << byte;
#endif
}

inline void LoadBool(CArchive& ar, bool& flag)
{
#if _MFC_VER >= 0x0700
	ar >> flag;
#else	// MFC 6 CArchive doesn't support bool
	BYTE	byte;
	ar >> byte;
	flag = byte != 0;
#endif
}

#if _MSC_VER < 1300
#define ACTIVATEAPPTASK HTASK
#else
#define ACTIVATEAPPTASK DWORD
#endif

// x64 wrappers
#include "Wrapx64.h"

// optimized rounding and truncation
#include "Round.h"

// range template and common ranges
#include "Range.h"
typedef CRange<int> CIntRange;
typedef CRange<UINT> CUIntRange;
typedef CRange<float> CFloatRange;
typedef CRange<double> CDblRange;
typedef CRange<W64INT> CW64IntRange;

#if _MFC_VER > 0x0600
// suppress spurious level 4 warning on ceil function
#pragma warning (push)
#pragma warning (disable: 4985)	// attributes not present on previous declaration.
#include <math.h>
#pragma warning (pop)
#endif

// app-specific globals

enum {	// user windows messages
	UWM_FIRST = WM_APP,
	UWM_HANDLEDLGKEY,			// wParam: MSG pointer, lParam: none
	UWM_CHAN_BAR_UPDATE_WIDTH,	// wParam: none, lParam: none
	UWM_VIEW_FIT_IN_WINDOW,		// wParam: none, lParam: none
	UWM_MODELESS_DESTROY,		// wParam: dialog pointer, lParam: none
	UWM_CAPTURE_WRITE,			// wParam: none, lParam: none
	UWM_CAPTURE_ERROR,			// wParam: none, lParam: none
	UWM_INITIAL_RECORD,			// wParam: none, lParam: none
	UWM_PLUGINSFOUND,			// wParam: none, lParam: none
};

// file extensions
#define WAV_EXT	_T(".wav")
#define MPEG_FILTER	_T(";*.mp1;*.mp2;*.mp3;*.mpa")
#define MP4_FILTER	_T(";*.aac;*.mp4;*.m4a")

#define UNDO_NATTER 0

// data validation method to flunk a control
void DDV_Fail(CDataExchange* pDX, int nIDC);

inline void AFXAPI DDX_Check(CDataExchange* pDX, int nIDC, bool& value)
{
	BOOL	v = value;
	DDX_Check(pDX, nIDC, v);
	value = v != 0;
}

#if _MFC_VER < 0x0700
inline unsigned char _BitScanReverse(unsigned long* Index, unsigned long Mask)
{
	__asm {
		bsr		eax, Mask		// bit scan reverse
		mov		edx, Index		// load destination address
		mov		[edx], eax		// store output bit index
	}
	return(Mask != 0);
}
#endif

enum {
	PLOT_DATA_TIP_PRECISION = 2,	// number of decimal places in plot data tip
};
