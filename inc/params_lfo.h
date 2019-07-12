/*
 * params_lfo.h
 *
 * Authors: Dan Green (danngreen1@gmail.com), Hugo Paris (hugoplho@gmail.com)
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

#include "globals.h"

#define LFO_PHASE_TABLELEN				24
#define GLO_CLK							6
#define REF_CLK							7

#define WT_REC_RAMP_AMPLITUDE			254
#define WT_REC_TRIGGER_AMPLITUDE		254

// SCALING
#define F_SCALING_FINE_LFO_SPEED		0.1
#define F_SCALING_LFO_GAIN				0.05
#define F_SCALING_FINE_LFO_GAIN			0.005
#define SCALING_LFO_PHASE				32
#define F_SCALING_FINE_LFO_PHASE		(1.0/12.0) //0.0078125 // 128th note

// DISPLAY
#define LFO_TOVCA_TIMER_LIMIT			1200
#define LFO_TOVCA_FLASH_PERIOD			200

#define LFO_MODE_TIMER_LIMIT			1200
#define LFO_MODE_FLASH_PERIOD			400

#define LFOBANK_DISPLAYTMR				700


#define LFO_INIT_PERIOD					8000
#define LFO_INIT_GAIN					0.625


enum lfoModes{ 

	lfot_SHAPE,
	lfot_GATE,
	lfot_TRIG,

	NUM_LFO_MODES
};


typedef struct o_lfos
{
	//Parameters
	float 			divmult_id			[NUM_CHANNELS + 2];
	float 			phase_id 			[NUM_CHANNELS];
	int8_t 			shape 				[NUM_CHANNELS];
	float 			gain 				[NUM_CHANNELS];
	uint8_t			locked 				[NUM_CHANNELS];
	enum lfoModes 	mode 				[NUM_CHANNELS];			//Shape/Trig/Gate
	uint8_t 		to_vca 				[NUM_CHANNELS];
	uint8_t 		muted		 		[NUM_CHANNELS];

	uint8_t			use_ext_clock;
	uint8_t			phase_switch;

	//Resultants
	float			divmult				[NUM_CHANNELS + 2];
	float			period 				[NUM_CHANNELS + 2];
	float 			inc					[NUM_CHANNELS + 2];
	float 			phase 				[NUM_CHANNELS];
	uint8_t			audio_mode			[NUM_CHANNELS];
	float 			divmult_id_global_locked[NUM_CHANNELS];

	//Running outputs
	float			cycle_pos			[NUM_CHANNELS + 2];		//0..1 cycle_pos is position within its cycle
	float 			wt_pos 				[NUM_CHANNELS]; 		//0..1 wt_pos = cycle_pos +/- phase
	uint8_t			div_cnt				[NUM_CHANNELS];			//number of base clocks passed, when dividing

	float 			preload 			[NUM_CHANNELS];
	uint32_t 		envout_pwm 			[NUM_CHANNELS];
	float 			out_lpf 			[NUM_CHANNELS];

	//Stashed values (for switching in and out of key/note mode)
	float			divmult_id_buf		[NUM_CHANNELS];
	int8_t 			phase_id_buf		[NUM_CHANNELS];
	float 			fine_phase_buf		[NUM_CHANNELS];
	int8_t 			shape_buf 			[NUM_CHANNELS];
	float 			gain_buf			[NUM_CHANNELS];
	enum lfoModes	mode_buf 			[NUM_CHANNELS];
	uint8_t 		to_vca_buf 			[NUM_CHANNELS];


	//Probably can be made into statics
	uint8_t 		trig_armed 			[NUM_CHANNELS];
	uint8_t  		trigout 			[NUM_CHANNELS];

} o_lfos;


void update_lfos(void);
void init_lfos(void);
void use_internal_lfo_base(void);
void clear_lfo_locks(void);
void init_lfo_object(o_lfos *t_lfo);
void init_lfos_shape(void);
void init_lfo_speed(void);
void update_lfo_sample(void);
void update_lfo_params(void);
void apply_lfo_reset(void);
void read_LFO_speed_gain(void);
void update_lfo_gain(int16_t turn);
void read_lfo_speed(int16_t turn);
void read_LFO_phase(void);
void read_LFO_shape(void);
void wrap_lfo_fine_phase(uint8_t chan, float fine_inc);
float calc_lfo_phase(float phase_id);
void read_lfo_cv(void);
void init_lfo_to_vc_mode(void);
void cache_uncache_all_lfo_to_vca(enum CacheUncache cache_uncache);
void set_all_lfo_to_vca(uint8_t newstate);
void cache_uncache_lfomode(uint8_t chan, enum CacheUncache cache_uncache);
void cache_uncache_all_lfomodes(enum CacheUncache cache_uncache);
void sync_LFO_phase(void);
void set_all_lfo_mode(enum lfoModes mode);
