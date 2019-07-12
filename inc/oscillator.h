/*
 * oscillator.c
 *
 * Author: Dan Green (danngreen1@gmail.com), Hugo Paris (hugoplho@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * See http://creativecommons.org/licenses/MIT/ for more information.
 *
 * -----------------------------------------------------------------------------
 */


#pragma once
#include <stm32f7xx.h>

#include "sphere.h"
#include "globals.h"


enum WtInterpRequests {
	WT_INTERP_REQ_NONE,
	WT_INTERP_REQ_REFRESH,
	WT_INTERP_REQ_FORCE
};


typedef struct o_wt_osc{

	// Current wavetable for each channel (interpolated from within the sphere)
	// Two buffers are kept, so we can crossfade when switching wavetables/spheres
	//
	float 						mc 						[2][NUM_CHANNELS][WT_TABLELEN];
	uint8_t						buffer_sel				[NUM_CHANNELS]		;

	// Status of interpolation and crossfade
	enum WtInterpRequests		wt_interp_request		[NUM_CHANNELS]		;
	float 						wt_xfade				[NUM_CHANNELS]		;

	// Position within sphere, calculated directly from calc_params.wt_pos[DIM][chan]
	//
	uint8_t 					m0						[3][NUM_CHANNELS]	;
	uint8_t 					m1						[3][NUM_CHANNELS]	;
	float 						m_frac					[3][NUM_CHANNELS]	;
	float 						m_frac_inv				[3][NUM_CHANNELS]	;

	// WT READING HEAD
	float 						wt_head_pos 			[NUM_CHANNELS]		;
	float						wt_head_pos_inc			[NUM_CHANNELS]		;
	uint16_t 					rh0						[NUM_CHANNELS]		;
	uint16_t 					rh1						[NUM_CHANNELS]		;
	float 						rhd						[NUM_CHANNELS]		; 
	float 						rhd_inv					[NUM_CHANNELS]		;
	
} o_wt_osc;


void	init_wt_osc(void);
void 	process_audio_block_codec(int32_t *src, int32_t *dst);
