/*
 * led_colors.h - led colors
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
#include "led_map.h"

enum colorCodes{
	ledc_OFF,		
	ledc_WHITE,		
	ledc_PINK,		
	ledc_PURPLE,	
	ledc_RED,		
	ledc_GREEN,		
	ledc_LIGHT_BLUE,
	ledc_GOLD,			
	ledc_CORAL,			
	ledc_ROTARY_WHITE,
	ledc_ROTARY_RED,
	ledc_MED_RED,	
	ledc_MED_GREEN,	
	ledc_MED_BLUE,	
	ledc_FUSHIA,	
	ledc_BLUE,		
	ledc_LIGHT_GREEN,
	ledc_DIM_GREEN,
	ledc_DIM_RED,
	ledc_DIM_YELLOW,
	ledc_BRIGHT_BLUE,
	ledc_AQUA,
	ledc_YELLOW,
	ledc_FULLRED,
	ledc_BUTTERCUP,
	ledc_DEEP_BLUE,
	ledc_BRIGHTPINK,

	NUM_PALETTE_COLORS
};

enum ledColors{
	c_RED,
	c_GREEN,
	c_BLUE,

	NUM_LED_COLORS
};

void set_rgb_color(o_rgb_led *led, uint8_t palette_i);
void set_rgb_color_brightness(o_rgb_led *led, uint8_t palette_i, float brightness);
void add_rgb_color_brightness(o_rgb_led *led, uint8_t palette_i, float brightness);
void set_rgb_color_by_array(o_rgb_led *led, const uint16_t *RGB, float brightness);
void set_rgb_color_by_rgb(o_rgb_led *led, o_rgb_led *srcled);
void init_color_palette(void);

