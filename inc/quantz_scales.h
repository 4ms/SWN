/*
 * quantz_scales.h
 *
 * Author: Dan Green (danngreen1@gmail.com)
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


#define MAX_NUM_QTZ_STEPS 12

enum scaleMap{
	sclm_NONE,		//0
	sclm_MAJOR,		//1
	sclm_MINOR,		//2
	sclm_SEMITONES,	//3

	NUM_QTZ_SCALES	//4
};

void init_quantz_scales(void);
//Todo: use previous note/oct to apply hysterisis correction (ie make sure we've moved far enough into another note/oct before changing)
//float quantize_to_scale(uint8_t scale_num, float unqtz_freq, uint8_t *qtz_note, uint8_t *qtz_oct, uint8_t prev_qtz_note, uint8_t prev_qtz_oct);
float quantize_to_scale(uint8_t scale_num, float unqtz_freq, uint8_t *qtz_note, int8_t *qtz_oct);

