/*
 * led_cont.h
 *
 * Author: Hugo Paris (hugoplho@gmail.com), Dan Green (danngreen1@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to                               the following conditions:
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
#include "hardware_controls.h"

#define FLASHTIME			5000
#define DIM_WT  			0.0f //0.37f
#define DIM_LFO 			0.0f //0.65f
#define F_MAX_BRIGHTNESS 	1.0f

enum ongoingDisplays{ 
	ONGOING_DISPLAY_NONE 		,
	ONGOING_DISPLAY_OSC_PARAM_LOCK ,
	ONGOING_DISPLAY_WT_POS_LOCK	,
	ONGOING_DISPLAY_FINETUNE 	,
	ONGOING_DISPLAY_TRANSPOSE 	,
	ONGOING_DISPLAY_SCALE  		,
	ONGOING_DISPLAY_OCTAVE 	 	,
	ONGOING_DISPLAY_LFO_MODE	,
	ONGOING_DISPLAY_LFO_TOVCA	,
	ONGOING_DISPLAY_PRESET		,
	ONGOING_DISPLAY_RECORD		,
	ONGOING_DISPLAY_SPHERE_SAVE	,
	ONGOING_DISPLAY_GLOBRIGHT	,
	ONGOING_DISPLAY_FX			,


	NUM_ONGOING_DISPLAYS
};


//Todo: 
// These flags are very application-specific, if we separated them into a different structure,
// the o_led_cont structure could be re-used for alt firmwares.
// (since it references only hardware and not feature functionality)

typedef struct o_led_cont{

	// OBJECTS
	o_rgb_led 	button[NUM_BUTTONS];
	o_rgb_led 	encoder[NUM_LED_ROTARIES];
	o_rgb_led 	outring[NUM_LED_OUTRING];
	o_rgb_led 	inring[NUM_LED_INRING];
	o_rgb_led 	array[NUM_LED_ARRAY];

	// GLOBAL
	uint8_t		flash_state;

	// FLAGS / TIMERS (WT)
	enum ongoingDisplays	ongoing_display	;
	uint8_t 	disable_transpose_disp;
	uint8_t 	disable_octave_disp;
	uint16_t	ongoing_timeout	;

	// uint16_t	trigdisp[NUM_CHANNELS];

	// FLAGS (LFO)
	uint8_t	 	ongoing_lfoshape[NUM_CHANNELS];
	uint16_t	lfoshape_timeout[NUM_CHANNELS];

	// EXTERNAL CLOCK INPUT
	uint8_t 	waiting_for_clockin;
	float 		clockin_wait_progress;

} o_led_cont;
	
static inline uint8_t lock_flash_state(void) {
	return (((HAL_GetTick()/TICKS_PER_MS) & 0xFF) > 200);
}

void 		init_led_cont(void);
void 		start_led_display(void);
void 		init_led_cont_ongoing_display(void);
void 		update_display_at_encoder_press(void);
void	 	update_led_flash(void);

void 		update_button_leds(void);
void 		update_encoder_leds(void);
void 		update_array_leds(void);
void		update_clockin_led(void);
void		update_audioin_led(void);
void 		calculate_lfocv_led(void);
void 		calculate_lfo_leds(void);
void 		update_mono_leds(void);

void 		get_wt_color(uint8_t wt_num, o_rgb_led *rgb);

void 		update_LED_rings(void);
void 		calculate_led_ring(void);
void 		turn_outring_off(void);
void 		display_wtpos_inring(void);
void 		flash_wt_lock(void);
void 		display_wt_pos(void);
void 		display_transpose(void);
void 		display_finetune(void);
void 		display_octave(void);
void 		display_preset(void);
void 		display_sphere_save(void);
void		display_fx(void);
void 		display_firmware_version(void);

// ------- display timers ------
void 		update_ongoing_display_timers(void);
void 		start_ongoing_display_osc_param_lock(void);
void 		start_ongoing_display_wt_pos_lock(void);
void 		start_ongoing_display_octave(void);
void 		start_ongoing_display_scale(void);
void 		start_ongoing_display_finetune(void);
void 		start_ongoing_display_transpose(void);
void 		start_ongoing_display_lfo_tovca(void);
void 		start_ongoing_display_lfo_mode(void);
void 		start_ongoing_display_preset(void);
void 		start_ongoing_display_sphere_save(void);
void 		start_ongoing_display_globright(void);
void 		start_ongoing_display_fx(void);
void 		stop_all_displays(void);


void 		calculate_led_inring(void);
void 		set_led_group(uint8_t led_group, uint16_t c_red, uint16_t c_green, uint16_t c_blue);
uint16_t 	return_display_timer(void);
