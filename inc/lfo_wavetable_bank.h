/*
 * lfo_wavetable_bank.h
 *
 * Authors:  Dan Green (danngreen1@gmail.com), Hugo Paris (hugoplho@gmail.com)
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

#define 	LFO_TABLELEN 	256 			//Must be a power of 2
#define 	F_LFO_TABLELEN 	256.0
#define	 	F_MAX_LFO_TABLELEN				(float)(F_LFO_TABLELEN - 1.0)	
#define 	NUM_LFO_SHAPES  25 
#define 	NUM_LFO_GROUPS 	6 		
#define 	MAX_LFO_WT_VAL 	256
#define 	KEY_SHAPE		0
	
extern 		uint8_t 		LFOS_TO_BANK_END[NUM_LFO_GROUPS];
extern const uint8_t 		lfo_wavetable[NUM_LFO_SHAPES][LFO_TABLELEN];
