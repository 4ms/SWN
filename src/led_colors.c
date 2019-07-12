/*
 * led_colors.c - led colors
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

#include "led_colors.h"
#include "led_cont.h"
#include "math_util.h"

//Off-state RGB LED
const o_rgb_led	RGB_LED_OFF = {0,0,0,0};

uint32_t colorPalette[NUM_LED_COLORS][NUM_PALETTE_COLORS];

void set_rgb_color(o_rgb_led *led, uint8_t palette_i)
{
	if (palette_i < NUM_PALETTE_COLORS)
	{
		led->brightness = F_MAX_BRIGHTNESS;
		led->c_red 		= colorPalette[c_RED]  [palette_i];
		led->c_green 	= colorPalette[c_GREEN][palette_i];
		led->c_blue 	= colorPalette[c_BLUE] [palette_i];
	}
}

//brightness is a value between 0..1
void set_rgb_color_brightness(o_rgb_led *led, uint8_t palette_i, float brightness)
{
	if (palette_i < NUM_PALETTE_COLORS)
	{
		led->brightness = F_MAX_BRIGHTNESS * brightness;
		led->c_red 		= colorPalette[c_RED]  [palette_i];
		led->c_green 	= colorPalette[c_GREEN][palette_i];
		led->c_blue 	= colorPalette[c_BLUE] [palette_i];
	}
}

//adds the given color RGB values (times given brightness) to the LED's existing RGB values
void add_rgb_color_brightness(o_rgb_led *led, uint8_t palette_i, float brightness)
{
	if (palette_i < NUM_PALETTE_COLORS)
	{
		led->c_red 		= _CLAMP_U32(((float)led->c_red   * led->brightness) + ((float)colorPalette[c_RED][palette_i] 	* brightness), 0, 4095);
		led->c_green 	= _CLAMP_U32(((float)led->c_green * led->brightness) + ((float)colorPalette[c_GREEN][palette_i] * brightness), 0, 4095);
		led->c_blue 	= _CLAMP_U32(((float)led->c_blue  * led->brightness) + ((float)colorPalette[c_BLUE][palette_i] 	* brightness), 0, 4095);
		led->brightness = F_MAX_BRIGHTNESS;
	}
}

void set_rgb_color_by_array(o_rgb_led *led, const uint16_t *RGB, float brightness)
{
	led->brightness = F_MAX_BRIGHTNESS * brightness;
	led->c_red 		= RGB[c_RED];
	led->c_green 	= RGB[c_GREEN];
	led->c_blue 	= RGB[c_BLUE];
}

void set_rgb_color_by_rgb(o_rgb_led *led, o_rgb_led *srcled)
{
	led->brightness = srcled->brightness;
	led->c_red 		= srcled->c_red;
	led->c_green 	= srcled->c_green;
	led->c_blue 	= srcled->c_blue;
}

//Todo: Make colorPalette an array of o_rgb_led elements
void init_color_palette(void)
{
	colorPalette[c_RED]	 [ledc_OFF] 	= 0;
	colorPalette[c_GREEN][ledc_OFF] 	= 0;
	colorPalette[c_BLUE] [ledc_OFF] 	= 0;

	colorPalette[c_RED]	 [ledc_WHITE] 	= 1023;
	colorPalette[c_GREEN][ledc_WHITE] 	= 1023;
	colorPalette[c_BLUE] [ledc_WHITE] 	= 1023;	

	colorPalette[c_RED]	 [ledc_PINK]  	= 1023;
	colorPalette[c_GREEN][ledc_PINK]  	= 290;
	colorPalette[c_BLUE] [ledc_PINK]  	= 290;

	colorPalette[c_RED]	 [ledc_PURPLE]  = 1023;
	colorPalette[c_GREEN][ledc_PURPLE]  = 200;
	colorPalette[c_BLUE] [ledc_PURPLE]  = 1023;

	colorPalette[c_RED]	 [ledc_RED]   	= 1023;
	colorPalette[c_GREEN][ledc_RED] 	= 0;
	colorPalette[c_BLUE] [ledc_RED] 	= 0;

	colorPalette[c_RED]	 [ledc_GREEN] 	= 0;
	colorPalette[c_GREEN][ledc_GREEN] 	= 1023;
	colorPalette[c_BLUE] [ledc_GREEN] 	= 0; 

	colorPalette[c_RED]	 [ledc_LIGHT_BLUE]  = 150;
	colorPalette[c_GREEN][ledc_LIGHT_BLUE]  = 600;
	colorPalette[c_BLUE] [ledc_LIGHT_BLUE]  = 1023;		


	colorPalette[c_RED]	 [ledc_GOLD]  = 1023;
	colorPalette[c_GREEN][ledc_GOLD]  = 200;
	colorPalette[c_BLUE] [ledc_GOLD]  = 0;

	colorPalette[c_RED]	 [ledc_CORAL]  = 3816;
	colorPalette[c_GREEN][ledc_CORAL]  = 220;
	colorPalette[c_BLUE] [ledc_CORAL]  = 84;		


	colorPalette[c_RED]	 [ledc_BRIGHTPINK]  	= 3816;
	colorPalette[c_GREEN][ledc_BRIGHTPINK]  	= 500;
	colorPalette[c_BLUE] [ledc_BRIGHTPINK]  	= 500;


	colorPalette[c_RED]	 [ledc_MED_RED]  = 1023;
	colorPalette[c_GREEN][ledc_MED_RED]  = 100;
	colorPalette[c_BLUE] [ledc_MED_RED]  = 100;		

	colorPalette[c_RED]	 [ledc_MED_GREEN]  = 0;
	colorPalette[c_GREEN][ledc_MED_GREEN]  = 1023;
	colorPalette[c_BLUE] [ledc_MED_GREEN]  = 100;	

	colorPalette[c_RED]	 [ledc_MED_BLUE]  = 50;
	colorPalette[c_GREEN][ledc_MED_BLUE]  = 150;
	colorPalette[c_BLUE] [ledc_MED_BLUE]  = 1023;	

	colorPalette[c_RED]	 [ledc_FUSHIA]  = 800;
	colorPalette[c_GREEN][ledc_FUSHIA]  = 1;
	colorPalette[c_BLUE] [ledc_FUSHIA]  = 100;		

	colorPalette[c_RED]	 [ledc_BLUE]  = 0;
	colorPalette[c_GREEN][ledc_BLUE]  = 0;
	colorPalette[c_BLUE] [ledc_BLUE]  = 1023;	

	colorPalette[c_RED]	 [ledc_BRIGHT_BLUE]  = 0;
	colorPalette[c_GREEN][ledc_BRIGHT_BLUE]  = 0;
	colorPalette[c_BLUE] [ledc_BRIGHT_BLUE]  = 4095;	


	colorPalette[c_RED]	 [ledc_LIGHT_GREEN]  = 20;
	colorPalette[c_GREEN][ledc_LIGHT_GREEN]  = 500;
	colorPalette[c_BLUE] [ledc_LIGHT_GREEN]  = 100;	

	colorPalette[c_RED]	 [ledc_DIM_GREEN]  = 0;
	colorPalette[c_GREEN][ledc_DIM_GREEN]  = 100;
	colorPalette[c_BLUE] [ledc_DIM_GREEN]  = 0;	

	colorPalette[c_RED]	 [ledc_DIM_RED]  = 100;
	colorPalette[c_GREEN][ledc_DIM_RED]  = 0;
	colorPalette[c_BLUE] [ledc_DIM_RED]  = 0;	

	colorPalette[c_RED]	 [ledc_DIM_YELLOW]  = 200;
	colorPalette[c_GREEN][ledc_DIM_YELLOW]  = 100;
	colorPalette[c_BLUE] [ledc_DIM_YELLOW]  = 0;

	colorPalette[c_RED]	 [ledc_AQUA]  = 150;
	colorPalette[c_GREEN][ledc_AQUA]  = 1500;
	colorPalette[c_BLUE] [ledc_AQUA]  = 700;	

	colorPalette[c_RED]	 [ledc_YELLOW]  = 900;
	colorPalette[c_GREEN][ledc_YELLOW]  = 900;
	colorPalette[c_BLUE] [ledc_YELLOW]  = 0;	

	colorPalette[c_RED]	 [ledc_FULLRED]   	= 4095;
	colorPalette[c_GREEN][ledc_FULLRED] 	= 0;
	colorPalette[c_BLUE] [ledc_FULLRED] 	= 0;

	colorPalette[c_RED]	 [ledc_BUTTERCUP]  = 4000;
	colorPalette[c_GREEN][ledc_BUTTERCUP]  = 1500;
	colorPalette[c_BLUE] [ledc_BUTTERCUP]  = 500;	

	colorPalette[c_RED]	 [ledc_DEEP_BLUE]  = 100;
	colorPalette[c_GREEN][ledc_DEEP_BLUE]  = 0;
	colorPalette[c_BLUE] [ledc_DEEP_BLUE]  = 900;	

	

}
