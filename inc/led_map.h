/*
 * led_map.h - maps of RGB LEDs as organized by the hardware
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

#define NUM_LED_OUTRING		18
#define NUM_LED_INRING		6
#define NUM_LED_ARRAY 		7


typedef struct o_rgb_led
{
	uint32_t 	c_red;
	uint32_t 	c_green;
	uint32_t 	c_blue;
	float 		brightness;
} o_rgb_led;


enum ledGroups {
	ledg_OUTERRING, 		// 1
	ledg_INNERRING,			// 2
	ledg_FULLRING,			// 3
	ledg_STRING,			// 4
	ledg_ALLSMD, 			// 5
	ledg_BUTTONS, 			// 6
	ledg_ENCODERS,	 		// 7
	ledg_ALLNONSMD, 		// 8
	ledg_ALL 				// 9
};

enum ledMap {
	ledm_F_BUTTON,			//  0
	NONE1,					//  1
	NONE2,					//	2
	ledm_D_BUTTON,			//  3
	ledm_E_BUTTON,			//  4
	ledm_B_BUTTON,			//  5
	ledm_C_BUTTON,			//	6
	ledm_LFOMODE_BUTTON,	//  7
	NONE3,					// 	8
	ledm_A_BUTTON,			//	9
								//CLKLED is a single-element LED inserted at this position
	ledm_STRING6,			//  10
	NONE4,					//  11
	ledm_STRING5,			//  12
	ledm_LFOCV,				//  13
	ledm_STRING4,			//  14
	ledm_STRING1,			//  15
	ledm_STRING2,			//  16
	ledm_STRING3,	 		//  17
	ledm_AUDIOIN,			//  18
	ledm_LFOVCA_BUTTON,		//  19
	ledm_OUTRING1,			//  20
	ledm_OUTRING2,			//  21
	ledm_OUTRING3,			//  22
	ledm_OUTRING4,			//  23
	ledm_INRING1,			//  24
	ledm_OUTRING15,			//  25
	ledm_OUTRING16,			//  26
	ledm_OUTRING17,			//  27
	ledm_OUTRING18,			//  28
	ledm_INRING6,			//  29
	ledm_OUTRING6,			//  30
	ledm_OUTRING5,			//  31
	ledm_INRING2,			//  32
	ledm_LATITUDE_ENC,		//  33
	ledm_LONGITUDE_ENC,		//  34
	ledm_INRING5,			//  35
	ledm_OUTRING14,			//  36
	ledm_OUTRING13,			//  37
	NONE5,					//  38
	ledm_DEPTH_ENC,			//  39
	ledm_OUTRING7,			//  40
	ledm_OUTRING8,			//  41
	NONE6,					//	42
	ledm_OUTRING9,			// 	43
	ledm_INRING3,			// 	44
	ledm_INRING4,			// 	45
	NONE7,					//  46
	ledm_OUTRING10,			// 	47
	ledm_OUTRING11,			// 	48
	ledm_OUTRING12,			// 	49

	NUM_LED_IDs
};

enum singlePwmLedMap {
	singleledm_CLKIN = 31		//second chip (address=1), last pin (15)
};

#define NUM_SINGLE_PWM_LEDS 1

