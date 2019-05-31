/*
 * wavetable_recording.c
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

#include "wavetable_recording.h"
#include "wavetable_editing.h"
#include "sphere_flash_io.h"
#include "ui_modes.h"
#include "params_update.h" 
#include "params_wt_browse.h" 
#include "oscillator.h"
#include "led_cont.h"
#include "led_colors.h"
#include "math_util.h"
#include "audio_util.h"
#include "codec_sai.h"

extern o_params params;
extern o_wt_osc			wt_osc;
extern o_led_cont 		led_cont;
extern enum 			UI_Modes ui_mode;
extern uint32_t 		colorPalette[NUM_LED_COLORS][NUM_PALETTE_COLORS];
extern SRAM1DATA o_spherebuf spherebuf;

SRAM1DATA o_recbuf 		recbuf;

void record_audio_buffer(int32_t *src, int32_t *dst);

void record_audio_buffer(int32_t *src, int32_t *dst)
{
	uint16_t i_sample;
	int32_t audio_in_sample;
	uint8_t enter_wtrender_when_done = 0;

	for (i_sample = 0; i_sample < MONO_BUFSZ; i_sample++)
	{
		audio_in_sample = convert_s24_to_s32(*src++);								
		UNUSED(*src++);  // ignore right channel input (not connected in hardware)
	
		if (ui_mode == WTREC_WAIT)
		{
			if ((audio_in_sample > REC_THRESHOLD) || (audio_in_sample < -REC_THRESHOLD))
				ui_mode = WTRECORDING;
		}

		if (ui_mode == WTRECORDING) {
			if (recbuf.wh < recbuf.end_pos)
			{
				recbuf.data[recbuf.wh] = _CLAMP_I32(-audio_in_sample/256, INT16_MIN, INT16_MAX);
				recbuf.wh++;
			}
			else {
				enter_wtrender_when_done = 1;
			}
		}

		*dst++ = (audio_in_sample);
		*dst++ = (audio_in_sample);
	}

	if (enter_wtrender_when_done)
	{
		init_wt_edit_settings();
		set_audio_callback(&process_audio_block_codec);
		enter_wtrendering();
	}
}


void enter_wtrecording(uint8_t record_window)
{
	uint8_t wave_browse_i;
	if (record_window==RECORD_ALL)
		wave_browse_i = RECORD_ALL;
	else
		wave_browse_i = get_browse_index(wt_osc.m0[0][0], wt_osc.m0[1][0], wt_osc.m0[2][0]);

	set_pitch_params_to_ttone();

	ui_mode = WTMONITORING; HAL_Delay(10); 	// The delay allows envout_pwm interrupt to run, which zeroes the LFOs so we don't begin recording with the first trigger already high. 
											// This only works if enter_wtrecording() is not called from an interrupt with >= urgency than envout_pwm
//	set_params_for_editing();
	spherebuf.data_source = SPHERESRC_RECBUFF;
	init_wtrecording(wave_browse_i);
}

void init_wtrecording(uint8_t wave_browse_i)
{
	if (wave_browse_i>=NUM_WAVEFORMS_IN_SPHERE) {
		recbuf.start_pos = 0;
		recbuf.end_pos = NUM_SAMPLES_IN_RECBUF_SMOOTHED;
	}
	else {
		recbuf.start_pos = spherebuf.start_pos[wave_browse_i];
		if ((wave_browse_i+1) == NUM_WAVEFORMS_IN_SPHERE)
			recbuf.end_pos = NUM_SAMPLES_IN_RECBUF_SMOOTHED;
		else
			recbuf.end_pos = spherebuf.start_pos[wave_browse_i+1];
	}

	recbuf.wh = recbuf.start_pos;
	ui_mode = WTREC_WAIT;
	set_audio_callback(&record_audio_buffer);
}

uint32_t get_recbuf_wh(void) {
	return recbuf.wh;
}

void display_wt_recbuff_fill_outring(void)
{
	float	led_len;
	uint8_t i_ll;
	float 	r_ll;
	uint8_t i;

	led_len 	= (float)(NUM_LED_OUTRING-1) * (float)(recbuf.wh) / (float)(NUM_SAMPLES_IN_RECBUF_SMOOTHED);
	i_ll 		= (uint8_t)(led_len);
	r_ll 		= led_len - i_ll;

	turn_outring_off();

	for (i =0; i <= i_ll; i++)
		set_rgb_color(&led_cont.outring[i], ledc_RED);

	set_rgb_color_brightness(&led_cont.outring[i], ledc_RED, r_ll);
}

void display_wt_rec_wait(void)
{
	turn_outring_off();

	if ((HAL_GetTick()/TICKS_PER_MS) & 0x80)
		set_rgb_color(&led_cont.outring[0], ledc_RED);
	else
		set_rgb_color(&led_cont.outring[0], ledc_OFF);

}
