/*
 * wavetable_play_export.c - Exporting a sphere by playing it out the audio
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


#include "wavetable_play_export.h"
#include "wavetable_recording.h"
#include "led_cont.h"
#include "params_update.h"
#include "math_util.h"
#include "sphere_flash_io.h"
#include "sphere.h"
#include "params_wt_browse.h"
#include "oscillator.h"
#include "led_colors.h"
#include "ui_modes.h"
#include "drivers/flashram_spidma.h"

extern const float BROWSE_TABLE[ NUM_WAVEFORMS_IN_SPHERE ][ NUM_WT_DIMENSIONS ];
extern o_wt_osc wt_osc;
extern o_params params;
extern enum UI_Modes ui_mode;

uint32_t play_export_sample_i;
uint16_t play_export_offset;
uint8_t play_export_wt_repeat_i;
uint8_t play_export_browse_i;
extern o_spherebuf spherebuf;

void start_play_export_sphere(void)
{
	// uint8_t x, y, z;
	// uint32_t repeat_i, sample_i;
	// uint8_t browse_i;
	// uint32_t dst=0;

	// ui_mode = WTPLAYEXPORT_LOAD;

	// for (browse_i=0; browse_i<NUM_WAVEFORMS_IN_SPHERE; browse_i++)
	// {
	// 	x = BROWSE_TABLE[browse_i][0];
	// 	y = BROWSE_TABLE[browse_i][1];
	// 	z = BROWSE_TABLE[browse_i][2];

	// 	for (repeat_i=0; repeat_i<REPEAT_EACH_WT; repeat_i++) {
	// 		for (sample_i=0; sample_i<WT_TABLELEN; sample_i++) {
	// 			recbuf.data[dst++] = spherebuf.data[x][y][z].wave[sample_i];
	// 		}
	// 	}
	// }
	// for (;dst<NUM_SAMPLES_IN_RECBUF_SMOOTHED;dst++)
	// 	recbuf.data[dst++] = spherebuf.data[x][y][z].wave[(sample_i++) % WT_TABLELEN];

	ui_mode = WTPLAYEXPORT;
	play_export_sample_i = 0;
	start_ongoing_display_sphere_play_export();
}

int16_t *get_play_export_ptr(void)
{
	uint8_t browse_i;
	uint8_t x, y, z;
	
	//repeat last waveform to provide a stretch buffer
	browse_i = (play_export_browse_i < NUM_WAVEFORMS_IN_SPHERE) ? play_export_browse_i : (NUM_WAVEFORMS_IN_SPHERE-1);
	x = BROWSE_TABLE[browse_i][0];
	y = BROWSE_TABLE[browse_i][1];
	z = BROWSE_TABLE[browse_i][2];

	return &(spherebuf.data[x][y][z].wave[play_export_offset]);
}

void increment_play_export(uint16_t samples)
{
	play_export_sample_i += samples;
	play_export_offset = play_export_sample_i % WT_TABLELEN;
	play_export_wt_repeat_i = (play_export_sample_i/WT_TABLELEN) % REPEAT_EACH_WT;
	play_export_browse_i = play_export_sample_i/(WT_TABLELEN*REPEAT_EACH_WT);

	if (play_export_sample_i>=NUM_SAMPLES_IN_RECBUF_SMOOTHED)
		stop_play_export_sphere();
}

void stop_play_export_sphere(void) {
	stop_all_displays();
	ui_mode = WTMONITORING;
}

void animate_play_export_ledring(uint8_t slot_i, o_rgb_led *rgb)
{
	// if (slot_i < (play_export_sample_i/ (NUM_SAMPLES_IN_RECBUF_SMOOTHED/NUM_LED_OUTRING) ))
	if (slot_i < (play_export_sample_i/ (WT_TABLELEN*REPEAT_EACH_WT*NUM_WAVEFORMS_IN_SPHERE/NUM_LED_OUTRING) ))
		set_rgb_color(rgb, ledc_GREEN);
	else
		set_rgb_color(rgb, ledc_OFF);
}
