/*
 * led_cont.c - handles SWN LEDs
 *
 * Author: Dan Green (danngreen1@gmail.com), Hugo Paris (hugoplho@gmail.com)
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


#include "globals.h"
#include "led_cont.h"
#include "led_colors.h"
#include "envout_pwm.h"
#include "params_update.h"
#include "params_lfo.h"
#include "params_lfo_period.h"
#include "gpio_pins.h"
#include "flash_params.h"
#include "analog_conditioning.h"
#include "UI_conditioning.h"
#include "preset_manager.h"
#include "preset_manager_UI.h"
#include "math_util.h"
#include "drivers/mono_led_driver.h"
#include "system_settings.h"
#include "ui_modes.h"
#include "wavetable_editing.h"
#include "key_combos.h"

#include "drivers/leds_pwm.h"
#include "wavetable_recording.h"
#include "wavetable_editing.h"
#include "wavetable_saveload.h"
#include "quantz_scales.h"
#include "calibrate_voct.h"
#include "wavetable_saveload_UI.h"
#include "flash_params.h"
#include "timekeeper.h"
#include "ui_modes.h"
#include "wavetable_play_export.h"

extern SystemCalibrations *system_calibrations;

// UI
extern enum UI_Modes 			ui_mode;

// LEDs
o_led_cont				 		led_cont;
extern const o_rgb_led			RGB_LED_OFF;

// Params
extern 		o_params 			params;
extern		o_calc_params		calc_params;
extern 		o_lfos				lfos;
extern		o_preset_manager	preset_mgr;
extern 		o_spherebuf	 		spherebuf;
extern		o_systemSettings	system_settings;

// Hardware
extern 		o_monoLed   		monoLed[NUM_MONO_LED];
extern 		o_macro_states		macro_states;


// LED maps
extern const uint8_t	led_button_map[NUM_BUTTONS];
extern const uint8_t	led_rotary_map[NUM_LED_ROTARIES];
extern const uint8_t 	ledstring_map[NUM_CHANNELS+1];
extern const uint8_t 	led_outring_map[NUM_LED_OUTRING];
extern const uint8_t 	led_inring_map[NUM_LED_INRING];

const uint8_t OCT_OUTRING_MAP[NUM_LED_OUTRING] 		    	= { 9, 10, 11, 12, 13, 14, 15, 16, 17, 0, 1, 2, 3, 4, 5, 6, 7, 8 };

// Color palettes
extern uint32_t colorPalette[NUM_LED_COLORS][NUM_PALETTE_COLORS];
extern const enum colorCodes qtz_scale_colors[NUM_QTZ_SCALES];
extern const enum colorCodes fx_colors[NUM_FX];

// TABLES
extern const float 	exp_1voct_10_41V[4096];

const uint16_t CH_COLOR_MAP[6][3] = {
	{ 1		, 600	, 954	},
	{ 1		, 12	, 954	},
	{ 941  	, 366	, 954	},
	{ 941 	, 35 	, 947	},
	{ 954 	, 176	, 21 	},
	{ 800 	, 1	 	, 50	}
};

const uint16_t LFO_BANK_COLOR[25][3]= {
	{ 1		, 600	, 954	},				// Shades of Blue
	{ 1		, 318	, 947	},
	{ 1		, 94 	, 950	},
	{ 1		, 12	, 954	},
	{ 1		, 1		, 379	},

	{ 941  	, 366	, 954	},				// Shades of Pink
	{ 935 	, 116	, 928	},
	{ 941 	, 35 	, 947	},
	{ 954 	, 1		, 282 	},
	{ 904 	, 1	 	, 126	},

	{ 502	, 309 	, 43 	},				// Shades of Yellow/Orange
	{ 947 	, 388	, 21	},
	// { 949 	, 256	, 21 	},
	// { 954 	, 176	, 21 	},
	// { 954 	, 130	, 22	},
	// { 954 	, 55	, 21	},

	{ 100  	, 100	, 100	},				// Shade of white

	{  588	, 928	, 199	},				// Shades of Green
	{  274	, 954	, 67	},
	{  83	, 949	, 1		},
	{  1	, 239	, 1		},
	{  1	, 101	, 9		},
	{  1	, 25	, 4		},

	{ 941  	, 366	, 100	},				// Shades of Red
	{ 935 	, 116	, 80	},
	{ 941 	, 35 	, 60	},
	{ 954 	, 1		, 40 	},
	{ 904 	, 1	 	, 20	},
	{ 800 	, 1	 	, 0	    }
};

enum colorCodes key_sw_mode_colors[NUM_MUTE_NOTE_KEY_STATES];

void update_pwm_leds(void);

void update_pwm_leds(void)
{
	update_display_at_encoder_press();
	update_led_flash();
	update_button_leds();
	update_encoder_leds();
	update_array_leds();
	update_clockin_led();
	update_audioin_led();
	update_LED_rings();
}

void start_led_display(void)
{
	start_timer_IRQ(LED_UPDATE_TIM_number, &update_pwm_leds);
}

void init_led_cont_ongoing_display(void)
{
	led_cont.ongoing_display	= ONGOING_DISPLAY_NONE;
	led_cont.ongoing_timeout	= 0;
}

void init_led_cont(void)
{
	uint8_t i;

	// BUTTONS
	for (i=0; i<NUM_BUTTONS; i++)
	{
		if (i< NUM_CHANNELS)
			set_rgb_color_brightness(&led_cont.button[i], ledc_WHITE, 1.0/1.7);
		else
			set_rgb_color_brightness(&led_cont.button[i], ledc_PINK, 1.0/2.5);
	}

	// LFOs
	for (i=0; i<NUM_CHANNELS; i++)
		set_rgb_color_brightness(&led_cont.array[i], ledc_WHITE, 1.0/2.0);

	set_rgb_color_brightness(&led_cont.array[GLO_CLK], ledc_WHITE, 1.0/2.0);

	// Flags (LFO)
	for (i=0; i<NUM_CHANNELS; i++)
	{
		led_cont.ongoing_lfoshape[i] = 0;
		led_cont.lfoshape_timeout[i] = 0;
	}


	key_sw_mode_colors[ksw_MUTE] = ledc_WHITE;
	key_sw_mode_colors[ksw_NOTE] = ledc_PINK;
	key_sw_mode_colors[ksw_KEYS] = ledc_PURPLE;
	key_sw_mode_colors[ksw_KEYS_EXT_TRIG] = ledc_GOLD;
	key_sw_mode_colors[ksw_KEYS_EXT_TRIG_SUSTAIN] = ledc_BUTTERCUP;
}

void update_display_at_encoder_press(void)
{
	if (rotary_pressed(rotm_TRANSPOSE))
	{
		if (switch_pressed(FINE_BUTTON))
			start_ongoing_display_finetune();
		else
			start_ongoing_display_transpose();
	}

	if (rotary_pressed(rotm_OCT) && !switch_pressed(FINE_BUTTON) && (led_cont.ongoing_display != ONGOING_DISPLAY_SCALE))
		start_ongoing_display_octave();

	else if (rotary_pressed(rotm_OCT) && switch_pressed(FINE_BUTTON))
		start_ongoing_display_scale();

	if (!UIMODE_IS_WT_RECORDING_EDITING(ui_mode) && rotary_pressed(rotm_WAVETABLE))
		start_ongoing_display_sphere_sel();
}

void update_led_flash(void)
{
	led_cont.flash_state = (HAL_GetTick()/TICKS_PER_MS) & 0x080; //128ms flash period
}

void update_button_leds(void){

	uint8_t 					i;
	uint8_t 					color;
	enum VoctCalStates 			voct_state;

	int32_t 					tri_period, tri_phase;
	float 						tri_wave, brightness, lock_brightness;

	uint32_t					now = (HAL_GetTick()/TICKS_PER_MS);
	static uint32_t				animation_phase = 0;
	static enum ongoingDisplays	display_cache = ONGOING_DISPLAY_NONE;
	int16_t oct;

	for (i = 0; i < NUM_BUTTONS; i++){

		if (ui_mode == PLAY){

			if (i<NUM_CHANNELS){

				if(led_cont.ongoing_display){

					if (params.osc_param_lock[i] && lock_flash_state())
						lock_brightness = 0;
					else
						lock_brightness = F_MAX_BRIGHTNESS;

					if (params.key_sw[i]==ksw_MUTE) {
						if (!params.note_on[i])
							lock_brightness = F_MAX_BRIGHTNESS - lock_brightness;
					}

					if ( (led_cont.ongoing_display == ONGOING_DISPLAY_SCALE)){
						set_rgb_color_brightness(&led_cont.button[i], qtz_scale_colors[ params.indiv_scale[i] ], lock_brightness);
					}

					else if( (led_cont.ongoing_display == ONGOING_DISPLAY_TRANSPOSE) ){
						set_rgb_color_by_array(&led_cont.button[i], CH_COLOR_MAP[i], lock_brightness);
					}

					else if( (led_cont.ongoing_display == ONGOING_DISPLAY_SPHERE_SEL) ){
						led_cont.button[i].brightness = lock_brightness*4.0;
						get_wt_color(params.wt_bank[i], &led_cont.button[i]);
					}

					else if( (led_cont.ongoing_display == ONGOING_DISPLAY_FINETUNE) ){
						set_rgb_color_by_array(&led_cont.button[i], CH_COLOR_MAP[i], lock_brightness);
					}

					else if ( led_cont.ongoing_display == ONGOING_DISPLAY_OCTAVE){
						oct = _CLAMP_I16(params.oct[i], MIN_OCT , MAX_OCT) - MIN_OCT;
						oct = _CLAMP_I16(oct, 0, NUM_LED_OUTRING-1);
						led_cont.button[i].c_red  	= led_cont.outring[OCT_OUTRING_MAP[oct]].c_red;
						led_cont.button[i].c_green 	= led_cont.outring[OCT_OUTRING_MAP[oct]].c_green;
						led_cont.button[i].c_blue  	= led_cont.outring[OCT_OUTRING_MAP[oct]].c_blue;
						led_cont.button[i].brightness  	= F_MAX_BRIGHTNESS * lock_brightness;
					}

					else if ( led_cont.ongoing_display == ONGOING_DISPLAY_LFO_TOVCA){
						if (lfos.to_vca[i]) {
							if ( display_cache != ONGOING_DISPLAY_LFO_TOVCA ){
								animation_phase = 0xFFFFFFFF - now + 1; //starting phase: results in tri_phase starting at 0
								display_cache = ONGOING_DISPLAY_LFO_TOVCA;
							}
							tri_period = LFO_TOVCA_FLASH_PERIOD;
							tri_phase = (now + animation_phase) % (tri_period*2);
							tri_wave = _FOLD_F(tri_phase, tri_period);
							brightness = (float)(tri_wave/tri_period);
							set_rgb_color_brightness(&led_cont.button[i], ledc_LIGHT_GREEN, brightness);
						}
						else
							set_rgb_color(&led_cont.button[i], ledc_DIM_YELLOW);
					}

					else if ( led_cont.ongoing_display == ONGOING_DISPLAY_LFO_MODE){
						if (params.key_sw[i] == ksw_MUTE) {
							if (display_cache != ONGOING_DISPLAY_LFO_MODE){
								display_cache =  ONGOING_DISPLAY_LFO_MODE;
								animation_phase = 0xFFFFFFFF - now + 1; //starting phase: results in tri_phase starting at 0
							}

							tri_period = LFO_MODE_FLASH_PERIOD;
							tri_phase = (now + animation_phase + ((tri_period*2*(NUM_CHANNELS-i))/NUM_CHANNELS)) % (tri_period*2);

							if (lfos.mode[i]==lfot_TRIG){
								brightness = (tri_phase < (tri_period/NUM_CHANNELS)) ? 1.0 : 0.05;
								color = ledc_AQUA;
							}
							else if (lfos.mode[i]==lfot_GATE){
								brightness = (tri_phase > tri_period) ? 1.0 : 0.0;
								color = ledc_LIGHT_BLUE;
							}
							else{ //lfot_SHAPE
								tri_wave = _FOLD_F(tri_phase, tri_period);
								brightness = (float)(tri_wave/tri_period);
								color = ledc_BLUE;
							}
							if ((animation_phase + now) < 250) brightness = 0;

						} else {
							brightness = 0.2;
							color = ledc_BLUE;
						}
						set_rgb_color_brightness(&led_cont.button[i], color, brightness);
					}
					else if ( led_cont.ongoing_display == ONGOING_DISPLAY_SELBUS ) {
						//nothing
					}
				}

				else { //no ongoing_display

					if (params.key_sw[i] == ksw_MUTE)
					{
						if (params.osc_param_lock[i] && lock_flash_state())
							brightness = 0;
						else
							brightness = F_MAX_BRIGHTNESS;

						if (!params.note_on[i])
							brightness = F_MAX_BRIGHTNESS - brightness;
					}
					else{

						if (params.osc_param_lock[i] && lock_flash_state())
							brightness = 0;
						else
							brightness = 0.30 + lfos.out_lpf[i]/2.0;
					}

					set_rgb_color_brightness(&led_cont.button[i], key_sw_mode_colors[params.key_sw[i]], brightness);
				}
			}

			//LFO buttons:
			else
			{
				if (led_cont.ongoing_display == ONGOING_DISPLAY_SELBUS) {
					color = (system_settings.selbus_can_recall == SELBUS_RECALL_ENABLED) ? ledc_MED_GREEN : ledc_DIM_GREEN;
					set_rgb_color(&led_cont.button[butm_LFOVCA_BUTTON], color);
					color = (system_settings.selbus_can_save == SELBUS_SAVE_ENABLED) ? ledc_MED_RED : ledc_DIM_RED;
					set_rgb_color(&led_cont.button[butm_LFOMODE_BUTTON], color);
				}
				else 
				{
					if (!calc_params.keymode_pressed)
					{
						//No channels are in mute mode ==> dim purple
						if ((params.key_sw[0] != ksw_MUTE) &&
							(params.key_sw[1] != ksw_MUTE) &&
							(params.key_sw[2] != ksw_MUTE) &&
							(params.key_sw[3] != ksw_MUTE) &&
							(params.key_sw[4] != ksw_MUTE) &&
							(params.key_sw[5] != ksw_MUTE) )					set_rgb_color_brightness(&led_cont.button[i], ledc_PURPLE, 0.1);
						else if (!button_pressed(i))							set_rgb_color(&led_cont.button[i], ledc_PINK);
						else if (calc_params.button_safe_release[i - NUM_CHANNELS])	set_rgb_color(&led_cont.button[i], ledc_CORAL);
						else													set_rgb_color(&led_cont.button[i], ledc_MED_BLUE);
					}
					else
					{
						if(calc_params.button_safe_release[i - NUM_CHANNELS])		set_rgb_color(&led_cont.button[i], ledc_CORAL);
						else													set_rgb_color(&led_cont.button[i], ledc_PURPLE);
					}
				}
			}

		} //if (ui_mode==PLAY)

		// WT RECORDING / EDITING
		else if (UIMODE_IS_WT_RECORDING_EDITING(ui_mode)){

			// mute
			if (i < NUM_CHANNELS){
				brightness = spherebuf.fx[i][(uint8_t)(calc_params.wt_pos[0][0])][(uint8_t)(calc_params.wt_pos[1][0])][(uint8_t)(calc_params.wt_pos[2][0])];
				brightness = exp_1voct_10_41V[_SCALE_F2U16(brightness, 0.0, 1.0, 2700, 4095)] / 1370.0;

				set_rgb_color_brightness(&led_cont.button[i], fx_colors[i], brightness);
			}

			// rec button
			else if (i==butm_LFOVCA_BUTTON){
				if (ui_mode==WTREC_WAIT)		brightness = ((HAL_GetTick()/TICKS_PER_MS) & 0x040) ? 1 : 0;
				else if (ui_mode==WTRECORDING) 	brightness = 1;
				else 							brightness = led_cont.flash_state;

				set_rgb_color_brightness(&led_cont.button[i], ledc_RED, brightness);
			}

			// monitor button
			else if (i==butm_LFOMODE_BUTTON){

				if (ui_mode==WTTTONE) 			color = ledc_PURPLE;
				else if (ui_mode==WTREC_WAIT) 	color = ledc_OFF;
				else 							color = ledc_GREEN;

				if (ui_mode == WTEDITING)
					brightness = led_cont.flash_state;
				else
					brightness = 1;

				set_rgb_color_brightness(&led_cont.button[i], color, brightness);
			}
		}

		else if (ui_mode==VOCT_CALIBRATE)
		{
			color = ledc_OFF;

			//Set the color of the channel buttons to the state of the Voct Calibration
			//(the Transpose calibration state is displayed on the led array)
			if (i<NUM_CHANNELS)
			{
				voct_state = get_voctcal_state(i);
				if (voct_state == VOCTCAL_READING_C1)		color = ledc_BLUE;
				if (voct_state == VOCTCAL_READING_C3)		color = ledc_RED;
				if (voct_state == VOCTCAL_CALIBRATED)		color = ledc_WHITE;
			}
			set_rgb_color(&led_cont.button[i], color);
		}
		set_pwm_led(led_button_map[i], &led_cont.button[i]);
	}
}

void update_encoder_leds(void){

	uint8_t i, color, set_color;

	if (led_cont.ongoing_display == ONGOING_DISPLAY_RECORD)
		color = ledc_FUSHIA;

	else if (UIMODE_IS_WT_RECORDING_EDITING(ui_mode))
		color = ledc_GOLD;

	else if (led_cont.ongoing_display == ONGOING_DISPLAY_TRANSPOSE)
		color = ledc_FUSHIA;

	else if (led_cont.ongoing_display == ONGOING_DISPLAY_FINETUNE)
		color = ledc_MED_BLUE;

	else
		color = ledc_PURPLE;

	for (i = ledrotm_DEPTH; i < NUM_LED_ROTARIES; i++)
	{
		if (color == ledc_PURPLE && rotary_pressed(i))
			set_color = ledc_CORAL;
		else
			set_color = color;

		set_rgb_color(&led_cont.encoder[i], set_color);
		set_pwm_led(led_rotary_map[i], &led_cont.encoder[i]);
	}
}

void update_mono_leds(void){
	uint8_t i;
	static uint32_t slider_pwm=0;
	float exp;

	// SLIDERS

	if (ui_mode==RGB_COLOR_ADJUST)
	{
		mono_led_off(mledm_SLIDER_A);
		mono_led_off(mledm_SLIDER_B);
		mono_led_off(mledm_SLIDER_C);
		mono_led_off(mledm_SLIDER_D);
		mono_led_off(mledm_SLIDER_E);
		mono_led_off(mledm_SLIDER_F);
	} else {

		if (slider_pwm-- == 0)
		{
			slider_pwm=32;		//32 steps of brightness

			for (i=0;i<NUM_CHANNELS;i++){
				mono_led_off(i);
			}
		}
		else{
			for (i=0;i<NUM_CHANNELS;i++) {
				uint32_t level = (uint32_t)calc_params.level[i];

				if (calc_params.adjusting_pan_state[i] == pan_CACHED_LEVEL && cached_param_flash_state())
					level = level < 3000 ? 4095 : 0;

				exp = exp_1voct_10_41V[level] * system_settings.global_brightness;
				if ((level > 50) && (exp > (slider_pwm*43))){
					mono_led_on(i);
				}
			}
		}
	}
}

void update_array_leds(void)
{
	uint8_t 	i;
	uint8_t		color;
	enum VoctCalStates voct_state;

	if (ui_mode != VOCT_CALIBRATE)
	{
		calculate_lfo_leds();
		calculate_lfocv_led();

	}
	else if (ui_mode == VOCT_CALIBRATE)
	{
		color = ledc_OFF;

		//Get voct calibration state of Transpose jack
		voct_state = get_voctcal_state(NUM_VOCT_CHANNELS-1);
		if (voct_state == VOCTCAL_READING_C1)		color = ledc_BLUE;
		if (voct_state == VOCTCAL_READING_C3)		color = ledc_RED;
		if (voct_state == VOCTCAL_CALIBRATED)		color = ledc_WHITE;

		//Set all array LEDs to the same color
		for (i = 0; i < NUM_CHANNELS+1; i++)
			set_rgb_color(&led_cont.array[i], color);
	}


	for (i=0; i<NUM_CHANNELS +1; i++)
	{
		set_pwm_led(ledstring_map[i], &led_cont.array[i]);
	}
}

void update_clockin_led(void)
{
	if (jack_plugged(CLK_SENSE)) {
		if (led_cont.waiting_for_clockin)
			set_single_pwm_led(singleledm_CLKIN, 0);
		if (lfos.use_ext_clock && lfos.cycle_pos[REF_CLK] < 0.5)
			set_single_pwm_led(singleledm_CLKIN, 1000);
		else
			set_single_pwm_led(singleledm_CLKIN, 0);
	}
	else if (system_settings.allow_bus_clock) {
		if (BUS_CLK())	set_single_pwm_led(singleledm_CLKIN, 100);
		else			set_single_pwm_led(singleledm_CLKIN, 0);
	}
	else
		set_single_pwm_led(singleledm_CLKIN, 0);
}

void update_audioin_led(void)
{

	o_rgb_led rgb;

	if (ui_mode == WTREC_WAIT)
		set_rgb_color_brightness(&rgb, ledc_RED, led_cont.flash_state);

	else if (jack_unplugged(WAVEFORMIN_SENSE))
		set_rgb_color(&rgb, ledc_OFF);

	else if (ui_mode == WTRECORDING)
		set_rgb_color(&rgb, ledc_RED);

	else if ((ui_mode == WTMONITORING) || (ui_mode == WTTTONE))
		set_rgb_color(&rgb, ledc_GREEN);

	else
		set_rgb_color(&rgb, ledc_OFF);

	set_pwm_led(ledm_AUDIOIN, &rgb);

}

void calculate_lfocv_led(void)
{
	if ( 	(params.key_sw[0] == ksw_MUTE) ||
			(params.key_sw[1] == ksw_MUTE) ||
			(params.key_sw[2] == ksw_MUTE) ||
			(params.key_sw[3] == ksw_MUTE) ||
			(params.key_sw[4] == ksw_MUTE) ||
			(params.key_sw[5] == ksw_MUTE)	){

		if (led_cont.waiting_for_clockin){
			led_cont.array[GLO_CLK].c_red 		= 1023 * (1.0 - led_cont.clockin_wait_progress);
			led_cont.array[GLO_CLK].c_green 	= 1023 * led_cont.clockin_wait_progress;
			led_cont.array[GLO_CLK].c_blue 		= 200;
		}
		else if (lfos.period[GLO_CLK] > F_LFO_AUDIO_RANGE_PERIOD_L){
			set_rgb_color(&led_cont.array[GLO_CLK], ledc_WHITE);
		}
		else{
			set_rgb_color(&led_cont.array[GLO_CLK], ledc_PURPLE);
		}
	}
	else{
		set_rgb_color(&led_cont.array[GLO_CLK], ledc_CORAL);
	}

	if (lfos.cycle_pos[GLO_CLK] < 0.5)
		led_cont.array[GLO_CLK].brightness = F_MAX_BRIGHTNESS;
	else
		led_cont.array[GLO_CLK].brightness = 0;
}

void calculate_lfo_leds(void)
{
	uint8_t chan=0;
	float brightness;

	for (chan = 0; chan < NUM_CHANNELS; chan++)
	{
		// Audio rate and shape selection- -> LFO static brightness
		if  ( (params.key_sw[chan] == ksw_MUTE) && (lfos.audio_mode[chan] || led_cont.lfoshape_timeout[chan]) ){
			brightness 	= lfos.gain[chan];
		}
		else
			brightness = lfos.out_lpf[chan];

		set_rgb_color_by_array(&led_cont.array[chan], LFO_BANK_COLOR[lfos.shape[chan]], brightness);
	}
}

void update_LED_rings(void)
{
	uint8_t i;

	calculate_led_ring();

	for (i = 0; i < NUM_LED_OUTRING; i++){
		set_pwm_led(led_outring_map[i], &led_cont.outring[i]);
	}

	for (i = 0; i < NUM_LED_INRING; i++){
		set_pwm_led(led_inring_map[i], &led_cont.inring[i]);
	}
}


void calculate_led_ring(void){
	uint8_t i;

	update_ongoing_display_timers();

	if (ui_mode == WTRECORDING) {
		display_wt_recbuff_fill_outring();
		display_wtpos_inring();
	}
	else if (ui_mode == WTREC_WAIT) {
		display_wt_rec_wait();
		display_wtpos_inring();
	}

	else if (UIMODE_IS_WT_RECORDING_EDITING(ui_mode)) {
		if (led_cont.ongoing_display == ONGOING_DISPLAY_FX) {
			display_fx();
		}
		else if (led_cont.ongoing_display == ONGOING_DISPLAY_SPHERE_SAVE) {
			display_sphere_save();
		}
		else if (led_cont.ongoing_display == ONGOING_DISPLAY_SPHERE_PLAYEXPORT) {
			display_sphere_play_export();
		}
		else {
			display_wt_recbuf_sel_outring();
			display_wtpos_inring();
		}
	}
	else if (ui_mode==VOCT_CALIBRATE) {
		turn_outring_off();

		for (i = 0; i < NUM_LED_INRING; i++){
			set_rgb_color(&led_cont.inring[i], ledc_OFF);
		}
	}
	else {
		switch (led_cont.ongoing_display)
		{
			case ONGOING_DISPLAY_TRANSPOSE:
				display_transpose();
				break;

			case ONGOING_DISPLAY_FINETUNE:
				display_finetune();
				break;

			case ONGOING_DISPLAY_OCTAVE:
				display_octave();
				break;

			case ONGOING_DISPLAY_PRESET:
				display_preset();
				break;

			case ONGOING_DISPLAY_SPHERE_SEL:
				display_sphere_sel();
				break;

			default:
				display_wt_pos();
				flash_wt_lock();
				break;
		}
	}
}

void turn_outring_off(void)
{
	uint8_t i;

	for (i =0; i< NUM_LED_OUTRING; i++){
		led_cont.outring[i].brightness 	= 0;
	}
}

void display_wtpos_inring(void)
{
	uint8_t 	i, j;
	uint16_t 	scaled_wt_pos[3];

	for ( i = 0 ; i < NUM_CHANNELS ; i++)
	{
		scaled_wt_pos[0] = _SCALE_F2U16(calc_params.wt_pos[0][i], 0, 2, 2048, 4095);
		scaled_wt_pos[1] = _SCALE_F2U16(calc_params.wt_pos[1][i], 0, 2, 2048, 3900);
		scaled_wt_pos[2] = _SCALE_F2U16(calc_params.wt_pos[2][i], 0, 2, 2048, 4095);

		j = rotate_origin(i, NUM_CHANNELS);
		led_cont.inring[j].c_red 		= 3 * exp_1voct_10_41V[scaled_wt_pos[0]];
		led_cont.inring[j].c_green 		= 	  exp_1voct_10_41V[scaled_wt_pos[1]];
		led_cont.inring[j].c_blue 		= 3 * exp_1voct_10_41V[scaled_wt_pos[2]];
		led_cont.inring[j].brightness 	= F_MAX_BRIGHTNESS;
	}
}

void flash_wt_lock(void)
{
	uint8_t chan, led;

	for ( chan = 0; chan < NUM_CHANNELS; chan++)
	{
		led = rotate_origin(chan, NUM_CHANNELS);
		if (params.osc_param_lock[chan] && lock_flash_state())
		{
			led_cont.outring[led * 3    ].brightness = 0;
			led_cont.outring[led * 3 + 1].brightness = 0;
			led_cont.outring[led * 3 + 2].brightness = 0;
			led_cont.inring[led].brightness = 0;
		}
	}
}

void get_wt_color(uint8_t wt_num, o_rgb_led *rgb)
{
	float fade, inv_fade;
	uint16_t scaled_wt_num;

	if (wt_num < NUM_FACTORY_SPHERES) {
		scaled_wt_num = _SCALE_U2U(wt_num, 0, NUM_FACTORY_SPHERES, 1024, 4095);
		fade = exp_1voct_10_41V[scaled_wt_num] / 1370.0;
		inv_fade = exp_1voct_10_41V[4095-scaled_wt_num] / 1370.0;

		rgb->c_red 		= (2048.0 * inv_fade) + 150.0;
		rgb->c_green  	= 0;
		rgb->c_blue 	= (200.0 * fade);
	}
	else if (wt_num < MAX_TOTAL_SPHERES) {
		scaled_wt_num = _SCALE_U2U((wt_num-NUM_FACTORY_SPHERES) % 18, 0, 17, 1024, 4095);
		//fade = exp_1voct_10_41V[scaled_wt_num] / 1370.0;
		inv_fade = exp_1voct_10_41V[4095-scaled_wt_num+1024] / 1370.0;
		fade = 1.0-inv_fade;

		if (wt_num < (NUM_FACTORY_SPHERES + 18*1)){
			rgb->c_red  	= (300.0 * fade)+0; //fade=.726 ->463
			rgb->c_green 	= (2048.0 * inv_fade) + 50; //inv fade = 0.0058 ->62
			rgb->c_blue 	= 0;
		}
		else if (wt_num < (NUM_FACTORY_SPHERES + 18*2)){
			rgb->c_red  	= 0;
			rgb->c_green 	= (100.0 * fade)+50;
			rgb->c_blue 	= (2048.0 * inv_fade) + 50;
		}
		else if (wt_num < (NUM_FACTORY_SPHERES + 18*3)){
			rgb->c_red 		= (2048.0 * inv_fade) + 150;
			rgb->c_green 	= (50.0 * fade)+0;
			rgb->c_blue 	= (2048.0 * inv_fade) + 50;
		}
		else if (wt_num < (NUM_FACTORY_SPHERES + 18*4)){
			rgb->c_red 		= (200.0 * fade);
			rgb->c_green 	= (2048.0 * inv_fade) + 50;
			rgb->c_blue 	= (2048.0 * inv_fade) + 50;
		}
		else if (wt_num < (NUM_FACTORY_SPHERES + 18*5)){
			rgb->c_red 		= (2800.0 * inv_fade) + 50;
			rgb->c_green 	= (1600.0 * inv_fade) + 50;
			rgb->c_blue 	= (100.0 * fade);
		}
		else{
			rgb->c_red 		= (2048.0 * inv_fade) + 50;
			rgb->c_green 	= (2048.0 * inv_fade) + 50;
			rgb->c_blue 	= (2048.0 * inv_fade) + 50;
		}
	}
	else {
		set_rgb_color(rgb, ledc_OFF);
	}
}

void display_wt_pos(void)
{
	uint8_t i, j;
	int8_t chan =-1;
	float folded_wt_pos;
	uint16_t scaled_wt_pos[3];

	for ( i = 0 ; i < NUM_LED_OUTRING ; i++)
	{
		if (!i || !(i%3))
		{
			chan++;
			folded_wt_pos 	 =_FOLD_F(calc_params.wt_pos[0][chan], 1.5);
			scaled_wt_pos[0] = _SCALE_F2U16(folded_wt_pos, 0, 1.5, 2048, 4095);

			folded_wt_pos 	 =_FOLD_F(calc_params.wt_pos[1][chan], 1.5);
			scaled_wt_pos[1] = _SCALE_F2U16(folded_wt_pos, 0, 1.5, 2048, 3900);

			folded_wt_pos 	 =_FOLD_F(calc_params.wt_pos[2][chan], 1.5);
			scaled_wt_pos[2] = _SCALE_F2U16(folded_wt_pos, 0, 1.5, 2048, 4095);
		}

		j = rotate_origin(i, NUM_LED_OUTRING);
		led_cont.outring[j].c_red 		= 3 * exp_1voct_10_41V[scaled_wt_pos[0]];
		led_cont.outring[j].c_green 	= 	  exp_1voct_10_41V[scaled_wt_pos[1]];
		led_cont.outring[j].c_blue 		= 3 * exp_1voct_10_41V[scaled_wt_pos[2]];
		led_cont.outring[j].brightness 	= F_MAX_BRIGHTNESS;
	}

	for ( i = 0; i < NUM_CHANNELS; i++)
	{
		j = rotate_origin(i, NUM_CHANNELS);
		led_cont.inring[j].brightness = F_MAX_BRIGHTNESS;
		get_wt_color(params.wt_bank[i], &led_cont.inring[j]);
	}
}

void display_firmware_version(void)
{
	uint8_t i, j;

	set_rgb_color(&led_cont.encoder[ledrotm_DEPTH], ledc_OFF);
	set_rgb_color(&led_cont.encoder[ledrotm_LATITUDE], ledc_OFF);
	set_rgb_color(&led_cont.encoder[ledrotm_LONGITUDE], ledc_OFF);

	set_pwm_led(led_rotary_map[ledrotm_DEPTH], &led_cont.encoder[ledrotm_DEPTH]);
	set_pwm_led(led_rotary_map[ledrotm_LATITUDE], &led_cont.encoder[ledrotm_LATITUDE]);
	set_pwm_led(led_rotary_map[ledrotm_LONGITUDE], &led_cont.encoder[ledrotm_LONGITUDE]);

	for (i=0; i<NUM_BUTTONS; i++) {
		set_rgb_color(&led_cont.button[i], ledc_OFF);
		set_pwm_led(led_button_map[i], &led_cont.button[i]);
	}

	for (i=0; i<NUM_LED_ARRAY; i++) {
		set_rgb_color(&led_cont.array[i], ledc_OFF);
		set_pwm_led(ledstring_map[i], &led_cont.array[i]);
	}

	for (i =0; i< NUM_LED_OUTRING; i++)
		set_rgb_color(&led_cont.outring[i], ledc_OFF);

	for (i=0; i<NUM_CHANNELS; i++)
		set_rgb_color(&led_cont.inring[i], ledc_OFF);

	if (system_calibrations->major_firmware_version > 0) {
		j = system_calibrations->major_firmware_version - 1;
		set_rgb_color(&led_cont.inring[j], ledc_RED);
	}

	j = system_calibrations->minor_firmware_version;
	// j = (i>=9) ? (i-9) : (i+9); //set bottom-left as origin
	set_rgb_color(&led_cont.outring[j], ledc_BLUE);

	for (i =0; i< NUM_LED_OUTRING; i++)
		set_pwm_led(led_outring_map[i], &led_cont.outring[i]);

	for (i=0; i<NUM_CHANNELS; i++)
		set_pwm_led(led_inring_map[i], &led_cont.inring[i]);

}

void display_transpose(void)
{
	uint8_t i, j, chan;
	int32_t t_transpose;
	int8_t transpose_pos[NUM_CHANNELS];
	uint32_t num_wraps;

	uint8_t overlap[NUM_LED_OUTRING][NUM_CHANNELS];
	uint8_t overlap_num[NUM_LED_OUTRING];
	static uint8_t overlap_ctr[NUM_LED_OUTRING]={0};

	uint8_t do_advance_overlap = 0;
	static uint32_t last_advance_overlap_tmr=0;
	uint32_t now = HAL_GetTick()/TICKS_PER_MS;

	if ((now - last_advance_overlap_tmr) > 300) {
		do_advance_overlap = 1;
		last_advance_overlap_tmr = now;
	}

	for (i = 0; i < NUM_LED_OUTRING; i++)
	{
		led_cont.outring[i].brightness = 0;
		overlap_num[i] = 0;
		for (chan=0; chan<NUM_CHANNELS; chan++)
			overlap[i][chan] = 99;
	}

	// Create overlap[led_position][channels_occupying_position] = channel#
	for (i = 0; i < NUM_CHANNELS; i++)
	{
		t_transpose = _CLAMP_I16(calc_params.transpose[i], MIN_TRANSPOSE_WRAP, MAX_TRANSPOSE_WRAP);
		transpose_pos[i] = (t_transpose - MIN_TRANSPOSE_WRAP)  % NUM_LED_OUTRING;

		j = rotate_origin(i, NUM_CHANNELS);

		if (params.osc_param_lock[i] && lock_flash_state() ){
			set_rgb_color(&led_cont.inring[j], ledc_OFF);
		} else {
			num_wraps = (uint32_t)((float)(t_transpose - transpose_pos[i] - MIN_TRANSPOSE_WRAP) / (float)(MAX_TRANSPOSE_WRAP - MIN_TRANSPOSE_WRAP) * 4096.0);
			set_rgb_color_by_array(&led_cont.inring[j], CH_COLOR_MAP[i], ((exp_1voct_10_41V[num_wraps] / 500.0) + 0.017) / F_MAX_BRIGHTNESS);
		}

		overlap[transpose_pos[i]][ overlap_num[transpose_pos[i]] ] = i;
		overlap_num[transpose_pos[i]]++;
	}

	for (i = 0; i<NUM_LED_OUTRING; i++)
	{
		if (do_advance_overlap) {
			overlap_ctr[i]++;
			if (overlap_ctr[i] >= overlap_num[i]) overlap_ctr[i]=0;
		}

		chan = overlap[i][overlap_ctr[i]];
		if (chan<=NUM_CHANNELS)
		{
			if (overlap_num[i]==1 && params.osc_param_lock[chan] && lock_flash_state() )
				set_rgb_color(&led_cont.outring[i], ledc_OFF);
			else
				set_rgb_color_by_array(&led_cont.outring[i], CH_COLOR_MAP[chan], 1.0);
		}

	}
}


void display_finetune (void)
{
	uint8_t i, j;
	int16_t t_finetune;
	uint32_t triangle, saw;
	uint32_t tm;
	uint32_t period;
	uint8_t	detune_pos_i;
	float brightness;

	tm = HAL_GetTick()/TICKS_PER_MS;

	for (i = 0; i < NUM_LED_OUTRING; i++)
		set_rgb_color(&led_cont.outring[i], ledc_OFF);

	for (i = 0; i < NUM_CHANNELS; i++)
	{
		//Inner ring: shows channel color (solid, full brightness = unlocked; flashing=locked)
		if (params.osc_param_lock[i] && lock_flash_state())
			brightness 	= 0.0;
		else
			brightness 	= F_MAX_BRIGHTNESS;

		j = rotate_origin(i, NUM_CHANNELS);
		set_rgb_color_by_array(&led_cont.inring[j], CH_COLOR_MAP[i], brightness);

		t_finetune = _CLAMP_I16(params.finetune[i], MIN_FINETUNE_WRAP, MAX_FINETUNE_WRAP);

		if (t_finetune==0) {
			detune_pos_i = j*3+1;
			set_rgb_color(&led_cont.outring[detune_pos_i], ledc_WHITE);
			// led_cont.outring[detune_pos_i].brightness = F_MAX_BRIGHTNESS*0.25; //dim tuned channels?
		}
		else {
			if (t_finetune<0) {
				period = _CLAMP_I16(1000+(t_finetune*8), 50, 1000);
				detune_pos_i = j*3;
				set_rgb_color(&led_cont.outring[detune_pos_i], ledc_BLUE);
			}
			else {
				period = _CLAMP_I16(1000-(t_finetune*8), 50, 1000);
				detune_pos_i = j*3+2;
				set_rgb_color(&led_cont.outring[detune_pos_i], ledc_RED);
			}

			//Calculate triangle wave to make LED fade faster as it gets more detuned
			saw = ((float)(tm % period)/(float)period) * 8191.0;
			triangle = (saw>4095) ? (8191-saw) : saw;

			led_cont.outring[detune_pos_i].brightness = (exp_1voct_10_41V[triangle] / 1367.0) + 0.03;
		}
	}
}

void display_fx(void)
{
	uint8_t slot_i, led;
	enum colorCodes led_color;

	for (slot_i = 0; slot_i < NUM_LED_OUTRING; slot_i++)
	{
		led = rotate_origin(slot_i, NUM_LED_OUTRING);
		led_color = animate_fx_level(slot_i);
		set_rgb_color(&led_cont.outring[led], led_color);
	}

}


void display_preset(void)
{
	uint8_t slot_i, bank_i, led;

	uint16_t hover_bank, preset_i;
	uint8_t slot_color = ledc_OFF;

	hover_bank = preset_mgr.hover_num / NUM_LED_OUTRING;

	for (slot_i = 0; slot_i < NUM_LED_OUTRING; slot_i++)
	{
		led = rotate_origin(slot_i, NUM_LED_OUTRING);
		preset_i = slot_i + (hover_bank*NUM_LED_OUTRING);
		slot_color = animate_preset_ledring(slot_i, preset_i);

		set_rgb_color(&led_cont.outring[led], slot_color);
	}

	for ( bank_i = 0; bank_i < NUM_CHANNELS; bank_i++)
	{
		led = rotate_origin(bank_i, NUM_CHANNELS);
		if (bank_i==hover_bank)	slot_color = ledc_WHITE;
		else					slot_color = ledc_OFF;

		set_rgb_color(&led_cont.inring[led], slot_color);
	}
}

void display_sphere_save(void)
{
	uint8_t slot_i, bank_i, led;

	uint16_t hover_bank;
	uint8_t slot_color = ledc_OFF;

	uint8_t hover_slot = get_sphere_hover();

	if (hover_slot >= NUM_FACTORY_SPHERES)
		hover_bank = (hover_slot - NUM_FACTORY_SPHERES) / NUM_LED_OUTRING;
	else
		hover_bank = 0xFF;

	for (slot_i = 0; slot_i < NUM_LED_OUTRING; slot_i++)
	{
		led = rotate_origin(slot_i, NUM_LED_OUTRING);
		animate_wt_saving_ledring(slot_i, &led_cont.outring[led]);
	}
	for ( bank_i = 0; bank_i < NUM_CHANNELS; bank_i++)
	{
		led = rotate_origin(bank_i, NUM_CHANNELS);

		if (hover_bank>=NUM_CHANNELS) 	slot_color = ledc_WHITE;
		else if (bank_i==hover_bank)	slot_color = ledc_WHITE;
		else							slot_color = ledc_OFF;

		set_rgb_color(&led_cont.inring[led], slot_color);
	}
}

void display_sphere_play_export(void)
{
	uint8_t slot_i, bank_i;//, led;

	for (slot_i = 0; slot_i < NUM_LED_OUTRING; slot_i++)
	{
		// led = rotate_origin(slot_i, NUM_LED_OUTRING);
		animate_play_export_ledring(slot_i, &led_cont.outring[slot_i]);
	}
	for ( bank_i = 0; bank_i < NUM_CHANNELS; bank_i++)
	{
		set_rgb_color(&led_cont.inring[bank_i], ledc_OFF);
	}

}
void display_sphere_sel(void)
{
	uint8_t i, led, chan;

	uint8_t overlap[NUM_LED_OUTRING][NUM_CHANNELS];
	uint8_t overlap_num[NUM_LED_OUTRING];
	static uint8_t overlap_ctr[NUM_LED_OUTRING]={0};
	uint8_t pos[NUM_CHANNELS];

	uint8_t do_advance_overlap = 0;
	static uint32_t last_advance_overlap_tmr=0;
	uint32_t now = HAL_GetTick()/TICKS_PER_MS;

	if ((now - last_advance_overlap_tmr) > 300)
	{
		do_advance_overlap = 1;
		last_advance_overlap_tmr = now;
	}

	for (i = 0; i < NUM_LED_OUTRING; i++){
		led_cont.outring[i].brightness = 0;
		overlap_num[i] = 0;
		for (chan=0; chan<NUM_CHANNELS; chan++)
			overlap[i][chan] = 99;
	}

	// Create overlap[led_position][channels_occupying_position] = channel#
	for (i = 0; i < NUM_CHANNELS; i++)
	{
		pos[i] = rotate_origin(calc_params.wtsel[i] % NUM_LED_OUTRING, NUM_LED_OUTRING);
		led = rotate_origin(i, NUM_CHANNELS);

		get_wt_color(params.wt_bank[i], &led_cont.inring[led]);

		if (params.osc_param_lock[i] && lock_flash_state())
			led_cont.inring[led].brightness = 0;
		else
			led_cont.inring[led].brightness = F_MAX_BRIGHTNESS;

		overlap[ pos[i] ][ overlap_num[pos[i]] ] = i;
		overlap_num[ pos[i] ]++;
	}

	for (i = 0; i<NUM_LED_OUTRING; i++)
	{
		if (do_advance_overlap) {
			overlap_ctr[i]++;
			if (overlap_ctr[i] >= overlap_num[i]) overlap_ctr[i]=0;
		}

		chan = overlap[i][overlap_ctr[i]];
		if (chan<=NUM_CHANNELS)  {
			get_wt_color(params.wt_bank[chan], &led_cont.outring[i]);

			if (params.osc_param_lock[chan] && lock_flash_state())
				led_cont.outring[i].brightness = 0;
			else
				led_cont.outring[i].brightness = F_MAX_BRIGHTNESS;
		}
	}
}

void display_octave(void)
{
	uint8_t i,j;
	int16_t oct;

	for (i = 0; i < NUM_LED_OUTRING; i++) {
		led_cont.outring[i].brightness 	= 0;
		led_cont.outring[i].c_red  		= 4032  - 4032 * (OCT_OUTRING_MAP[i])  / 18;
		led_cont.outring[i].c_green 	= 3800 * (OCT_OUTRING_MAP[i]) / 18;
		led_cont.outring[i].c_blue  	= 0;
	}

	// Light up LED ring positions corresponding to current indiv oct
	for (i=0; i<NUM_CHANNELS; i++){
		oct = _CLAMP_I16(params.oct[i], MIN_OCT , MAX_OCT) - MIN_OCT;
		oct = _CLAMP_I16(oct, 0, NUM_LED_OUTRING-1);

		j = rotate_origin(i, NUM_CHANNELS);
		set_rgb_color_by_rgb(&led_cont.inring[j], &led_cont.outring[OCT_OUTRING_MAP[oct]]);

		// flash locked channels
		if (led_cont.flash_state && params.osc_param_lock[i]){
			led_cont.outring[OCT_OUTRING_MAP[oct]].brightness = 0;
			led_cont.inring[j].brightness = 0;
		}
		else
		{
			if(macro_states.all_af_buttons_released) {
				led_cont.outring[OCT_OUTRING_MAP[oct]].brightness = F_MAX_BRIGHTNESS;
				led_cont.inring[j].brightness = F_MAX_BRIGHTNESS;
			}
			else
			{
				if (button_pressed(i) || (led_cont.ongoing_display == ONGOING_DISPLAY_FINETUNE)){ //Todo: What is FINETUNE doing here?
					led_cont.outring[OCT_OUTRING_MAP[oct]].brightness = F_MAX_BRIGHTNESS;
					led_cont.inring[j].brightness = F_MAX_BRIGHTNESS;
				}
				// idle channels are grey-ed
				else if (!button_pressed(i)){
					led_cont.outring[OCT_OUTRING_MAP[oct]].brightness = 0.1;
					led_cont.inring[j].brightness = 0.1;
				}
			}
		}
	}
}

void update_ongoing_display_timers(void){

	static uint32_t 	last_systick = 0;
	uint32_t 			elapsed_ticks;
	uint8_t				tick_down=0;
	uint32_t 			now = HAL_GetTick()/TICKS_PER_MS;

	elapsed_ticks = now - last_systick;
	last_systick = now;

	if (led_cont.ongoing_display == ONGOING_DISPLAY_NONE)
		return;

	if ( led_cont.ongoing_display == ONGOING_DISPLAY_OSC_PARAM_LOCK)
		tick_down = 1;

	else if ( led_cont.ongoing_display == ONGOING_DISPLAY_OCTAVE &&  macro_states.all_af_buttons_released )
		tick_down = 1;

	else if ( led_cont.ongoing_display == ONGOING_DISPLAY_SCALE && macro_states.all_af_buttons_released && !rotary_pressed(rotm_OCT) && !switch_pressed(FINE_BUTTON) )
		tick_down = 1;

	else if (led_cont.ongoing_display == ONGOING_DISPLAY_TRANSPOSE && macro_states.all_af_buttons_released && !rotary_pressed(rotm_TRANSPOSE) )
		tick_down = 1;

	else if (led_cont.ongoing_display == ONGOING_DISPLAY_FINETUNE && macro_states.all_af_buttons_released && !rotary_pressed(rotm_TRANSPOSE) && !switch_pressed(FINE_BUTTON) )
		tick_down = 1;

	else if (led_cont.ongoing_display == ONGOING_DISPLAY_LFO_TOVCA)
		tick_down = 1;

	else if (led_cont.ongoing_display == ONGOING_DISPLAY_LFO_MODE)
		tick_down = 1;

	else if (led_cont.ongoing_display == ONGOING_DISPLAY_SPHERE_SEL)
		tick_down = 1;

	else if ( led_cont.ongoing_display == ONGOING_DISPLAY_FX &&  macro_states.all_af_buttons_released )
		tick_down = 1;

	else if ((led_cont.ongoing_display == ONGOING_DISPLAY_PRESET) && rotary_released(rotm_PRESET))
		tick_down = 1;

	else if ((led_cont.ongoing_display == ONGOING_DISPLAY_SPHERE_SAVE) && rotary_released(rotm_PRESET))
		tick_down = 1;

	else if ((led_cont.ongoing_display == ONGOING_DISPLAY_GLOBRIGHT) && rotary_released(rotm_PRESET))
		tick_down = 1;
	
	else if (led_cont.ongoing_display == ONGOING_DISPLAY_SELBUS)
		tick_down = 1;

	if (!tick_down)
		return;

	if (led_cont.ongoing_timeout)
	{
		if (led_cont.ongoing_timeout < elapsed_ticks)
			led_cont.ongoing_timeout = 0;
		else
			led_cont.ongoing_timeout -= elapsed_ticks;
	}

	if (!led_cont.ongoing_timeout)
	{
		led_cont.ongoing_display 	= ONGOING_DISPLAY_NONE;
		led_cont.ongoing_timeout 	= 0;
	}
}

void start_ongoing_display_finetune(void){
	led_cont.ongoing_display 	= ONGOING_DISPLAY_FINETUNE;
	led_cont.ongoing_timeout  	= FINETUNE_TIMER_LIMIT;
}

void start_ongoing_display_octave(void){
	led_cont.ongoing_display 	= ONGOING_DISPLAY_OCTAVE;
	led_cont.ongoing_timeout	= OCTAVE_TIMER_LIMIT;
}

void start_ongoing_display_scale(void){
	led_cont.ongoing_display 	= ONGOING_DISPLAY_SCALE;
	led_cont.ongoing_timeout 	= SCALE_TIMER_LIMIT;
}

void start_ongoing_display_transpose(void){
	led_cont.ongoing_display	= ONGOING_DISPLAY_TRANSPOSE;
	led_cont.ongoing_timeout  	= TRANSPOSE_TIMER_LIMIT;
}

void start_ongoing_display_lfo_tovca(void){
	led_cont.ongoing_display = ONGOING_DISPLAY_LFO_TOVCA;
	led_cont.ongoing_timeout = LFO_TOVCA_TIMER_LIMIT;
}

void start_ongoing_display_lfo_mode(void){
	led_cont.ongoing_display = ONGOING_DISPLAY_LFO_MODE;
	led_cont.ongoing_timeout = LFO_MODE_TIMER_LIMIT;
}

void start_ongoing_display_preset(void)
{
	if(led_cont.ongoing_display != ONGOING_DISPLAY_GLOBRIGHT){
		led_cont.ongoing_display 	= ONGOING_DISPLAY_PRESET;
		led_cont.ongoing_timeout 	= PRESET_TIMER_LIMIT;
	}
}

void start_ongoing_display_fx(void){
	led_cont.ongoing_display = ONGOING_DISPLAY_FX;
	led_cont.ongoing_timeout = FX_TIMER_LIMIT;
}

void start_ongoing_display_sphere_save(void){
	led_cont.ongoing_display = ONGOING_DISPLAY_SPHERE_SAVE;
	led_cont.ongoing_timeout = SPHERE_SAVE_TIMER_LIMIT;
}
void start_ongoing_display_sphere_play_export(void){
	led_cont.ongoing_display = ONGOING_DISPLAY_SPHERE_PLAYEXPORT;
	led_cont.ongoing_timeout = SPHERE_SAVE_TIMER_LIMIT;
}

void start_ongoing_display_globright(void){
	led_cont.ongoing_display = ONGOING_DISPLAY_GLOBRIGHT;
	led_cont.ongoing_timeout = GLOBRIGHT_TIMER_LIMIT;
}

void start_ongoing_display_sphere_sel(void){
	led_cont.ongoing_display = ONGOING_DISPLAY_SPHERE_SEL;
	led_cont.ongoing_timeout = SPHERE_SEL_TIMER_LIMIT;
}

void start_ongoing_display_selbus(void) {
	led_cont.ongoing_display = ONGOING_DISPLAY_SELBUS;
	led_cont.ongoing_timeout = PRESET_TIMER_LIMIT;
}

void stop_all_displays(void){
	led_cont.ongoing_display = ONGOING_DISPLAY_NONE;
	led_cont.ongoing_timeout = 0;
}

uint16_t return_display_timer(void){
	return led_cont.ongoing_timeout;
}
