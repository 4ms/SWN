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
#include "led_cont.h"
#include "params_update.h"
#include "math_util.h"
#include "sphere_flash_io.h"
#include "sphere.h"
#include "params_wt_browse.h"
#include "oscillator.h"
#include "led_colors.h"
#include "ui_modes.h"

extern const float BROWSE_TABLE[ NUM_WAVEFORMS_IN_SPHERE ][ NUM_WT_DIMENSIONS ];
extern o_wt_osc wt_osc;
extern o_params params;
extern enum UI_Modes ui_mode;

uint16_t play_export_offset;
uint16_t play_export_wt_repeat_i;
uint16_t play_export_browse_i;
o_waveform tmp_waveform;

void start_play_export_sphere(void)
{
	play_export_offset = 0;
	play_export_wt_repeat_i = 0;
	play_export_browse_i = 0;

	load_extflash_wavetable(params.wt_bank[0], &tmp_waveform, 0, 0, 0);
	//todo: wait until loaded
	//todo: new function to load wave data only from flash 
	for (uint16_t i=0; i<WT_TABLELEN; i++)
		wt_osc.mc[wt_osc.buffer_sel[0]][0][i] = tmp_waveform.wave[i];

	ui_mode = WTPLAYEXPORT;
	start_ongoing_display_sphere_play_export();
}

uint16_t get_play_export_offset(void) {
	return play_export_offset;
}

void increment_play_export(uint16_t samples)
{
	play_export_offset += samples;

	if (play_export_offset>=WT_TABLELEN) {
		play_export_wt_repeat_i++;
		play_export_offset = 0;
	}

	if (play_export_wt_repeat_i==REPEAT_EACH_WT)
	{
		play_export_wt_repeat_i = 0;
		play_export_browse_i++;

		if (play_export_browse_i<NUM_WAVEFORMS_IN_SPHERE)
		{
			uint16_t x = BROWSE_TABLE[play_export_browse_i][0];
			uint16_t y = BROWSE_TABLE[play_export_browse_i][1];
			uint16_t z = BROWSE_TABLE[play_export_browse_i][2];
			load_extflash_wavetable(params.wt_bank[0], &tmp_waveform, x, y, z);
			//wait until loaded
			//todo: new function to load wave data only from flash 
			for (uint16_t i=0; i<WT_TABLELEN; i++)
				wt_osc.mc[wt_osc.buffer_sel[0]][0][i] = tmp_waveform.wave[i];
		}
		else 
			stop_play_export_sphere();
	}
}

void stop_play_export_sphere(void)
{
	stop_all_displays();
	ui_mode = WTEDITING;
}


void animate_play_export_ledring(uint8_t slot_i, o_rgb_led *rgb)
{
	float amount_played = play_export_wt_repeat_i + play_export_browse_i*REPEAT_EACH_WT;

	if (slot_i<(amount_played/(REPEAT_EACH_WT*NUM_WAVEFORMS_IN_SPHERE/NUM_LED_OUTRING)))
		set_rgb_color(rgb, ledc_GREEN);
	else
		set_rgb_color(rgb, ledc_OFF);
}
