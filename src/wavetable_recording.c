/*
 * wavetable_recording.c
 *
 * Author: Hugo Paris (hugoplho@gmail.com), Dan Green (danngreen1@gmail.com)
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

#include "wavetable_recording.h"
#include "wavetable_editing.h"
#include "globals.h"
#include "sphere_flash_io.h"
#include "ui_modes.h"
#include "params_update.h" 
#include "oscillator.h"
#include "led_cont.h"
#include "led_colors.h"
#include "math_util.h"

extern o_params params;
extern o_led_cont 		led_cont;
extern enum 			UI_Modes ui_mode;
extern uint32_t 		colorPalette[NUM_LED_COLORS][NUM_PALETTE_COLORS];

SRAM1DATA o_recbuf 		recbuf;

void record_audio_buffer(int32_t audio_in_sample)
{
	if (ui_mode == WTREC_WAIT)
	{
		if ((audio_in_sample > REC_THRESHOLD) || (audio_in_sample < -REC_THRESHOLD))
			ui_mode = WTRECORDING;
	}

	if (ui_mode == WTRECORDING) {
		if (recbuf.wh < NUM_SAMPLES_IN_RECBUF_SMOOTHED)	{
			recbuf.data[recbuf.wh] = _CLAMP_I32(-audio_in_sample/256, INT16_MIN, INT16_MAX);
			recbuf.wh++;
		}
		else {
			enter_wtrender_recbuff();
		}
	}
}

void init_wtrec(void){	
	recbuf.wh = 0;
}

uint32_t get_recbuf_wh(void){
	return recbuf.wh;
}



// -------------------------
// 			DISPLAY
// -------------------------

// LEDs are updated show the recorfing buffer fill status
// Recording buffer is divided in 18 sections, one per LED
// full ring is buffer 100% full
// empty ring is buffer 0% full
void display_wt_recbuff_fill_outring(void){

	float	led_len;
	uint8_t i_ll;
	float 	r_ll;

	// uint8_t	end_led;
	uint8_t i;

	led_len 	= (float)(NUM_LED_OUTRING-1) * (float)(recbuf.wh) / (float)(NUM_SAMPLES_IN_RECBUF_SMOOTHED);
	i_ll 		= (uint8_t)(led_len);
	r_ll 		= led_len - i_ll;

	turn_outring_off();

	// light up leds corresponding to buffer sections that are 100% full
	for (i =0; i <= i_ll; i++){
		set_rgb_color(&led_cont.outring[i], ledc_RED);
	}

	// light up last led for current recording fill status
	// brightness is proportional to recording buffer section fill status
	set_rgb_color_brightness(&led_cont.outring[i], ledc_RED, r_ll);
	// led_cont.outring[i_ll + 1].c_red 		= colorPalette[c_RED]  [ledc_RED];
	// led_cont.outring[i_ll + 1].c_green 		= colorPalette[c_GREEN][ledc_RED];
	// led_cont.outring[i_ll + 1].c_blue 		= colorPalette[c_BLUE] [ledc_RED];
	// led_cont.outring[i_ll + 1].brightness 	= F_MAX_BRIGHTNESS * r_ll;
}

void display_wt_rec_wait(void)
{
	turn_outring_off();

	if ((HAL_GetTick()/TICKS_PER_MS) & 0x80)
		set_rgb_color(&led_cont.outring[0], ledc_RED);
	else
		set_rgb_color(&led_cont.outring[0], ledc_OFF);

}




