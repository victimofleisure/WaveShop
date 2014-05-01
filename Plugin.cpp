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

#include "stdafx.h"
#include "Resource.h"
#include "Plugin.h"
#include "ladspa.h"
#include "WaveProcess.h"
#include "ProgressDlg.h"

CPlugin::CPlugin()
{
	m_Desc = NULL;
	m_Plugin = NULL;
}

CPlugin::~CPlugin()
{
	Cleanup();
}

bool CPlugin::Instantiate(UINT SampleRate)
{
	ASSERT(m_Desc != NULL);
	ASSERT(m_Plugin == NULL);	// else logic error and likely memory leak
	m_Plugin = m_Desc->instantiate(m_Desc, SampleRate);	// create instance
	return(m_Plugin != NULL);
}

void CPlugin::Cleanup()
{
	if (m_Plugin != NULL) {	// if instance exists
		m_Desc->cleanup(m_Plugin);	// destroy instance
		m_Plugin = NULL;	// mark instance destroyed
	}
}

const _LADSPA_Descriptor *CPlugin::Load(CDLLWrap& Dll, LPCTSTR Path)
{
	// It's convenient to keep plugins in a different folder than the app,
	// but this can cause problems for plugins that depend on support DLLs.
	// Such dependencies are presumably in the same folder as the plugins,
	// but the DLL search path wouldn't normally include the plugins folder.
	// The LoadLibraryEx flag LOAD_WITH_ALTERED_SEARCH_PATH is intended for
	// this case: it temporarily modifies the DLL search path, replacing the
	// application folder with the loaded module's folder.
	//
	if (!Dll.LoadLibraryEx(Path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH))
		return(NULL);
	LADSPA_Descriptor_Function	pFunc = 
		(LADSPA_Descriptor_Function)Dll.GetProcAddress(_T("ladspa_descriptor"));
	if (pFunc == NULL)
		return(NULL);
	return(pFunc(0));	// return descriptor
}

CString	CPlugin::GetName() const
{
	ASSERT(m_Desc != NULL);
	return(CString(m_Desc->Name));
}

bool CPlugin::Create(LPCTSTR Path, UINT SampleRate)
{
	ASSERT(m_Desc == NULL);	// reuse not allowed
	ASSERT(m_Plugin == NULL);	// reuse not allowed
	m_Desc = Load(m_Dll, Path);	// load plugin DLL and get descriptor
	if (m_Desc == NULL)	// if can't load plugin
		return(FALSE);
	return(Instantiate(SampleRate));	// create plugin instance
}

bool CPlugin::GetPortStats(const _LADSPA_Descriptor *Desc, CPortStats& Stats)
{
	ASSERT(Desc != NULL);
	bool	retc = TRUE;	// assume success
	UINT	ports = Desc->PortCount;
	for (UINT iPort = 0; iPort < ports; iPort++) {	// for each port descriptor
		const LADSPA_PortDescriptor&	PortDesc = Desc->PortDescriptors[iPort];
		if (LADSPA_IS_PORT_CONTROL(PortDesc)) {	// if port is control
			if (LADSPA_IS_PORT_INPUT(PortDesc))		// if port is input
				Stats.m_ControlIn.Add(iPort);	// store control input port's index
			else if (LADSPA_IS_PORT_OUTPUT(PortDesc))	// if port is output
				Stats.m_ControlOut.Add(iPort);	// store control output port's index
			else	// port is neither input nor output
				retc = FALSE;	// invalid port descriptor
		} else if (LADSPA_IS_PORT_AUDIO(PortDesc)) {	// if port is audio
			if (LADSPA_IS_PORT_INPUT(PortDesc))		// if port is input
				Stats.m_AudioIn.Add(iPort);	// store audio input port's index
			else if (LADSPA_IS_PORT_OUTPUT(PortDesc))	// if port is output
				Stats.m_AudioOut.Add(iPort);	// store audio output port's index
			else	// port is neither input nor output
				retc = FALSE;	// invalid port descriptor
		} else	// port is neither control nor audio
			retc = FALSE;	// invalid port descriptor
	}
	return(retc);	// true if all port descriptors are valid
}

void CPlugin::GetSelectedChannelIndices(const CByteArray& ChanSel, CDWordArrayEx& SelChanIdx)
{
	int	chans = INT64TO32(ChanSel.GetSize());
	SelChanIdx.SetSize(chans);
	int	sels = 0;
	for (int iChan = 0; iChan < chans; iChan++) {	// for each channel
		if (ChanSel[iChan])	// if channel is selected
			SelChanIdx[sels++] = iChan;	// add its index to destination array
	}
	SelChanIdx.SetSize(sels);
}

bool CPlugin::IsChannelCountCompatible(const CByteArray& ChanSel) const
{
	CPortStats	PortStats;
	GetPortStats(m_Desc, PortStats);	// get port statistics
	int	inputs = INT64TO32(PortStats.m_AudioIn.GetSize());
	int	outputs = INT64TO32(PortStats.m_AudioOut.GetSize());
	CDWordArrayEx	SelChanIdx;
	GetSelectedChannelIndices(ChanSel, SelChanIdx);	// get indices of selected channels
	int	SelChans = INT64TO32(SelChanIdx.GetSize());
	if (inputs == 1 && outputs == 1) {	// if mono plugin
		ASSERT(SelChans > 0);	// logic error: at least one channel must be selected
	} else {	// plugin has multiple inputs and/or outputs
		int	NeededChans = max(inputs, outputs);	// calculate number of channels required
		if (NeededChans != SelChans) {	// if plugin incompatible with selected channels
			CString	msg;
			msg.Format(IDS_PLUGIN_CHANNEL_MISMATCH, GetName(), NeededChans);
			AfxMessageBox(msg);
			return(FALSE);
		}
	}
	return(TRUE);
}

void CPlugin::ConnectControlPorts(const CPortStats& PortStats, CParamArray& Input, CParamArray& Output)
{
	// connect control input ports to source variables
	int	CtrlIns = INT64TO32(PortStats.m_ControlIn.GetSize());
	for (int iIn = 0; iIn < CtrlIns; iIn++) {	// for each control input
		int	iPort = PortStats.m_ControlIn[iIn];
		m_Desc->connect_port(m_Plugin, iPort, &Input[iIn]);
	}
	// connect control output ports to destination variables
	int	CtrlOuts = INT64TO32(PortStats.m_ControlOut.GetSize());
	for (int iOut = 0; iOut < CtrlOuts; iOut++) {	// for each control output
		int	iPort = PortStats.m_ControlOut[iOut];
		m_Desc->connect_port(m_Plugin, iPort, &Output[iOut]);
	}
}

bool CPlugin::Run(CWave& Wave, const CW64IntRange& Selection, const CByteArray& ChanSel, const CParamArray& Param)
{
	ASSERT(m_Desc != NULL);
	ASSERT(IsCreated());	// plugin instance must already exist
	ASSERT(Wave.IsValid());
	CPortStats	PortStats;
	if (!GetPortStats(m_Desc, PortStats)) {	// get port statistics
		CString	msg;
		AfxFormatString1(msg, IDS_PLUGIN_BROKEN, GetName());
		AfxMessageBox(msg);
		return(FALSE);	// invalid port descriptor(s); avoid potential crash
	}
	if (!IsChannelCountCompatible(ChanSel))	// validate channel selection
		return(FALSE);	// plugin incompatible with selected channel count
	CDWordArrayEx	SelChanIdx;
	GetSelectedChannelIndices(ChanSel, SelChanIdx);	// get indices of selected channels
	int	SelChans = INT64TO32(SelChanIdx.GetSize());
	int	inputs = INT64TO32(PortStats.m_AudioIn.GetSize());
	int	outputs = INT64TO32(PortStats.m_AudioOut.GetSize());
	CParamArray	ControlIn(Param);	// copy parameters to input controls
	CParamArray	ControlOut;
	ControlOut.SetSize(PortStats.m_ControlOut.GetSize());	// allocate output controls
	CWaveProcess::CConvert	cvt(Wave);
	UINT	FrameSize = Wave.GetFrameSize();
	const int	BUFFER_SIZE = 0x100000;	// space allocated for sample buffers, in bytes
	if (inputs == 1 && outputs == 1) {	// if mono plugin
		CSampleBuffer	SampBuf;
		int	BufferFrames = BUFFER_SIZE / sizeof(float);
		SampBuf.SetSize(BufferFrames);	// allocate sample buffer
		CProgressDlg	ProgDlg;
		ProgDlg.Create();	// create progress dialog
		ProgDlg.SetWindowText(LDS(IDS_PLUGIN_APPLYING));
		int	PrevPctDone = 0;
		for (int iSel = 0; iSel < SelChans; iSel++) {	// for each selected channel
			UINT	iChan = SelChanIdx[iSel];	// get channel's index
			if (m_Plugin == NULL) {	// if plugin not already instantiated
				if (!Instantiate(Wave.GetSampleRate())) {	// instantiate plugin
					CString	msg;
					AfxFormatString1(msg, IDS_PLUGIN_CANT_INSTANTIATE, GetName());
					AfxMessageBox(msg);
					return(FALSE);	// can't instantiate plugin
				}
			}
			ConnectControlPorts(PortStats, ControlIn, ControlOut);
			// connect audio ports to sample buffer
			m_Desc->connect_port(m_Plugin, PortStats.m_AudioIn[0], SampBuf.GetData());
			m_Desc->connect_port(m_Plugin, PortStats.m_AudioOut[0], SampBuf.GetData());
			if (m_Desc->activate != NULL)	// if plugin requires activation
				m_Desc->activate(m_Plugin);	// activate plugin
			W64INT	frames = Selection.Length();
			W64INT	chunks = (frames - 1) / BufferFrames + 1;
			W64INT	remain = frames;
			W64INT	PerChanOfs = Wave.GetByteOffset(iChan, Selection.Start);
			for (int iChunk = 0; iChunk < chunks; iChunk++) {	// for each chunk
				int	PctDone = round((double(iSel) / SelChans 
					+ double(iChunk) / chunks / SelChans) * 100);
				if (PctDone != PrevPctDone) {	// if percent done changed
					ProgDlg.SetPos(PctDone);	// update progress bar
					PrevPctDone = PctDone;
				}
				if (ProgDlg.Canceled())	// if user canceled
					return(FALSE);
				int	ChunkFrames = INT64TO32(min(BufferFrames, remain));
				int	iFrame;
				W64INT	offset = PerChanOfs;
				for (iFrame = 0; iFrame < ChunkFrames; iFrame++) {	// for each frame
					SampBuf[iFrame] = float(cvt.SampleToNorm(Wave.GetSampleAt(offset)));
					offset += FrameSize;
				}
				m_Desc->run(m_Plugin, ChunkFrames);	// process chunk
				offset = PerChanOfs;
				for (iFrame = 0; iFrame < ChunkFrames; iFrame++) {	// for each frame
					Wave.SetSampleAt(offset, round(cvt.ClampNormToSample(SampBuf[iFrame])));
					offset += FrameSize;
				}
				remain -= ChunkFrames;	// decrement chunks remaining
				PerChanOfs += ChunkFrames * FrameSize;	// increment per-channel offset
			}
			if (m_Desc->deactivate != NULL)	// if plugin requires deactivation
				m_Desc->deactivate(m_Plugin);	// deactivate plugin
			// destroy plugin instance, forcing subsequent passes to reinstantiate;
			// this shouldn't be necessary, but certain plugins misbehave otherwise
			Cleanup();
		}
	} else {	// plugin has multiple inputs and/or outputs
		if (m_Desc->activate != NULL)	// if plugin requires activation
			m_Desc->activate(m_Plugin);	// activate plugin
		ConnectControlPorts(PortStats, ControlIn, ControlOut);
		// allocate sample buffers for selected channels
		CSampleBufferArray	ChanSampBuf;
		ChanSampBuf.SetSize(SelChans);
		// limit sum of all sample buffer allocations to BUFFER_SIZE
		int	BufferFrames = BUFFER_SIZE / (sizeof(float) * SelChans);
		ASSERT(BufferFrames > 0);	// else logic error
		// connect audio ports to channel sample buffers
		for (int iSel = 0; iSel < SelChans; iSel++) {	// for each selected channel
			CSampleBuffer&	SampBuf = ChanSampBuf[iSel];
			SampBuf.SetSize(BufferFrames);	// allocate sample buffer
			if (iSel < inputs)	// if selected channel is an input
				m_Desc->connect_port(m_Plugin, PortStats.m_AudioIn[iSel], SampBuf.GetData());
			if (iSel < outputs)	// if selected channel is an output
				m_Desc->connect_port(m_Plugin, PortStats.m_AudioOut[iSel], SampBuf.GetData());
		}
		W64INT	frames = Selection.Length();
		W64INT	chunks = (frames - 1) / BufferFrames + 1;
		W64INT	remain = frames;
		W64INT	iChunkFrame = Selection.Start;
		CProgressDlg	ProgDlg;
		ProgDlg.Create();	// create progress dialog
		ProgDlg.SetWindowText(LDS(IDS_PLUGIN_APPLYING));
		int	PrevPctDone = 0;
		for (int iChunk = 0; iChunk < chunks; iChunk++) {	// for each chunk
			int	PctDone = round(double(iChunk) / chunks * 100);
			if (PctDone != PrevPctDone) {	// if percent done changed
				ProgDlg.SetPos(PctDone);	// update progress bar
				PrevPctDone = PctDone;
			}
			if (ProgDlg.Canceled())	// if user canceled
				return(FALSE);
			int	ChunkFrames = INT64TO32(min(BufferFrames, remain));
			// copy input chunk from wave to sample buffers
			for (int iIn = 0; iIn < inputs; iIn++) {	// for each input
				UINT	iChan = SelChanIdx[iIn];	// get channel's index
				CSampleBuffer&	SampBuf = ChanSampBuf[iIn];
				W64INT	offset = Wave.GetByteOffset(iChan, iChunkFrame);
				for (int iFrame = 0; iFrame < ChunkFrames; iFrame++) {	// for each frame
					SampBuf[iFrame] = float(cvt.SampleToNorm(Wave.GetSampleAt(offset)));
					offset += FrameSize;
				}
			}
			m_Desc->run(m_Plugin, ChunkFrames);	// process chunk
			// copy processed chunk from sample buffers back to wave
			for (int iOut = 0; iOut < outputs; iOut++) {	// for each output
				UINT	iChan = SelChanIdx[iOut];	// get channel's index
				const CSampleBuffer&	SampBuf = ChanSampBuf[iOut];
				W64INT	offset = Wave.GetByteOffset(iChan, iChunkFrame);
				for (int iFrame = 0; iFrame < ChunkFrames; iFrame++) {	// for each frame
					Wave.SetSampleAt(offset, round(cvt.ClampNormToSample(SampBuf[iFrame])));
					offset += FrameSize;
				}
			}
			remain -= ChunkFrames;	// decrement chunks remaining
			iChunkFrame += ChunkFrames;	// increment chunk frame index
		}
		if (m_Desc->deactivate != NULL)	// if plugin requires deactivation
			m_Desc->deactivate(m_Plugin);	// deactivate plugin
	}
	return(TRUE);
}
