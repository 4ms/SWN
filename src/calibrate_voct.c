/*
 * calibrate_voct.c - calibration of V/oct jacks
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

#include "calibrate_voct.h"
#include "ui_modes.h"
#include "UI_conditioning.h"
#include "analog_conditioning.h"
#include "params_update.h"
#include "params_lfo.h"

#include "math.h"
#include "math_util.h"

enum VoctCalStates	cal_state[NUM_VOCT_CHANNELS];
float	C1_adc[NUM_VOCT_CHANNELS];
float	C3_adc[NUM_VOCT_CHANNELS];
uint8_t	first_calibration = 1;

static float adc_lpf[NUM_VOCT_CHANNELS][VOCTCAL_LPF_SIZE];

extern enum 			UI_Modes ui_mode;
extern SystemCalibrations *system_calibrations;
extern o_analog 	analog[NUM_ANALOG_ELEMENTS];
extern o_params params;
extern o_lfos lfos;

//Private:
uint8_t range_check_offset(float offset);
uint8_t range_check_tracking(float tracking);
uint8_t calculate_cal_values(uint8_t chan);

static uint8_t note_on_cache[NUM_CHANNELS];
static uint8_t to_vca_cache[NUM_CHANNELS];
static enum MuteNoteKeyStates key_sw_cache[NUM_CHANNELS];

void enter_voct_calibrate_mode(void)
{
	uint8_t chan;
	uint32_t i;

	for (chan=0; chan<(NUM_VOCT_CHANNELS); chan++)
	{
		cal_state[chan] = VOCTCAL_READING_C1;
		C1_adc[chan] = 0;
		C3_adc[chan] = 0;

		//initialize LPF to current adc value
		for (i=0;i<VOCTCAL_LPF_SIZE;i++)
			adc_lpf[chan][i] = analog[A_VOCT+chan].lpf_val; 
	}

	cal_state[NUM_VOCT_CHANNELS-1] = VOCTCAL_NOT_CALIBRATED;

	//cache all pitch parameters and set them to 1.0 (no xposition, no fine tune, etc)
	cache_uncache_pitch_params(CACHE);
	init_pitch_params();

	for (chan=0; chan<NUM_CHANNELS; chan++) {
		note_on_cache[chan] = params.note_on[chan];
		to_vca_cache[chan] = lfos.to_vca[chan];
		key_sw_cache[chan] = params.key_sw[chan];

		params.note_on[chan] = 1;
		lfos.to_vca[chan] = 0;
		params.key_sw[chan] = ksw_MUTE;
	}


	for (chan=0;chan<NUM_CHANNELS;chan++)
		params.oct[chan] = INIT_OCT-1;

	first_calibration = 1;
	ui_mode = VOCT_CALIBRATE;
}

void save_exit_voct_calibrate_mode(void)
{
	uint8_t chan;

	//Save to flash
	save_flash_params();

	//uncache all pitch parameters
	cache_uncache_pitch_params(UNCACHE);

	for (chan=0; chan<NUM_CHANNELS; chan++) {
		params.note_on[chan] = note_on_cache[chan];
		lfos.to_vca[chan] = to_vca_cache[chan];
		params.key_sw[chan] = key_sw_cache[chan];
	}

	ui_mode = PLAY;
}

void cancel_voct_calibrate_mode(void)
{
	uint8_t chan;

	//uncache all pitch parameters
	//
	cache_uncache_pitch_params(UNCACHE);
	
	for (chan=0; chan<NUM_CHANNELS; chan++) {
		params.note_on[chan] = note_on_cache[chan];
		lfos.to_vca[chan] = to_vca_cache[chan];
		params.key_sw[chan] = key_sw_cache[chan];
	}

	ui_mode = PLAY;
}


void process_voct_calibrate_mode(void)
{
	uint8_t 				chan, c;
	enum PressTypes 		button_state;
	static enum PressTypes 	last_button_state[NUM_VOCT_CHANNELS]={RELEASED};
	static uint32_t			lpf_i=0;

	for (chan=0; chan<NUM_VOCT_CHANNELS; chan++)
	{
		//Record ADC value in the LPF buffer
		//Note: we're lucky that in the analog[] array, the channel after F_VOCT happens to be TRANSPOSE_CV!
		adc_lpf[A_VOCT+chan][lpf_i] = analog[A_VOCT + chan].lpf_val;


		//Check if button was just released
		if (chan<NUM_CHANNELS)
			button_state = button_pressed(butm_A_BUTTON + chan);
		else 
			//transpose jack has no channel button so it's selected by the TRANSPOSE rotary press
			button_state = rotary_pressed(rotm_TRANSPOSE);

		if (button_state==RELEASED && last_button_state[chan]>=PRESSED)
		{
			if (cal_state[chan]==VOCTCAL_READING_C3) 	
			{
				C3_adc[chan] = _AVERAGE_EXCL_MINMAX_F(adc_lpf[A_VOCT+chan], VOCTCAL_LPF_SIZE);

				cal_state[chan] = calculate_cal_values(chan) ? VOCTCAL_CALIBRATED : VOCTCAL_RANGE_ERROR;

				if (chan==0 && first_calibration && cal_state[chan]==VOCTCAL_CALIBRATED) 	
				{
					// If we calibrate channel A first before anything else, then
					// automatically copy calibration to all channels
					for (c=1;c<NUM_VOCT_CHANNELS;c++)
					{
						system_calibrations->voct_tracking_adjustment[c] = system_calibrations->voct_tracking_adjustment[0];
						system_calibrations->voct_offset_adjustment[c] = system_calibrations->voct_offset_adjustment[0];
						cal_state[c] = VOCTCAL_CALIBRATED;
					}
				}
				first_calibration = 0;
			}

			else if (cal_state[chan]==VOCTCAL_READING_C1) 
			{
				C1_adc[chan] = _AVERAGE_EXCL_MINMAX_F(adc_lpf[A_VOCT+chan], VOCTCAL_LPF_SIZE);

				cal_state[chan]=VOCTCAL_READING_C3;
			}

			else if (cal_state[chan]==VOCTCAL_NOT_CALIBRATED)
				cal_state[chan]=VOCTCAL_READING_C1;

			else if (cal_state[chan]==VOCTCAL_RANGE_ERROR)
				cal_state[chan]=VOCTCAL_READING_C1;
		}
		last_button_state[chan] = button_state;
	}

	lpf_i++;
	if (lpf_i==VOCTCAL_LPF_SIZE) lpf_i=0;
}


uint8_t calculate_cal_values(uint8_t chan)
{
	float	tracking, offset;

	//Range in ADC values = 4096.0
	//Range in volts of our lookup table = 10.42
	//Octave difference to track = 2.0
	tracking = (2.0*(4096.0/10.41974052)) / (C3_adc[chan] - C1_adc[chan]);

	//C3 - C1 is two octaves, so find what one octave is and that's our offset from C1 to C0
	if (analog[A_VOCT + chan].polarity == AP_BIPOLAR)
		offset = C1_adc[chan] - ((C3_adc[chan] - C1_adc[chan])/2) - 2048;
	else
		offset = C1_adc[chan] - ((C3_adc[chan] - C1_adc[chan])/2);

	if (range_check_tracking(tracking) && range_check_offset(offset))
	{
		system_calibrations->voct_tracking_adjustment[chan] = tracking;
		system_calibrations->voct_offset_adjustment[chan] = offset;
		return 1;
	}
	else
		return 0;
}

enum VoctCalStates get_voctcal_state(uint8_t chan)
{
	if (chan<NUM_VOCT_CHANNELS)	return cal_state[chan];
	else return VOCTCAL_NOT_CALIBRATED;
}

float apply_voct_calibration(float adc_val, uint8_t chan)
{
	float corrected_adc_val;

	corrected_adc_val = ((float)adc_val - system_calibrations->voct_offset_adjustment[chan]) * system_calibrations->voct_tracking_adjustment[chan];

	// corrected_adc_val = ((float)adc_val * system_calibrations->voct_tracking_adjustment[chan]) - system_calibrations->voct_offset_adjustment[chan];
	return _CLAMP_F((uint16_t)corrected_adc_val, 0.0, 4095.0);
}

//
// Flash memory functions:
//
void set_default_voct_calibrate(void)
{
	uint8_t i;

	for (i=0; i<(NUM_VOCT_CHANNELS); i++)
	{
		//values found empirically
		system_calibrations->voct_tracking_adjustment[i] = 0.95719;
		system_calibrations->voct_offset_adjustment[i] 	 = 0;
	}
}

uint8_t range_check_tracking(float tracking)
{
	if (isnan(tracking) || (tracking > 4.0) || (tracking < 0.25)) return 0;
	else return 1;
}
uint8_t range_check_offset(float offset)
{
	if (isnan(offset) || (offset > 2000.0) || (offset < -2000.0)) return 0;
	else return 1;
}



uint8_t range_check_calibrate_voct_values(SystemCalibrations *sys_cal)
{
	uint8_t i;
	uint8_t range_errors=0;

	for (i=0; i<(NUM_VOCT_CHANNELS); i++)
	{
		if (isnan(sys_cal->voct_tracking_adjustment[i]) || (sys_cal->voct_tracking_adjustment[i] > 4.0) || (sys_cal->voct_tracking_adjustment[i] < 0.25))
		{
			sys_cal->voct_tracking_adjustment[i] = 1.0;
			range_errors++;
		}

		if (isnan(sys_cal->voct_offset_adjustment[i]) || (sys_cal->voct_offset_adjustment[i] > 2000.0) || (sys_cal->voct_offset_adjustment[i] < -2000.0))
		{
			sys_cal->voct_offset_adjustment[i] = 0.0;
			range_errors++;
		}
	}
	return range_errors;
}
