/*
 * params_changes.c - Handles Updating a Change to a Parameter
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

#include "params_changes.h"
#include "params_update.h"
#include "hardware_controls.h"
#include "UI_conditioning.h"

const uint8_t ALL_CHANNEL_MASK = 0b111111;

extern o_params 		params;
extern o_calc_params 	calc_params;
extern o_macro_states 	macro_states;

//Returns bitmask of which channels were changed
uint8_t change_param_f(float *param_array, float amount)
{
	uint8_t chan;
	uint8_t made_changes = 0;

	for (chan = 0; chan < NUM_CHANNELS; chan++)
	{
		if (button_pressed(chan) || (macro_states.all_af_buttons_released && !params.osc_param_lock[chan]))
		{
			param_array[chan] += amount;
			made_changes += 1<<chan;
			if (button_pressed(chan))
				calc_params.already_handled_button[chan] = 1;
		}
	}
	return made_changes;
}

//Returns bitmask of which channels were changed
uint8_t change_param_i16(int16_t *param_array, int16_t amount)
{
	uint8_t chan;
	uint8_t made_changes = 0;

	for (chan = 0; chan < NUM_CHANNELS; chan++)
	{
		if (button_pressed(chan) || (macro_states.all_af_buttons_released && !params.osc_param_lock[chan]))
		{
			param_array[chan] += amount;
			made_changes += 1<<chan;
			if (button_pressed(chan))
				calc_params.already_handled_button[chan] = 1;
		}
	}
	return made_changes;
}

//Returns bitmask of which channels were changed
uint8_t change_param_i32(int32_t *param_array, int32_t amount)
{
	uint8_t chan;
	uint8_t made_changes = 0;

	for (chan = 0; chan < NUM_CHANNELS; chan++)
	{
		if (button_pressed(chan) || (macro_states.all_af_buttons_released && !params.osc_param_lock[chan]))
		{
			param_array[chan] += amount;
			made_changes += 1<<chan;
			if (button_pressed(chan))
				calc_params.already_handled_button[chan] = 1;
		}
	}
	return made_changes;
}

