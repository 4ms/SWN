/*
 * wavetable_recording.h
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
//#include "sphere.h"
#include "wavetable_editing.h"

typedef struct o_recbuf{
	uint32_t 				wh;
	int16_t 				data[NUM_SAMPLES_IN_RECBUF_SMOOTHED];
	uint32_t				start_pos;
	uint32_t				end_pos;
} o_recbuf;

enum RecordWindows {
	RECORD_CURRENT = 0,
	RECORD_ALL = 0xFF
};

// Threshold of audio to trigger recording, in mV 
#define REC_THRESHOLD_mV 48
//#define MAX_RANGE_V 20
//#define REC_THRESHOLD (((REC_THRESHOLD_mV/1000) / MAX_RANGE_V) * (INT32_MAX>>8))
#define REC_THRESHOLD (REC_THRESHOLD_mV * 419)

void init_wtrecording(uint8_t wave_browse_i);
void display_wt_recbuff_fill_outring(void);
void display_wt_rec_wait(void);
void enter_wtrecording(uint8_t record_window);

uint32_t get_recbuf_wh(void);
