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
#include "codec_sai.h"

extern const float BROWSE_TABLE[ NUM_WAVEFORMS_IN_SPHERE ][ NUM_WT_DIMENSIONS ];
extern o_wt_osc wt_osc;
extern o_params params;
extern enum UI_Modes ui_mode;

uint32_t play_export_sample_i;
uint16_t play_export_offset;
//uint8_t play_export_wt_repeat_i;
uint8_t play_export_browse_i;
extern o_spherebuf spherebuf;

float dc_offsets[NUM_WAVEFORMS_IN_SPHERE];

void play_export_audio_block(int32_t *src, int32_t *dst);
void calc_sphere_dc_offsets(float *avg);
void stop_play_export_sphere(void);
int16_t *get_play_export_ptr(void);
void increment_play_export(uint16_t samples);


void start_play_export_sphere(void)
{
	calc_sphere_dc_offsets(dc_offsets);

	ui_mode = WTPLAYEXPORT;
	set_audio_callback(&play_export_audio_block);

	play_export_sample_i = 0;
	start_ongoing_display_sphere_play_export();
}

void stop_play_export_sphere(void)
{
	stop_all_displays();
	ui_mode = WTMONITORING;
	set_audio_callback(&process_audio_block_codec);
}

void play_export_audio_block(int32_t *src, int32_t *dst)
{
	int16_t i;
	float smpl;
	float level;
	int16_t *ptr;

	level = 0.65*256.0;

	ptr = get_play_export_ptr();

	//assert(MONO_BUFSZ*WT_TABLELEN/MONO_BUFSZ == WT_TABLELEN); //WT_TABLELEN must be an integer multiple of MONO_BUFSZ
	for (i=0; i < MONO_BUFSZ; i++) 
	{
		smpl = (float)(*ptr++) - dc_offsets[play_export_browse_i];

		*dst++ = (int32_t)(smpl*level);
		*dst++ = (int32_t)(smpl*level);
		UNUSED(*src++);
		UNUSED(*src++);
	}

	increment_play_export(MONO_BUFSZ);
}

int16_t *get_play_export_ptr(void)
{
	//uint8_t browse_i;
	uint8_t x, y, z;
	
	//repeat last waveform to provide a stretch buffer
	//browse_i = (play_export_browse_i < NUM_WAVEFORMS_IN_SPHERE) ? play_export_browse_i : (NUM_WAVEFORMS_IN_SPHERE-1);
	x = BROWSE_TABLE[play_export_browse_i][0];
	y = BROWSE_TABLE[play_export_browse_i][1];
	z = BROWSE_TABLE[play_export_browse_i][2];

	return &(spherebuf.data[x][y][z].wave[play_export_offset]);
}

void increment_play_export(uint16_t samples)
{
	uint8_t raw_browse_i;

	play_export_sample_i += samples;
	play_export_offset = play_export_sample_i % WT_TABLELEN;
//	play_export_wt_repeat_i = (play_export_sample_i/WT_TABLELEN) % REPEAT_EACH_WT;
	raw_browse_i = play_export_sample_i/(WT_TABLELEN*REPEAT_EACH_WT);
	play_export_browse_i = (raw_browse_i < NUM_WAVEFORMS_IN_SPHERE) ? raw_browse_i : (NUM_WAVEFORMS_IN_SPHERE-1);

	if (play_export_sample_i>=NUM_SAMPLES_IN_RECBUF_SMOOTHED)
		stop_play_export_sphere();
}

void calc_sphere_dc_offsets(float *avg)
{
	uint8_t browse_i;
	uint8_t x, y, z;
	uint16_t i;
	int32_t tot;

	for (browse_i=0; browse_i<NUM_WAVEFORMS_IN_SPHERE; browse_i++) 
	{
		x = BROWSE_TABLE[browse_i][0];
		y = BROWSE_TABLE[browse_i][1];
		z = BROWSE_TABLE[browse_i][2];

		tot = 0;
		for (i=0; i<WT_TABLELEN; i++)
			tot += spherebuf.data[x][y][z].wave[i];

		avg[browse_i] = (float)tot/F_WT_TABLELEN;
	}
}


void animate_play_export_ledring(uint8_t slot_i, o_rgb_led *rgb)
{
	if (slot_i < (play_export_sample_i/ (WT_TABLELEN*REPEAT_EACH_WT*NUM_WAVEFORMS_IN_SPHERE/NUM_LED_OUTRING) ))
		set_rgb_color(rgb, ledc_GREEN);
	else
		set_rgb_color(rgb, ledc_OFF);
}
