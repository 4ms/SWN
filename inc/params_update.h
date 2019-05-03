/*
 * params_update.h
 *
 * Author: Hugo Paris (hugoplho@gmail.com)
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
#include "globals.h"
#include "hardware_controls.h"
#include "sphere.h"
#include "lfo_wavetable_bank.h"

#define F_MIN_FREQ			16.35 						// C-1 is 8.175 , C0 is 16.35. C1 is 32.7
#define F_MAX_FREQ			(F_SAMPLERATE*3 - 36000.0) //96300.0 					
#define INIT_OCT			3	// C4 ~261Hz
#define TTONE_OCT			2	// C2 ~87Hz
#define TTONE_TRANSPOSE 	5	// F2 if oct at TTONE_OCT
#define MAX_OCT				14	// MAX_OCT = flp2((uint32_t)(F_MAX_FREQ / F_MIN_FREQ))/2;
#define NUM_NOTES 			12
#define NUM_CHORDS			26
#define NUM_WTSEL_SPREADS	6
#define NUM_SCALES			2
#define NOTES_PER_SCALE		NUM_OUTRING_LEDS +1

// #define XP_GAIN 			1

//WAVETABLE
#define NUM_WT_XP 						0 		//0=Disabled. It was 8 when it was last enabled 	// number of wavetable expanders
#define XFADE_INC						0.025 // 1/4000 ~ 0.1s


// ...
#define FREQNUDGE_LPF					0.995
#define FREDIMCV_LPF					0.99
#define NUDGEPOT_MIN_CHANGE 			0.0029304029304 // f_nudge_range / max pot val * num_pts_to_ignore <=> 1.2/4095*10 
#define DEP_POT_MIN_CHANGE 				80 
#define ENABLE_M 						1
#define ROTARY_BUTTON_HOLD 				100000
#define CHANNEL_LEVEL_MIN_LPF 			0.75
#define ADC_POT_LPF						0.99
#define ADC_JACK_LPF					0.90
#define POSPOT_MIN_CHANGE 				100
#define POSPOT_LPF 						0.95
#define DIMCV_LPF 						0.5
#define QLOCK_FLASH_SPEED 				1500
#define BUTTON_BUTTON_LFOLOCK_HOLD_TIME 50000
#define BUTTON_BUTTON_LONG_HOLD_TIME 	80000
#define BANK_LPF 						0.99


// DISPLAY TIMERS: specified in # of tick
#define OSC_PARAM_LOCK_TIMER_LIMIT		700
#define WT_POS_LOCK_TIMER_LIMIT			PRESET_TIMER_LIMIT
#define FINETUNE_TIMER_LIMIT			700
#define SCALE_TIMER_LIMIT				700
#define TRANSPOSE_TIMER_LIMIT			5000
#define OCTAVE_TIMER_LIMIT				700
#define PRESET_TIMER_LIMIT				2000
#define SPHERE_SAVE_TIMER_LIMIT			2000
#define FX_TIMER_LIMIT					500
#define GLOBRIGHT_TIMER_LIMIT			100

// SCALING
#define F_SCALING_FINETUNE_NOWRAP		1.003 //0.0025
#define F_SCALING_FINETUNE_WRAP			1.00057779 /*100th root of 12th root of 2*/
#define F_SCALING_TRANSPOSE				1.05946309436

#define NUM_DIM_STEPS					50.0
#define NUM_DIM_FINE_STEPS				250.0
#define F_SCALING_NAVIGATE				(WT_DIM_SIZE / NUM_DIM_STEPS) // 0.06
#define F_SCALING_FINE_NAVIGATE			(WT_DIM_SIZE / NUM_DIM_FINE_STEPS) // 0.012

#define F_SCALING_DISPERSION_LPF		0.2
#define F_SCALING_DISPERSION			0.05
#define F_SCALING_FINE_DISPERSION		0.01

#define F_SCALING_RANDOM				0.0015
#define RANDOM_UPDATE_TIME				7

#define F_SCALING_MAX_VCACV_GAIN		1.0

// TUNING
#define MAX_FINETUNE_NOWRAP 			9
#define MIN_FINETUNE_NOWRAP 			-9
#define MAX_TRANSPOSE_NOWRAP 			9
#define MIN_TRANSPOSE_NOWRAP 			-9
#define F_MAX_TRANSPOSITION 			1.68179283051 // 2^( 9/12)
#define F_MIN_TRANSPOSITION 			0.5946035575  // 2^(-9/12)


#define MAX_FINETUNE_WRAP 				(2160)
#define MIN_FINETUNE_WRAP 				(-2160)

#define MAX_TRANSPOSE_WRAP				125
#define MIN_TRANSPOSE_WRAP				-126

#define QTZ_CHANGE_LOCKOUT_PERIOD		50

// LED BRIGHTNESS
#define F_SCALING_NAVIGATE_GLOBAL_BRIGHTNESS 	0.01
#define F_MIN_GLOBAL_BRIGHTNESS					0.01
#define F_DEFAULT_GLOBAL_BRIGHTNESS				0.8

enum LPFActions 
{
	NO_LPF_ACTION,
	CLEAR_LPF
};


enum armFlags
{ 
	armf_NOTE_ON,
	armf_LFOTOVCA,
	armf_LFOMODE,
	armf_KEYMODE,
	NUM_ARM_FLAGS
};

typedef struct o_calc_params{
	uint8_t		wtsel 					[NUM_CHANNELS]		;
	float		wt_pos 					[3][NUM_CHANNELS]	;
	int32_t 	transpose				[NUM_CHANNELS]		;		//Combination of transpose encoder + chord
	float 		transposition 			[NUM_CHANNELS]		;		//Calcuated pitch change based on transpose amount
	float 		tuning 					[NUM_CHANNELS]		;
	float 		pitch 					[NUM_CHANNELS]		;
	float		level  					[NUM_CHANNELS]		; 		// FixMe:  try uint16_t to match slider resolution. pay attention to env
	float 		voct 					[NUM_CHANNELS]		;
	float		qtz_freq				[NUM_CHANNELS]		;

	// FLAGS
	uint8_t 	already_handled_button 	[NUM_CHANNELS]		; 
	uint8_t 	button_safe_release		[2]					;
	uint8_t		keymode_pressed								;
	enum armFlags armed[NUM_ARM_FLAGS]	[NUM_CHANNELS]		;
	uint8_t		lock_change_staged		[NUM_CHANNELS]		;
} o_calc_params;


typedef struct o_params{

	// SHPERES
	int8_t		wtsel_cv									;
	int8_t		wtsel_enc				[NUM_CHANNELS]		;
	uint8_t 	wtsel_spread_cv								;
	uint8_t 	wt_bank 				[NUM_CHANNELS]		;	//Sphere slot number (physical location in FLASH)

	// SPHERE NAVIGATION / BROWSE
	float		wt_nav_enc				[3][NUM_CHANNELS]	;
	float		wt_nav_cv				[3]					;
	float 		dispersion_enc								;
	float 		dispersion_cv								;
	int8_t		disppatt_enc 								;
	int8_t		disppatt_cv									;
	float		wt_browse_step_pos_enc	[NUM_CHANNELS]		;
	float		wt_browse_step_pos_cv						;
	int8_t		wtsel_spread_enc		[NUM_CHANNELS]		;

	// OSCILLATORS
	int8_t		oct 					[NUM_CHANNELS]		;
	int16_t 	finetune 				[NUM_CHANNELS]		;
	int32_t 	transpose_enc			[NUM_CHANNELS]		;
	float		transpose_cv								;
	int32_t 	spread_enc				[NUM_CHANNELS]		;
	uint8_t 	spread_cv 									;
	int8_t 		indiv_scale 			[NUM_CHANNELS]		;
	int8_t 		indiv_scale_buf 		[NUM_CHANNELS]		;		//Stashed scale value when in note/key mode

	
	// KEYS 
	uint8_t 	note_on 				[NUM_CHANNELS]		;		//Mute state
	uint8_t		note_on_buf				[NUM_CHANNELS]		;		//Stashed mute state for when in note/key mode
	uint8_t 	new_key 				[NUM_CHANNELS]		;
	enum MuteNoteKeyStates	key_sw		[NUM_CHANNELS]		;
	uint8_t		qtz_note_changed 		[NUM_CHANNELS]		;


	// NOISE LINE 
	float		random 					[NUM_CHANNELS]		;
	uint8_t		noise_on									;

	enum VoctVcaStates 	voct_switch_state[NUM_CHANNELS]		;
	uint8_t 	osc_param_lock 			[NUM_CHANNELS]		;	
	uint8_t 	wt_pos_lock 			[NUM_CHANNELS]		;	//For v1.0 this is always the same as osc_param_lock
	uint8_t		wtsel_lock				[NUM_CHANNELS]		;	//For v1.0 this is always the same as osc_param_lock


	uint8_t		PADDING					[NUM_CHANNELS*32]   ; //padding for future features: 8 x 32b x 6chan, or 32 x 8b x 6chan
} o_params;



void 		init_params(void);
void 		init_calc_params(void);
void 		init_pitch_params();
void 		init_param_object(o_params *t_params);

void 		set_pitch_params_to_ttone(void);
void 		set_pitch_param_to_ttone(uint8_t chan);

void 		check_reset_navigation(void);
void 		cache_uncache_pitch_params(enum CacheUncache cache_uncache);
void 		init_ksw_params(uint8_t chan);

void 		read_noteon(uint8_t i);
void 		read_level(uint8_t chan);
float 		read_vca_cv(uint8_t chan);

void 		read_lfoto_vca_vco(uint8_t i);
void 		read_lfomode(uint8_t i);

void 		apply_keymode(uint8_t chan, enum MuteNoteKeyStates new_keymode);
void 		read_all_keymodes(void);

void 		cache_uncache_keys_params_and_lfos(uint8_t chan, enum CacheUncache cache_uncache);

void 		update_lfomode(uint8_t i);
void 		update_pitch(uint8_t chan);
void 		update_wt_head_pos_inc(uint8_t chan);
void 		update_noise(uint8_t chan);

void 		read_freq(void);

// ##########################
//			OSC PARAMS
// ##########################

// --------- LOCK ---------
void 		update_osc_param_lock(void);
void 		cache_uncache_locks(enum CacheUncache cache_uncache);
void 		unlock_all(void);
void 		toggle_lock(uint8_t chan);

// --------- OCTAVE ---------
void 		update_oct(int16_t tmp);
void		reset_octaves(void);

// --------- SCALE ---------
void 		update_scale(int16_t tmp);

// --------- TUNE ---------
void 		update_finetune(int16_t tmp);
void 		retune_oscillators(void);
void 		resync_audio_osc(uint8_t channels);
void 		spread_finetune(int16_t tmp);
void 		compute_tuning (uint8_t chan);

// --------- TRANSPOSE ---------
void 		update_transpose(int16_t tmp);
void 		update_transpose_cv(void);
void 		update_spread(int16_t tmp);
void 		update_spread_cv(void);
void 		combine_transpose_spread(void);
void 		trim_transpose(int32_t *transpose);
void 		compute_transpositions(void);
float 		compute_transposition(int32_t transpose);
void 		reset_notes(void);


// ##########################
//			WT PARAMS
// ##########################

// -------- 
void 		update_wt(void);

// ##### READ ENCODERS ######
void 		read_nav_encoder(uint8_t dim);
void		read_browse_encoder(void);
void 		read_load_save_encoder(void);
// ##########################

void 		update_wt_interp(void);
void 		interp_wt(uint8_t chan, int16_t *p_waveform[8]);
uint8_t 	is_loaded_from_flash(uint8_t bank, uint8_t x, uint8_t y, uint8_t z);
void 		update_oscillators(void);
void 		start_osc_updates(void);
void 		start_osc_interp_updates(void);


// --------- WT SEL ---------
// void 		read_wtsel_lock(uint8_t i);
void 		read_wtsel(int8_t wtsel);
void 		read_wtsel_cv(void);
void 		read_wtsel_spread(void);
void 		read_wtsel_spread_cv(void);
void 		update_wtsel(void);
void 		set_wtsel(uint8_t selection);
void 		update_wt_bank(void);
int8_t 		calc_static_wtsel(uint8_t chan);
void 		fix_wtsel_wtbank_offset(void);


// --------- WT POS ---------
// void 		update_wt_pos_lock(void);

void 		update_wt_nav(uint8_t wt_dim, float wt_pos_increment);
void 		update_wt_nav_cv(uint8_t wt_dim);
void 		calc_wt_pos(uint8_t chan);
void 		set_wtpos_to_int(uint8_t chan);

void 		update_wt_pos_interp_params(uint8_t chan, uint8_t wt_dim);
void 		update_all_wt_pos_interp_params(void);
void 		req_wt_interp_update (uint8_t chan);
void 		force_wt_interp_update (uint8_t chan);
void 		force_all_wt_interp_update (void);

// -------- WT DISPERSION ----------
void 		update_wt_disp(uint8_t clear_lpf);


// ##########################
//			MEMORY
// ##########################
void		read_clear_settings(void);

void 		read_switches(void);
void 		read_wt_bankCV(void);

void 		update_num_sphere_filled(uint8_t num_user_filled);

uint32_t 	abs(int32_t v);
