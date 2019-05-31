/*
 * wavetable_saveload.h - User Interface for preset manager (reading controls and display lights)
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
#include "led_colors.h"
#include "sphere.h"

#define SPHERE_SAVE_ANIMATION_TIME (18*40)

enum WTSavingMgrStates{
	WTS_INACTIVE,
	WTS_SELECTING,

	WTS_LOAD_CONFIRM,
	WTS_PRESSED_TO_DO_LOAD,
	WTS_DOING_LOAD,

	WTS_SAVE_HELD,
	WTS_SAVE_CONFIRM,
	WTS_PRESSED_TO_DO_SAVE,
	WTS_DOING_SAVE,

	WTS_CLEAR_HELD,
	WTS_CLEAR_CONFIRM,
	WTS_PRESSED_TO_DO_CLEAR,
	WTS_DOING_CLEAR,

	WTS_UNCLEAR_HELD,
	WTS_UNCLEAR_CONFIRM,
	WTS_PRESSED_TO_DO_UNCLEAR,
	WTS_DOING_UNCLEAR,

	WTS_PRESSED_TO_DO_ENABLE,
	WTS_DOING_ENABLE,

	WTS_PRESSED_TO_DO_DISABLE,
	WTS_DOING_DISABLE,

};

typedef struct o_UserSphereManager{
	int8_t 					hover_num;
	uint8_t					filled[NUM_USER_SPHERES_ALLOWED];
	uint32_t 				animation_ctr;
	uint32_t				activity_tmr;
	enum WTSavingMgrStates	mode;
} o_UserSphereManager;

uint8_t get_sphere_hover(void);

void handle_wt_saving_events(int16_t enc_turn);
void animate_wt_saving_ledring(uint8_t slot_i, o_rgb_led *rgb);
void exit_wt_saving(void);
void init_user_sphere_mgr(uint8_t initial_sphere_num);

