/*
 * wavetable_effects.c
 *
 * Author: Dan Green (danngreen1@gmail.com),  Hugo Paris (hugoplho@gmail.com)
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

#include <stm32f7xx.h>
#include <math.h>
#include "globals.h"
#include "wavetable_effects.h"
#include "wavetable_editing.h"
#include "math_util.h"
#include "hardware_controls.h"
#include "UI_conditioning.h"
#include "params_update.h"
#include "led_cont.h"
#include "fft_filter.h"


// Displays
const enum colorCodes fx_colors[NUM_FX] = {	
	ledc_RED,		//WAVEFOLD
	ledc_PINK,		//DECIMATE
	ledc_PURPLE,	//METALIZE
	ledc_FUSHIA,	//DISTORT	
	ledc_YELLOW,	//NORMALIZE
	ledc_GOLD		//SMOOTH
};

const float FX_SCALING[NUM_FX] = {
	0.015, 			//WAVEFOLD
	0.015, 			//DECIMATE
	0.015, 			//METALIZE
	0.015, 			//DISTORT	
	0.025, 			//NORMALIZE
	0.1 			//SMOOTH
};
const float FX_FINE_SCALING[NUM_FX] = {
	0.0015, 		//WAVEFOLD
	0.0015, 		//DECIMATE
	0.0015, 		//METALIZE
	0.0015, 		//DISTORT	
	0.0025, 		//NORMALIZE
	0.01 			//SMOOTH
};

extern SRAM1DATA 	o_spherebuf 			spherebuf;
extern o_params params;
extern o_calc_params calc_params;


void overlap_smooth_wave(int16_t *in, int16_t *out, uint32_t in_size, uint32_t out_size){

	uint32_t i;
	uint32_t overlap_size;
	float xfade;

	//invalid case, just pad with zeros
	if (in_size < out_size) {
		for (i=0; i<in_size; i++)			out[i] = in[i];
		for (i=in_size; i<out_size; i++)	out[i] = 0;
	}

	//trivial case: no overlap smoothing
	else if (in_size == out_size){		
		for (i=0; i<in_size; i++)			out[i] = in[i];
	}
	else {
		overlap_size = in_size - out_size;

		for (i=0; i<out_size; i++)
		{
			if (i<overlap_size) {
				xfade = (float)(i) / (float)overlap_size;
				out[i] = _CROSSFADE(in[i + out_size], in[i], xfade);
			} else {
				out[i] = in[i];
			}
		}
	}
}



void remove_DC_offset(uint8_t dim1,uint8_t dim2,uint8_t dim3){

	uint16_t i;
	float average_val;
	float total_val = 0;
	int16_t this_val;
	int16_t shift_amount;
	int16_t max_val = 0;
	int16_t min_val = 0;
	int16_t max_upward_shift, max_downward_shift;


	for (i=0; i<WT_TABLELEN; i++) {
		this_val = spherebuf.data[dim1][dim2][dim3].wave[i];
		if (this_val < min_val) min_val = this_val;
		if (this_val > max_val) max_val = this_val;

		total_val += this_val;
	}
	average_val = total_val / WT_TABLELEN;

	max_upward_shift = INT16_MAX - max_val;
	max_downward_shift = INT16_MIN - min_val;
	
	shift_amount = -1*average_val;
	shift_amount = _CLAMP_I32(shift_amount, max_downward_shift, max_upward_shift);

	if (shift_amount > 1 || shift_amount < -1) { //optimization: don't bother shifting up/down by 1, which is can be due to rounding errors
		for (i=0; i<WT_TABLELEN; i++) {
			spherebuf.data[dim1][dim2][dim3].wave[i] += shift_amount;
		}
	}
}

void normalize_waveform(uint8_t dim1,uint8_t dim2,uint8_t dim3){

	float normgain, gain;
	uint16_t i;
	uint32_t amp;

	remove_DC_offset(dim1, dim2, dim3);

	if (spherebuf.fx[FX_NORMALIZE][dim1][dim2][dim3] == 0.5)
		return;

	spherebuf.maxvalbuf[dim1][dim2][dim3] = 0;
	for (i=0; i<WT_TABLELEN; i++){
		//spherebuf.maxvalbuf[dim1][dim2][dim3] = _MAX_I32(_ABS_I32(spherebuf.data[dim1][dim2][dim3].wave[i]),s pherebuf.maxvalbuf[dim1][dim2][dim3]);

		amp = _ABS_I32(spherebuf.data[dim1][dim2][dim3].wave[i]);
		if(spherebuf.maxvalbuf[dim1][dim2][dim3] < amp) { spherebuf.maxvalbuf[dim1][dim2][dim3] = amp;}
	}

	normgain = (float)(INT16_MAX) / (float)(spherebuf.maxvalbuf[dim1][dim2][dim3]);
	
	if (spherebuf.fx[FX_NORMALIZE][dim1][dim2][dim3] >= 0.5)
		gain = _CROSSFADE(1.0, normgain, (spherebuf.fx[FX_NORMALIZE][dim1][dim2][dim3] - 0.5) * 2.0);
	else
		gain = 0.3 + (spherebuf.fx[FX_NORMALIZE][dim1][dim2][dim3] * 1.4);
 
	 //optimize out gain adjustments less than our resolution
 	if (fabs(gain - 1.0) > (1.0/(float)INT16_MAX)) {
		for (i=0; i<WT_TABLELEN; i++){
			spherebuf.data[dim1][dim2][dim3].wave[i] = (int16_t)((float)(spherebuf.data[dim1][dim2][dim3].wave[i]) * gain);
		}
	}
}



// -------------------------
// 			FXs
// -------------------------

void update_wt_fx_params(uint8_t dim1, uint8_t dim2, uint8_t dim3, int16_t increment){
	
	uint8_t i;
	float amt;

	if(increment){
		
		for(i=0; i<NUM_FX; i++){ 
			if(button_pressed(butm_A_BUTTON + i)){
				amt = switch_pressed(FINE_BUTTON) ? FX_FINE_SCALING[i] : FX_SCALING[i];
				amt *= (float)(increment);
				spherebuf.fx[i][dim1][dim2][dim3] = _CLAMP_F(spherebuf.fx[i][dim1][dim2][dim3] + amt, 0.0, 1.0);
				calc_params.already_handled_button[i] = 1;
			}
		}
		start_ongoing_display_fx();
		enter_wtrendering();
	}
}


void lowpass_fft(uint8_t dim1,uint8_t dim2,uint8_t dim3){
	float tmp_buf[WT_TABLELEN*2];

	if (spherebuf.fx[FX_LPF][dim1][dim2][dim3] < 0.00001)
		return;
	
	// float freq = (float)(15040 - _SCALE_F2U16(spherebuf.fx[FX_LPF][dim1][dim2][dim3], 0.0, 1.0, 40, 15000)) / (F_SAMPLERATE/2.0);

	float freq = powf(21000.0, 1.0-spherebuf.fx[FX_LPF][dim1][dim2][dim3]) + 200.0;
	float freq_ratio = freq / (F_SAMPLERATE/2.0);
	
	do_cfft_lpf_512_f32(spherebuf.data[dim1][dim2][dim3].wave, spherebuf.data[dim1][dim2][dim3].wave, tmp_buf, freq_ratio);
}


void decimate_waveform(uint8_t dim1, uint8_t dim2, uint8_t dim3){
	
	uint16_t i;
	int8_t   dec_idx = 0;

	if (spherebuf.fx[FX_DECIMATING][dim1][dim2][dim3] < FX_FINE_SCALING[FX_DECIMATING]) 
		return;

	for (i=1; i<WT_TABLELEN; i++){

		if (dec_idx >= ((F_TABLE_DECIMATEMAX / FX_SCALING[FX_DECIMATING]) * spherebuf.fx[FX_DECIMATING][dim1][dim2][dim3])){	
			dec_idx = 0;
		}
		else{
			spherebuf.data[dim1][dim2][dim3].wave[i] = spherebuf.data[dim1][dim2][dim3].wave[i-dec_idx-1];
			dec_idx++;
		}
	}
	
	normalize_waveform (dim1,dim2,dim3);
}


// add highs to waveform
// easter egg from LPF development
void metalizer(uint8_t dim1,uint8_t dim2,uint8_t dim3){
	
	uint16_t 		i;
	int16_t 		index;
	float 			lpf_val;
	uint16_t 		prev_val;

	if (spherebuf.fx[FX_METALIZE][dim1][dim2][dim3] < FX_FINE_SCALING[FX_METALIZE]) 
		return;

	prev_val = 0;
	lpf_val = spherebuf.fx[FX_METALIZE][dim1][dim2][dim3] * 0.95;

	for (i=0; i < (WT_TABLELEN * 2 - 1); i++){
		if 		(i 	 >= WT_TABLELEN) 	{index 		= i   - WT_TABLELEN  ;}
		else 							{index 		= i 				 ;}
		spherebuf.data[dim1][dim2][dim3].wave[index] = (1-lpf_val) * spherebuf.data[dim1][dim2][dim3].wave[index] + lpf_val * prev_val; 
		prev_val = spherebuf.data[dim1][dim2][dim3].wave[index];
	}
	normalize_waveform (dim1,dim2,dim3);
}

// Chebyshev waveshaping (sine shape)
void chebyshev_wavefold(uint8_t dim1, uint8_t dim2, uint8_t dim3){

	if (spherebuf.fx[FX_WAVEFOLDING][dim1][dim2][dim3] < FX_FINE_SCALING[FX_WAVEFOLDING]) 
		return;

	float n = powf(50.0, spherebuf.fx[FX_WAVEFOLDING][dim1][dim2][dim3]);

	for (uint32_t i=0; i < WT_TABLELEN; i++){
		float smpl = (float)spherebuf.data[dim1][dim2][dim3].wave[i] / (float)(INT16_MAX+1);
		smpl = sinf(n * asinf(smpl));
		spherebuf.data[dim1][dim2][dim3].wave[i] = _CLAMP_F(smpl * INT16_MAX, INT16_MIN, INT16_MAX);
	}
	normalize_waveform (dim1,dim2,dim3);
}



void slew_limit(uint8_t dim1,uint8_t dim2,uint8_t dim3){
	uint16_t i;
	float dxdt, dydt;
	float slew, y;
	float adjustment;

	if (spherebuf.fx[FX_SLEW_LIMIT][dim1][dim2][dim3] < 0.00001)
		return;

	slew = powf(0.001, spherebuf.fx[FX_SLEW_LIMIT][dim1][dim2][dim3]) * INT16_MAX;

	y = spherebuf.data[dim1][dim2][dim3].wave[0];
	for (i = 1; i < WT_TABLELEN; i++) {
		dxdt = spherebuf.data[dim1][dim2][dim3].wave[i] - y;
		dydt = _CLAMP_F(dxdt, -1.0*slew, slew);
		y += dydt;
		spherebuf.data[dim1][dim2][dim3].wave[i] = y;
	}

	float start = spherebuf.data[dim1][dim2][dim3].wave[0];
	float end = (float)spherebuf.data[dim1][dim2][dim3].wave[WT_TABLELEN - 1] / (float)(WT_TABLELEN - 1) * (float)WT_TABLELEN;

	for (i = 0; i < WT_TABLELEN; i++) {
		adjustment = (end - start) * (i - (float)WT_TABLELEN / 2.0) / (float)WT_TABLELEN;
		spherebuf.data[dim1][dim2][dim3].wave[i] = _CLAMP_I32(spherebuf.data[dim1][dim2][dim3].wave[i] - adjustment, INT16_MIN, INT16_MAX);
	}
}

void distort_waveform(uint8_t dim1, uint8_t dim2, uint8_t dim3){
	
	uint16_t i;
	float smpl_in, smpl_out;

	if (spherebuf.fx[FX_DISTORTION][dim1][dim2][dim3] < FX_FINE_SCALING[FX_DISTORTION]) 
		return;

	float a = (spherebuf.fx[FX_DISTORTION][dim1][dim2][dim3] * 10.0) + 1.0;

	for (i=0; i<WT_TABLELEN; i++){
		smpl_in = (float)spherebuf.data[dim1][dim2][dim3].wave[i]/(float)INT16_MAX;

		smpl_out = smpl_in*(fabs(smpl_in) + a)/(smpl_in*smpl_in + (a-1.0)*fabs(smpl_in) + 1.0);

		spherebuf.data[dim1][dim2][dim3].wave[i] = smpl_out * INT16_MAX;
	}
}


void linear_wavefold(uint8_t dim1, uint8_t dim2, uint8_t dim3){
	
	uint16_t i;
	float sample_val;
	float gain;
	float fold_amt;
	float maxval;

	if (spherebuf.fx[FX_WAVEFOLDING][dim1][dim2][dim3] < FX_FINE_SCALING[FX_WAVEFOLDING]) 
		return;

	gain = 0.95 + F_TABLE_FOLDMAX * spherebuf.fx[FX_WAVEFOLDING][dim1][dim2][dim3];
	maxval = INT16_MAX * 0.95;

	for (i=0; i<WT_TABLELEN; i++){
		
		sample_val = gain * (float)(spherebuf.data[dim1][dim2][dim3].wave[i]);
		
		while (fabsf(sample_val)> maxval){
		
			if (sample_val > 0){
				fold_amt = sample_val - maxval;
				sample_val = (float)(maxval) - fold_amt;
			}
			
			else if (sample_val < 0){
				fold_amt = maxval + sample_val;
				sample_val = -(float)(maxval) - fold_amt;
			}
		}

		spherebuf.data[dim1][dim2][dim3].wave[i] = sample_val;
	}
}



// biquad
// #define LPF_FX_PADDING 20
void lowpass_biquad(uint8_t dim1,uint8_t dim2,uint8_t dim3, int16_t *inbuf, int16_t bufsize){
	
	// uint16_t freq;
	// float tmp_inwave[WT_TABLELEN+(LPF_FX_PADDING*2)];
	// float tmp_outwave[WT_TABLELEN+(LPF_FX_PADDING*2)];
	// uint16_t i, j=0;

	// if (spherebuf.fx[FX_LPF][dim1][dim2][dim3] < 0.00001) return;

	// freq = 22040 - _SCALE_F2U16(spherebuf.fx[FX_LPF][dim1][dim2][dim3], 0.0, 1.0, 40, 22000);

	// for (i=WT_TABLELEN-LPF_FX_PADDING; i<WT_TABLELEN; i++)
	// 	tmp_inwave[j++] = (float)(spherebuf.data[dim1][dim2][dim3].wave[i]);

	// for (i=0; i<WT_TABLELEN; i++)
	// 	tmp_inwave[j++] = (float)(spherebuf.data[dim1][dim2][dim3].wave[i]);

	// for (i=0; i<LPF_FX_PADDING; i++)
	// 	tmp_inwave[j++] = (float)(spherebuf.data[dim1][dim2][dim3].wave[i]);

	// process_lpf(F_SAMPLERATE, (float)freq, tmp_inwave, tmp_outwave, WT_TABLELEN+(LPF_FX_PADDING*2), spherebuf.fx[FX_WAVEFOLDING][dim1][dim2][dim3]);

	//process_lpf(F_SAMPLERATE, (float)freq, inbuf, inbuf, bufsize, 0.1);


	// for (i=0; i<WT_TABLELEN; i++){
		// spherebuf.data[dim1][dim2][dim3].wave[i] = _CLAMP_F(tmp_outwave[i+LPF_FX_PADDING], INT16_MIN, INT16_MAX);
	// }
}

void lowpass_fft_q15(uint8_t dim1,uint8_t dim2,uint8_t dim3){
	// int16_t tmp_buf[WT_TABLELEN];

	// if (spherebuf.fx[FX_LPF][dim1][dim2][dim3] < 0.00001)
	// 	return;
	
	// float freq = (float)(22040 - _SCALE_F2U16(spherebuf.fx[FX_LPF][dim1][dim2][dim3], 0.0, 1.0, 40, 22000)) / F_SAMPLERATE;

	// do_fft_lpf_q15(spherebuf.data[dim1][dim2][dim3].wave, spherebuf.data[dim1][dim2][dim3].wave, tmp_buf, WT_TABLELEN, freq);

}

enum colorCodes animate_fx_level(uint8_t slot_i){

	static uint8_t fx_num_active = NUM_CHANNELS;
	enum colorCodes led_color;
	uint8_t i;
	uint8_t dim1, dim2, dim3;

	dim1 = calc_params.wt_pos[0][0];
	dim2 = calc_params.wt_pos[1][0];
	dim3 = calc_params.wt_pos[2][0];

	for (i=0; i<NUM_CHANNELS; i++) {
		if (button_pressed(butm_A_BUTTON + i))
			fx_num_active = i;
	}

	if (fx_num_active < NUM_CHANNELS)
	{
		if (spherebuf.fx[fx_num_active][dim1][dim2][dim3] > ((float)slot_i/(float)NUM_LED_OUTRING))
			led_color = fx_colors[fx_num_active];
		else 
			led_color = ledc_OFF;
	}
	else 
		led_color = ledc_OFF; //error

	return led_color;
}
