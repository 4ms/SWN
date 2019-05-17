/*
 * params_lfo_clk.c
 *
 * Author: Hugo Paris (hugoplho@gmail.com), Dan Green (danngreen1@gmail.com)
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

#include <stm32f7xx.h>
#include "params_lfo.h"
#include "params_lfo_clk.h"
#include "params_lfo_period.h"
#include "gpio_pins.h"
#include "UI_conditioning.h"
#include "math_util.h"
#include "led_cont.h"
#include "params_update.h"
#include "system_settings.h"

extern o_led_cont 		led_cont;
extern o_lfos 			lfos;
extern o_systemSettings	system_settings;

uint32_t	clkinmon;

void read_ext_clk(void)
{
	// uint8_t debounce = 1;
	// float 	ext_clk_phase_shift;
	// static float prev_ticks;

	// static uint8_t 	waiting_for_clock=0; 
	static uint32_t cur_time=0;
	static uint32_t prev_clock_timestamp = 0;
	static uint8_t 	clock_up;
	static uint32_t noclock_elapsed_ticks;
	int32_t test_period;
	int32_t clock_difference;

	uint8_t		clock_state;
	// uint8_t 	chan;

	// Determine source of external clock, and read it
	if (jack_plugged(CLK_SENSE))
		clock_state = CLK_IN();

	else if (system_settings.allow_bus_clock == 1)
		clock_state = BUS_CLK();

	else
		clock_state = 0;

	//This runs every 138uS with a jitter of +/-34us (104-172us), so 7 or 8 times within every millisecond range
	cur_time = HAL_GetTick();

	// if (waiting_for_clock > debounce){
		
		// if (clock_state && (clock_up < debounce)){
		// 	clock_up ++;
		// 	noclock_elapsed_ticks = 0;
		// }

		// else if (clock_state && (clock_up == debounce)){
		// 	clock_up++;
		// 	led_cont.waiting_for_clockin = 0;
		// }

		// else if (clock_up > debounce)
		// {

		if (clock_state && !clock_up)
		{
			clock_up = 1;

			led_cont.waiting_for_clockin = 0;
			test_period = cur_time - prev_clock_timestamp;

			if (prev_clock_timestamp && test_period)
			{
				//Divide down audio-rate incoming clocks
				while (test_period < F_LFO_AUDIO_RANGE_PERIOD_L) test_period*=2;

				clock_difference = _ABS_I32(test_period - (int32_t)lfos.period[REF_CLK]);

				if (clock_difference > 0)
				{
					//Re-sync phases if clock changes by more than 1%
					if ( ((float)clock_difference / (float)lfos.period[REF_CLK]) > 0.01 )
						stage_resync_lfos();

					lfos.period[REF_CLK] = test_period;
					lfos.cycle_pos[REF_CLK] = 0;
					
					//lfos.inc[REF_CLK] = calc_lfo_inc(lfos.period[REF_CLK]);
					flag_all_lfos_recalc();
				}

				//Todo: Keep track of previous 8 periods and if the range is not 1, then discard and skip everything else.
				//Otherwise if range is 1, then find the average. Round to nearest int. Use this value for the period
				//If measured period ever changes by more than 1, then use it instead.
				//clear the buffer of 8 if ext clock is inactive
					
				// ext_clk_phase_shift 	= lfos.cycle_spent[GLO_CLK];
				
				// // phase shift global clock to match external clk
				// if (lfos.use_ext_clock)
				// 	lfos.cycle_spent[GLO_CLK] = (int)(lfos.cycle_spent[GLO_CLK]+0.5);
				// else 
				// 	lfos.cycle_spent[GLO_CLK] = 0; 

				// // phase shift LFOs by same amount
				// ext_clk_phase_shift = lfos.cycle_spent[GLO_CLK] - ext_clk_phase_shift;
				
				// for (chan = 0; chan <NUM_CHANNELS; chan++)
				// {
				// 	//Dividing on an ext clock and cycles spent rounds to spro
				// 	if (lfos.use_ext_clock && (lfos.spro[chan]<1) && ((int)(lfos.cycle_spent[chan] + 0.5) >(int)((1/lfos.spro[chan])+0.5) ))
				// 		lfos.cycle_spent[chan] 	=  lfos.phase[chan];
				// 	else
				// 		lfos.cycle_spent[chan] += ext_clk_phase_shift;
				// }

				// if  ( !lfos.use_ext_clock ||
				// 	( (base_period-prev_ticks) > 0.1 || (base_period-prev_ticks) < -0.1 ) ){
					
				// 	for (chan = 0; chan <NUM_CHANNELS; chan++){
				// 		// stage_resync_lfos();
				// 		if((lfos.spro[chan]>=1) || !lfos.use_ext_clock)
				// 			stage_resync(chan);	
				// 	}
				// 	prev_ticks = base_period;
				// }

				lfos.use_ext_clock = 1;
			}

			prev_clock_timestamp = cur_time;
			// waiting_for_clock 	 = 0;
		}
	// }

 	if (!clock_state)
 	{
 		// if (waiting_for_clock < (2*debounce))
 		// 	waiting_for_clock++;
 		clock_up = 0;

 		// CLOCK DETECTION TIMEOUT
		noclock_elapsed_ticks = cur_time - prev_clock_timestamp;
		if ( noclock_elapsed_ticks > (lfos.period[REF_CLK]*2))
		{
			// prev_clock_timestamp = 0;
			led_cont.waiting_for_clockin = 0;
			lfos.use_ext_clock = 0;
			led_cont.clockin_wait_progress = 0;
		}
		else if ((noclock_elapsed_ticks > lfos.period[REF_CLK]) && lfos.use_ext_clock)
		{
			led_cont.waiting_for_clockin = 1;
			led_cont.clockin_wait_progress = ((float)(noclock_elapsed_ticks)) / (2.0*(float)(lfos.period[REF_CLK]));
		}
	}
}





