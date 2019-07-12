/*
 * params_wt_browse.c - Multi-dimentional Wavetable Browse
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


#include "params_wt_browse.h"
#include "params_changes.h"
#include "UI_conditioning.h"
#include "key_combos.h"
#include "gpio_pins.h"
#include "ui_modes.h" // UIMODE_IS_WT_RECORDING_EDITING
#include "params_update.h"
#include "params_lfo.h"

extern o_params params;
extern o_calc_params calc_params;
extern o_lfos   lfos;
extern enum UI_Modes ui_mode;
extern 	o_analog 	analog[NUM_ANALOG_ELEMENTS];
extern 	o_macro_states macro_states;
extern	o_button 	button[NUM_BUTTONS];
extern	o_switch 	hwSwitch[NUM_SWITCHES];						//switch position = note when mutes_sw and keys_sw state is both off

const float F_SCALING_BROWSE 			= 0.030;
const float F_SCALING_FINE_BROWSE 		= 0.005;
const float F_SCALING_BROWSE_FADE_STEP  = 0.010;
const float VELOCITY 					= 40.0;

const float BROWSE_TABLE[ NUM_WAVEFORMS_IN_SPHERE ][ NUM_WT_DIMENSIONS ] =
{
	{0.0,0.0,0.0},
	{0.0,0.0,1.0},
	{0.0,0.0,2.0},

	{0.0,1.0,2.0},
	{0.0,1.0,1.0},
	{0.0,1.0,0.0},

	{0.0,2.0,0.0},
	{0.0,2.0,1.0},
	{0.0,2.0,2.0},

	{1.0,2.0,2.0},
	{1.0,2.0,1.0},
	{1.0,2.0,0.0},

	{1.0,1.0,0.0},
	{1.0,1.0,1.0},
	{1.0,1.0,2.0},

	{1.0,0.0,2.0},
	{1.0,0.0,1.0},
	{1.0,0.0,0.0},

	{2.0,0.0,0.0},
	{2.0,0.0,1.0},
	{2.0,0.0,2.0},

	{2.0,1.0,2.0},
	{2.0,1.0,1.0},
	{2.0,1.0,0.0},
	
	{2.0,2.0,0.0},
	{2.0,2.0,1.0},
	{2.0,2.0,2.0}
};


void get_browse_nav(float browse_pos, float *x, float *y, float *z)
{
	uint8_t fade0_i, fade1_i;
	float fade_f;

	//browse_pos = _WRAP_F(browse_pos, 0, 27);
	while(browse_pos>=NUM_WAVEFORMS_IN_SPHERE) 	browse_pos -= NUM_WAVEFORMS_IN_SPHERE;
	while(browse_pos<0.0) 						browse_pos += NUM_WAVEFORMS_IN_SPHERE;

	fade0_i = (uint8_t)browse_pos;
	if (fade0_i==26) 	fade1_i = 0; 
	else 				fade1_i = fade0_i + 1;

	fade_f = browse_pos - fade0_i;

	if (fade_f > 0) //need to cross-fade
	{
		*x = (BROWSE_TABLE[fade0_i][0] * (1.0-fade_f)) + (BROWSE_TABLE[fade1_i][0] * fade_f);
		*y = (BROWSE_TABLE[fade0_i][1] * (1.0-fade_f)) + (BROWSE_TABLE[fade1_i][1] * fade_f);
		*z = (BROWSE_TABLE[fade0_i][2] * (1.0-fade_f)) + (BROWSE_TABLE[fade1_i][2] * fade_f);
	}
	else
	{
		*x = BROWSE_TABLE[fade0_i][0];
		*y = BROWSE_TABLE[fade0_i][1];
		*z = BROWSE_TABLE[fade0_i][2];
	}
}

uint8_t get_browse_index(uint8_t x, uint8_t y, uint8_t z)
{
	uint8_t i;
	for (i=0; i<NUM_WAVEFORMS_IN_SPHERE; i++)
	{
		if (BROWSE_TABLE[i][0]==x && BROWSE_TABLE[i][1]==y && BROWSE_TABLE[i][2]==z)
			break;
	}
	if (i<NUM_WAVEFORMS_IN_SPHERE)
		return i;
	else
		return 0;
}


static float wbrowse_dest[NUM_CHANNELS]={0};

void bypass_wbrowse_morph(void)
{
	uint8_t chan;

	for (chan=0; chan<NUM_CHANNELS; chan++)
		params.wt_browse_step_pos_enc[chan] = wbrowse_dest[chan];
}

void reset_wbrowse_morph(uint8_t chan)
{
	wbrowse_dest[chan]  = params.wt_browse_step_pos_enc[chan];
}

void init_wbrowse_morph(void)
{
	uint8_t chan;

	for (chan=0; chan<NUM_CHANNELS; chan++)
		wbrowse_dest[chan]  = params.wt_browse_step_pos_enc[chan];
}


void update_wbrowse_step_pos(uint8_t chan)
{
	float wrapped_step = _WRAP_F(params.wt_browse_step_pos_enc[chan], 0, 27);
	float wrapped_dest = _WRAP_F(wbrowse_dest[chan], 0, 27);

	if (fabs(wrapped_step - wrapped_dest)<0.0001) 
	{
		params.wt_browse_step_pos_enc[chan] = wbrowse_dest[chan];
		return;
	}

	//Move wt_browse_step_pos_enc[] in the correct direction towards wbrowse_dest[]
	else if (params.wt_browse_step_pos_enc[chan] < wbrowse_dest[chan])
	{
		params.wt_browse_step_pos_enc[chan] += F_SCALING_BROWSE_FADE_STEP;
		if (params.wt_browse_step_pos_enc[chan] > wbrowse_dest[chan])
			params.wt_browse_step_pos_enc[chan] = wbrowse_dest[chan];
	}
	else if (params.wt_browse_step_pos_enc[chan] > wbrowse_dest[chan])
	{
		params.wt_browse_step_pos_enc[chan] -= F_SCALING_BROWSE_FADE_STEP;
		if (params.wt_browse_step_pos_enc[chan] < wbrowse_dest[chan])
			params.wt_browse_step_pos_enc[chan] = wbrowse_dest[chan];
	}
}


void update_wbrowse(int16_t inc)
{	
	uint32_t time_since;
	float new_inc; 
	static uint32_t last_inc_time=0;
	static int8_t last_inc;
	uint32_t now = (HAL_GetTick()/TICKS_PER_MS);

	if (!inc)
		return;

	//No LPF in WTREC modes (unless Fine is pressed)
	if(UIMODE_IS_WT_RECORDING_EDITING(ui_mode) && !switch_pressed(FINE_BUTTON)){
		new_inc = inc;
		last_inc_time = 0;
	}
	//Normal browsing (no Fine pressed)
	//In WTREC modes, Fine+Browse is same as not pressing Fine in normal mode
	else if(!switch_pressed(FINE_BUTTON) || (UIMODE_IS_WT_RECORDING_EDITING(ui_mode) && switch_pressed(FINE_BUTTON)))
	{

		if ((last_inc * inc) < 0 || last_inc_time==0) //different sign = changed direction
			new_inc = inc * F_SCALING_BROWSE;
		else {
			time_since = (now-last_inc_time);
			if (time_since > VELOCITY*abs(inc)) 
				time_since = VELOCITY*abs(inc);
			else if (time_since==0)
				time_since = 1;
			new_inc = inc*F_SCALING_BROWSE*VELOCITY/(float)time_since;
		}

		last_inc_time = now;
	}
	//Fine Browsing
	else {
		new_inc = inc * F_SCALING_FINE_BROWSE;
		last_inc_time = 0;
	}
	

	if (new_inc)
	{
		change_param_f(wbrowse_dest, new_inc);
	}

	if (UIMODE_IS_WT_RECORDING_EDITING(ui_mode) && !switch_pressed(FINE_BUTTON)) {
//			params.disppatt_enc = 0;
			params.dispersion_enc = 0;
			update_wt_disp(CLEAR_LPF);
			bypass_wbrowse_morph();
	}
}

void update_wbrowse_cv(void)
{
	params.wt_browse_step_pos_cv = ((float)analog[WBROWSE_CV].bracketed_val)/(4095.0/27.0);
}
