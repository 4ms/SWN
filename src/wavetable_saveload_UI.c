/*
 * wavetable_saveload_UI.c - Saving and recalling spheres
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


#include "wavetable_saveload_UI.h"
#include "wavetable_saveload.h"
#include "wavetable_editing.h"
#include "led_cont.h"
#include "UI_conditioning.h"
#include "params_update.h"
#include "math_util.h"
#include "sphere_flash_io.h"
#include "params_sphere_enable.h"

o_UserSphereManager user_sphere_mgr;

void init_user_sphere_mgr(uint8_t initial_sphere_num)
{
	uint16_t i;

	user_sphere_mgr.hover_num 		= initial_sphere_num; 
	user_sphere_mgr.animation_ctr 	= 0;
	user_sphere_mgr.activity_tmr	= 0;
	user_sphere_mgr.mode			= WTS_INACTIVE;

	for (i=0;i<NUM_USER_SPHERES_ALLOWED;i++){
		user_sphere_mgr.filled[i] = get_spheretype(i);
	}

}

uint8_t get_sphere_hover(void)
{
	return user_sphere_mgr.hover_num;
}


void handle_wt_saving_events(int16_t enc_turn)
{
	static uint32_t 	last_time = 0xFFFFFFFF;
	uint32_t 			now;
	uint32_t 			elapsed_time;
	static uint8_t 		last_preset_press_state = RELEASED;
	uint8_t 			preset_press_state;
	uint8_t 			preset_just_released;
	static uint8_t 		last_wtspread_press_state = RELEASED;
	uint8_t 			wtspread_press_state;
	uint8_t 			wtspread_just_released;
	uint8_t				sphere_index;
	uint8_t				sphere_type = user_sphere_mgr.filled[user_sphere_mgr.hover_num];

	now = (HAL_GetTick()/TICKS_PER_MS);
	elapsed_time = now - last_time;
	if (last_time==0xFFFFFFFF) elapsed_time = 0;
	last_time = now;

	preset_press_state = rotary_pressed(rotm_PRESET);
	wtspread_press_state = rotary_pressed(rotm_LONGITUDE);

	if (user_sphere_mgr.activity_tmr)
	{
		if (user_sphere_mgr.activity_tmr>elapsed_time)
			user_sphere_mgr.activity_tmr-=elapsed_time;
		else
		{
			exit_wt_saving();
			stop_all_displays();
		}
	}

	if (user_sphere_mgr.animation_ctr)
	{
		user_sphere_mgr.animation_ctr += elapsed_time;
		if (user_sphere_mgr.animation_ctr >= SPHERE_SAVE_ANIMATION_TIME)
		{
			exit_wt_saving();
			stop_all_displays();
		}
		else
			user_sphere_mgr.activity_tmr = SPHERE_SAVE_TIMER_LIMIT;
	}
	else if (enc_turn)
	{
		user_sphere_mgr.hover_num 	= _CLAMP_I16(enc_turn + user_sphere_mgr.hover_num, 0, MAX_TOTAL_SPHERES-1);
		user_sphere_mgr.mode		= WTS_SELECTING;
		user_sphere_mgr.activity_tmr= SPHERE_SAVE_TIMER_LIMIT;

		start_ongoing_display_sphere_save();
	}

	preset_just_released = (preset_press_state==RELEASED && last_preset_press_state>=PRESSED) ? 1 : 0;
	wtspread_just_released = (wtspread_press_state==RELEASED && last_wtspread_press_state>=PRESSED) ? 1 : 0;

	switch(user_sphere_mgr.mode)
	{
		case (WTS_INACTIVE):
			if (preset_press_state)
			{
				user_sphere_mgr.mode				= WTS_SELECTING;
				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
				start_ongoing_display_sphere_save();
			}
			break;

		case (WTS_SELECTING):
		case (WTS_SAVE_HELD):
			if (preset_just_released)
			{
				if (last_preset_press_state>SHORT_PRESSED) {
					if (sphere_type!=SPHERE_TYPE_FACTORY)
						user_sphere_mgr.mode 		= WTS_SAVE_CONFIRM;
				}
				else if (sphere_type==SPHERE_TYPE_USER || sphere_type==SPHERE_TYPE_FACTORY)
					user_sphere_mgr.mode 			= WTS_LOAD_CONFIRM;

				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
				start_ongoing_display_sphere_save();
			}
			if (preset_press_state>=LONG_PRESSED)
			{
				if (sphere_type==SPHERE_TYPE_USER) {
					user_sphere_mgr.mode			= WTS_CLEAR_HELD;
					user_sphere_mgr.activity_tmr	= SPHERE_SAVE_TIMER_LIMIT;
				}
				else if (sphere_type==SPHERE_TYPE_CLEARED) {
					user_sphere_mgr.mode			= WTS_UNCLEAR_HELD;
					user_sphere_mgr.activity_tmr	= SPHERE_SAVE_TIMER_LIMIT;
				}

			}
			else if (preset_press_state>SHORT_PRESSED && sphere_type!=SPHERE_TYPE_FACTORY)
			{
				user_sphere_mgr.mode				= WTS_SAVE_HELD;
				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
			}
			else if (wtspread_press_state)
			{
				if (is_sphere_enabled(user_sphere_mgr.hover_num))
					user_sphere_mgr.mode			= WTS_PRESSED_TO_DO_DISABLE;
				else
					user_sphere_mgr.mode			= WTS_PRESSED_TO_DO_ENABLE;

				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
				start_ongoing_display_sphere_save();
			}
			break;

		case (WTS_CLEAR_HELD):
		case (WTS_UNCLEAR_HELD):
			if (preset_just_released)
			{
				user_sphere_mgr.mode++; 			//= WTS_XXXX_CONFIRM;
				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
				start_ongoing_display_sphere_save();
			}
			break;

		case (WTS_LOAD_CONFIRM):
		case (WTS_SAVE_CONFIRM):
		case (WTS_CLEAR_CONFIRM):
		case (WTS_UNCLEAR_CONFIRM):
			if (preset_press_state)
				user_sphere_mgr.mode++;				//= WTS_PRESSED_TO_DO_XXXX;
			
			if (wtspread_press_state)
			{
				if (is_sphere_enabled(user_sphere_mgr.hover_num))
					user_sphere_mgr.mode			= WTS_PRESSED_TO_DO_DISABLE;
				else
					user_sphere_mgr.mode			= WTS_PRESSED_TO_DO_ENABLE;

				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
				start_ongoing_display_sphere_save();
			}
			break;

		case (WTS_PRESSED_TO_DO_LOAD):
		case (WTS_PRESSED_TO_DO_SAVE):
		case (WTS_PRESSED_TO_DO_CLEAR):
		case (WTS_PRESSED_TO_DO_UNCLEAR):
			if (preset_just_released)
			{
				user_sphere_mgr.mode++;				//= WTS_DOING_XXXX;
				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
				user_sphere_mgr.animation_ctr		= 1;


				if (user_sphere_mgr.mode==WTS_DOING_LOAD) {
					load_sphere(user_sphere_mgr.hover_num);
				}

				else if (user_sphere_mgr.mode==WTS_DOING_SAVE)
				{
					enable_sphere(user_sphere_mgr.hover_num);
					save_user_sphere(user_sphere_mgr.hover_num);
					sphere_index = bank_to_sphere_index(user_sphere_mgr.hover_num);
					set_wtsel(sphere_index);
					force_all_wt_interp_update();
					exit_wtediting();
				}

				else if (user_sphere_mgr.mode==WTS_DOING_CLEAR) {
					user_sphere_mgr.filled[user_sphere_mgr.hover_num] = clear_user_sphere(user_sphere_mgr.hover_num);
					update_number_of_user_spheres_filled();
				}

				else if (user_sphere_mgr.mode==WTS_DOING_UNCLEAR)
					user_sphere_mgr.filled[user_sphere_mgr.hover_num] = unclear_user_sphere(user_sphere_mgr.hover_num);

			}
			break;

		case (WTS_PRESSED_TO_DO_ENABLE):
		case (WTS_PRESSED_TO_DO_DISABLE):
			if (wtspread_just_released)
			{
				user_sphere_mgr.mode++;				//= WTS_DOING_XXXX;
				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
				// user_sphere_mgr.animation_ctr		= SPHERE_SAVE_ANIMATION_TIME - 1;

				if (is_sphere_enabled(user_sphere_mgr.hover_num))
					disable_sphere(user_sphere_mgr.hover_num);
				else
					enable_sphere(user_sphere_mgr.hover_num);

				update_number_of_user_spheres_filled();
				start_ongoing_display_sphere_save();
			}
			break;

		default:
			break;
	}

	last_preset_press_state = preset_press_state;
	last_wtspread_press_state = wtspread_press_state;
}

void exit_wt_saving(void)
{
	user_sphere_mgr.mode 			= WTS_INACTIVE;
	user_sphere_mgr.activity_tmr	= 0;
	user_sphere_mgr.animation_ctr 	= 0;
}

void animate_wt_saving_ledring(uint8_t slot_i, o_rgb_led *rgb)
{
	const enum colorCodes save_animation_color 	= ledc_RED;
	const enum colorCodes load_animation_color 	= ledc_GREEN;
	const enum colorCodes clear_animation_color = ledc_WHITE;
	const enum colorCodes hover_color 			= ledc_BUTTERCUP;
	const enum colorCodes save_color 			= ledc_RED;
	const enum colorCodes load_color 			= ledc_GREEN;

	enum colorCodes slot_color 					= ledc_GOLD;

	enum colorCodes ring_color;
	int8_t direction;
	uint8_t hover_slot;
	uint8_t hover_bank;
	uint8_t user_sphere_num;
	uint32_t now;
	
	now = (HAL_GetTick()/TICKS_PER_MS);

	if (user_sphere_mgr.hover_num >= NUM_FACTORY_SPHERES) {
		hover_slot = ((user_sphere_mgr.hover_num - NUM_FACTORY_SPHERES) % NUM_LED_OUTRING);
		hover_bank = (user_sphere_mgr.hover_num - NUM_FACTORY_SPHERES) / NUM_LED_OUTRING;
		user_sphere_num = slot_i + NUM_FACTORY_SPHERES + hover_bank*NUM_LED_OUTRING;
	}
	else
	{
		hover_slot = (user_sphere_mgr.hover_num % NUM_LED_OUTRING);
		user_sphere_num = (slot_i < NUM_FACTORY_SPHERES) ? slot_i : 0xFF;
	}

	if (user_sphere_mgr.animation_ctr > 0)
	{
		if (user_sphere_mgr.mode == WTS_DOING_LOAD)	{		
			ring_color = load_animation_color;
			direction = 1;
		}
		else if (user_sphere_mgr.mode == WTS_DOING_SAVE) {	
			ring_color = save_animation_color;
			direction = 1;
		}
		else if (user_sphere_mgr.mode == WTS_DOING_CLEAR) {
			ring_color = clear_animation_color;
			direction = 1;
		}
		else if (user_sphere_mgr.mode == WTS_DOING_UNCLEAR) {
			ring_color = clear_animation_color;
			direction = -1;
		}
		else {
			ring_color = load_animation_color;
			direction = 1;
		}

		if (user_sphere_mgr.animation_ctr > SPHERE_SAVE_ANIMATION_TIME)
			user_sphere_mgr.animation_ctr = SPHERE_SAVE_ANIMATION_TIME;
		
		if (slot_i==hover_slot)
			set_rgb_color(rgb, (now & 0x040) ? ledc_OFF : slot_color);

		else if ((direction==1) && slot_i<(user_sphere_mgr.animation_ctr/(SPHERE_SAVE_ANIMATION_TIME/NUM_LED_OUTRING)))
			set_rgb_color(rgb, ring_color);

		else if ((direction==-1) && (NUM_LED_OUTRING-slot_i)>(user_sphere_mgr.animation_ctr/(SPHERE_SAVE_ANIMATION_TIME/NUM_LED_OUTRING))) 
			set_rgb_color(rgb, ring_color);

		else
			set_rgb_color(rgb, ledc_OFF);
	}
	else {
		if (slot_i==hover_slot)
		{
			switch (user_sphere_mgr.mode)
			{
				case (WTS_INACTIVE):
				case (WTS_SELECTING):
				case (WTS_DOING_ENABLE):
				case (WTS_DOING_DISABLE):
				case (WTS_PRESSED_TO_DO_ENABLE):
				case (WTS_PRESSED_TO_DO_DISABLE):
					if (now & 0x0C0)
						get_wt_color(user_sphere_num, rgb);
					else
						set_rgb_color(rgb, hover_color);

					rgb->brightness = F_MAX_BRIGHTNESS*2.0;
					break;

				case (WTS_LOAD_CONFIRM):
					set_rgb_color(rgb, (now & 0x080) ? load_color : ledc_OFF);
					break;

				case (WTS_SAVE_CONFIRM):
				case (WTS_SAVE_HELD):
					set_rgb_color(rgb, (now & 0x080) ? save_color : ledc_OFF);
					break;

				case (WTS_CLEAR_HELD):
				case (WTS_UNCLEAR_HELD):
					set_rgb_color (rgb, (now >> 6) % NUM_PALETTE_COLORS);
					break;

				case (WTS_CLEAR_CONFIRM):
				case (WTS_UNCLEAR_CONFIRM):
					set_rgb_color(rgb, (now >> 4) % NUM_PALETTE_COLORS);
					break;

				default:
					set_rgb_color(rgb, ledc_OFF);
					break;
			}
		}
		else {

			if (is_sphere_filled(user_sphere_num))
			{
				get_wt_color(user_sphere_num, rgb);
				if (is_sphere_enabled(user_sphere_num))
					rgb->brightness = F_MAX_BRIGHTNESS*0.8;
				else
					rgb->brightness = (now & 0x0C0) ? 0 : F_MAX_BRIGHTNESS*0.8;
			}
			else
			{
				set_rgb_color(rgb, ledc_OFF);
				
				// debug colors:
				// get_wt_color(user_sphere_num + NUM_FACTORY_SPHERES, rgb);
			}
		}
	}
}
