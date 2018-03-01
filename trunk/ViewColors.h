// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      12feb13	initial version
        01      16jun13	add excluded channel colors

        define view colors
 
*/

//				name				tag					R		G		B
VIEW_COLOR_DEF(	WaveBk,				WAVE_BK,			255,	255,	255	)
VIEW_COLOR_DEF(	WaveData,			WAVE_DATA,			0,		0,		0	)
VIEW_COLOR_DEF(	SelectedWaveBk,		SELECTED_WAVE_BK,	196,	196,	196	)
VIEW_COLOR_DEF(	SelectedWaveData,	SELECTED_WAVE_DATA,	60,		60,		60	)
VIEW_COLOR_DEF(	Interpolation,		INTERPOLATION,		0,		255,	0	)
VIEW_COLOR_DEF(	ExcludedChanBk,		EXCLUDED_CHAN_BK,	255,	255,	255	)
VIEW_COLOR_DEF(	ExcludedChanData,	EXCLUDED_CHAN_DATA,	196,	196,	196	)

#undef VIEW_COLOR_DEF
