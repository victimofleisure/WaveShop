// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      19mar13	initial version

        wrap Mark Borgerding's Kiss FFT

*/

#ifndef CKISSFFT_INCLUDED
#define CKISSFFT_INCLUDED

#include "kiss_fft\kiss_fftr.h"

class CKissFFT : public WObject {
public:
// Construction
	CKissFFT();
	CKissFFT(int nfft, int inverse_fft = 0);
	~CKissFFT();
	void	Create(int nfft, int inverse_fft = 0);
	void	Destroy();

// Operations
	void	RealFFT(const kiss_fft_scalar *timedata, kiss_fft_cpx *freqdata);
	void	RealInvFFT(const kiss_fft_cpx *freqdata, kiss_fft_scalar *timedata);

protected:
// Data members
	kiss_fftr_cfg	m_pCfg;		// pointer to configuration

// Helpers
	void	Alloc(int nfft, int inverse_fft);
};

inline CKissFFT::CKissFFT()
{
	m_pCfg = NULL;
}

inline CKissFFT::CKissFFT(int nfft, int inverse_fft)
{
	Alloc(nfft, inverse_fft);
}

inline CKissFFT::~CKissFFT()
{
	Destroy();
}

inline void CKissFFT::Create(int nfft, int inverse_fft)
{
	Destroy();
	Alloc(nfft, inverse_fft);
}

inline void CKissFFT::Destroy()
{
	if (m_pCfg != NULL) {	// if configuration exists
		free(m_pCfg);	// free it
		m_pCfg = NULL;	// mark it freed
	}
}

inline void CKissFFT::Alloc(int nfft, int inverse_fft)
{
	m_pCfg = kiss_fftr_alloc(nfft, inverse_fft, NULL, NULL);
	if (m_pCfg == NULL)	// if allocation failed
		AfxThrowMemoryException();
}

inline void CKissFFT::RealFFT(const kiss_fft_scalar *timedata, kiss_fft_cpx *freqdata)
{
	ASSERT(m_pCfg != NULL);	// configuration must exist
	kiss_fftr(m_pCfg, timedata, freqdata);
}

inline void CKissFFT::RealInvFFT(const kiss_fft_cpx *freqdata, kiss_fft_scalar *timedata)
{
	ASSERT(m_pCfg != NULL);	// configuration must exist
	kiss_fftri(m_pCfg, freqdata, timedata);
}

#endif
