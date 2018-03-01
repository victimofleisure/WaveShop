// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30oct12	initial version
        01      07may13	add sweep frequency range

		wave generator parameters
 
*/

#ifndef WAVEGENPARM_INCLUDED
#define WAVEGENPARM_INCLUDED

struct WAVEGEN_MAIN_PARMS {
	enum {	// oscillators
		CARRIER,
		MODULATOR,
		OSCILLATORS
	};
	UINT	m_Channels;			// number of channels
	UINT	m_SampleRate;		// sampling frequency in Hz
	UINT	m_SampleBits;		// number of bits per sample
	double	m_Duration;			// duration in seconds
	double	m_Volume;			// volume percentage
	double	m_Attack;			// attack time, as percentage of duration
	double	m_Decay;			// decay time, as percentage of duration
	BOOL	m_LogFade;			// if true, use log fades instead of linear
	int		m_ModType;			// modulation type
	double	m_ModDepth;			// modulation depth
	double	m_SweepStartFreq;	// start of sweep frequency range, in Hz
	double	m_SweepEndFreq;		// end of sweep frequency range, in Hz
	int		m_SelOsc;			// index of selected oscillator tab
	int		m_FocusCtrlID;		// ID of focused control, or zero if none
};

struct WAVEGEN_OSC_PARMS {
	int		m_Waveform;			// waveform enumeration
	double	m_Frequency;		// frequency in Hz
	double	m_PulseWidth;		// pulse width percentage
	double	m_Phase;			// initial phase in degrees
	double	m_DCBias;			// DC bias percentage
};

struct WAVEGEN_PARMS : WAVEGEN_MAIN_PARMS {
	WAVEGEN_OSC_PARMS	m_Osc[OSCILLATORS];	// oscillator parameters
};

#endif
