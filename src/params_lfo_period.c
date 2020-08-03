/*
 * params_lfo_period.c
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


#include "params_lfo_period.h"
#include "params_lfo_clk.h"
#include "params_lfo.h"
#include "params_update.h"
#include "oscillator.h"
#include "math_util.h"
#include "gpio_pins.h"
#include "lfo_wavetable_bank.h"
#include <math.h>

extern o_lfos 		lfos;
extern o_params 	params;
extern o_calc_params 	calc_params;
extern o_wt_osc 	wt_osc;
extern uint16_t 	divmult_cv;

const float LFO_DIVMULTS[NUM_DIVMULTS] = {
	1.0/64.0, 1.0/48.0, 1.0/32.0, 1.0/24.0, 1.0/16.0, 1.0/8.0, 1.0/7.0, 1.0/6.0, 1.0/5.0, 1.0/4.0, 1.0/3.0, 1.0/2.0,\
	1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 12.0, 16.0, 24.0, 32.0 };

uint8_t resync_staged[NUM_CHANNELS+1] = {0};

uint8_t recalc_flagged[NUM_CHANNELS+2] = {0};



void stage_resync(uint8_t chan){
	resync_staged[chan] = 1;
}

void stage_resync_lfos(void)
{
	if (!lfos.locked[0]) resync_staged[0] = 1;
	if (!lfos.locked[1]) resync_staged[1] = 1;
	if (!lfos.locked[2]) resync_staged[2] = 1;
	if (!lfos.locked[3]) resync_staged[3] = 1;
	if (!lfos.locked[4]) resync_staged[4] = 1;
	if (!lfos.locked[5]) resync_staged[5] = 1;
	resync_staged[GLO_CLK] = 1;
}

void flag_lfo_recalc(uint8_t chan)
{
	recalc_flagged[chan] = 1;
}

void flag_all_lfos_recalc(void)
{
	if (!lfos.locked[0]) recalc_flagged[0] = 1;
	if (!lfos.locked[1]) recalc_flagged[1] = 1;
	if (!lfos.locked[2]) recalc_flagged[2] = 1;
	if (!lfos.locked[3]) recalc_flagged[3] = 1;
	if (!lfos.locked[4]) recalc_flagged[4] = 1;
	if (!lfos.locked[5]) recalc_flagged[5] = 1;
	recalc_flagged[GLO_CLK] = 1;
	recalc_flagged[REF_CLK] = 1;
}

void update_lfo_wt_pos(void)
{
	uint8_t chan;

	lfos.cycle_pos[REF_CLK] = _WRAP_F(lfos.cycle_pos[REF_CLK] + lfos.inc[REF_CLK], 0, 1.0);

	if (resync_staged[GLO_CLK])
	{
		resync_staged[GLO_CLK] = 0;
		lfos.cycle_pos[GLO_CLK] = _WRAP_F(lfos.cycle_pos[REF_CLK] * lfos.divmult[GLO_CLK], 0, 1);
	 } else {
		lfos.cycle_pos[GLO_CLK] = _WRAP_F(lfos.cycle_pos[GLO_CLK] + lfos.inc[GLO_CLK], 0, 1);
	}

	for (chan=0; chan<NUM_CHANNELS; chan++)
	{
		if (lfos.muted[chan])
			continue;

		else if (params.key_sw[chan] == ksw_MUTE)
		{
			if (resync_staged[chan])
			{
				lfos.cycle_pos[chan] = _WRAP_F(lfos.cycle_pos[GLO_CLK] * lfos.divmult[chan], 0, 1);
				resync_staged[chan] = 0;
			}
			else {
				lfos.cycle_pos[chan] = _WRAP_F(lfos.cycle_pos[chan] + lfos.inc[chan], 0, 1);
			}
		}

		//Note/Key mode: one-shot LFOs (Envelopes)
		else
		{
			if (params.note_on[chan] && ((lfos.cycle_pos[chan] + lfos.inc[chan])>=1.0) ) {
				params.note_on[chan] = 0;
			}

			else if (!params.note_on[chan]) {
				wt_osc.wt_head_pos[chan] = 0;
				lfos.cycle_pos[chan] = 0;
			}

			else if (!calc_params.gate_in_is_sustaining[chan]) {
				lfos.cycle_pos[chan] += lfos.inc[chan];
			}
			else {
				float sustain_pos = lfo_sustain_pos[ lfos.shape[chan] ] / 256.f;
				uint8_t will_cross_sustain_position = (lfos.cycle_pos[chan]<=sustain_pos) && ((lfos.cycle_pos[chan] + lfos.inc[chan])>sustain_pos);
				if (will_cross_sustain_position) {
					lfos.cycle_pos[chan] = sustain_pos;
				} else {
					lfos.cycle_pos[chan] += lfos.inc[chan];
				}
			}
		}

		lfos.wt_pos[chan] = _WRAP_F(lfos.cycle_pos[chan] + lfos.phase[chan], 0, 1.0);
	}
}

void update_lfo_calcs(void)
{
	uint8_t chan;

	if (recalc_flagged[REF_CLK]) {
		recalc_flagged[REF_CLK] = 0;
		lfos.divmult[REF_CLK] = lfos.period[REF_CLK] / (float)LFO_INIT_PERIOD;
		lfos.divmult_id[REF_CLK] = calc_divmult_id(lfos.divmult[REF_CLK]);
		lfos.inc[REF_CLK] = calc_lfo_inc(lfos.period[REF_CLK]);
	}

	if (recalc_flagged[GLO_CLK]) {
		recalc_flagged[GLO_CLK] = 0;
		lfos.divmult[GLO_CLK] = calc_divmult_amount(lfos.divmult_id[GLO_CLK] + divmult_cv);
		lfos.period[GLO_CLK] = lfos.period[REF_CLK] / calc_divmult_amount(lfos.divmult_id[GLO_CLK] + divmult_cv);
		lfos.inc[GLO_CLK] = calc_lfo_inc(lfos.period[GLO_CLK]);
	}

	for (chan=0; chan<NUM_CHANNELS; chan++)
	{
		if (recalc_flagged[chan])
		{
			recalc_flagged[chan] = 0;
			lfos.divmult[chan] 	= calc_divmult_amount(lfos.divmult_id[chan]);

			if (!lfos.locked[chan])
				lfos.divmult_id_global_locked[chan] = lfos.divmult_id[GLO_CLK] + divmult_cv;

			lfos.period[chan] = calc_lfo_period(chan, lfos.divmult_id_global_locked[chan], lfos.period[REF_CLK]);
			lfos.inc[chan] = calc_lfo_inc(lfos.period[chan]);
		}
	}
}

//	1.0/64.0, 1.0/48.0, 1.0/32.0, 1.0/24.0, 1.0/16.0, 1.0/8.0, 1.0/7.0, 1.0/6.0, 1.0/5.0, 1.0/4.0, 1.0/3.0, 1.0/2.0,
//	1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 12.0, 16.0, 24.0, 32.0 };


float calc_divmult_amount(float divmult_id)
{
	int32_t i_divmult_id = (int32_t)divmult_id;
	float xfade = divmult_id - i_divmult_id;
	float max = LFO_DIVMULTS[NUM_DIVMULTS-1];
	float min = LFO_DIVMULTS[0];

	if (i_divmult_id < 0)
		return 1.0/((1.0/min) + 8.0*(-divmult_id));		// /64, /72, /80, /88, /96, /104, /112

	else if (i_divmult_id<(NUM_DIVMULTS-1))
		return _CROSSFADE(LFO_DIVMULTS[i_divmult_id], LFO_DIVMULTS[i_divmult_id+1], xfade);

	else
		return max + 2.0*(divmult_id - (float)NUM_DIVMULTS + 1.0); //x32, x34, x36, ... x166
}

float calc_divmult_id(float divmult)
{
	uint8_t i;

	if (divmult <= LFO_DIVMULTS[0]) {
		return 0.125 * (1.0/LFO_DIVMULTS[0] - 1.0/divmult);
	}
	else if (divmult >= LFO_DIVMULTS[NUM_DIVMULTS-1]) {
		return (divmult - LFO_DIVMULTS[NUM_DIVMULTS-1])/2.0 + NUM_DIVMULTS-1;
	}
	else {
		i = NUM_DIVMULTS;
		while (i--)
		{
			if (divmult == LFO_DIVMULTS[i])
				return i;

			else if (divmult > LFO_DIVMULTS[i])
				return (float)(i) + ((divmult - LFO_DIVMULTS[i]) / (LFO_DIVMULTS[i+1] - LFO_DIVMULTS[i]));
		}
		return i;
	}
}

float calc_lfo_period_audiorange(float chan_divmult_id, float global_divmult_id, uint32_t base_period_ms)
{
	// float f = _N_OVER_12TH_ROOT_TWO((global_divmult_id + chan_divmult_id + lfos.divmult_id[REF_CLK] - 60));
	float f = powf(2.0, (global_divmult_id + chan_divmult_id + lfos.divmult_id[REF_CLK] - 60)/12.0);
	return base_period_ms / (LFO_DIVMULTS[NUM_DIVMULTS-1] * f);
}

float calc_lfo_period_lforange(float chan_divmult_id, float global_divmult_id, uint32_t base_period_ms)
{
	float global_divmult = calc_divmult_amount(global_divmult_id);
	float chan_divmult = calc_divmult_amount(chan_divmult_id);
	return (float)base_period_ms / (global_divmult * chan_divmult);
}

float calc_lfo_period(uint8_t chan, float global_divmult_id, uint32_t base_period_ms)
{
	float period, test_period;
	float chan_divmult_id = lfos.divmult_id[chan];

	test_period = calc_lfo_period_lforange(chan_divmult_id, global_divmult_id, base_period_ms);

	if (!lfos.audio_mode[chan])
	{
		if (test_period < F_LFO_AUDIO_RANGE_PERIOD_H)
		{
			lfos.audio_mode[chan] = 1;
			period = calc_lfo_period_audiorange(chan_divmult_id, global_divmult_id, base_period_ms);
		}
		else
			period = test_period;
	}
	else
	{
		if (test_period > F_LFO_AUDIO_RANGE_PERIOD_L)
		{
			lfos.audio_mode[chan] = 0;
			period = test_period;
		}
		else
			period = calc_lfo_period_audiorange(chan_divmult_id, global_divmult_id, base_period_ms);
	}

	return period;
}

float calc_lfo_inc(float period_ms)
{
	return F_LFO_UPDATE_RATIO / period_ms;
}
