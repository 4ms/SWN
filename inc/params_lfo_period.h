/*
 * params_lfo_period.h
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

#define NUM_DIVMULTS				24
#define LFO_UNITY_DIVMULT_ID		12
#define LFO_MAX_DIVMULT_ID			84
#define LFO_MIN_DIVMULT_ID			(-30)

#define F_LFO_UPDATE_FREQ				7200.0

//Ratio between LFO output update frequency and period measurement timer update frequency
#define F_LFO_UPDATE_RATIO			 	((float)TICKS_PER_MS*1000.0 / F_LFO_UPDATE_FREQ)


#define F_LFO_AUDIO_RANGE_PERIOD_L			((float)TICKS_PER_MS*1000.0 / 20.0) //400
#define F_LFO_AUDIO_RANGE_PERIOD_H			((float)TICKS_PER_MS*1000.0 / 21.0) //380.95


void update_lfo_calcs(void);
void update_lfo_wt_pos(void);

float calc_lfo_period(uint8_t chan, float global_divmult_id, uint32_t base_period_ms);
float calc_lfo_period_audiorange(float chan_divmult_id, float global_divmult_id, uint32_t base_period_ms);
float calc_lfo_period_lforange(float chan_divmult_id, float global_divmult_id, uint32_t base_period_ms);

float calc_lfo_inc(float period_ms);
float calc_divmult_amount(float divmult_id);
float calc_divmult_id(float divmult);

void flag_lfo_recalc(uint8_t chan);
void flag_all_lfos_recalc(void);
void stage_resync_lfos(void);
void stage_resync(uint8_t chan);
