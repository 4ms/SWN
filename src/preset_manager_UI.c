/*
 * preset_manager_UI.c - User Interface for preset manager (reading controls and display lights)
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

#include "preset_manager.h"
#include "preset_manager_UI.h"
#include "preset_manager_undo.h"
#include "startup_preset_storage.h"
#include "led_cont.h"
#include "globals.h"
#include "params_update.h"
#include "params_lfo.h"
#include "UI_conditioning.h"
#include "gpio_pins.h"
#include "math_util.h"

extern	o_preset_manager		preset_mgr;

const uint32_t PRESET_ANIMATION_TIME = (NUM_LED_OUTRING*40);

enum PresetMgrStates	last_preset_action=PM_INACTIVE;

void handle_preset_events(int16_t enc_turn, int16_t enc_pushturn)
{
	static uint32_t 	last_time = 0xFFFFFFFF;
	uint32_t 			now;
	uint32_t 			elapsed_time;
	static uint8_t 		last_press_state = RELEASED;
	uint8_t 			press_state;
	uint8_t 			just_released;

	now = (HAL_GetTick()/TICKS_PER_MS);
	elapsed_time = now - last_time;
	if (last_time==0xFFFFFFFF) elapsed_time = 0;
	last_time = now;

	press_state = rotary_pressed(rotm_PRESET);

	if (preset_mgr.activity_tmr)
	{
		if (preset_mgr.activity_tmr>elapsed_time)
			preset_mgr.activity_tmr-=elapsed_time;
		else
		{
			exit_preset_manager();
			stop_all_displays();
		}
	}

	if (preset_mgr.animation_ctr)
	{
		preset_mgr.animation_ctr += elapsed_time;
		if (preset_mgr.animation_ctr >= PRESET_ANIMATION_TIME)
		{
			exit_preset_manager();
			stop_all_displays();
		}
		else
			preset_mgr.activity_tmr = PRESET_TIMER_LIMIT;
	}
	else if (enc_turn)
	{
		preset_mgr.hover_num 		= _CLAMP_I16(enc_turn + preset_mgr.hover_num, 0, MAX_PRESETS-1);
		preset_mgr.mode				= PM_SELECTING;
		preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT; //could be any value, doesn't need to be linked to the display timer

		start_ongoing_display_preset();
	}
	else if (enc_pushturn)
	{
		//ToDo: Browse/Preview mode?
		preset_mgr.hover_num 		= _CLAMP_I16(enc_pushturn + preset_mgr.hover_num, 0, MAX_PRESETS-1);
		preset_mgr.mode				= PM_SELECTING;
		preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT;

		start_ongoing_display_preset();
	}

	just_released = (press_state==RELEASED && last_press_state>=PRESSED) ? 1 : 0;

	switch(preset_mgr.mode)
	{
		case (PM_INACTIVE):
			if (press_state)
			{
				if (switch_pressed(FINE_BUTTON)){
					preset_mgr.mode				= PM_UNDO_HELD;
				} else {
					preset_mgr.mode				= PM_SELECTING;
				}
				preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT;
				start_ongoing_display_preset();
			}
			break;

		case (PM_UNDO_HELD):
			if (just_released)
			{
				undo_preset_action();
			}
			break;

		case (PM_CLEAR_HELD):
			if (just_released)
			{
				preset_mgr.mode 			= PM_CLEAR_CONFIRM;
				preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT;
				start_ongoing_display_preset();
			}
			break;

		case (PM_SELECTING):
		case (PM_SAVE_HELD):
			if (just_released)
			{
				if (last_press_state>SHORT_PRESSED) 	preset_mgr.mode = PM_SAVE_CONFIRM;
				else 									preset_mgr.mode = PM_LOAD_CONFIRM;
				preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT;
				start_ongoing_display_preset();
			}

			if (press_state>=LONG_PRESSED)
			{
				if (preset_mgr.filled[preset_mgr.hover_num]) {
					preset_mgr.mode				= PM_CLEAR_HELD;
					preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT;
				}
			}
			else if (press_state>SHORT_PRESSED)
			{
				preset_mgr.mode				= PM_SAVE_HELD;
				preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT;
			}
			else if (press_state && switch_pressed(FINE_BUTTON)){
				preset_mgr.mode				= PM_UNDO_HELD;
				preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT;
			}

			break;

		case (PM_LOAD_CONFIRM):
			if (press_state)
			{
				preset_mgr.mode				= PM_PRESSED_TO_DO_LOAD;
			}
			break;

		case (PM_PRESSED_TO_DO_LOAD):
			if (just_released)
			{
				recall_preset_into_active(preset_mgr.hover_num);
				set_startup_preset(preset_mgr.hover_num);
			}
			break;

		case (PM_SAVE_CONFIRM):
			if (press_state)
			{
				preset_mgr.mode				= PM_PRESSED_TO_DO_SAVE;
			}
			break;

		case (PM_PRESSED_TO_DO_SAVE):
			if (just_released)
			{
				if (preset_mgr.filled[preset_mgr.hover_num])
				{
					preset_mgr.last_action 		= PM_DOING_SAVE;
					recall_slot_into_undo_buffer(preset_mgr.hover_num);
				}
				else
					preset_mgr.last_action 		= PM_DOING_SAVE_OVER_EMPTY;

				store_preset_from_active(preset_mgr.hover_num);
			}
			break;

		case (PM_CLEAR_CONFIRM):
			if (press_state)
			{
				preset_mgr.mode				= PM_PRESSED_TO_DO_CLEAR;
			}
			break;

		case (PM_PRESSED_TO_DO_CLEAR):
			if (just_released)
			{
				preset_mgr.mode				= PM_DOING_CLEAR;
				preset_mgr.last_action 		= PM_DOING_CLEAR;
				preset_mgr.last_action_slot	= preset_mgr.hover_num;
				preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT;
				preset_mgr.animation_ctr	= 1;

				recall_slot_into_undo_buffer(preset_mgr.hover_num);
				clear_preset(preset_mgr.hover_num);
			}
			break;

		default:
			break;
	}

	last_press_state = press_state;
}

enum colorCodes animate_preset_ledring(uint8_t slot_i, uint8_t preset_i)
{
	enum colorCodes ring_fill_color, slot_color;
	uint8_t hover_slot = (preset_mgr.hover_num % NUM_LED_OUTRING);
	int8_t direction;
	uint32_t now;

	now = (HAL_GetTick()/TICKS_PER_MS);

	if (preset_mgr.animation_ctr > 0)
	{
		if (preset_mgr.mode == PM_DOING_LOAD) {
			ring_fill_color = ledc_MED_GREEN;
			slot_color 		= ledc_GOLD;
			direction 		= 1;
		}
		else if (preset_mgr.mode == PM_DOING_SAVE) {
			ring_fill_color = ledc_RED;
			slot_color 		= ledc_GOLD;
			direction 		= 1;
		}
		else if (preset_mgr.mode == PM_DOING_CLEAR) {
			ring_fill_color = ledc_WHITE;
			slot_color 		= ledc_BUTTERCUP;
			direction 		= 1;
		}
		else if (preset_mgr.mode == PM_DOING_UNDO) {
			if (preset_mgr.last_action==PM_DOING_SAVE_OVER_EMPTY)	ring_fill_color = ledc_MED_RED;
			else if (preset_mgr.last_action==PM_DOING_SAVE)			ring_fill_color = ledc_MED_RED;
			else if (preset_mgr.last_action==PM_DOING_CLEAR)		ring_fill_color = ledc_WHITE;
			else if (preset_mgr.last_action==PM_DOING_LOAD)			ring_fill_color = ledc_MED_GREEN;
			else													ring_fill_color = ledc_MED_GREEN;

			slot_color 		= ledc_BUTTERCUP;
			direction 		= -1;
		}
		else { //should not get here
			ring_fill_color = ledc_DIM_YELLOW;
			slot_color 		= ledc_DIM_YELLOW;
			direction 		= 1;
		}

		if (preset_mgr.animation_ctr > PRESET_ANIMATION_TIME) preset_mgr.animation_ctr = PRESET_ANIMATION_TIME;

		if (slot_i==hover_slot)			return (now & 0x040) ? ledc_OFF : slot_color;			//Flash slot light gold if it's the one being saved into
		else
		if ((direction==1) && slot_i<(preset_mgr.animation_ctr/(PRESET_ANIMATION_TIME/NUM_LED_OUTRING)))
										return ring_fill_color;											//Turn on lights, one at a time
		else
		if ((direction==-1) && (NUM_LED_OUTRING-slot_i)>(preset_mgr.animation_ctr/(PRESET_ANIMATION_TIME/NUM_LED_OUTRING)))
										return ring_fill_color;											//Turn on lights, one at a time
		else							return ledc_OFF;


	}
	else {
		if (slot_i==hover_slot)
		{
			switch (preset_mgr.mode)
			{
				case (PM_INACTIVE):
				case (PM_SELECTING):
					return ledc_BUTTERCUP;
					break;

				case (PM_LOAD_CONFIRM):
					return ((now & 0x080) ? ledc_MED_GREEN : ledc_OFF);
					break;

				case (PM_SAVE_CONFIRM):
				case (PM_SAVE_HELD):
					return ((now & 0x080) ? ledc_RED : ledc_OFF);
					break;

				case (PM_CLEAR_HELD):
					return ((now >> 6) % NUM_PALETTE_COLORS);
					break;

				case (PM_CLEAR_CONFIRM):
					return ((now >> 4) % NUM_PALETTE_COLORS);
					break;

				default:
					return ledc_OFF;
					break;
			}
		}
		else {
			if (preset_mgr.filled[preset_i])			return ledc_DIM_YELLOW;
			else										return ledc_OFF;
		}
	}
}
void preset_set_hover_preset_num(uint16_t preset_num) {
	preset_mgr.hover_num = preset_num;
}

void preset_start_load_animation(void) {
	start_ongoing_display_preset();
	preset_mgr.mode				= PM_DOING_LOAD;
	preset_mgr.last_action 		= PM_DOING_LOAD;
	preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT;
	preset_mgr.animation_ctr	= 1;
}

void preset_start_save_animation(void) {
	start_ongoing_display_preset();
	preset_mgr.mode				= PM_DOING_SAVE;
	preset_mgr.last_action_slot	= preset_mgr.hover_num;
	preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT;
	preset_mgr.animation_ctr	= 1;
}

void preset_start_undo_animation(void) {
	start_ongoing_display_preset();
	preset_mgr.mode				= PM_DOING_UNDO;
	preset_mgr.activity_tmr		= PRESET_TIMER_LIMIT;
	preset_mgr.animation_ctr	= 1;
}


