/*
 * UI_conditioning.c - Conditions encoder, switch, buttons
 *
 * Author: Dan Green (danngreen1@gmail.com),  Hugo Paris (hugoplho@gmail.com)
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

#define ENCODER_MAX 			8000  	// < abs(int16_t)

//Note: These must be in the same order as enum rotaryMap{...NUM_ROTARIES} in hardware_controls.h
//because update_encoder_q() equates turning rotary[XX] with encoder_q[XX]
enum PrimaryDigitalControls{
	pec_OCT,
	pec_TRANSPOSE,
	pec_LFOSPEED,
	pec_LFOSHAPE,
	pec_LOADPRESET,
	pec_DEPTH,
	pec_LATITUDE,
	pec_LONGITUDE,

	pec_WBROWSE,

	NUM_PRIMARY_DIGITAL_CONT
};

//Note: These must be in the same order as enum rotaryMap{...NUM_ROTARIES} in hardware_controls.h
//because update_encoder_q() equates pressing+turning rotary[XX] with encoder_q[XX + NUM_PRIMARY_DIGITAL_CONT]
enum SecondaryDigitalControls{
	sec_SCALE			= NUM_PRIMARY_DIGITAL_CONT 		,
	sec_OSC_SPREAD		= NUM_PRIMARY_DIGITAL_CONT 	+ 1	,
	sec_LFOGAIN			= NUM_PRIMARY_DIGITAL_CONT 	+ 2	,
	sec_LFOPHASE		= NUM_PRIMARY_DIGITAL_CONT 	+ 3	,
	sec_SAVEPRESET		= NUM_PRIMARY_DIGITAL_CONT 	+ 4	,
	sec_DISPERSION		= NUM_PRIMARY_DIGITAL_CONT 	+ 5	,
	sec_DISPPATT		= NUM_PRIMARY_DIGITAL_CONT 	+ 6	,
	sec_WTSEL_SPREAD	= NUM_PRIMARY_DIGITAL_CONT 	+ 7	,

	sec_WTSEL			= NUM_PRIMARY_DIGITAL_CONT 	+ 8	,

	NUM_DIGITAL_CONTROLS
};

enum PressTypes rotary_pressed(uint8_t rotary_num);
uint8_t rotary_reg_pressed(uint8_t rotary_num);
uint8_t rotary_short_pressed(uint8_t rotary_num);
uint8_t rotary_med_pressed(uint8_t rotary_num);
uint8_t rotary_long_pressed(uint8_t rotary_num);
uint8_t rotary_released(uint8_t rotary_num);

enum PressTypes button_pressed(uint8_t button_num);
uint8_t button_short_pressed(uint8_t button_num);
uint8_t button_med_pressed(uint8_t button_num);
uint8_t button_long_pressed(uint8_t button_num);
uint8_t button_released(uint8_t button_num);

uint8_t switch_pressed(uint8_t switch_num);
uint8_t switch_released(uint8_t switch_num);

uint8_t jack_plugged(uint8_t switch_num);
uint8_t jack_unplugged(uint8_t switch_num);

void 	init_encoders(void);
int16_t pop_encoder_q(uint8_t digital_control);

void 	start_UI_conditioning_updates(void);
