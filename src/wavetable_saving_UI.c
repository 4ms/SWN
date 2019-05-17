/*
 * wavetable_saving_UI.c - Saving and recalling spheres
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


#include "wavetable_saving_UI.h"
#include "wavetable_saving.h"
#include "wavetable_editing.h"
#include "led_cont.h"
#include "UI_conditioning.h"
#include "params_update.h"
#include "math_util.h"
#include "sphere_flash_io.h"


o_UserSphereManager user_sphere_mgr;

void init_wt_saving(void)
{
	user_sphere_mgr.hover_num 		= 0; 
	user_sphere_mgr.animation_ctr 	= 0;
	user_sphere_mgr.activity_tmr	= 0;
	user_sphere_mgr.mode			= WTS_INACTIVE;

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
	static uint8_t 		last_press_state = RELEASED;
	uint8_t 			press_state;
	uint8_t 			just_released;
	uint8_t				sphere_index;

	now = (HAL_GetTick()/TICKS_PER_MS);
	elapsed_time = now - last_time;
	if (last_time==0xFFFFFFFF) elapsed_time = 0;
	last_time = now;

	press_state = rotary_pressed(rotm_PRESET);

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
		if (user_sphere_mgr.animation_ctr >= SPHERE_ANIMATION_TIME)
		{
			exit_wt_saving();
			stop_all_displays();
		}
		else
			user_sphere_mgr.activity_tmr = SPHERE_SAVE_TIMER_LIMIT;
	}
	else if (enc_turn)
	{
		user_sphere_mgr.hover_num 	= _CLAMP_I16(enc_turn + user_sphere_mgr.hover_num, 0, NUM_USER_SPHERES_ALLOWED-1);
		user_sphere_mgr.mode		= WTS_SELECTING;
		user_sphere_mgr.activity_tmr= SPHERE_SAVE_TIMER_LIMIT; //could be any value, doesn't need to be linked to the display timer

		start_ongoing_display_sphere_save();
	}

	just_released = (press_state==RELEASED && last_press_state>=PRESSED) ? 1 : 0;

	switch(user_sphere_mgr.mode)
	{
		case (WTS_INACTIVE):
			if (press_state)
			{
				user_sphere_mgr.mode				= WTS_SELECTING;
				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
				start_ongoing_display_sphere_save();
			}
			break;

		case (WTS_SELECTING):
		case (WTS_SAVE_HELD):
			if (just_released)
			{
				if (last_press_state>SHORT_PRESSED)
					user_sphere_mgr.mode			 = WTS_SAVE_CONFIRM;

				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
				start_ongoing_display_sphere_save();
			}
			if (press_state>SHORT_PRESSED)
			{
				user_sphere_mgr.mode				= WTS_SAVE_HELD;
				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
			}
			break;

		case (WTS_SAVE_CONFIRM):
			if (press_state)
				user_sphere_mgr.mode				= WTS_PRESSED_TO_DO_SAVE;
			break;

		case (WTS_PRESSED_TO_DO_SAVE):
			if (just_released)
			{
				user_sphere_mgr.mode				= WTS_DOING_SAVE;
				user_sphere_mgr.activity_tmr		= SPHERE_SAVE_TIMER_LIMIT;
				user_sphere_mgr.animation_ctr		= 1;


				//save sphere
				save_user_sphere(user_sphere_mgr.hover_num);

				//Jump to the new sphere and exit
				sphere_index = bank_to_sphere_index(NUM_FACTORY_SPHERES + user_sphere_mgr.hover_num);
				set_wtsel(sphere_index);
				force_all_wt_interp_update();
				exit_wtediting();
			}
			break;

		default:
			break;
	}

	last_press_state = press_state;
}

void exit_wt_saving(void)
{
	user_sphere_mgr.mode 			= WTS_INACTIVE;
	user_sphere_mgr.activity_tmr	= 0;
	user_sphere_mgr.animation_ctr 	= 0;
}


void animate_wt_saving_ledring(uint8_t slot_i, o_rgb_led *rgb)
{
	const enum colorCodes ring_animation_color 	= ledc_RED;
	const enum colorCodes slot_color 			= ledc_GOLD;
	const enum colorCodes hover_color 			= ledc_BUTTERCUP;
	const enum colorCodes save_color 			= ledc_RED;

	uint8_t hover_slot = (user_sphere_mgr.hover_num % NUM_LED_OUTRING);
	uint8_t hover_bank = user_sphere_mgr.hover_num / NUM_LED_OUTRING;
	uint8_t user_sphere_num = slot_i + hover_bank*NUM_LED_OUTRING;
	uint32_t now;
	
	now = (HAL_GetTick()/TICKS_PER_MS);

	if (user_sphere_mgr.animation_ctr > 0)
	{
		if (user_sphere_mgr.animation_ctr > SPHERE_ANIMATION_TIME)
			user_sphere_mgr.animation_ctr = SPHERE_ANIMATION_TIME;
		
		if (slot_i==hover_slot)
			set_rgb_color(rgb, (now & 0x040) ? ledc_OFF : slot_color);			//Flash slot light gold if it's the one being saved into

		else if (slot_i<(user_sphere_mgr.animation_ctr/(SPHERE_ANIMATION_TIME/NUM_LED_OUTRING)))
			set_rgb_color(rgb, ring_animation_color);									//Turn on lights, one at a time

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
					if (now & 0x0C0)
						get_wt_color(user_sphere_num + NUM_FACTORY_SPHERES, rgb);
					else
						set_rgb_color(rgb, hover_color);

					rgb->brightness = F_MAX_BRIGHTNESS*2.0;
					break;

				case (WTS_SAVE_CONFIRM):
				case (WTS_SAVE_HELD):
					set_rgb_color(rgb, (now & 0x080) ? save_color : ledc_OFF);
					break;

				default:
					set_rgb_color(rgb, ledc_OFF);
					break;
			}
		}
		else {

			if (is_spheretype_user(user_sphere_num + NUM_FACTORY_SPHERES))
			{
				get_wt_color(user_sphere_num + NUM_FACTORY_SPHERES, rgb);
				rgb->brightness = F_MAX_BRIGHTNESS*0.8;
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
