/*
 * sphere.h - Spherical Wavetable Navigator "spherical" wavetable structure
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

#define 	WT_TABLELEN 					512 
#define 	F_WT_TABLELEN 					512.0 
#define		BITDEPTH						16
#define		BYTEDEPTH						(BITDEPTH/8)
#define 	NUM_WT_DIMENSIONS 				3
#define 	WT_DIM_SIZE		 				3
#define		NUM_WAVEFORMS_IN_SPHERE			(WT_DIM_SIZE * WT_DIM_SIZE * WT_DIM_SIZE)
#define		NUM_SAMPLES_IN_SPHERE  			(WT_TABLELEN * NUM_WAVEFORMS_IN_SPHERE)
#define 	WT_NAME_MONITOR_CHARSIZE		30
#define 	SPHERE_WAVEFORM_SIZE  			((WT_TABLELEN * BYTEDEPTH) + WT_NAME_MONITOR_CHARSIZE)		//1044


#define 	NUM_FACTORY_SPHERES 			12
#define 	NUM_USER_SPHERES_ALLOWED		108
#define 	MAX_TOTAL_SPHERES 				(NUM_FACTORY_SPHERES + NUM_USER_SPHERES_ALLOWED)

typedef struct{
    char 		name[WT_NAME_MONITOR_CHARSIZE];
    int16_t 	wave[WT_TABLELEN];
} o_waveform;
