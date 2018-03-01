/*
 * function: Header file for aacDECdrop
 *
 * This program is distributed under the GNU General Public License, version 2.
 * A copy of this license is included with this source.
 *
 * Copyright (C) 2002 John Edwards
 */
// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      18apr13	adapted from libfaad2's decode.h

*/

#ifndef __LIBMP4AD_H__
#define __LIBMP4AD_H__

#include "neaacdec.h"

typedef int (*aac_output_handler)(void *param, long total_samples, long samples, const NeAACDecFrameInfo *frame_info, const void *sample_buffer);
typedef void (*aac_error_handler)(void *param, const char *error_message);

typedef struct
{
	aac_output_handler on_output;	// pointer to output handler
	aac_error_handler on_error;		// pointer to error handler
	int output_format;	// output format; must be non-zero
	int down_matrix;	// if non-zero, mix surround down to stereo
	int	def_srate;		// default sample rate, or zero for none
	int object_type;	// default object type, or zero for none
	void *param;		// user-defined parameter
} aac_dec_opt;

int aac_decode(const char *path, const aac_dec_opt *opt);

#endif
