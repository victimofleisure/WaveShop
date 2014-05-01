// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09oct12	initial version
        01      17apr13	add temporary files folder
		02		27apr13	add record device
		03		28apr13	add persistence and defaults
		04		05may13	use GUID instead of description for device persistence
		05		08may13	upcast via assignment operator
		06		10may13	add meter clip threshold
		07		17may13	add record parameters
		08		21may13	in GetTempFolderPath, make path absolute

		container for options information
 
*/

#include "stdafx.h"
#include "WaveShop.h"
#include "OptionsInfo.h"

// view
#define RK_ZOOM_STEP_HORZ		_T("ZoomStepHorz")
#define RK_ZOOM_STEP_VERT		_T("ZoomStepVert")
#define RK_TIME_IN_FRAMES		_T("TimeInFrames")
#define RK_VERT_SYNC_CHANS		_T("VertSyncChans")
#define RK_VERT_ZOOM_CURSOR		_T("VertZoomCursor")
#define RK_SHOW_INTERPOLATION	_T("ShowInterpolation")
#define RK_CHECK_FOR_UPDATES	_T("CheckForUpdates")
#define RK_SHOW_CHANNEL_NAMES	_T("ShowChannelNames")
#define RK_MAX_DENSITY			_T("MaxDensity")
#define RK_VIEW_PALETTE			_T("ViewPalette")
#define RK_CUSTOM_COLORS		_T("CustomColors")

// edit
#define RK_UNDO_LEVELS			_T("UndoLevels")
#define RK_DISK_THRESHOLD		_T("DiskThreshold")
#define RK_CUSTOM_TEMP_FOLDER	_T("CustomTempFolder")
#define RK_TEMP_FOLDER_PATH		_T("TempFolderPath")

// audio
#define RK_PLAY_DEVICE			_T("PlayDevice")
#define RK_PLAY_BUFFER_SIZE		_T("PlayBufferSize")
#define RK_RECORD_DEVICE		_T("RecordDevice")
#define RK_RECORD_BUFFER_SIZE	_T("RecordBufferSize")
#define RK_MP3_IMPORT_QUALITY	_T("MP3ImportQuality")
#define RK_VBR_ENCODING_QUALITY	_T("VBREncodingQuality")
#define RK_MP4_IMPORT_QUALITY	_T("MP4ImportQuality")
#define RK_MP4_DOWNMIX			_T("MP4Downmix")
#define RK_METER_CLIP_THRESHOLD	_T("MeterClipThreshold")

// RTSA
#define RK_RTSA					REG_SETTINGS _T("\\RTSA")
#define RK_WINDOW_FUNCTION		_T("WindowFunction")
#define RK_WINDOW_SIZE			_T("WindowSize")
#define RK_PLOT_STYLE			_T("PlotStyle")
#define RK_FREQ_AXIS			_T("FreqAxis")
#define RK_CHANNEL_MODE			_T("ChannelMode")
#define RK_AVERAGING			_T("Averaging")
#define RK_PLOT_BKGND_COLOR		_T("PlotBkgndColor")
#define RK_PLOT_GRID_COLOR		_T("PlotGridColor")
#define RK_SHOW_PEAKS			_T("ShowPeaks")
#define RK_PEAK_HOLD_TIME		_T("PeakHoldTime")
#define RK_PEAK_DECAY			_T("PeakDecay")

// Record
#define RK_RECORD				REG_SETTINGS _T("\\Record")
#define RK_REC_ACTIVATION		_T("Activation")
#define RK_REC_CHANNELS			_T("Channels")
#define RK_REC_SAMPLE_RATE		_T("SampleRate")
#define RK_REC_SAMPLE_SIZE		_T("SampleSize")
#define RK_REC_START_LEVEL		_T("StartLevel")
#define RK_REC_START_DURATION	_T("StartDuration")
#define RK_REC_STOP_LEVEL		_T("StopLevel")
#define RK_REC_STOP_DURATION	_T("StopDuration")
#define RK_REC_FOLDER_PATH		_T("FolderPath")
#define RK_REC_HOT_KEYS			_T("HotKeys")

const OPTIONS_INFO COptionsInfo::m_DefaultOptionsInfo = {
	200,	// m_ZoomStepHorz
	200,	// m_ZoomStepVert
	FALSE,	// m_TimeInFrames
	TRUE,	// m_VertSyncChans
	TRUE,	// m_VertZoomCursor
	FALSE,	// m_ShowInterpolation
	TRUE,	// m_CheckForUpdates
	FALSE,	// m_ShowCaptions
	FALSE,	// m_CustomTempFolder
	FALSE,	// m_MP4Downmix
	-1,		// m_UndoLevels
	100,	// m_PlayBufferSize
	1000,	// m_RecordBufferSize
	0,		// m_DiskThreshold
	300,	// m_MaxDensity
	0,		// m_MP3ImportQuality
	40,		// m_VBREncodingQuality
	0,		// m_MP4ImportQuality
	3,		// m_MeterClipThreshold
	{		// m_ViewPalette
		#define VIEW_COLOR_DEF(name, tag, R, G, B) RGB(R, G, B),
		#include "ViewColors.h"
	},
	{		// m_CustomColors
		#define VIEW_COLOR_DEF(name, tag, R, G, B) RGB(R, G, B),
		#include "ViewColors.h"
	},
	{		// m_RTSA
		CSpectrumAnal::WF_HANN,		// WindowFunction
		4096,						// WindowSize
		0,							// Averaging
		CSpectrumBar::PS_LINE,		// PlotStyle
		CSpectrumBar::FAT_LOG,		// FreqAxisType
		CSpectrumBar::CM_SEPARATE,	// ChannelMode
		FALSE,						// ShowPeaks
		500,						// PeakHoldTime
		20,							// PeakDecay
		RGB(255, 255, 255),			// PlotBkgndColor
		RGB(192, 192, 192),			// PlotGridColor
	},
	{		// m_Record
		0,			// ActivationType
		2,			// Channels
		44100,		// SampleRate
		16,			// SampleSize
		-12,		// StartLevel
		0,			// StartDuration
		-36,		// StopLevel
		1,			// StopDuration
	}
};

const COptionsInfo COptionsInfo::m_Defaults(m_DefaultOptionsInfo);

IMPLEMENT_SERIAL(COptionsInfo, CObject, 1);

COptionsInfo::COptionsInfo()
{
	ZeroMemory(&GetBaseInfo(), sizeof(OPTIONS_INFO));
	ZeroMemory(&m_PlayDeviceGuid, sizeof(GUID));
	ZeroMemory(&m_RecordDeviceGuid, sizeof(GUID));
}

void COptionsInfo::Copy(const COptionsInfo& Info)
{
	GetBaseInfo().operator=(Info);	// copy base struct
	m_PlayDeviceGuid	= Info.m_PlayDeviceGuid;
	m_RecordDeviceGuid	= Info.m_RecordDeviceGuid;
	m_TempFolderPath	= Info.m_TempFolderPath;
	m_RecordFolderPath	= Info.m_RecordFolderPath;
}

void COptionsInfo::Load()
{
	// view
	theApp.RdReg2Int(RK_ZOOM_STEP_HORZ, m_ZoomStepHorz);
	theApp.RdReg2Int(RK_ZOOM_STEP_VERT, m_ZoomStepVert);
	theApp.RdReg2Bool(RK_TIME_IN_FRAMES, m_TimeInFrames);
	theApp.RdReg2Bool(RK_VERT_SYNC_CHANS, m_VertSyncChans);
	theApp.RdReg2Bool(RK_VERT_ZOOM_CURSOR, m_VertZoomCursor);
	theApp.RdReg2Bool(RK_VERT_ZOOM_CURSOR, m_VertZoomCursor);
	theApp.RdReg2Bool(RK_SHOW_INTERPOLATION, m_ShowInterpolation);
	theApp.RdReg2Bool(RK_CHECK_FOR_UPDATES, m_CheckForUpdates);
	theApp.RdReg2Bool(RK_SHOW_CHANNEL_NAMES, m_ShowChannelNames);
	theApp.RdReg2Int(RK_MAX_DENSITY, m_MaxDensity);
	theApp.RdReg2Struct(RK_VIEW_PALETTE, m_ViewPalette);
	theApp.RdReg2Struct(RK_CUSTOM_COLORS, m_CustomColors);
	// edit
	theApp.RdReg2Int(RK_UNDO_LEVELS, m_UndoLevels);
	theApp.RdReg2Int(RK_DISK_THRESHOLD, m_DiskThreshold);
	theApp.RdReg2Bool(RK_CUSTOM_TEMP_FOLDER, m_CustomTempFolder);
	theApp.RdReg2String(RK_TEMP_FOLDER_PATH, m_TempFolderPath);
	// audio
	theApp.RdReg2Struct(RK_PLAY_DEVICE, m_PlayDeviceGuid);
	theApp.RdReg2UInt(RK_PLAY_BUFFER_SIZE, m_PlayBufferSize);
	theApp.RdReg2Struct(RK_RECORD_DEVICE, m_RecordDeviceGuid);
	theApp.RdReg2UInt(RK_RECORD_BUFFER_SIZE, m_RecordBufferSize);
	theApp.RdReg2Int(RK_MP3_IMPORT_QUALITY, m_MP3ImportQuality);
	theApp.RdReg2Int(RK_VBR_ENCODING_QUALITY, m_VBREncodingQuality);
	theApp.RdReg2Int(RK_MP4_IMPORT_QUALITY, m_MP4ImportQuality);
	theApp.RdReg2Bool(RK_MP4_DOWNMIX, m_MP4Downmix);
	theApp.RdReg2Int(RK_METER_CLIP_THRESHOLD, m_MeterClipThreshold);
	// RTSA
	theApp.RdRegEx2Int(RK_RTSA, RK_WINDOW_FUNCTION, m_RTSA.WindowFunction);
	theApp.RdRegEx2Int(RK_RTSA, RK_WINDOW_SIZE, m_RTSA.WindowSize);
	theApp.RdRegEx2Int(RK_RTSA, RK_PLOT_STYLE, m_RTSA.PlotStyle);
	theApp.RdRegEx2Int(RK_RTSA, RK_FREQ_AXIS, m_RTSA.FreqAxisType);
	theApp.RdRegEx2Int(RK_RTSA, RK_CHANNEL_MODE, m_RTSA.ChannelMode);
	theApp.RdRegEx2Int(RK_RTSA, RK_AVERAGING, m_RTSA.Averaging);
	theApp.RdRegEx2ULong(RK_RTSA, RK_PLOT_BKGND_COLOR, m_RTSA.PlotBkgndColor);
	theApp.RdRegEx2ULong(RK_RTSA, RK_PLOT_GRID_COLOR, m_RTSA.PlotGridColor);
	theApp.RdRegEx2Int(RK_RTSA, RK_SHOW_PEAKS, m_RTSA.ShowPeaks);
	theApp.RdRegEx2Int(RK_RTSA, RK_PEAK_HOLD_TIME, m_RTSA.PeakHoldTime);
	theApp.RdRegEx2Int(RK_RTSA, RK_PEAK_DECAY, m_RTSA.PeakDecay);
	// Record
	theApp.RdRegEx2Int(RK_RECORD, RK_REC_ACTIVATION, m_Record.ActivationType);
	theApp.RdRegEx2UInt(RK_RECORD, RK_REC_CHANNELS, m_Record.Channels);
	theApp.RdRegEx2UInt(RK_RECORD, RK_REC_SAMPLE_RATE, m_Record.SampleRate);
	theApp.RdRegEx2UInt(RK_RECORD, RK_REC_SAMPLE_SIZE, m_Record.SampleSize);
	theApp.RdRegEx2Float(RK_RECORD, RK_REC_START_LEVEL, m_Record.StartLevel);
	theApp.RdRegEx2Float(RK_RECORD, RK_REC_START_DURATION, m_Record.StartDuration);
	theApp.RdRegEx2Float(RK_RECORD, RK_REC_STOP_LEVEL, m_Record.StopLevel);
	theApp.RdRegEx2Float(RK_RECORD, RK_REC_STOP_DURATION, m_Record.StopDuration);
	theApp.RdRegEx2String(RK_RECORD, RK_REC_FOLDER_PATH, m_RecordFolderPath);
	theApp.RdRegEx2Struct(RK_RECORD, RK_REC_HOT_KEYS, m_Record.HotKeys);
	// verify device GUIDs
	CDSPlayer::CDSDeviceInfoArray	DevInfo;
	CDSPlayer::EnumDevices(DevInfo);	// enumerate playback devices
	if (CDSPlayer::FindDeviceGuid(DevInfo, m_PlayDeviceGuid) < 0)
		m_PlayDeviceGuid = GUID_NULL;
	CDSCapture::EnumDevices(DevInfo);	// enumerate recording devices
	if (CDSPlayer::FindDeviceGuid(DevInfo, m_RecordDeviceGuid) < 0)
		m_RecordDeviceGuid = GUID_NULL;
}

void COptionsInfo::Store()
{
	// view
	theApp.WrRegInt(RK_ZOOM_STEP_HORZ, m_ZoomStepHorz);
	theApp.WrRegInt(RK_ZOOM_STEP_VERT, m_ZoomStepVert);
	theApp.WrRegBool(RK_TIME_IN_FRAMES, m_TimeInFrames);
	theApp.WrRegBool(RK_VERT_SYNC_CHANS, m_VertSyncChans);
	theApp.WrRegBool(RK_VERT_ZOOM_CURSOR, m_VertZoomCursor);
	theApp.WrRegBool(RK_SHOW_INTERPOLATION, m_ShowInterpolation);
	theApp.WrRegBool(RK_CHECK_FOR_UPDATES, m_CheckForUpdates);
	theApp.WrRegBool(RK_SHOW_CHANNEL_NAMES, m_ShowChannelNames);
	theApp.WrRegInt(RK_MAX_DENSITY, m_MaxDensity);
	theApp.WrRegStruct(RK_VIEW_PALETTE, m_ViewPalette);
	theApp.WrRegStruct(RK_CUSTOM_COLORS, m_CustomColors);
	// edit
	theApp.WrRegInt(RK_UNDO_LEVELS, m_UndoLevels);
	theApp.WrRegInt(RK_DISK_THRESHOLD, m_DiskThreshold);
	theApp.WrRegBool(RK_CUSTOM_TEMP_FOLDER, m_CustomTempFolder);
	theApp.WrRegString(RK_TEMP_FOLDER_PATH, m_TempFolderPath);
	// audio
	theApp.WrRegStruct(RK_PLAY_DEVICE, m_PlayDeviceGuid);
	theApp.WrRegInt(RK_PLAY_BUFFER_SIZE, m_PlayBufferSize);
	theApp.WrRegStruct(RK_RECORD_DEVICE, m_RecordDeviceGuid);
	theApp.WrRegInt(RK_RECORD_BUFFER_SIZE, m_RecordBufferSize);
	theApp.WrRegInt(RK_MP3_IMPORT_QUALITY, m_MP3ImportQuality);
	theApp.WrRegInt(RK_VBR_ENCODING_QUALITY, m_VBREncodingQuality);
	theApp.WrRegInt(RK_MP4_IMPORT_QUALITY, m_MP4ImportQuality);
	theApp.WrRegBool(RK_MP4_DOWNMIX, m_MP4Downmix);
	theApp.WrRegInt(RK_METER_CLIP_THRESHOLD, m_MeterClipThreshold);
	// RTSA
	theApp.WrRegExInt(RK_RTSA, RK_WINDOW_FUNCTION, m_RTSA.WindowFunction);
	theApp.WrRegExInt(RK_RTSA, RK_WINDOW_SIZE, m_RTSA.WindowSize);
	theApp.WrRegExInt(RK_RTSA, RK_PLOT_STYLE, m_RTSA.PlotStyle);
	theApp.WrRegExInt(RK_RTSA, RK_FREQ_AXIS, m_RTSA.FreqAxisType);
	theApp.WrRegExInt(RK_RTSA, RK_CHANNEL_MODE, m_RTSA.ChannelMode);
	theApp.WrRegExInt(RK_RTSA, RK_AVERAGING, m_RTSA.Averaging);
	theApp.WrRegExInt(RK_RTSA, RK_PLOT_BKGND_COLOR, m_RTSA.PlotBkgndColor);
	theApp.WrRegExInt(RK_RTSA, RK_PLOT_GRID_COLOR, m_RTSA.PlotGridColor);
	theApp.WrRegExInt(RK_RTSA, RK_SHOW_PEAKS, m_RTSA.ShowPeaks);
	theApp.WrRegExInt(RK_RTSA, RK_PEAK_HOLD_TIME, m_RTSA.PeakHoldTime);
	theApp.WrRegExInt(RK_RTSA, RK_PEAK_DECAY, m_RTSA.PeakDecay);
	// Record
	theApp.WrRegExInt(RK_RECORD, RK_REC_ACTIVATION, m_Record.ActivationType);
	theApp.WrRegExInt(RK_RECORD, RK_REC_CHANNELS, m_Record.Channels);
	theApp.WrRegExInt(RK_RECORD, RK_REC_SAMPLE_RATE, m_Record.SampleRate);
	theApp.WrRegExInt(RK_RECORD, RK_REC_SAMPLE_SIZE, m_Record.SampleSize);
	theApp.WrRegExFloat(RK_RECORD, RK_REC_START_LEVEL, m_Record.StartLevel);
	theApp.WrRegExFloat(RK_RECORD, RK_REC_START_DURATION, m_Record.StartDuration);
	theApp.WrRegExFloat(RK_RECORD, RK_REC_STOP_LEVEL, m_Record.StopLevel);
	theApp.WrRegExFloat(RK_RECORD, RK_REC_STOP_DURATION, m_Record.StopDuration);
	theApp.WrRegExString(RK_RECORD, RK_REC_FOLDER_PATH, m_RecordFolderPath);
	theApp.WrRegExStruct(RK_RECORD, RK_REC_HOT_KEYS, m_Record.HotKeys);
}

CString COptionsInfo::GetTempFolderPath() const
{
	CString	path;
	if (m_CustomTempFolder) {
		path = m_TempFolderPath;
		theApp.MakeAbsolutePath(path);
	}
	return(path);
}
