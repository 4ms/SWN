/*
 * led_color_adjust.c - handles color adjustments for leds
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

#include "led_color_adjust.h"
#include "led_cont.h"
#include "hardware_controls.h"

#include "drivers/switch_driver.h"
#include "drivers/rotary_driver.h"
#include "drivers/leds_pwm.h"
#include "drivers/mono_led_driver.h"

#include "led_colors.h"
#include "flash_params.h"
#include "analog_conditioning.h"
#include "UI_conditioning.h"

#include "ui_modes.h"

#define delay()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 100000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)


o_rgb_led test_rgb_color;

extern const uint8_t	led_button_map[NUM_BUTTONS];
extern const uint8_t	led_rotary_map[NUM_LED_ROTARIES];
extern const uint8_t 	ledstring_map[NUM_CHANNELS+1];

extern o_led_cont		led_cont;
extern o_button 		button[NUM_BUTTONS];
extern o_rotary 		rotary[NUM_ROTARIES];
extern o_monoLed 		monoLed[NUM_MONO_LED];

extern enum 			UI_Modes ui_mode;

extern SystemCalibrations *system_calibrations;

//Private:
uint8_t check_enter_led_adjust_buttons(uint32_t button_state);
uint8_t check_exit_led_adjust_buttons(uint32_t button_state);
void init_led_color_adjust(void);



uint8_t check_enter_led_adjust_mode(void)
{
	if (check_enter_led_adjust_buttons(PRESSED))
	{
		return 1;
	}
	else return 0;
}

uint8_t exit_is_armed;

uint8_t check_exit_led_adjust_mode(void)
{

	if ((exit_is_armed==1) && (check_exit_led_adjust_buttons(MED_PRESSED)))
		return 1;

	if (check_exit_led_adjust_buttons(RELEASED))
		exit_is_armed = 1;

	return 0;
}

void enter_led_adjust_mode(void)
{
	exit_is_armed = 0;

	while (!check_enter_led_adjust_buttons(PRESSED)); //wait until buttons are released

	init_led_color_adjust();

	//Set test_rgb_color set to white for testing the button lights
	test_rgb_color.c_red		=1023;
	test_rgb_color.c_green		=1023;
	test_rgb_color.c_blue		=1023;
	test_rgb_color.brightness	=1.0;

}

void exit_led_adjust_mode(void)
{
	uint8_t i;

	//Turn all buttons to green
	test_rgb_color.c_red = 0;
	test_rgb_color.c_green = 1023;
	test_rgb_color.c_blue = 0;
	for (i=0;i<NUM_BUTTONS;i++)	set_pwm_led(led_button_map[i],&test_rgb_color);

	//Save to flash
	save_flash_params();

	//Turn all buttons to white
	test_rgb_color.c_red = 1023;
	test_rgb_color.c_green = 1023;
	test_rgb_color.c_blue = 1023;
	for (i=0;i<NUM_BUTTONS;i++)	set_pwm_led(led_button_map[i],&test_rgb_color);

	ui_mode=PLAY;
}

uint8_t check_enter_led_adjust_buttons(uint32_t button_state)
{
	return ((button[butm_LFOVCA_BUTTON].hwswitch.pressed >= button_state)
		&& (button[butm_LFOMODE_BUTTON].hwswitch.pressed >= button_state)
		) ? 1 : 0;
		 
}

uint8_t check_exit_led_adjust_buttons(uint32_t button_state)
{
	return ((rotary[rotm_WAVETABLE].hwswitch.pressed >= button_state)
		) ? 1 : 0;
		 
}

void set_default_led_color_adjust(void)
{
	uint8_t i;

	for (i=0;i<NUM_LED_IDs;i++)
	{
		system_calibrations->rgbled_adjustments[i][c_RED] 		= 1.0;
		system_calibrations->rgbled_adjustments[i][c_GREEN] 	= 1.0;
		system_calibrations->rgbled_adjustments[i][c_BLUE] 		= 1.0;
	}
	system_calibrations->rgbled_adjustments[ledm_LFOMODE_BUTTON][c_RED] = 0.35;
	system_calibrations->rgbled_adjustments[ledm_LFOVCA_BUTTON][c_RED] 	= 0.35;
}

void init_led_color_adjust(void)
{
	uint8_t i;

	ui_mode = RGB_COLOR_ADJUST;

	//Turn all buttons to solid red
	test_rgb_color.c_red		=1023;
	test_rgb_color.c_green		=0;
	test_rgb_color.c_blue		=0;
	test_rgb_color.brightness	=F_MAX_BRIGHTNESS;
	for (i=0;i<NUM_BUTTONS;i++)		set_pwm_led(led_button_map[i],&test_rgb_color);

	delay();

	//Turn all buttons to solid green
	test_rgb_color.c_red		=0;
	test_rgb_color.c_green		=1023;
	test_rgb_color.c_blue		=0;
	for (i=0;i<NUM_BUTTONS;i++)		set_pwm_led(led_button_map[i],&test_rgb_color);

	delay();

	//Turn all buttons to solid blue
	test_rgb_color.c_red		=0;
	test_rgb_color.c_green		=0;
	test_rgb_color.c_blue		=1023;
	for (i=0;i<NUM_BUTTONS;i++)		set_pwm_led(led_button_map[i],&test_rgb_color);

	delay();

	//Turn all buttons to solid white
	test_rgb_color.c_red		=1023;
	test_rgb_color.c_green		=1023;
	test_rgb_color.c_blue		=1023;
	for (i=0;i<NUM_BUTTONS;i++)		set_pwm_led(led_button_map[i],&test_rgb_color);	

	//Turn all encoders to solid white
	test_rgb_color.c_red		=1023;
	test_rgb_color.c_green		=1023;
	test_rgb_color.c_blue		=1023;
	for (i=0;i<NUM_LED_ROTARIES-1;i++)	set_pwm_led(led_rotary_map[i],&test_rgb_color);



	//Turn sliders A B C on, and D E F off
	set_led_state(mledm_SLIDER_A, LED_OFF);
	set_led_state(mledm_SLIDER_B, LED_OFF);
	set_led_state(mledm_SLIDER_C, LED_OFF);
	set_led_state(mledm_SLIDER_D, LED_OFF);
	set_led_state(mledm_SLIDER_E, LED_OFF);
	set_led_state(mledm_SLIDER_F, LED_OFF);


	//Set three encoders to R G B
	set_pwm_led_direct(ledm_DEPTH_ENC, 1023, 0, 0);
	set_pwm_led_direct(ledm_LATITUDE_ENC,  0, 1023, 0);
	set_pwm_led_direct(ledm_LONGITUDE_ENC,  0, 0, 1023);


	//Turn the string of RGB LEDs off
	test_rgb_color.c_red		=0;
	test_rgb_color.c_green		=0;
	test_rgb_color.c_blue		=0;
	set_pwm_led(ledstring_map[0],&test_rgb_color);
	set_pwm_led(ledstring_map[1],&test_rgb_color);
	set_pwm_led(ledstring_map[2],&test_rgb_color);
	set_pwm_led(ledstring_map[3],&test_rgb_color);
	set_pwm_led(ledstring_map[4],&test_rgb_color);
	set_pwm_led(ledstring_map[5],&test_rgb_color);

}

static const uint8_t NO_LED_SELECTED=0xFF;
void process_led_color_adjust_mode(void)
{
	uint8_t i;
	static uint8_t selected_led_id = 0xFF;
	float old_r, old_g, old_b;
	uint8_t red_rotary = pec_DEPTH;
	uint8_t green_rotary = pec_LATITUDE;
	uint8_t blue_rotary = pec_LONGITUDE;

	//Read all buttons in order and pick the first one detected as pressed down
	for (i=0;i<NUM_BUTTONS;i++)
	{
		if (button[i].hwswitch.pressed)
		{
			selected_led_id = led_button_map[i];
			break;
		}
	}

	//Read all LED rotaries in order and pick the first one detected as pressed down
	for (i=0;i<NUM_LED_ROTARIES;i++)
	{
		if (rotary[i].hwswitch.pressed)
		{
			selected_led_id = led_rotary_map[i];
			break;
		}
	}

	//Disable adjustments if center rotary is pressed
	if (rotary[rotm_WAVETABLE].hwswitch.pressed)
	{
		selected_led_id = NO_LED_SELECTED;
	}


	if (selected_led_id != NO_LED_SELECTED)
	{
		// Check if adjustment values changed. If so, force an update of the LED
		old_r = system_calibrations->rgbled_adjustments[selected_led_id][c_RED];
		old_g = system_calibrations->rgbled_adjustments[selected_led_id][c_GREEN];
		old_b = system_calibrations->rgbled_adjustments[selected_led_id][c_BLUE];

		//Read rotaries
		system_calibrations->rgbled_adjustments[selected_led_id][c_RED] 	+= pop_encoder_q(red_rotary)	*(1.0/32.0);
		system_calibrations->rgbled_adjustments[selected_led_id][c_GREEN] 	+= pop_encoder_q(green_rotary)	*(1.0/32.0);
		system_calibrations->rgbled_adjustments[selected_led_id][c_BLUE] 	+= pop_encoder_q(blue_rotary)	*(1.0/32.0);

		//Range check: 0.01 to 3.0
		if (system_calibrations->rgbled_adjustments[selected_led_id][c_RED] < 0.01) 	system_calibrations->rgbled_adjustments[selected_led_id][c_RED] = 0.01;
		if (system_calibrations->rgbled_adjustments[selected_led_id][c_GREEN] < 0.01) 	system_calibrations->rgbled_adjustments[selected_led_id][c_GREEN] = 0.01;
		if (system_calibrations->rgbled_adjustments[selected_led_id][c_BLUE] < 0.01) 	system_calibrations->rgbled_adjustments[selected_led_id][c_BLUE] = 0.01;
		if (system_calibrations->rgbled_adjustments[selected_led_id][c_RED] > 3.0) 		system_calibrations->rgbled_adjustments[selected_led_id][c_RED] = 3.0;
		if (system_calibrations->rgbled_adjustments[selected_led_id][c_GREEN] > 3.0) 	system_calibrations->rgbled_adjustments[selected_led_id][c_GREEN] = 3.0;
		if (system_calibrations->rgbled_adjustments[selected_led_id][c_BLUE] > 3.0) 	system_calibrations->rgbled_adjustments[selected_led_id][c_BLUE] = 3.0;


		if (	old_r != system_calibrations->rgbled_adjustments[selected_led_id][c_RED]
			|| 	old_g != system_calibrations->rgbled_adjustments[selected_led_id][c_GREEN]
			||	old_b != system_calibrations->rgbled_adjustments[selected_led_id][c_BLUE])
		{
			// set_pwm_led_direct(selected_led_id, 
			// 					1023 * system_calibrations->rgbled_adjustments[selected_led_id][c_RED],
			//  					1023 * system_calibrations->rgbled_adjustments[selected_led_id][c_GREEN],
			// 					1023 * system_calibrations->rgbled_adjustments[selected_led_id][c_BLUE]
			//  );
			//Force update of LED by toggling brightness by an insignificant amount
			test_rgb_color.brightness = 0.999;
			set_pwm_led(selected_led_id, &test_rgb_color);
			test_rgb_color.brightness = 1.0;
			set_pwm_led(selected_led_id, &test_rgb_color);
		}
	}
}
