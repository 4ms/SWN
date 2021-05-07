/*
 * params_update.c - Parameters
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


#include "params_update.h"
#include "params_changes.h"
#include "led_cont.h"
#include "gpio_pins.h"
#include "exp_1voct_10_41V.h"
#include "flash_params.h"
#include "math.h"
#include "sphere_flash_io.h"
#include "lfo_wavetable_bank.h"
#include "quantz_scales.h"
#include "led_cont.h"
#include "adc_interface.h"
#include "analog_conditioning.h"
#include "UI_conditioning.h"
#include "timekeeper.h"
#include "system_settings.h"
#include "math_util.h"
#include "params_wt_browse.h"
#include "quantz_scales.h"
#include "wavetable_recording.h"
#include "wavetable_editing.h"
#include "ui_modes.h"
#include "key_combos.h"
#include "wavetable_saveload.h"
#include "wavetable_effects.h"
#include "preset_manager.h"
#include "preset_manager_UI.h"
#include "params_pitch.h"
#include "params_lfo_clk.h"
#include "params_lfo_period.h"
#include "params_lfo.h"
#include "flash_params.h"
#include "ui_modes.h"
#include "oscillator.h"
#include "wavetable_saveload_UI.h"
#include "drivers/flashram_spidma.h"
#include "wavetable_play_export.h"
#include "preset_manager_selbus.h"

extern o_wt_osc wt_osc;
extern enum UI_Modes ui_mode;
extern o_lfos lfos;
extern SystemCalibrations *system_calibrations;


extern enum UI_Modes ui_mode;
extern	o_systemSettings	system_settings;
extern	o_analog	analog[NUM_ANALOG_ELEMENTS];
extern	o_macro_states macro_states;
extern	o_rotary	rotary[NUM_ROTARIES];
extern	o_button	button[NUM_BUTTONS];
extern	o_switch	hwSwitch[NUM_SWITCHES];
extern	o_monoLed 	monoLed[NUM_MONO_LED];
extern	o_led_cont	led_cont;

extern uint8_t audio_in_gate;

extern const uint8_t ALL_CHANNEL_MASK;

extern	SRAM1DATA o_spherebuf spherebuf;
extern const int16_t TTONE[WT_TABLELEN];

const int8_t		CHORD_LIST[NUM_CHORDS][NUM_CHANNELS] =
{
	// DEFAULT
	{ 0 	, 0 	, 0 	, 0 	, 0 	, 0		}  , 	// NONE

	// FIFTH
	{ 0 	, 0 	, 7 	, 7 	, 0 	, 0		}  , 	// FIFTH
	{ 0 	, 0 	, 7 	, 7 	, -5 	, -5	}  , 	// FIFTH w/ inversion
	{ 0 	, 12	, 7	, 19	, -12	, -5	}  , 	// FIFTH w/ oct

	// MAJOR
	{ 0 	, 0 	, 4 	, 4 	, 7 	, 7		}  , 	// M3rd
	{ -8	, -5	, 0 	, 4 	, 7 	, 7	}  ,	// M3rd w inv
	{ -12	, -5	, 0 	, 4 	, 7 	, 12	}  ,	// M3rd w/ oct

	// MINOR
	{ 0 	, 0 	, 3 	, 3 	, 7 	, 7		}  , 	// m3rd
	{ -9	, -5	, 0 	, 3 	, 7 	, 7	}  ,	// m3rd w inv
	{ -12	, -5	, 0 	, 3 	, 7 	, 12	}  ,	// m3rd w/ oct

	// MAJOR EXT chords (6th, 7th 9th, 11th)
	{ 0 	, 0 	, 4 	, 4 	, 7 	, 9		}  , 	// M6th
	{ -12 	, 0 	, 4 	, 7 	, 9 	, 9		}  , 	// M6th w/oct
	{ -12 	, -3	, 0 	, 4 	, 4 	, 9		}  , 	// M6th w/ inv
	{ 0 	, 0 	, 4 	, 4 	, 7 	, 11	}  , 	// M7th
	{ -12 	, 0 	, 4 	, 7 	, 11 	, 11	}  , 	// M7th w/oct
	{ -12 	, -1	, 0 	, 4 	, 7 	, 11	}  , 	// M7th w/ inv
	{ 0 	, 0		, 4 	, 7 	, 11	, 14	}  , 	// M9th
	{ 0 	, 4		, 7 	, 11 	, 14	, 18	}  , 	// M11th

	// MINOR EXT chords (6th, 7th 9th, 11th)
	{ 0 	, 0 	, 3 	, 3 	, 7 	, 9		}  , 	// m6th
	{ -12 	, 0 	, 3 	, 7 	, 9 	, 9		}  , 	// m6th w/oct
	{ -12 	, -3	, 0 	, 3 	, 3 	, 9		}  , 	// m6th w/ inv
	{ 0 	, 0 	, 3 	, 3 	, 7 	, 10	}  , 	// m7th
	{ -12 	, 0 	, 3 	, 7 	, 10 	, 10	}  , 	// m7th w/oct
	{ -12 	, -2	, 0 	, 3 	, 7 	, 10	}  , 	// m7th w/ inv
	{ 0 	, 0		, 3 	, 7 	, 10	, 14	}  , 	// m9th
	{ 0 	, 3		, 7 	, 10 	, 14	, 17	}   	// m11th
};

const float DISP_PATTERN[NUM_DISPPAT][6][3] =	{
	{
		{1,	1,      1},
		{1,  	1,		1},
		{1,  	1,   	1},
		{1,		1,  	1},
		{1,  	1,		1},
		{1,  	1,   	1},
	},
	{
		{1,	0,      0},
		{0,  	1,		0},
		{0,  	0,  	1},
		{-1,	0,  	0},
		{0,  	-1,	0},
		{0,  	0,   	-1},
	},
	{
		{0.50,	1.30,	0.80},
		{1.20,	-.30,	0.60},
		{0.80,	1.30,	-1.2},
		{0.50,	0.20,	1.50},
		{-2.0,	0,		0.30},
		{1.0,	0.20,	0.30},
	},
	{
		{-1.2,	0.30,	0.60},
		{2.00,	1.30,	1.20},
		{0.50,	0.30,	0.80},
		{2.00,	2.00,	0.30},
		{0.10,	0.20,	1.20},
		{0.50,	0.20,	-1.5},
	},
	{
		{-2,	0.30,	0.60},
		{-0.20, -1,	1.2},
		{2,	2,		.80},
		{-2,	-1.50,	0.30},
		{1,	2,		1.2},
		{0.30,	-2,	-1.50},
	},
	{
		{1,	-3,	2},
		{-2,  	2,		.40},
		{-1.50,	.40,   1.2},
		{0.1,	2,  	-2},
		{1.90,	-2,	-.80},
		{0.20,	.90,   -1}
	}
};

extern const float exp_1voct_10_41V[4096];

o_params			params;
o_calc_params		calc_params;

uint32_t num_spheres_filled;
o_waveform	waveform[NUM_CHANNELS][2][2][2];


//Todo: replace with init_param_object(&params), plus a few other differences
void init_params(void){

	uint8_t  i;
	uint16_t j;

	// global & display
	params.dispersion_enc = 0;
	params.disppatt_enc = 1;
	params.noise_on = 0;
	params.spread_cv = 0;

	for (i=0; i<NUM_CHANNELS; i++){
		params.key_sw[i] = ksw_MUTE;

		params.note_on[i] = 1;
		calc_params.level[i] = 4093;
		calc_params.adjusting_pan_state[i] = pan_INACTIVE;
		calc_params.cached_level[i] = 0.f;

		// individual locks
		params.osc_param_lock[i] = 0;
		params.wtsel_lock[i] = 0;
		params.wt_pos_lock[i] = 0;

		params.wtsel_spread_enc[i] = 0;
		calc_params.wtsel[i] = 1;
		params.wtsel_enc[i] = 0;
		params.wt_bank[i] = 0;

		params.wt_browse_step_pos_enc[i] = 0;
		params.wt_nav_enc[0][i] = 0;
		params.wt_nav_enc[1][i] = 0;
		params.wt_nav_enc[2][i] = 0;

		params.spread_enc[i] = 0;
		calc_params.transpose[i] = 0;
		params.transpose_enc[i] = 0;

		params.pan[i] = default_pan(i);

		calc_params.gate_in_is_sustaining[i]	= 0;

		calc_wt_pos(i);
		for (j=0; j<3; j++)
			update_wt_pos_interp_params(i, j);

		// flags
		calc_params.already_handled_button[i] = 0;

		for (j=0; j<NUM_ARM_FLAGS; j++)
			calc_params.armed[j][i] = 0;

		params.random[i] = 1.0;
	}

	calc_params.already_handled_button[butm_LFOVCA_BUTTON] = 0;
	calc_params.already_handled_button[butm_LFOMODE_BUTTON] = 0;
	for (i=0; i<(MAX_TOTAL_SPHERES/8); i++)
		params.enabled_spheres[i]=0xFF;

}


void init_pitch_params(void)
{
	uint8_t chan;
	for (chan = 0; chan<NUM_CHANNELS; chan++)
	{
		calc_params.tuning[chan]				= 1;
		calc_params.transposition[chan]		= 1;

		params.finetune[chan]					= 0;
		params.transpose_enc[chan]				= 0;

		params.oct[chan]						= INIT_OCT;
		params.indiv_scale[chan]				= 0;
		params.indiv_scale_buf[chan]			= params.indiv_scale[chan];

		params.qtz_note_changed[chan]			= 0;
	}
}

void init_param_object(o_params *t_params){
	uint8_t chan;

	// global & display
	t_params->dispersion_enc			= 0;
	t_params->disppatt_enc				= 1;
	t_params->noise_on					= 0;
	t_params->spread_cv				= 0;

	// individual params
	for (chan=0; chan<NUM_CHANNELS; chan++)
	{
		t_params->key_sw[chan]					= ksw_MUTE;
		t_params->note_on[chan]				= 1;

		// individual locks
		t_params->osc_param_lock[chan]			= 0;
		t_params->wtsel_lock[chan]				= 0;

		t_params->wtsel_enc[chan]				= 0;
		t_params->wtsel_spread_enc[chan]		= 0;
		t_params->wt_bank[chan]					= 0;

		t_params->wt_browse_step_pos_enc[chan]	= 0.0;

		t_params->osc_param_lock[chan]			= 0;

		t_params->wt_browse_step_pos_enc[chan]	= 0;
		t_params->wt_pos_lock[chan]				= 0;
		t_params->wt_nav_enc[0][chan]			= 0;
		t_params->wt_nav_enc[1][chan]			= 0;
		t_params->wt_nav_enc[2][chan]			= 0;

		t_params->spread_enc[chan]				= 0;
		t_params->transpose_enc[chan]			= 0;

		t_params->random[chan]					= 1.0;

		t_params->finetune[chan]				= 0;
		t_params->transpose_enc[chan]			= 0;

		t_params->oct[chan]					= INIT_OCT;
		t_params->indiv_scale[chan]			= 0;
		t_params->indiv_scale_buf[chan]		= 0;

		t_params->pan[chan]						= default_pan(chan);
		t_params->qtz_note_changed[chan]		= 0;

	}

	for (uint8_t i=0; i<(MAX_TOTAL_SPHERES/8); i++)
		t_params->enabled_spheres[i]=0xFF;

}

void init_calc_params(void)
{
	uint8_t chan, j;
	for (chan=0; chan<NUM_CHANNELS; chan++)
	{

		calc_params.prev_qtz_note[chan] = 0xFF;
		calc_params.prev_qtz_oct[chan] = 0xFF;
		calc_params.gate_in_is_sustaining[chan] = 0;

		for (j = 0; j < NUM_ARM_FLAGS; j++)
			calc_params.armed[j][chan] = 0;

		calc_params.adjusting_pan_state[chan] = pan_INACTIVE;
		calc_params.cached_level[chan] = 0.f;
		calc_params.already_handled_button[chan] = 0;

	}
	calc_params.keymode_pressed = 0;
	calc_params.already_handled_button[butm_LFOVCA_BUTTON] = 0;
	calc_params.already_handled_button[butm_LFOMODE_BUTTON] = 0;
	calc_params.button_safe_release[0] = 0;
	calc_params.button_safe_release[1] = 0;
}

void set_pitch_params_to_ttone(void) {
	for (uint8_t chan=0; chan<NUM_CHANNELS; chan++)
	{
		params.finetune[chan]					= 0;
		compute_tuning(chan);

		params.transpose_enc[chan]				= TTONE_TRANSPOSE;
		params.spread_enc[chan]				= 0;

		params.oct[chan]						= TTONE_OCT;
		params.indiv_scale[chan]				= 0;
		params.indiv_scale_buf[chan]			= params.indiv_scale[chan];

		params.qtz_note_changed[chan]			= 0;

	}

	combine_transpose_spread();
	compute_transpositions();

	for (uint8_t chan=0; chan<NUM_CHANNELS; chan++)
		update_pitch(chan);
}

void check_reset_navigation(void)
{
	uint8_t i;

	if (key_combo_reset_navigation())
	{
		exit_preset_manager();
		stop_all_displays();

		params.dispersion_enc				= 0;
		params.disppatt_enc				= 1;

		for (i=0; i<NUM_CHANNELS; i++)
		{
			if (!params.wt_pos_lock[i])
			{
				params.wt_browse_step_pos_enc[i]		= 0;
				reset_wbrowse_morph(i);
				params.wt_nav_enc[0][i]					= 0;
				params.wt_nav_enc[1][i]					= 0;
				params.wt_nav_enc[2][i]					= 0;
			}
		}
	}

	if (key_combo_reset_sphere_sel())
	{
		exit_preset_manager();
		stop_all_displays();

		for (i=0; i<NUM_CHANNELS; i++)
		{
			if (!params.wtsel_lock[i])
			{
				params.wtsel_spread_enc[i]				= 0;
				calc_params.wtsel[i]					= 1;
				params.wtsel_enc[i]						= 0;
				params.wt_bank[i]						= 0;
				req_wt_interp_update(i);
			}
		}
	}
}

void cache_uncache_nav_params(enum CacheUncache cache_uncache)
{
	uint8_t i;
	static float cached_dispersion_enc;
	static int8_t cached_wtsel_enc[NUM_CHANNELS];
	static int8_t cached_wtsel_spread_enc[NUM_CHANNELS];
	static float cached_wt_nav_enc[3][NUM_CHANNELS];
	static float cached_wt_browse_step_pos_enc[NUM_CHANNELS];
	static uint8_t cached_wt_bank[NUM_CHANNELS];

	if (cache_uncache==CACHE)
	{
		cached_dispersion_enc = params.dispersion_enc;
		for (i=0;i<NUM_CHANNELS;i++)
		{
			cached_wtsel_enc[i] = params.wtsel_enc[i];
			cached_wtsel_spread_enc[i] = params.wtsel_spread_enc[i];
			cached_wt_bank[i] = params.wt_bank[i];
			cached_wt_nav_enc[0][i] = params.wt_nav_enc[0][i];
			cached_wt_nav_enc[1][i] = params.wt_nav_enc[1][i];
			cached_wt_nav_enc[2][i] = params.wt_nav_enc[2][i];
			cached_wt_browse_step_pos_enc[i] = params.wt_browse_step_pos_enc[i];
		}
	}
	else
	{
		params.dispersion_enc = cached_dispersion_enc;
		for (i=0;i<NUM_CHANNELS;i++)
		{
			if (params.wtsel_lock[i])
			{
				params.wtsel_enc[i] = cached_wtsel_enc[i];
				params.wtsel_spread_enc[i] = cached_wtsel_spread_enc[i];
				params.wt_bank[i] = cached_wt_bank[i];
			}
			if (params.wt_pos_lock[i])
			{
				params.wt_nav_enc[0][i] = cached_wt_nav_enc[0][i];
				params.wt_nav_enc[1][i] = cached_wt_nav_enc[1][i];
				params.wt_nav_enc[2][i] = cached_wt_nav_enc[2][i];
				params.wt_browse_step_pos_enc[i] = cached_wt_browse_step_pos_enc[i];
				reset_wbrowse_morph(i);
			}
		}
	}
}


void cache_uncache_pitch_params(enum CacheUncache cache_uncache)
{
	static int16_t		finetune				[NUM_CHANNELS];
	static float		tuning					[NUM_CHANNELS];
	static int32_t		transpose				[NUM_CHANNELS];
	static float		transposition			[NUM_CHANNELS];
	static int8_t		oct					[NUM_CHANNELS];
	static uint8_t		indiv_scale			[NUM_CHANNELS];
	static uint8_t		indiv_scale_buf			[NUM_CHANNELS];

	for (uint8_t chan=0; chan<NUM_CHANNELS; chan++){
		switch (cache_uncache)
		{
			case CACHE:
				finetune[chan]					= params.finetune[chan];
				tuning[chan]					= calc_params.tuning[chan];
				transpose[chan]				= params.transpose_enc[chan];
				transposition[chan]			= calc_params.transposition[chan];
				oct[chan]						= params.oct[chan];
				indiv_scale[chan]				= params.indiv_scale[chan];
				indiv_scale_buf[chan]			= params.indiv_scale_buf[chan];
				break;

			case UNCACHE:
				params.finetune[chan]			= finetune[chan];
				calc_params.tuning[chan]		= tuning[chan];
				params.transpose_enc[chan]		= transpose[chan];
				calc_params.transposition[chan] = transposition[chan];
				params.oct[chan]				= oct[chan];
				params.indiv_scale[chan]		= indiv_scale[chan];
				params.indiv_scale_buf[chan]  	= indiv_scale_buf[chan];
				break;
		}
	}
}

static uint8_t new_key_armed[NUM_CHANNELS] = {0};

void read_ext_trigs(void)
{
	uint8_t chan;
	uint8_t chans_in_cvgate_mode=0;

	static uint8_t last_trig_level[NUM_CHANNELS+1]={0};
	uint8_t trig_level[NUM_CHANNELS+1]={0};

	trig_level[0] = (params.key_sw[0]==ksw_KEYS_EXT_TRIG) ? audio_in_gate : (analog[WTSEL_CV].raw_val > 2048);
	trig_level[1] = analog[DISP_CV].raw_val > 2048;
	trig_level[2] = analog[DEPTH_CV].raw_val > 2048;
	trig_level[3] = analog[DISPPAT_CV].raw_val > 2048;
	trig_level[4] = analog[LATITUDE_CV].raw_val > 2048;
	trig_level[5] = analog[WTSEL_SPREAD_CV].raw_val > 2048;
	trig_level[6] = analog[CHORD_CV].raw_val > 2048;

	for (chan=0; chan<NUM_CHANNELS; chan++)
	{
		uint8_t is_sustaining = 0;
		if (params.key_sw[chan]==ksw_KEYS_EXT_TRIG || params.key_sw[chan]==ksw_KEYS_EXT_TRIG_SUSTAIN)
		{
			chans_in_cvgate_mode++;

			if (analog_jack_plugged(A_VOCT+chan) || button_pressed(chan)) {
				if ((trig_level[chan] || button_pressed(chan)) && !last_trig_level[chan])
				{
					lfos.cycle_pos[chan] = 0.f;
					params.note_on[chan] = 1;
					params.new_key[chan] = 1;
					new_key_armed[chan] = 0;
				}
				if (params.key_sw[chan]==ksw_KEYS_EXT_TRIG_SUSTAIN && (trig_level[chan] || button_pressed(chan)))
					is_sustaining = 1;
			}
		}
		last_trig_level[chan] = (trig_level[chan] || button_pressed(chan));
		calc_params.gate_in_is_sustaining[chan] = is_sustaining;
	}

	//Transpose CV jack must be patched for gate to work on Chord CV
	if (chans_in_cvgate_mode && analog_jack_plugged(TRANSPOSE_CV))
	{
		for (chan=0; chan<NUM_CHANNELS; chan++)
		{
			if (trig_level[6] && !last_trig_level[6])
			{
				lfos.cycle_pos[chan] = 0.f;
				params.note_on[chan] = 1;
				params.new_key[chan] = 1;
				new_key_armed[chan]	= 0;
			}
			if (params.key_sw[chan]==ksw_KEYS_EXT_TRIG_SUSTAIN && trig_level[6])
				calc_params.gate_in_is_sustaining[chan] = 1;
		}
	}
	last_trig_level[6] = trig_level[6];
}
enum SelBusActions {SELBUS_NO_ACTION, SELBUS_TOGGLE_RECALL, SELBUS_TOGGLE_SAVE, SELBUS_STORE_SETTINGS};
void read_selbus_buttons(void)
{
	static enum SelBusActions selbus_action_armed = SELBUS_NO_ACTION;

	if (key_combo_show_selbus_allows()) {
		calc_params.already_handled_button[1] = 1;
		calc_params.already_handled_button[butm_LFOVCA_BUTTON] = 1;
		calc_params.already_handled_button[butm_LFOMODE_BUTTON] = 1;
		start_ongoing_display_selbus();
	}
	if (key_combo_toggle_selbus_recall()) {
		if (selbus_action_armed != SELBUS_TOGGLE_RECALL)
			sel_bus_toggle_recall_allow();
		selbus_action_armed = SELBUS_TOGGLE_RECALL;
	}
	else if (system_settings.selbus_can_save == SELBUS_SAVE_ENABLED && key_combo_disable_selbus_save()) {
		if (selbus_action_armed != SELBUS_TOGGLE_SAVE)
			sel_bus_toggle_save_allow();
		selbus_action_armed = SELBUS_TOGGLE_SAVE;
	}
	else if (system_settings.selbus_can_save == SELBUS_SAVE_DISABLED && key_combo_enable_selbus_save()) {
		if (selbus_action_armed != SELBUS_TOGGLE_SAVE)
			sel_bus_toggle_save_allow();
		selbus_action_armed = SELBUS_TOGGLE_SAVE;
	}

	if (!key_combo_toggle_selbus_recall() && (selbus_action_armed == SELBUS_TOGGLE_RECALL)) {
		start_ongoing_display_selbus();
		selbus_action_armed = SELBUS_STORE_SETTINGS;
	}
	else if (!key_combo_disable_selbus_save() && (selbus_action_armed == SELBUS_TOGGLE_SAVE)) {
		start_ongoing_display_selbus();
		selbus_action_armed = SELBUS_STORE_SETTINGS;
	}
	if (!key_combo_show_selbus_allows()) {
		//stop_all_displays();
		if (selbus_action_armed == SELBUS_STORE_SETTINGS) {
			save_flash_params();
			selbus_action_armed = SELBUS_NO_ACTION;
		}
	}

}

void read_noteon(uint8_t i)
{
	if (ui_mode == PLAY)
	{
		// Button mode: Mute
		if (params.key_sw[i] == ksw_MUTE)
		{
			if (!calc_params.already_handled_button[i] && button_pressed(i))
			{
				calc_params.armed[armf_NOTE_ON][i] = 1;
			}

			else if (button_released(i))
			{
				if (calc_params.armed[armf_NOTE_ON][i])
				{
					calc_params.armed[armf_NOTE_ON][i] = 0;
					if (!calc_params.already_handled_button[i])
					{
						if (calc_params.lock_change_staged[i]==1) {
							toggle_lock(i);
							calc_params.lock_change_staged[i] = 2;
						}
						else
							params.note_on[i] = 1 - params.note_on[i];
					}
					else
						calc_params.lock_change_staged[i] = 0;
				}
				calc_params.already_handled_button[i] = 0;
			}
		}

		// Button Mode: Note/Keyboard/CVGate/CVGateSus
		// ... and auto-notes at qtz crossings
		else
		{
			if (button_pressed(i))
			{
				if (params.key_sw[i]==ksw_NOTE) {
					lfos.cycle_pos[i] = 5.0/F_MAX_LFO_TABLELEN;  // read 5th element of LFO table to avoid silence at start
				}

				if (!new_key_armed[i]) {
					new_key_armed[i] = 1;
					params.new_key[i] = 1;
					params.note_on[i] = 1;
					if (params.key_sw[i]==ksw_KEYS_EXT_TRIG)
						lfos.cycle_pos[i] = 0;
				}
				// if (params.key_sw[i]==ksw_KEYS_EXT_TRIG_SUSTAIN) {
				// 	calc_params.gate_in_is_sustaining[i] = 1;
				// 	lfos.cycle_pos[i] = 0;
				// }
			}
			else //button_released(i)
			{
				// NOTE AUTO EG trig
				if( (params.key_sw[i] == ksw_NOTE) && !params.note_on[i] && params.qtz_note_changed[i]==1 ){
					lfos.cycle_pos[i] = 0;
					params.note_on[i] = 1;
				}
				else
				{
					new_key_armed[i] = 0;
					if (params.key_sw[i]==ksw_KEYS) {
						params.note_on[i] = 0;
						lfos.cycle_pos[i] = 0;
					}
					else if (params.key_sw[i]==ksw_NOTE){
						params.new_key[i] = 0;
					}

					if (!calc_params.already_handled_button[i])
					{
						if (calc_params.lock_change_staged[i]==1) {
							toggle_lock(i);
							calc_params.lock_change_staged[i] = 2;
						}
					}
					else
						calc_params.lock_change_staged[i] = 0;

					calc_params.already_handled_button[i] = 0;
				}
			}
		}
	}

	// other UI modes
	else{
		params.note_on[i] = 1;
	}
}

float default_pan(uint8_t chan)
{
	return (chan&1) ? 0.f : 1.f;
}

void read_level_and_pan(uint8_t chan)
{
	static float last_slider_val[NUM_CHANNELS] = {0};
	float slider_val = analog[A_SLIDER + chan].lpf_val;
	float slider_motion = fabs(analog[A_SLIDER + chan].lpf_val - last_slider_val[chan]);
	float level = 0.f;

	if (slider_motion > 20)
	{
		last_slider_val[chan] = analog[A_SLIDER + chan].lpf_val;
		if (button_pressed(chan))
		{
			calc_params.already_handled_button[chan] = 1;
			calc_params.adjusting_pan_state[chan] = pan_PANNING;
			params.pan[chan] = slider_val / 4095.f;
		}
	}

	switch (calc_params.adjusting_pan_state[chan]) {
		case (pan_PANNING):
			level = calc_params.cached_level[chan];
			if (!button_pressed(chan)) {
				calc_params.adjusting_pan_state[chan] = pan_CACHED_LEVEL;
			}
			break;

		case (pan_CACHED_LEVEL):
			level = calc_params.cached_level[chan];
			if (fabs(slider_val - calc_params.cached_level[chan]) < 10) {
				calc_params.adjusting_pan_state[chan] = pan_INACTIVE;
			}
			break;

		case (pan_INACTIVE):
			level = slider_val - 20.f;
			calc_params.cached_level[chan] = level;
			break;
	}

	//Adjust level by CV and Mute button
	if (!params.note_on[chan])
		calc_params.level[chan] = 0.f;

	else {
		if (lfos.to_vca[chan])	level *= lfos.out_lpf[chan];

		level *= read_vca_cv(chan);

		calc_params.level[chan] = _CLAMP_F(level, 0.f, 4095.f);
	}

}

void set_master_gain(void)
{
	static uint32_t last_slider_a=0;
	float new_gain;
	int16_t slider_motion = analog[A_SLIDER].lpf_val - last_slider_a;

	if (abs(slider_motion)>10)
	{
		last_slider_a = analog[A_SLIDER].lpf_val;

		if (rotary_med_pressed(rotm_LFOSPEED) && rotary_med_pressed(rotm_OCT)){
			new_gain = _SCALE_U2F(analog[A_SLIDER].bracketed_val, 0, 4095, 80.0, 24.0);
			system_settings.master_gain = 1.0/new_gain;
		}
	}
}


float read_vca_cv(uint8_t chan)
{
	//No VCA CV if switch is set to V/oct, or if Key Mode is Key or Note
	if ((params.voct_switch_state[chan] == SW_VOCT) || (params.key_sw[chan] != ksw_MUTE))
		return 1.0;
	else
	if (analog[A_VOCT + chan].plug_sense_switch.pressed == RELEASED)
		return 1.0;
	else
	{
		//Handle Bipolar setting by treating negative voltage as 0
		if (analog[A_VOCT + chan].polarity == AP_UNIPOLAR)
			return ((analog[A_VOCT + chan].lpf_val / 2047.5) * F_SCALING_MAX_VCACV_GAIN);
		else {
			return (_CLAMP_F(analog[A_VOCT + chan].lpf_val - 2048.0, 0.0, 2048.0) / 2048.0 ) * F_SCALING_MAX_VCACV_GAIN;
		}
	}
}


/*** Move to params_lfos.c ***/

void read_lfoto_vca_vco(uint8_t i){

	static uint8_t any_button_pressed;

	if (button_pressed(butm_LFOVCA_BUTTON)
			&& !calc_params.keymode_pressed
			&& (params.key_sw[i] == ksw_MUTE)
			&& !calc_params.already_handled_button[butm_LFOVCA_BUTTON]) {

		start_ongoing_display_lfo_tovca();

		if (!macro_states.all_af_buttons_released){
			any_button_pressed = 1;
			calc_params.armed[armf_LFOTOVCA][i] = 0;

			if (button_pressed(i))
			{
				calc_params.armed[armf_LFOTOVCA][i] = 1;
				calc_params.already_handled_button[i] = 1;
				calc_params.button_safe_release[0] = 1;
			}
		}

		if (!button_pressed(i) && any_button_pressed && calc_params.armed[armf_LFOTOVCA][i]){
			//if (!lfos.locked[i])
				lfos.to_vca[i] = 1 - lfos.to_vca[i];
			calc_params.armed[armf_LFOTOVCA][i] = 0;
		}

		else if(!any_button_pressed){
			if  (button_pressed(butm_LFOVCA_BUTTON) < MED_PRESSED) {
				calc_params.button_safe_release[0] = 0;
				if (!lfos.locked[i])
					calc_params.armed[armf_LFOTOVCA][i] = 1;
			} else {
				calc_params.button_safe_release[0] = 1;
				calc_params.armed[armf_LFOTOVCA][i] = 0;
			}
		}
	}

	else if (!button_pressed(butm_LFOVCA_BUTTON))
	{
		if (calc_params.armed[armf_LFOTOVCA][i]){
			//if (!lfos.locked[i])
				lfos.to_vca[i] = 1 - lfos.to_vca[i];
			calc_params.armed[armf_LFOTOVCA][i] = 0;
		}
		if (calc_params.button_safe_release[0]) {
			calc_params.button_safe_release[0] = 0;
			stop_all_displays();
		}
		any_button_pressed = 0;
		calc_params.already_handled_button[butm_LFOVCA_BUTTON] = 0;
	}
}

/*** Move to params_lfos.c ***/

void read_lfomode(uint8_t i)
{
	static uint8_t any_button_pressed;
	static uint8_t cached[NUM_CHANNELS] = {0};

	if (!lfos.audio_mode[i])
	{
		if (cached[i]){ // uncache lfo mode when exiting audio range
			cache_uncache_lfomode(i, UNCACHE);
			cached[i] = 0;
		}

		else if (button_pressed(butm_LFOMODE_BUTTON)
				&& !calc_params.keymode_pressed
				&& (params.key_sw[i] == ksw_MUTE)
				&& !calc_params.already_handled_button[butm_LFOMODE_BUTTON]) {

			start_ongoing_display_lfo_mode();

			if (!macro_states.all_af_buttons_released){
				any_button_pressed = 1;
				calc_params.armed[armf_LFOMODE][i] = 0;

				if(button_pressed(i)){
					//if (!lfos.locked[i])
						calc_params.armed[armf_LFOMODE][i] = 1;
					calc_params.already_handled_button[i] = 1;
					calc_params.button_safe_release[1] = 1;
				}
			}

			if (!button_pressed(i) && any_button_pressed && calc_params.armed[armf_LFOMODE][i]){
				//if (!lfos.locked[i]) {
					lfos.mode[i] ++;
					lfos.mode[i] %= NUM_LFO_MODES;
				//}
				calc_params.armed[armf_LFOMODE][i]   = 0;
			}

			else if(!any_button_pressed){
				if  (button_pressed(butm_LFOMODE_BUTTON) < MED_PRESSED) {
					calc_params.button_safe_release[1] = 0;
					if (!lfos.locked[i]) calc_params.armed[armf_LFOMODE][i] = 1;
				} else {
					calc_params.button_safe_release[1] = 1;
					calc_params.armed[armf_LFOMODE][i] = 0;
				}
			}
		}

		else if (!button_pressed(butm_LFOMODE_BUTTON)){
			if(calc_params.armed[armf_LFOMODE][i]){
				if (!lfos.locked[i]) {
					lfos.mode[i] ++;
					lfos.mode[i] %= NUM_LFO_MODES;
				}
				calc_params.armed[armf_LFOMODE][i] = 0;
			}
			if (calc_params.button_safe_release[1]) {
				calc_params.button_safe_release[1] = 0;
				stop_all_displays();
			}
			calc_params.already_handled_button[butm_LFOMODE_BUTTON] = 0;
			any_button_pressed = 0;
		}
	}

	else if (!cached[i]){ // cache lfo mode and set to shape when entering audio range
		cache_uncache_lfomode(i, CACHE);
		lfos.mode[i] = lfot_SHAPE;
		cached[i] = 1;
	}
}

/*** Move to params_keymode.c ***/

void read_all_keymodes(void){

	uint8_t				i;
	static uint8_t			any_button_pressed=0;
	uint8_t				change_keymode[NUM_CHANNELS] = {0};
	enum MuteNoteKeyStates	new_keymode;

	for (i = 0; i < NUM_CHANNELS; i++)
	{
		if (key_combo_keymode_pressed())
		{
			stop_all_displays();
			calc_params.keymode_pressed = 1;

			calc_params.armed[armf_LFOMODE][i]  = 0;
			calc_params.armed[armf_LFOTOVCA][i] = 0;

			if (!macro_states.all_af_buttons_released){
				any_button_pressed = 1;
				calc_params.armed[armf_KEYMODE][i] = 0;

				if(/*!lfos.locked[i] && */button_pressed(i)){
					calc_params.armed[armf_KEYMODE][i]= 1;
					calc_params.already_handled_button[i] = 1;
					calc_params.button_safe_release[0] = 1;
					calc_params.button_safe_release[1] = 1;
				}
			}

			if (/*!lfos.locked[i] && */ !button_pressed(i) && any_button_pressed && calc_params.armed[armf_KEYMODE][i]){
				change_keymode[i] = 1;
				calc_params.armed[armf_KEYMODE][i] = 0;
			}

			else if(!any_button_pressed){
				if (button_pressed(butm_LFOVCA_BUTTON) < MED_PRESSED) {
					calc_params.button_safe_release[0] = 0;
					calc_params.button_safe_release[1] = 0;
					if (!lfos.locked[i]) calc_params.armed[armf_KEYMODE][i]= 1;
				} else {
					calc_params.button_safe_release[0] = 1;
					calc_params.button_safe_release[1] = 1;
					if (!lfos.locked[i]) calc_params.armed[armf_KEYMODE][i]= 0;
				}
			}
		}

		else if (key_combo_keymode_released()){
			if (calc_params.armed[armf_KEYMODE][i]){
				//if (!lfos.locked[i])
					change_keymode[i] = 1;
				calc_params.armed[armf_KEYMODE][i] = 0;
			}
			if (calc_params.button_safe_release[0] && calc_params.button_safe_release[1]){
				calc_params.button_safe_release[0] = 0;
				calc_params.button_safe_release[1] = 0;
				stop_all_displays();
			}
			any_button_pressed = 0;
			calc_params.keymode_pressed = 0;
		}
	}

	for (i=0; i<NUM_CHANNELS; i++){
		if (change_keymode[i]){
			new_keymode = (params.key_sw[i]+1) % NUM_MUTE_NOTE_KEY_STATES;
			apply_keymode(i, new_keymode);
		}
	}
}

//Flip Mute to Note: cache indiv_scale and LFO params, set LFOs to envelope speed/shape, remove phase
//Flip Note to Keys: do nothing
//Flip Keys to Mute: uncache LFO params
void apply_keymode(uint8_t chan, enum MuteNoteKeyStates new_keymode)
{
	if (params.key_sw[chan] != new_keymode)
	{
		//Switched from MUTE to NOTE
		if (params.key_sw[chan] == ksw_MUTE)
		{
			lfos.muted[chan] = 1; //mute to prevent race condition with update_lfo_wt_pos()

			cache_uncache_keys_params_and_lfos(chan, CACHE);
			params.note_on[chan] = 0;
			lfos.to_vca[chan] = 1;
			lfos.mode[chan] = 0;
			lfos.shape[chan] = KEY_SHAPE;

			if(params.indiv_scale[chan]==sclm_NONE) //set scale to a useful value because Note auto-triggering is disabled when scale is unquantized
				params.indiv_scale[chan]=sclm_SEMITONES;

			lfos.cycle_pos[chan] = 1.0;
			lfos.divmult_id[chan] = LFO_UNITY_DIVMULT_ID+2;
			flag_lfo_recalc(chan);

			lfos.phase_id[chan] = 0;
			lfos.phase[chan] = 0;
		}

		//Switched to MUTE
		else if (new_keymode == ksw_MUTE)
		{
			calc_params.gate_in_is_sustaining[chan] = 0;
			lfos.muted[chan] = 1; //mute to prevent race condition with update_lfo_wt_pos()
			cache_uncache_keys_params_and_lfos(chan, UNCACHE);
		}

		else if (new_keymode == ksw_KEYS)
		{
			//Todo: Restore cached scale unless indiv_scale was changed between entering and exiting NOTE mode,
			// (even if it was changed and changed back)
			//params.indiv_scale[chan] = params.indiv_scale_buf[chan];
		}

		//Switched to Ext Trig
		else if (new_keymode == ksw_KEYS_EXT_TRIG)
		{
			params.indiv_scale[chan] = sclm_NONE;
			force_wt_interp_update(chan);
		}
		else if (new_keymode == ksw_KEYS_EXT_TRIG_SUSTAIN)
		{
			calc_params.gate_in_is_sustaining[chan] = 0;
			force_wt_interp_update(chan);
		}

		params.key_sw[chan] = new_keymode;
	}

	lfos.muted[chan] = 0;
}

void cache_uncache_keymodes(enum CacheUncache cache_uncache)
{
	static enum MuteNoteKeyStates cached_key_sw[NUM_CHANNELS];

	for (uint8_t chan=0; chan<NUM_CHANNELS; chan++)
	{
		if (cache_uncache==CACHE){
			cached_key_sw[chan] = params.key_sw[chan];
		}
		else {
			apply_keymode(chan, cached_key_sw[chan]);
		}
	}
}

void apply_all_keymodes(enum MuteNoteKeyStates new_keymode)
{
	for (uint8_t chan=0; chan<NUM_CHANNELS; chan++)
		apply_keymode(chan, new_keymode);
}

void cache_uncache_keys_params_and_lfos(uint8_t chan, enum CacheUncache cache_uncache)
{
	if (cache_uncache==CACHE)
	{
		lfos.divmult_id_buf[chan]		= lfos.divmult_id[chan];
		lfos.shape_buf[chan]			= lfos.shape[chan];
		lfos.gain_buf[chan]			= lfos.gain[chan];
		lfos.phase_id_buf[chan]		= lfos.phase_id[chan];
		lfos.mode_buf[chan]				= lfos.mode[chan];
		lfos.to_vca_buf[chan]			= lfos.to_vca[chan];

		params.note_on_buf[chan] 		= params.note_on[chan];
		params.indiv_scale_buf[chan]	= params.indiv_scale[chan];
	}
	else
	{
		// set_lfo_divmult_id(chan, lfos.divmult_id_buf[chan]);
		lfos.divmult_id[chan]			= lfos.divmult_id_buf[chan];
		flag_lfo_recalc(chan);

		lfos.phase_id[chan]			= lfos.phase_id_buf[chan];
		lfos.phase[chan]				= calc_lfo_phase(lfos.phase_id[chan]);

		lfos.shape[chan]				= lfos.shape_buf[chan];
		lfos.gain[chan]				= lfos.gain_buf[chan];
		lfos.mode[chan]					= lfos.mode_buf[chan];
		lfos.to_vca[chan]				= lfos.to_vca_buf[chan];

		params.note_on[chan] 			= params.note_on_buf[chan];
		params.indiv_scale[chan]		= params.indiv_scale_buf[chan];

		stage_resync(chan);
	}
}



/*** Move to params_pitch.c ***/

void update_transpose_cv(void)
{
	// Calculate pitch multiplier from transpose jack CV
	params.transpose_cv = calc_expo_pitch(TRANSPOSE_CV, analog[TRANSPOSE_CV].lpf_val);
}

void update_pitch(uint8_t chan)
{
	float ch_freq, ch_freq_adc, qtz_ch_freq;
	uint8_t note;
	int8_t oct;
	int16_t oct_clamped;

	if ( params.key_sw[chan]==ksw_MUTE || params.key_sw[chan]==ksw_KEYS_EXT_TRIG_SUSTAIN || params.key_sw[chan]==ksw_KEYS_EXT_TRIG || params.new_key[chan] || ((params.key_sw[chan] == ksw_NOTE) && !params.note_on[chan]) )
	{
		// Calculate pitch multiplier from individual jack 1V/oct CV
		if ((params.voct_switch_state[chan] == SW_VOCT) || (params.key_sw[chan] != ksw_MUTE))
		{
			if (params.indiv_scale[chan] != sclm_NONE)
				ch_freq_adc = analog[A_VOCT + chan].bracketed_val;
			else
				ch_freq_adc = analog[A_VOCT + chan].lpf_val;

			calc_params.voct[chan] = calc_expo_pitch(A_VOCT+chan, ch_freq_adc);
		} else
			calc_params.voct[chan] = 1.0;

		if (!params.osc_param_lock[chan])
			calc_params.voct[chan] *= params.transpose_cv;

		if (params.new_key[chan]) params.new_key[chan] = 0;
	}

	ch_freq = F_BASE_FREQ  * calc_params.transposition[chan] * calc_params.voct[chan];
	oct_clamped = _CLAMP_I16(params.oct[chan], MIN_OCT , MAX_OCT);
	if (oct_clamped < 0)
		ch_freq /= (float)(1 << (-oct_clamped));
	else
		ch_freq *= (1 << oct_clamped);

	if (params.indiv_scale[chan]==sclm_NONE)
	{
		calc_params.qtz_freq[chan] = ch_freq;
	}
	else
	{
//Todo:
//		qtz_ch_freq = quantize_to_scale(params.indiv_scale[chan], ch_freq, &note, &oct, prev_qtz_note[chan], prev_qtz_oct[chan]);
		qtz_ch_freq = quantize_to_scale(params.indiv_scale[chan], ch_freq, &note, &oct);

		if (qtz_ch_freq!=ch_freq && params.qtz_note_changed[chan]==0 && (calc_params.prev_qtz_note[chan]!=note || calc_params.prev_qtz_oct[chan]!=oct))
		{
			calc_params.prev_qtz_note[chan] = note;
			calc_params.prev_qtz_oct[chan] = oct;
			calc_params.qtz_freq[chan] = qtz_ch_freq;
			if ((params.key_sw[chan]==ksw_KEYS || params.key_sw[chan]==ksw_NOTE) && !params.note_on[chan])
				params.qtz_note_changed[chan] = 1;
		}
		else {
			if (params.qtz_note_changed[chan]>0)
				params.qtz_note_changed[chan]++;
			if (params.qtz_note_changed[chan]>QTZ_CHANGE_LOCKOUT_PERIOD)
				params.qtz_note_changed[chan]=0;
		}
	}

	// Apply fine-tuning
	calc_params.pitch[chan] = _CLAMP_F(calc_params.qtz_freq[chan] * calc_params.tuning[chan], F_MIN_FREQ, F_MAX_FREQ);

	update_wt_head_pos_inc(chan);
}

void update_wt_head_pos_inc(uint8_t chan){
	wt_osc.wt_head_pos_inc[chan] = (calc_params.pitch[chan] * F_WT_TABLELEN) / F_SAMPLERATE ;
}

/*** Move to params_pitch.c ***/

void update_noise(uint8_t chan)
{
	float random_cv;
	static uint32_t noise_poll_ctr=0;

	//If the channel is fine-tuned to 0, then don't apply noise
	params.noise_on = (params.finetune[chan]==0) ? 0 : 1;

	if (params.noise_on)
	{
		if ( noise_poll_ctr++ > RANDOM_UPDATE_TIME)
		{
			noise_poll_ctr	= 0;

			random_cv = _CLAMP_F(analog[RANDOM_CV].lpf_val, 0.0, 6.0);
			params.random[chan] = 1 + ((random_cv - 3.0) * F_SCALING_RANDOM);
		}
		calc_params.pitch[chan] *= params.random[chan];
	}
}



//##########################################################
//						OSC PARAMS /*** Move to params_pitch.c ***/
//##########################################################


void read_freq(void){

	int16_t tmp1, tmp2, tmp3, tmp4;
	static uint8_t reset_detuning_enabled = 1;
	static uint8_t reset_transpose_enabled = 1;
	static uint8_t reset_octaves_enabled = 1;

	// if (key_combo_osc_lock_pressed()){update_osc_param_lock();}
	// else {

		tmp1 = pop_encoder_q (pec_OCT  );
		tmp2 = pop_encoder_q (sec_SCALE);
		tmp3 = pop_encoder_q (pec_TRANSPOSE );
		tmp4 = pop_encoder_q (sec_OSC_SPREAD);

		// ---------------------
		//	OCT / SCALE
		// ---------------------

		if (tmp1) 		{update_oct 	(tmp1);}
		else if (tmp2)	{update_scale	(tmp2);}

		// ----------------------------------------------
		//		TRANSPOSE / TUNE / CHORD / DETUNE SPREAD
		// ----------------------------------------------

		else if(tmp3){
			if (switch_pressed(FINE_BUTTON)) { update_finetune (tmp3); }
			else							{ update_transpose(tmp3); }
		}

		else if (tmp4){
			if		(switch_pressed(FINE_BUTTON)  && macro_states.all_af_buttons_released)	spread_finetune(tmp4);
			else if (!switch_pressed(FINE_BUTTON) && macro_states.all_af_buttons_released) update_spread(tmp4);
		}


		if (key_combo_reset_detuning())	{
			if (reset_detuning_enabled) {
				retune_oscillators();
				reset_detuning_enabled = 0;
			}
		} else reset_detuning_enabled = 1;

		if (key_combo_reset_transpose())	{
			if (reset_transpose_enabled) {
				reset_notes();
				reset_transpose_enabled = 0;
			}
		} else reset_transpose_enabled = 1;

		if (key_combo_reset_octaves())	{
			if (reset_octaves_enabled) {
				reset_octaves();
				reset_octaves_enabled = 0;
			}
		} else reset_octaves_enabled = 1;

	// }
	update_spread_cv();
}


void cache_uncache_locks(enum CacheUncache cache_uncache)
{
	uint8_t chan;

	static uint8_t osc_param_lock[NUM_CHANNELS];
	static uint8_t wt_pos_lock[NUM_CHANNELS];
	static uint8_t wtsel_lock[NUM_CHANNELS];
	static uint8_t lfo_locked[NUM_CHANNELS];

	for (chan=0; chan<NUM_CHANNELS; chan++)
	{
		switch (cache_uncache)
		{
			case CACHE:
				osc_param_lock[chan]	= params.osc_param_lock[chan];
				wt_pos_lock[chan]		= params.wt_pos_lock[chan];
				wtsel_lock[chan]		= params.wtsel_lock[chan];
				lfo_locked[chan]		= lfos.locked[chan];
				break;

			case UNCACHE:
				params.osc_param_lock[chan] = osc_param_lock[chan];
				params.wt_pos_lock[chan]	= wt_pos_lock[chan];
				params.wtsel_lock[chan]	= wtsel_lock[chan];
				lfos.locked[chan]			= lfo_locked[chan];
				break;
		}
	}
}

void unlock_all(void)
{
	uint8_t chan;
	for (chan=0; chan<NUM_CHANNELS; chan++)
	{
		params.osc_param_lock[chan] = 0;
		params.wt_pos_lock[chan]	= 0;
		params.wtsel_lock[chan]	= 0;
		lfos.locked[chan]			= 0;
	}
}

void toggle_lock(uint8_t chan)
{
	uint8_t lock_status = params.osc_param_lock[chan] ? 0 : 1;
	params.osc_param_lock[chan] = lock_status;
	params.wt_pos_lock[chan] = lock_status;
	params.wtsel_lock[chan] = lock_status;
	lfos.locked[chan] = lock_status;
}

void update_osc_param_lock(void)
{
	uint8_t chan;

	for (chan=0; chan<NUM_CHANNELS; chan++)
	{
		if (key_combo_lock_channel(chan))
		{
			if (calc_params.lock_change_staged[chan]==0)
				calc_params.lock_change_staged[chan] = 1;
		}
		else
			if (calc_params.lock_change_staged[chan]==2)
				calc_params.lock_change_staged[chan] = 0;
	}
}




void update_oct(int16_t tmp)
{
	uint8_t i;
	int32_t oct[NUM_CHANNELS];

	for (i = 0; i < NUM_CHANNELS; i++)
		oct[i] = params.oct[i];

	if (change_param_i32(oct, tmp))
	{
		start_ongoing_display_octave();

		//Clamp to active range, but preserve relative spacing
		trim_array(oct, NUM_CHANNELS, MIN_OCT, MAX_OCT);

		//don't allow any channel to be more than MAX_OCT away from active range
		for (i = 0; i < NUM_CHANNELS; i++)
			params.oct[i] = _CLAMP_I16(oct[i], -MAX_OCT, MAX_OCT*2);
	}
}


void update_scale(int16_t tmp)
{
	uint8_t i;
	int16_t indiv_scale[NUM_CHANNELS];

	for (i = 0; i < NUM_CHANNELS; i++)
		indiv_scale[i] = params.indiv_scale[i];

	if (change_param_i16(indiv_scale, tmp))
	{
		start_ongoing_display_scale();
		for (i = 0; i < NUM_CHANNELS; i++)
			params.indiv_scale[i] = _WRAP_I8(indiv_scale[i], 0, NUM_QTZ_SCALES);
	}
}


void update_finetune(int16_t tmp)
{
	uint8_t i;
	int16_t finetune[NUM_CHANNELS];
	uint8_t channels_changed;
	uint8_t do_resync_osc=0;

	for (i = 0; i < NUM_CHANNELS; i++)
		finetune[i] = params.finetune[i];

	channels_changed = change_param_i16(finetune, tmp);

	if (channels_changed)
	{
		for (i = 0; i < NUM_CHANNELS; i++)
		{
			if (channels_changed & (1<<i))
			{
				params.finetune[i] = finetune[i];
				if (finetune[i]==0)	do_resync_osc+=(1<<i);
				compute_tuning(i);
				start_ongoing_display_finetune();
			}
		}
	}

	if (do_resync_osc)
		resync_audio_osc(do_resync_osc);
}

void reset_octaves(void)
{
	uint8_t i;

	for (i=0; i<NUM_CHANNELS; i++)
	{
		if (!params.osc_param_lock[i])
			params.oct[i] = INIT_OCT;
	}
}

void reset_notes(void)
{
	uint8_t i;

	for (i=0; i<NUM_CHANNELS; i++)
	{
		if (!params.osc_param_lock[i]) {
			params.spread_enc[i] = 0;
			params.transpose_enc[i] = 0;
		}
	}
	combine_transpose_spread();
	compute_transpositions();
}

void retune_oscillators(void)
{
	uint8_t i;
	uint8_t chan_mask=0;

	for (i=0; i<NUM_CHANNELS; i++){
		if (!params.osc_param_lock[i]) {
			params.finetune[i] = 0;
			compute_tuning(i);
			chan_mask += 1<<i;
		}
	}
	resync_audio_osc(chan_mask);
}

void resync_audio_osc(uint8_t channels)
{
	uint8_t i;

	for (i=0; i<NUM_CHANNELS; i++)
	{
		if (channels & (1<<i))
			wt_osc.wt_head_pos[i] = 0;
	}
}



void spread_finetune(int16_t tmp)
{
	int8_t i;
	uint8_t do_resync_osc=0;

	for (i = 0; i < NUM_CHANNELS; i++ )
	{
		if (!params.osc_param_lock[i])
		{
			if (i<=2)
				params.finetune[i] += (i - 3) * tmp;
			else
				params.finetune[i] += (i - 2) * tmp;

			if (params.finetune[i]==0)
				do_resync_osc += (1<<i);

			compute_tuning(i);
			start_ongoing_display_finetune();
		}
	}

	if (do_resync_osc)
		resync_audio_osc(do_resync_osc);
}


void compute_tuning (uint8_t chan){

	int16_t i;
	float f_scaling_finetune;

	calc_params.tuning[chan] = 1;

	params.finetune[chan] = _CLAMP_I16(params.finetune[chan], MIN_FINETUNE_WRAP, MAX_FINETUNE_WRAP);
	f_scaling_finetune = F_SCALING_FINETUNE_WRAP;

	if(params.finetune[chan] > 0){
		for (i = 0; i < params.finetune[chan]; i++){
			calc_params.tuning[chan] *= f_scaling_finetune;
		}
	}

	else if (params.finetune[chan]<0) {
		for (i = 0; i > params.finetune[chan]; i--){
			calc_params.tuning[chan] /= f_scaling_finetune;
		}
	}
}


void update_transpose(int16_t tmp){
	uint8_t i;
	int32_t transpose_enc[NUM_CHANNELS];
	uint8_t channels_changed;
	int32_t min, max;

	for (i = 0; i < NUM_CHANNELS; i++)
		transpose_enc[i] = params.transpose_enc[i];

	channels_changed = change_param_i32(transpose_enc, tmp);
	if (channels_changed)
	{
		max = MAX_TRANSPOSE_WRAP;
		min = MIN_TRANSPOSE_WRAP;

		trim_array(transpose_enc, NUM_CHANNELS, min, max);

		start_ongoing_display_transpose();

		for (i = 0; i < NUM_CHANNELS; i++)
			params.transpose_enc[i] = transpose_enc[i];
	}
}


void update_spread(int16_t tmp){

	uint8_t chan;

	for (chan=0; chan<NUM_CHANNELS; chan++){
		if (!params.osc_param_lock[chan])
			params.spread_enc[chan] += tmp;
	}
	start_ongoing_display_transpose();
}

static uint8_t any_key_sw(enum MuteNoteKeyStates state);
static uint8_t any_key_sw(enum MuteNoteKeyStates state) {
	if (   params.key_sw[0]==state
		|| params.key_sw[1]==state
		|| params.key_sw[2]==state
		|| params.key_sw[3]==state
		|| params.key_sw[4]==state
		|| params.key_sw[5]==state)
		return 1;
	else
		return 0;
}

void update_spread_cv(void)
{
	if (analog_jack_plugged(TRANSPOSE_CV) && (any_key_sw(ksw_KEYS_EXT_TRIG) || any_key_sw(ksw_KEYS_EXT_TRIG_SUSTAIN)))
		params.spread_cv = 0;
	else
		params.spread_cv = (int8_t)((float)(analog[CHORD_CV].bracketed_val)  * (float)(NUM_CHORDS) / (4095.0*1.04));//4% down-scaling allows black keys to select chords (C#0 to C#5)
}

void combine_transpose_spread(void){

	uint8_t chan;
	uint8_t chord_num;
	uint8_t spread_cv;

	for (chan=0; chan<NUM_CHANNELS; chan++)
	{
		spread_cv = params.osc_param_lock[chan] ? 0: params.spread_cv;
		chord_num = _WRAP_I8(spread_cv + params.spread_enc[chan], 0, NUM_CHORDS);

		calc_params.transpose[chan] = params.transpose_enc[chan] + CHORD_LIST[chord_num][chan];
	}
}

// trim_array() trims extra steps beyond MAX/MIN values, when all elements are beyond threshold of display_min or display_max
// It serves to clip values while still preserving the spacing between them
void trim_array(int32_t *a, uint32_t num_elements, int32_t display_min, int32_t display_max){

	uint8_t chan;
	int16_t min,max;

	uint8_t all_over_max, all_under_min;

	all_over_max = 1;
	for (chan=0; chan<num_elements; chan++)
	{
		if (a[chan] <= display_max) {
			all_over_max = 0;
			break;
		}
	}

	if (all_over_max)
	{
		min = INT16_MAX;
		for (chan = 0; chan < num_elements; chan++){
			if(a[chan] < min) min = a[chan];
		}

		for (chan = 0; chan < num_elements; chan++){
			a[chan] -= (min - display_max);
		}
		return;
	}

	all_under_min = 1;
	for (chan=0; chan<num_elements; chan++)
	{
		if (a[chan] >= display_min) {
			all_under_min = 0;
			break;
		}
	}
	if (all_under_min)
	{
		max = INT16_MIN;
		for (chan = 0; chan < num_elements; chan++){
			if(a[chan] > max) max = a[chan];
		}

		for (chan = 0; chan < num_elements; chan++){
			a[chan] -= (max - display_min);
		}
	}
}

void compute_transpositions(void)
{
	uint8_t chan;

	for( chan = 0; chan < NUM_CHANNELS; chan++){
		calc_params.transposition[chan] = compute_transposition(calc_params.transpose[chan]);
	}
}

float compute_transposition(int32_t transpose)
{
	int16_t i;
	float transposition;

	transposition = 1.0;


	if(transpose > 0) {
		while(transpose>=12) {transposition *= 2.0; transpose-=12;}
		for (i = 0; i < transpose; i++){
			transposition *= F_SCALING_TRANSPOSE;
		}
	}

	else if (transpose<0) {
		while(transpose<=-12) {transposition /= 2.0; transpose+=12;}
		for (i = 0; i > transpose; i--){
			transposition /= F_SCALING_TRANSPOSE;
		}
	}

	return transposition;
}



// ########################################################################
//						WAVETABLE POSITION + SELECTION  /*** Move to params_wt.c ***/
// ########################################################################


void update_wt(void)
{
	uint8_t		i;
	static uint8_t  poll_ctr =  0;


	for (i = 0; i < NUM_CHANNELS; i++){
		update_wbrowse_step_pos(i);
	}

	switch(poll_ctr++){

		// ------------
		// WT SELECTION
		// -------------

		// spread
		case 2:
			read_wtsel_spread();	//updates and wraps wtsel_spread_enc
			read_wtsel_spread_cv();	//updates wtsel_spread_cv based on cv on the jack

			read_browse_encoder();	//calls read_wtsel() if encoder is pushed+turned and updates wtsel_enc[]
			read_wtsel_cv();		//updates wtsel_cv based on cv on the jack

			update_wtsel();		//sets calc_params.wtsel[] based on sum of spread and calc_params.wtsel (cv and enc)

			update_wt_bank();		//sets wt_bank = calc_params.wtsel and requests wt update if it's changed

		break;


		// ------------
		// WT POSITION
		// ------------

		case 4:
			update_wt_disp (0);
			update_wbrowse_cv();
		break;

		// depth
		case 6:
			update_wt_nav_cv(0);
			read_nav_encoder(0);
		break;

		// latitude
		case 9:
			update_wt_nav_cv(1);
			read_nav_encoder(1);
		break;

		// longitude
		case 12:
			update_wt_nav_cv(2);
			read_nav_encoder(2);
		break;

		case 15:
			//Todo: make these functions into one looping function
			for (i = 0; i < NUM_CHANNELS; i++){
				calc_wt_pos				(i);
				update_wt_pos_interp_params	(i, 0);
				update_wt_pos_interp_params	(i, 1);
				update_wt_pos_interp_params	(i, 2);
			}
			poll_ctr=0;
		break;
	}
}

void read_nav_encoder(uint8_t dim){

	uint8_t i;
	int16_t enc;
	uint8_t	WT_ENCODERS[3] = {pec_DEPTH, pec_LATITUDE, pec_LONGITUDE};
	float wt_pos_increment;

	enc = pop_encoder_q(WT_ENCODERS[dim]);

	if(enc) {
		if ((ui_mode == WTEDITING) && (!macro_states.all_af_buttons_released))
		{
			if	 (dim == 0){for (i = 0; i < WT_DIM_SIZE; i++){update_wt_fx_params(i,					wt_osc.m0[1][0],	wt_osc.m0[2][0],	enc);}}
			else if(dim == 1){for (i = 0; i < WT_DIM_SIZE; i++){update_wt_fx_params(wt_osc.m0[0][0],	i,					wt_osc.m0[2][0],	enc);}}
			else if(dim == 2){for (i = 0; i < WT_DIM_SIZE; i++){update_wt_fx_params(wt_osc.m0[0][0],	wt_osc.m0[1][0],	i,					enc);}}
		}
		else
		{
			if (UIMODE_IS_WT_RECORDING_EDITING(ui_mode))
				wt_pos_increment = enc * (switch_pressed(FINE_BUTTON) ? F_SCALING_NAVIGATE : 1);
			else
				wt_pos_increment = enc * (switch_pressed(FINE_BUTTON) ? F_SCALING_FINE_NAVIGATE : F_SCALING_NAVIGATE);

			update_wt_nav(dim, wt_pos_increment);
		}
	}
}


void read_browse_encoder(void)
{
	int16_t enc, enc2;
	int16_t i,j,k;
	static uint8_t set_new_global_brightness=0;
	static uint8_t browse_pressed = 0;
	static uint8_t browse_moved = 0;

	enc = pop_encoder_q(pec_WBROWSE);
	enc2 = pop_encoder_q(sec_WTSEL);

	if (enc || enc2) browse_moved = 1;

	if (rotary_pressed(rotm_WAVETABLE))
	{
		if (!browse_pressed) {
			browse_pressed = 1;
			browse_moved = 0;
		}
	} else {
		if (browse_pressed) {
			browse_pressed = 0;
			if (!browse_moved && (ui_mode == WTMONITORING)) {
				start_play_export_sphere();
			}
		}
	}

	if (ui_mode == WTEDITING && !macro_states.all_af_buttons_released)
	{
		params.dispersion_enc = 0;
		// params.disppatt_enc = 1;
		update_wt_disp(CLEAR_LPF);
		update_wt_fx_params(wt_osc.m0[0][0], wt_osc.m0[1][0], wt_osc.m0[2][0], enc);
	}
	else
	{
		if (!rotary_pressed(rotm_PRESET))
			update_wbrowse(enc);

		if (enc)
		{
			if (rotary_pressed(rotm_PRESET)) {
				exit_preset_manager();
				stop_all_displays();
				start_ongoing_display_globright();
				system_settings.global_brightness = _CLAMP_F(system_settings.global_brightness + ((float)enc * F_SCALING_NAVIGATE_GLOBAL_BRIGHTNESS), F_MIN_GLOBAL_BRIGHTNESS, 1.0);
				set_new_global_brightness =1;
			}
		}
		if (!rotary_pressed(rotm_PRESET) && set_new_global_brightness) {
			save_flash_params();
			set_new_global_brightness=0;
		}
	}

	if (enc2)
	{
		if (ui_mode == WTEDITING)
		{
			if (macro_states.all_af_buttons_released)
				update_sphere_stretch_position(enc2);
			else
			{
				for (i=0; i< WT_DIM_SIZE; i++){
					for (j=0; j< WT_DIM_SIZE; j++){
						for (k=0; k< WT_DIM_SIZE; k++){
							update_wt_fx_params(i,j,k,enc2);
						}
					}
				}
			}
		}

		else if (ui_mode == PLAY) {
			read_wtsel(enc2);
		}
	}
}


enum WTFlashLoadQueueStates{
	WT_FLASH_NO_ACTION,
	WT_FLASH_LOAD_0,
	WT_FLASH_LOAD_1,
	WT_FLASH_LOAD_2,
	WT_FLASH_LOAD_3,
	WT_FLASH_LOAD_4,
	WT_FLASH_LOAD_5,
	WT_FLASH_LOAD_6,
	WT_FLASH_LOAD_7,
	WT_FLASH_LOAD_8,
	WT_FLASH_INTERP,

};
void update_wt_interp(void)
{
	int8_t chan;
	uint8_t x[2],y[2],z[2];

	static uint8_t loadx[2][NUM_CHANNELS]={0}, loady[2][NUM_CHANNELS]={0}, loadz[2][NUM_CHANNELS]={0};
	static int16_t	*p_waveform[NUM_CHANNELS][8]; // addresses for 8x waveforms used for interpolation
	static uint8_t	old_x0[NUM_CHANNELS] = {0xFF};
	static uint8_t	old_y0[NUM_CHANNELS] = {0xFF};
	static uint8_t	old_z0[NUM_CHANNELS] = {0xFF};
	static uint8_t	old_bank[NUM_CHANNELS] = {0xFF};

	static enum WTFlashLoadQueueStates state[NUM_CHANNELS] = {WT_FLASH_NO_ACTION};

	for (chan = 0; chan < NUM_CHANNELS; chan++)
	{
		if (wt_osc.wt_interp_request[chan] == WT_INTERP_REQ_NONE)
			continue;

		//Todo: If wt_osc.m0[][chan] changes while a


		if ((wt_osc.wt_interp_request[chan] == WT_INTERP_REQ_REFRESH)
				&& (state[chan] == WT_FLASH_NO_ACTION)
				&& (wt_osc.m0[0][chan] == old_x0[chan]) && (wt_osc.m0[1][chan] == old_y0[chan]) && (wt_osc.m0[2][chan] == old_z0[chan])
				&& (params.wt_bank[chan] == old_bank[chan])
			)
		{
			interp_wt(chan, p_waveform[chan]);
		}
		else
		{
			if (ui_mode == PLAY)
			{
				if (get_flash_state() != sFLASH_NOTBUSY)
					continue;

				if (state[chan]==WT_FLASH_NO_ACTION)
				{
					loadx[0][chan] = wt_osc.m0[0][chan];
					loady[0][chan] = wt_osc.m0[1][chan];
					loadz[0][chan] = wt_osc.m0[2][chan];
					loadx[1][chan] = wt_osc.m1[0][chan];
					loady[1][chan] = wt_osc.m1[1][chan];
					loadz[1][chan] = wt_osc.m1[2][chan];
				}

				state[chan]++;

				if (state[chan] < WT_FLASH_INTERP) {
					uint8_t s = state[chan]-1;
					uint8_t sb0 = s&1;
					uint8_t sb1 = (s&2)>>1;
					uint8_t sb2 = (s&4)>>2;
					load_extflash_wavetable(params.wt_bank[chan], &(waveform[chan][sb2][sb1][sb0]), loadx[sb2][chan], loady[sb1][chan], loadz[sb0][chan]);
				}
				else {
					old_x0[chan] = loadx[0][chan];
					old_y0[chan] = loady[0][chan];
					old_z0[chan] = loadz[0][chan];
					old_bank[chan] = params.wt_bank[chan];

					p_waveform[chan][0] = waveform[chan][0][0][0].wave;
					p_waveform[chan][1] = waveform[chan][1][0][0].wave;
					p_waveform[chan][2] = waveform[chan][0][1][0].wave;
					p_waveform[chan][3] = waveform[chan][1][1][0].wave;
					p_waveform[chan][4] = waveform[chan][0][0][1].wave;
					p_waveform[chan][5] = waveform[chan][1][0][1].wave;
					p_waveform[chan][6] = waveform[chan][0][1][1].wave;
					p_waveform[chan][7] = waveform[chan][1][1][1].wave;
					state[chan] = WT_FLASH_NO_ACTION;
					interp_wt(chan, p_waveform[chan]);
				}
			}

			else if (UIMODE_IS_WT_RECORDING_EDITING(ui_mode))
			{
				x[0] = wt_osc.m0[0][chan];
				y[0] = wt_osc.m0[1][chan];
				z[0] = wt_osc.m0[2][chan];
				x[1] = wt_osc.m1[0][chan];
				y[1] = wt_osc.m1[1][chan];
				z[1] = wt_osc.m1[2][chan];
				old_x0[chan] = x[0];
				old_y0[chan] = y[0];
				old_z0[chan] = z[0];

				p_waveform[chan][0] =  spherebuf.data[x[0]][y[0]][z[0]].wave;
				p_waveform[chan][1] =  spherebuf.data[x[1]][y[0]][z[0]].wave;
				p_waveform[chan][2] =  spherebuf.data[x[0]][y[1]][z[0]].wave;
				p_waveform[chan][3] =  spherebuf.data[x[1]][y[1]][z[0]].wave;
				p_waveform[chan][4] =  spherebuf.data[x[0]][y[0]][z[1]].wave;
				p_waveform[chan][5] =  spherebuf.data[x[1]][y[0]][z[1]].wave;
				p_waveform[chan][6] =  spherebuf.data[x[0]][y[1]][z[1]].wave;
				p_waveform[chan][7] =  spherebuf.data[x[1]][y[1]][z[1]].wave;
				state[chan] = WT_FLASH_NO_ACTION;
				interp_wt(chan, p_waveform[chan]);
			}
		}
	}
}


void interp_wt(uint8_t chan, int16_t *p_waveform[8]){

	uint16_t  i = 0;
	// float xfade0, xfade1, yfade0, yfade1;

	if (ui_mode == WTTTONE) {
		while (i < WT_TABLELEN){
			wt_osc.mc[1 - wt_osc.buffer_sel[chan]][chan][i] = (float)(TTONE[i]);
			i++;
		}
	}
	else{
		while (i < WT_TABLELEN){
		//100us
			wt_osc.mc[1 - wt_osc.buffer_sel[chan]][chan][i] =
				(
					( (float) ( *(p_waveform[0] + i))  * wt_osc.m_frac_inv[0][chan] + (float) ( *(p_waveform[1] + i))  * wt_osc.m_frac[0][chan] )  * wt_osc.m_frac_inv[1][chan] +
					( (float) ( *(p_waveform[2] + i))  * wt_osc.m_frac_inv[0][chan] + (float) ( *(p_waveform[3] + i))  * wt_osc.m_frac[0][chan] )  * wt_osc.m_frac	[1][chan]
				) * wt_osc.m_frac_inv[2][chan]
				+
				(
					( (float) ( *(p_waveform[4] + i))  * wt_osc.m_frac_inv[0][chan] + (float) ( *(p_waveform[5] + i))   * wt_osc.m_frac[0][chan] )   * wt_osc.m_frac_inv[1][chan] +
					( (float) ( *(p_waveform[6] + i))	* wt_osc.m_frac_inv[0][chan] + (float) ( *(p_waveform[7] + i))   * wt_osc.m_frac[0][chan] )   * wt_osc.m_frac	[1][chan]
				) * wt_osc.m_frac[2][chan];

			i++;
		}
	}
	wt_osc.buffer_sel[chan]		= 1 - wt_osc.buffer_sel[chan];
	wt_osc.wt_xfade[chan]			= 1.0;
	wt_osc.wt_interp_request[chan]	= WT_INTERP_REQ_NONE;

}

void req_wt_interp_update (uint8_t chan){
	wt_osc.wt_interp_request[chan] = WT_INTERP_REQ_REFRESH;
}


void force_all_wt_interp_update (void){
	force_wt_interp_update(0);
	force_wt_interp_update(1);
	force_wt_interp_update(2);
	force_wt_interp_update(3);
	force_wt_interp_update(4);
	force_wt_interp_update(5);
}

void force_wt_interp_update (uint8_t chan){
	wt_osc.wt_interp_request[chan] = WT_INTERP_REQ_FORCE;
}


void set_num_sphere_filled(uint8_t num_filled){
	num_spheres_filled = num_filled;
}


// ####################################
//			WT SELECTION
// ####################################

//Todo: if we ever have more than 127 spheres then we'll need to use uint8_t or int16_t or something for params.wtsel_enc[]
//or else wrap it properly from +127 to -128 (must wrap to same modulus of number of spheres)
void read_wtsel(int8_t wtsel)
{
	uint8_t i;
	int16_t wtsel_enc[NUM_CHANNELS];

	if (wtsel)
	{
		for (i = 0; i < NUM_CHANNELS; i++)
			wtsel_enc[i] = params.wtsel_enc[i];

		if (change_param_i16(wtsel_enc, wtsel))
		{
			for (i = 0; i < NUM_CHANNELS; i++)
				params.wtsel_enc[i] = wtsel_enc[i];
		}

		start_ongoing_display_sphere_sel();
	}
}


void read_wtsel_cv(void){
	if ((params.key_sw[0]==ksw_KEYS_EXT_TRIG_SUSTAIN) && analog_jack_plugged(A_VOCT))
		params.wtsel_cv = 0;
	else
		params.wtsel_cv = analog[WTSEL_CV].bracketed_val * num_spheres_filled / 4095;
}


void read_wtsel_spread(void)
{
	int8_t wtsel_spread;
	uint8_t chan;

	wtsel_spread = pop_encoder_q(sec_WTSEL_SPREAD);

	if (wtsel_spread && !UIMODE_IS_WT_RECORDING_EDITING(ui_mode))
	{
		for (chan=0; chan<NUM_CHANNELS; chan++)
		{
			if (!params.wtsel_lock[chan])
				params.wtsel_spread_enc[chan] = _WRAP_I16(params.wtsel_spread_enc[chan] + wtsel_spread, 0, NUM_WTSEL_SPREADS);
		}

		start_ongoing_display_sphere_sel();
	}
}

void read_wtsel_spread_cv(void)
{
	if (!UIMODE_IS_WT_RECORDING_EDITING(ui_mode))
	{
		if ((params.key_sw[5]==ksw_KEYS_EXT_TRIG || params.key_sw[5]==ksw_KEYS_EXT_TRIG_SUSTAIN) && analog_jack_plugged(F_VOCT))
			params.wtsel_spread_cv = 0;
		else
			params.wtsel_spread_cv = analog[WTSEL_SPREAD_CV].bracketed_val * NUM_WTSEL_SPREADS / 4095;
	}
}

int8_t calc_wtspread_offset(uint8_t spread_amt, uint8_t chan)
{
	int8_t offset_per_spread = (int8_t)chan - ((int8_t)NUM_CHANNELS/2);

	if (chan >= (NUM_CHANNELS/2))
		offset_per_spread++;

	return (int8_t)spread_amt * offset_per_spread;

	//0: -3, 1: -2, 2: -1, 3: 1, 4: 2, 5: 3
}

void update_wtsel(void){
	uint16_t spread_sum;
	int8_t wtsel_sum;
	uint8_t chan;
	uint8_t wtsel_spread_cv, wtsel_cv;

	for (chan=0; chan<NUM_CHANNELS; chan++)
	{
		wtsel_spread_cv =  params.wtsel_lock[chan] ? 0 : params.wtsel_spread_cv;
		wtsel_cv = params.wtsel_lock[chan] ? 0 : params.wtsel_cv;
		spread_sum = _WRAP_U8(wtsel_spread_cv + params.wtsel_spread_enc[chan], 0, NUM_WTSEL_SPREADS);
		wtsel_sum = params.wtsel_enc[chan] + wtsel_cv + calc_wtspread_offset(spread_sum, chan);

		calc_params.wtsel[chan] = _WRAP_I8(wtsel_sum, 0, num_spheres_filled);
	}
}

void update_wt_bank(void)
{
	uint8_t i;
	uint8_t test_bank;

	for (i=0; i<NUM_CHANNELS; i++){
		test_bank = sphere_index_to_bank(calc_params.wtsel[i]);

		if ((params.wt_bank[i]!=test_bank) ) {
			params.wt_bank[i] = test_bank;
			req_wt_interp_update(i);
		}

	}
}


//Given Spread and WTSel encoder positions (ignoring CV), return the wtsel
int8_t calc_static_wtsel(uint8_t chan)
{
	uint16_t spread_sum;
	int8_t wtsel_sum;

	spread_sum = _WRAP_U8(params.wtsel_spread_enc[chan], 0, NUM_WTSEL_SPREADS);
	wtsel_sum = params.wtsel_enc[chan] + calc_wtspread_offset(spread_sum, chan);
	return _WRAP_I8(wtsel_sum, 0, num_spheres_filled);
}

//Makes adjustments to wtbank when recalling a preset saved since new spheres have been added
//This maintains wtsel <--> wtbank consistancy
void fix_wtsel_wtbank_offset(void)
{
	uint8_t chan;
	int8_t wtsel;
	uint8_t test_bank;
	uint8_t target_sphere_i;
	int8_t offset;

	for (chan=0; chan<NUM_CHANNELS; chan++)
	{
		wtsel = calc_static_wtsel(chan);
		test_bank = sphere_index_to_bank(wtsel);
		if (test_bank != params.wt_bank[chan])
		{
			target_sphere_i = bank_to_sphere_index(params.wt_bank[chan]);
			offset = target_sphere_i - wtsel;
			params.wtsel_enc[chan] += offset;
		}
	}
}


void set_wtsel(uint8_t selection)
{
	uint8_t i;

	for (i=0; i<NUM_CHANNELS; i++)
	{
		if (!params.wtsel_lock[i]) {
			params.wtsel_spread_enc[i] = 0;
			params.wtsel_enc[i] = selection;
		}
	}
	update_wtsel();
	update_wt_bank();
}


// ####################################
//			WT NAVIGATION
// ####################################



void update_wt_nav(uint8_t wt_dim, float wt_nav_increment)
{
	if (wt_nav_increment)
	{
		change_param_f(params.wt_nav_enc[wt_dim], wt_nav_increment);

		if (UIMODE_IS_WT_RECORDING_EDITING(ui_mode) && !switch_pressed(FINE_BUTTON)) {
			params.dispersion_enc = 0;
			// params.disppatt_enc = 1;
			update_wt_disp(CLEAR_LPF);
		}
	}
}

void update_wt_nav_cv(uint8_t wt_dim)
{
	switch (wt_dim) {
		case 0:
			if ((params.key_sw[2]==ksw_KEYS_EXT_TRIG || params.key_sw[2]==ksw_KEYS_EXT_TRIG_SUSTAIN) && analog_jack_plugged(C_VOCT))
				params.wt_nav_cv[0] = 0;
			else
				params.wt_nav_cv[0] = (float)(analog[DEPTH_CV].bracketed_val) * (float)WT_DIM_SIZE / 4095.0; //0..3
			break;

		case 1:
			if ((params.key_sw[4]==ksw_KEYS_EXT_TRIG || params.key_sw[4]==ksw_KEYS_EXT_TRIG_SUSTAIN) && analog_jack_plugged(E_VOCT))
				params.wt_nav_cv[1] = 0;
			else
				params.wt_nav_cv[1] = (float)(analog[LATITUDE_CV].bracketed_val) * (float)WT_DIM_SIZE / 4095.0; //0..3
			break;

		case 2:
			params.wt_nav_cv[2] = 0; //we only have 2 CV jacks for wt nav_enc
			break;

		default:
			break;
	}

}


void update_wt_disp(uint8_t clear_lpf){

	int8_t			patt_encoder_motion;
	float			disp_encoder_motion;
	static float	disp_encoder_motion_lpf = 0.0f;

	if (clear_lpf==CLEAR_LPF)
		disp_encoder_motion_lpf = 0;

	else {
		// Dispersion encoder
		disp_encoder_motion = pop_encoder_q(sec_DISPERSION) * (switch_pressed(FINE_BUTTON) ? F_SCALING_FINE_DISPERSION : F_SCALING_DISPERSION);
		disp_encoder_motion_lpf = (disp_encoder_motion_lpf * (1.0-F_SCALING_DISPERSION_LPF)) + (disp_encoder_motion * F_SCALING_DISPERSION_LPF);
		if (fabs(disp_encoder_motion_lpf)>0.001)
			params.dispersion_enc = _WRAP_F(params.dispersion_enc + disp_encoder_motion_lpf, 0 ,2.0);

		// Dispersion cv
		if ((params.key_sw[1]==ksw_KEYS_EXT_TRIG || params.key_sw[1]==ksw_KEYS_EXT_TRIG_SUSTAIN) && analog_jack_plugged(B_VOCT))
			params.dispersion_cv = 0;
		else
			params.dispersion_cv = ((float)analog[DISP_CV].bracketed_val)/4095.0;


		// Pattern encoder
		patt_encoder_motion = pop_encoder_q(sec_DISPPATT);
		if(patt_encoder_motion)
			params.disppatt_enc = _WRAP_I8(params.disppatt_enc + patt_encoder_motion, 0, NUM_DISPPAT);

		// Pattern cv
		if ((params.key_sw[3]==ksw_KEYS_EXT_TRIG || params.key_sw[3]==ksw_KEYS_EXT_TRIG_SUSTAIN) && analog_jack_plugged(D_VOCT))
			params.disppatt_cv = 0;
		else
			params.disppatt_cv = analog[DISPPAT_CV].bracketed_val * NUM_DISPPAT / 4095;
	}
}


// Set the wt_pos[dim][chan], properly wrapping it, and requesting an update if needed
// Given:
// params.dispersion_enc: float 0..1
// params.dispersion_cv: float 0..1
//
// params.disppatt_enc: int8_t 0.. NUM_DISPPAT
// params.disppatt_cv: int8_t 0.. NUM_DISPPAT
//
// params.wt_browse_step_pos_enc[dim][chan] float 0..27
// params.wt_browse_step_pos_cv[dim] float 0..27
//
// params.wt_nav_enc[dim][chan] float 0..3
// params.wt_nav_cv[dim] float 0..3
//
void calc_wt_pos(uint8_t chan){

	uint8_t wt_dim;
	float	disp_amt, browse_nav[3], nav_enc[3];
	float	total_disp, total_browse;
	uint8_t disp_pattern;
	float	new_wt_pos = 10;
	float	disp_cv, disppat_cv, nav_cv, browse_cv;
	uint8_t snap_to_int=0;

	if (UIMODE_IS_WT_RECORDING_EDITING(ui_mode) && !switch_pressed(FINE_BUTTON))
		snap_to_int = 1;

	// NAVIGATION
	nav_enc[0]	= params.wt_nav_enc[0][chan];
	nav_enc[1]	= params.wt_nav_enc[1][chan];
	nav_enc[2]	= params.wt_nav_enc[2][chan];

	// BROWSE
	browse_cv = params.wt_pos_lock[chan] ? 0: params.wt_browse_step_pos_cv;
	total_browse = params.wt_browse_step_pos_enc[chan] + browse_cv;
	get_browse_nav(total_browse, &browse_nav[0], &browse_nav[1], &browse_nav[2]);

	// DISPERSION
	disp_cv = params.wt_pos_lock[chan] ? 0: params.dispersion_cv;
	total_disp = _FOLD_F(params.dispersion_enc, 1.0) + disp_cv;

	disppat_cv = params.wt_pos_lock[chan] ? 0: params.disppatt_cv;
	disp_pattern = _WRAP_U8(params.disppatt_enc + disppat_cv, 0, NUM_DISPPAT);

	// COMBINING
	for (wt_dim=0;wt_dim<NUM_WT_DIMENSIONS;wt_dim++){

		disp_amt = WT_DIM_SIZE * total_disp * (DISP_PATTERN[disp_pattern][chan][wt_dim]);

		nav_cv = params.wt_pos_lock[chan] ? 0: params.wt_nav_cv[wt_dim];
		new_wt_pos = _WRAP_F(disp_amt + browse_nav[wt_dim] + nav_cv + nav_enc[wt_dim], 0, WT_DIM_SIZE);

		if (snap_to_int)
			new_wt_pos = _WRAP_F((int32_t)(new_wt_pos+0.5),0,3);

		if (new_wt_pos != calc_params.wt_pos[wt_dim][chan]){
			calc_params.wt_pos[wt_dim][chan] = new_wt_pos;

			req_wt_interp_update(chan);
		}
	}
}

// This uses wt_pos[] to calculate the integer and fractional morph position within the sphere
void update_wt_pos_interp_params(uint8_t chan, uint8_t wt_dim){
	wt_osc.m0		[wt_dim][chan]	= ((uint8_t)(calc_params.wt_pos[wt_dim][chan]))     % WT_DIM_SIZE;
	wt_osc.m1		[wt_dim][chan]	= ((uint8_t)(calc_params.wt_pos[wt_dim][chan]) + 1) % WT_DIM_SIZE;
	wt_osc.m_frac	[wt_dim][chan]	= calc_params.wt_pos[wt_dim][chan] - (float)(wt_osc.m0[wt_dim][chan]);
	wt_osc.m_frac_inv [wt_dim][chan]	= 1.0 - wt_osc.m_frac[wt_dim][chan];
}


void update_all_wt_pos_interp_params(void){
	uint8_t i, j;

	for(i=0; i<NUM_CHANNELS; i++){
		calc_wt_pos(i);

		for(j=0; j<WT_DIM_SIZE; j++){
			update_wt_pos_interp_params(i, j);
		}
	}
}

void read_load_save_encoder(void){
	static uint8_t preset_feature_armed = 0;
	int16_t enc, enc2;

	enc   = pop_encoder_q (pec_LOADPRESET);
	enc2  = pop_encoder_q (sec_SAVEPRESET);

	if (rotary_released(rotm_PRESET) && macro_states.all_af_buttons_released && !button_pressed(butm_LFOVCA_BUTTON) && !button_pressed(butm_LFOMODE_BUTTON))
		preset_feature_armed = 1;

	if (!key_combo_all_but_preset_released()) {
		preset_feature_armed = 0;
		exit_preset_manager();
	}

	if (ui_mode==PLAY && key_combo_all_but_preset_released() && preset_feature_armed)
		handle_preset_events(enc,enc2);

	else if (UIMODE_IS_WT_RECORDING_EDITING(ui_mode))
		handle_wt_saving_events(enc);
}



void read_switches(void)
{
	uint8_t chan;

	if (switch_pressed(VOCTSW))
	{
		for (chan=0; chan<NUM_CHANNELS; chan++) {
			if (!params.osc_param_lock[chan])
				params.voct_switch_state[chan] = SW_VCA;
		}
	}
	else
	{
		for (chan=0; chan<NUM_CHANNELS; chan++) {
			if (!params.osc_param_lock[chan])
				params.voct_switch_state[chan] = SW_VOCT;
		}
	}
}
