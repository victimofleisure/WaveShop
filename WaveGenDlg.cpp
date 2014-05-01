// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30oct12	initial version
        01      07may13	add sweep subdialog

		wave generator dialog
 
*/

// WaveGenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "WaveGenDlg.h"
#include "Wave.h"
#include "Oscillator.h"
#include "ProgressDlg.h"
#include <math.h>
#include "DSPlayer.h"	// only for DecibelsToLinear
#include "SweepDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaveGenDlg dialog

const int CWaveGenDlg::m_OscName[OSCILLATORS] = {
	IDS_WGEN_OSC_CARRIER,
	IDS_WGEN_OSC_MODULATOR,
};

const WAVEGEN_MAIN_PARMS CWaveGenDlg::m_Defaults = {
	2,			// m_Channels
	44100,		// m_SampleRate
	16,			// m_SampleBits
	1.0,		// m_Duration
	100.0,		// m_Volume
	0.0,		// m_Attack
	0.0,		// m_Decay
	FALSE,		// m_LogFade
	MOD_NONE,	// m_ModType
	100.0,		// m_ModDepth
	20,			// m_SweepStartFreq
	20000,		// m_SweepEndFreq
	CARRIER,	// m_SelOsc
	0,			// m_FocusCtrlID
};

CWaveGenDlg::CWaveGenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CWaveGenDlg)
	//}}AFX_DATA_INIT
	WAVEGEN_MAIN_PARMS::operator=(m_Defaults);
	m_OscDlg[MODULATOR].m_Frequency = 1;	// differ from carrier for clarity
}

void CWaveGenDlg::GetParms(WAVEGEN_PARMS& Parms) const
{
	Parms.WAVEGEN_MAIN_PARMS::operator=(*this);	// copy main parameters
	for (int iOsc = 0; iOsc < OSCILLATORS; iOsc++)	// for each oscillator
		Parms.m_Osc[iOsc] = m_OscDlg[iOsc];	// copy oscillator parameters
}

void CWaveGenDlg::SetParms(const WAVEGEN_PARMS& Parms)
{
	WAVEGEN_MAIN_PARMS::operator=(Parms);	// copy main parameters
	for (int iOsc = 0; iOsc < OSCILLATORS; iOsc++)	// for each oscillator
		m_OscDlg[iOsc].WAVEGEN_OSC_PARMS::operator=(Parms.m_Osc[iOsc]);	// copy parameters
}

bool CWaveGenDlg::MakeWave(const WAVEGEN_PARMS& Parms, CWave& Wave, CProgressDlg *ProgressDlg)
{
	Wave.Empty();	// delete preexisting data if any
	Wave.SetFormat(Parms.m_Channels, Parms.m_SampleRate, Parms.m_SampleBits);
	double	fFrameCount = Parms.m_Duration * Parms.m_SampleRate;
#ifndef _WIN64	// if 64-bit
	// check for potential 32-bit wraparound
	if (fFrameCount * Wave.GetSampleSize() > INT_MAX)
		return(FALSE);
#endif
	W64INT	FrameCount = roundW64INT(fFrameCount);
	// convert percentages to normalized
	double	Attack = Parms.m_Attack / 100;
	double	Decay = 1 - Parms.m_Decay / 100;
	double	Volume = Parms.m_Volume / 100;
	double	CarBias = Parms.m_Osc[CARRIER].m_DCBias / 100;
	double	ModBias = Parms.m_Osc[MODULATOR].m_DCBias / 100;
	double	ModDepth = Parms.m_ModDepth / 100;
	int	PrevPctDone = 0;
	COscillator	osc[OSCILLATORS];
	for (int iOsc = 0; iOsc < OSCILLATORS; iOsc++) {	// for each oscillator
		COscillator&	oi = osc[iOsc];	// set oscillator attributes
		const WAVEGEN_OSC_PARMS&	OscParms = Parms.m_Osc[iOsc];
		oi.SetTimerFreq(Parms.m_SampleRate);
		oi.SetWaveform(OscParms.m_Waveform);
		oi.SetFreqSync(OscParms.m_Frequency);
		oi.SetPulseWidth(OscParms.m_PulseWidth / 100);
		if (OscParms.m_Phase)
			oi.SetPhase(OscParms.m_Phase / 360);
	}
	if (FrameCount) {	// if non-empty
		CWave::SAMPLE	NegRail, PosRail;
		Wave.GetSampleRails(NegRail, PosRail);
		Wave.SetFrameCount(FrameCount);
		for (W64INT iFrame = 0; iFrame < FrameCount; iFrame++) {
			double	NormPos = double(iFrame) / FrameCount;
			if (ProgressDlg != NULL) {
				if (ProgressDlg->Canceled())
					return(FALSE);
				int	PctDone = round(NormPos * 100);
				if (PctDone != PrevPctDone) {	// if progress percentage changed
					ProgressDlg->SetPos(PctDone);
					PrevPctDone = PctDone;
				}
			}
			double	CarAmp = 1;	// set default carrier amplitude
			if (Parms.m_ModType != MOD_NONE) {	// if modulating
				double	ModVal = osc[MODULATOR].GetVal();	// get modulator value
				osc[MODULATOR].TimerHook();	// increment modulator clock
				ModVal *= ModDepth;	// apply modulation depth
				ModVal += ModBias;	// add DC bias
				switch (Parms.m_ModType) {
				case MOD_FREQUENCY:	// modulate carrier frequency
					{
						double	freq = Parms.m_Osc[CARRIER].m_Frequency * pow(2, ModVal);
						osc[CARRIER].SetFreq(max(0, freq));
					}
					break;
				case MOD_AMPLITUDE:	// modulate carrier amplitude
					CarAmp *= (ModVal + 1) / 2;
					break;
				case MOD_PULSE:	// modulate carrier pulse width
					osc[CARRIER].SetPulseWidth((ModVal + 1) / 2);
					break;
				}
			}
			double	CarVal = osc[CARRIER].GetVal();	// get carrier value
			osc[CARRIER].TimerHook();	// increment carrier clock
			double	EnvVal;	// compute envelope value
			if (NormPos >= Decay)	// if within decay
				EnvVal = 1 - (NormPos - Decay) / (1 - Decay);
			else if (NormPos < Attack)	// if within attack
				EnvVal = NormPos / Attack;
			else	// sustain
				EnvVal = 1;
			if (Parms.m_LogFade)	// if logarithmic fade
				EnvVal = CDSPlayer::DecibelsToLinear((1 - EnvVal) * -100);
			// apply amplitude, envelope, and volume to carrier value
			CarVal *= CarAmp * EnvVal * Volume;
			CarVal += CarBias;	// add DC bias
			CarVal = CLAMP(CarVal, -1, 1);	// clip to rails
			CWave::SAMPLE	samp = round(CarVal * PosRail);	// convert to sample
			for (UINT iChan = 0; iChan < Parms.m_Channels; iChan++)	// for each channel
				Wave.SetSample(iChan, iFrame, samp);	// store sample
		}
	}
	return(TRUE);
}

void CWaveGenDlg::ShowOscDlg(int TabIdx)
{
	for (int iOsc = 0; iOsc < OSCILLATORS; iOsc++) {	// for each oscillator
		int	ShowCmd = (iOsc == TabIdx) ? SW_SHOW : SW_HIDE;
		m_OscDlg[iOsc].ShowWindow(ShowCmd);	// show selected dialog, hide others
		if (iOsc == TabIdx) {	// if selected dialog
			// move selected child dialog after tab control in Z-order
			UINT	Flags = SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE;
			m_OscDlg[iOsc].SetWindowPos(&m_OscTab, 0, 0, 0, 0, Flags);
		}
	}
}

void CWaveGenDlg::ResizeOscDlgs()
{
	CRect	rOscTab;
	m_OscTab.GetClientRect(rOscTab);	// client rect is input to AdjustRect
	m_OscTab.AdjustRect(FALSE, rOscTab);	// get tab control's display area
	for (int iOsc = 0; iOsc < OSCILLATORS; iOsc++) {	// for each oscillator
		CRect	rOscDlg(rOscTab);
		// convert from tab control's client coords to our client coords
		m_OscTab.MapWindowPoints(this, rOscDlg);
		m_OscDlg[iOsc].MoveWindow(rOscDlg);	// move child dialog onto tab control
	}
}

void CWaveGenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaveGenDlg)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_WGEN_OSC_TAB, m_OscTab);
	DDX_Text(pDX, IDC_WGEN_VOLUME, m_Volume);
	DDX_Text(pDX, IDC_WGEN_CHANNELS, m_Channels);
	DDV_MinMaxUInt(pDX, m_Channels, 1, 65535);
	DDX_Text(pDX, IDC_WGEN_DURATION, m_Duration);
	DDV_MinMaxDouble(pDX, m_Duration, 0., 2147483647.);
	DDX_Text(pDX, IDC_WGEN_SAMPLE_BITS, m_SampleBits);
	DDV_MinMaxUInt(pDX, m_SampleBits, 1, 32);
	DDX_Text(pDX, IDC_WGEN_SAMPLE_RATE, m_SampleRate);
	DDV_MinMaxUInt(pDX, m_SampleRate, 1, 4294967295);
	DDX_Text(pDX, IDC_WGEN_ATTACK, m_Attack);
	DDX_Text(pDX, IDC_WGEN_DECAY, m_Decay);
	DDX_Check(pDX, IDC_WGEN_LOG_FADE, m_LogFade);
	DDX_Text(pDX, IDC_WGEN_MOD_DEPTH, m_ModDepth);
	DDX_CBIndex(pDX, IDC_WGEN_MOD_TYPE, m_ModType);
	for (int iOsc = 0; iOsc < OSCILLATORS; iOsc++) {	// for each oscillator
		if (!m_OscDlg[iOsc].UpdateData(pDX->m_bSaveAndValidate))
			return;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWaveGenDlg message map

BEGIN_MESSAGE_MAP(CWaveGenDlg, CDialog)
	//{{AFX_MSG_MAP(CWaveGenDlg)
	ON_WM_CREATE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_WGEN_OSC_TAB, OnSelchangeOscTab)
	ON_BN_CLICKED(IDC_WGEN_RESET, OnReset)
	ON_BN_CLICKED(IDC_WGEN_SWEEP, OnSweep)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaveGenDlg message handlers

BOOL CWaveGenDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// insert tab control items
	for (int iOsc = 0; iOsc < OSCILLATORS; iOsc++)	// for each oscillator
		m_OscTab.InsertItem(iOsc, LDS(m_OscName[iOsc]));	// insert tab item
	ResizeOscDlgs();
	ShowOscDlg(m_SelOsc);
	m_OscTab.SetCurSel(m_SelOsc);
	if (m_FocusCtrlID) {
		CWnd	*pWnd = GetDlgItem(m_FocusCtrlID);
		if (pWnd == NULL)
			pWnd = m_OscDlg[m_SelOsc].GetDlgItem(m_FocusCtrlID);
		if (pWnd != NULL) {
			GotoDlgCtrl(pWnd);
			return FALSE;	// set focus to a control
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CWaveGenDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	for (int iOsc = 0; iOsc < OSCILLATORS; iOsc++) {	// for each oscillator
		if (!m_OscDlg[iOsc].Create(IDD_WAVE_GEN_OSC, this))	// create child dialog
			return -1;
	}
	
	return 0;
}

void CWaveGenDlg::OnOK() 
{
	int	sel = m_OscTab.GetCurSel();
	if (sel >= 0)
		m_SelOsc = sel;
	CWnd	*pWnd = GetFocus();
	if (pWnd != NULL)
		m_FocusCtrlID = pWnd->GetDlgCtrlID();
	CDialog::OnOK();
}

void CWaveGenDlg::OnSelchangeOscTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int	sel = m_OscTab.GetCurSel();
	if (sel >= 0) {
		ShowOscDlg(sel);
		m_OscTab.SetFocus();	// showing child dialog unexpectedly gives it focus
	}
	*pResult = 0;
}

void CWaveGenDlg::OnReset() 
{
	CWaveGenDlg	dlg;
	WAVEGEN_PARMS parms;
	dlg.GetParms(parms);
	SetParms(parms);	// reset member parameters to defaults
	UpdateData(FALSE);	// update dialog data 
}

void CWaveGenDlg::OnSweep() 
{
	if (!UpdateData())	// save and validate dialog data
		return;	// validation failed
	CSweepDlg	dlg;
	dlg.m_StartFreq = m_SweepStartFreq;
	dlg.m_EndFreq = m_SweepEndFreq;
	dlg.m_Duration = m_Duration;
	dlg.m_Waveform = m_OscDlg[CARRIER].m_Waveform;
	if (dlg.DoModal() != IDOK)
		return;	// user canceled
	ASSERT(dlg.m_StartFreq > 0);	// else logic error in sweep dialog validation
	ASSERT(dlg.m_EndFreq > 0);
	ASSERT(dlg.m_Duration > 0);	// avoid divide by zero
	OnOK();	// close dialog before setting member data
	m_SweepStartFreq = dlg.m_StartFreq;	// set sweep frequency range
	m_SweepEndFreq = dlg.m_EndFreq;
	// compute center frequency: geometric mean of start and end frequency
	double	CenterFreq = sqrt(m_SweepStartFreq * m_SweepEndFreq);
	// compute frequency modulation amplitude; negative if start > end
	double	FreqModAmp = log(m_SweepEndFreq / CenterFreq) / log(2.0);
	CWaveGenOscDlg&	CarDlg = m_OscDlg[CARRIER];
	CarDlg.m_Waveform = dlg.m_Waveform;	// set carrier waveform to sweep waveform
	CarDlg.m_Frequency = CenterFreq;	// set carrier frequency to center frequency
	// set modulation frequency to reciprocal of duration
	CWaveGenOscDlg&	ModDlg = m_OscDlg[MODULATOR];
	ModDlg.m_Frequency = 1 / dlg.m_Duration;
	// set modulation waveform to ramp up; direction is constant because
	// frequency modulation amplitude can be negative, inverting ramp
	ModDlg.m_Waveform = COscillator::RAMP_UP;
	m_Duration = dlg.m_Duration;	// set duration to sweep duration
	// set modulation depth to normalized frequency modulation amplitude 
	m_ModDepth = FreqModAmp * 100;
	m_ModType = MOD_FREQUENCY;	// set modulation type to frequency
}
