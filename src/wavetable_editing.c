/*
 * wavetable_editing.c
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
#include "wavetable_editing.h"
#include "wavetable_effects.h"
#include "wavetable_saveload.h"
#include "wavetable_saveload_UI.h"
#include "oscillator.h"
#include "params_update.h"
#include "params_sphere_enable.h"
#include "globals.h"
#include "wavetable_recording.h"
#include "key_combos.h"
#include "params_lfo.h"
#include "led_map.h"
#include "math_util.h"
#include "resample.h"
#include "ui_modes.h"
#include "led_cont.h"
#include "led_colors.h"
#include "sphere_flash_io.h"
#include "params_wt_browse.h"
#include "flashram_spidma.h"
#include "codec_sai.h"

extern const float BROWSE_TABLE[ NUM_WAVEFORMS_IN_SPHERE ][ NUM_WT_DIMENSIONS ];
extern enum UI_Modes ui_mode;
extern o_led_cont 		led_cont;
extern uint32_t colorPalette[NUM_LED_COLORS][NUM_PALETTE_COLORS];

extern o_wt_osc	wt_osc;
extern o_params params;
extern o_calc_params calc_params;
extern const float 	exp_1voct_10_41V[4096];

// sphere data
SRAM1DATA o_spherebuf spherebuf;


const int16_t TTONE[WT_TABLELEN]	= {-320,-49,468,956,1328,1681,2115,2572,2975,3354,3768,4207,4619,5002,5411,5827,6248,6622,7025,7427,7840,8217,8607,9004,9405,9784,10165,10552,10944,11324,11688,12072,12451,12822,13182,13547,13913,14279,14623,14981,15330,15681,16019,16358,16700,17035,17363,17686,18015,18337,18652,18959,19275,19577,19885,20169,20471,20759,21048,21327,21608,21889,22161,22435,22697,22970,23226,23486,23736,23992,24236,24476,24712,24944,25177,25398,25613,25833,26042,26259,26457,26670,26866,27073,27267,27465,27661,27849,28037,28224,28402,28579,28750,28918,29081,29239,29385,29544,29693,29838,29976,30120,30268,30414,30553,30685,30836,30978,31115,31245,31369,31504,31619,31729,31825,31921,32012,32084,32151,32210,32271,32322,32365,32410,32450,32496,32535,32574,32615,32649,32688,32713,32744,32759,32764,32766,32749,32728,32686,32627,32563,32479,32394,32283,32173,32049,31926,31793,31659,31520,31383,31244,31106,30965,30825,30687,30540,30399,30244,30095,29938,29774,29610,29431,29258,29077,28886,28701,28499,28312,28109,27907,27703,27493,27296,27079,26870,26647,26430,26210,25982,25746,25508,25264,25022,24767,24510,24247,23981,23718,23440,23166,22884,22604,22318,22033,21737,21445,21146,20845,20537,20226,19908,19592,19267,18940,18603,18266,17926,17582,17234,16879,16521,16164,15806,15439,15071,14698,14323,13952,13569,13186,12798,12409,12023,11626,11230,10826,10428,10023,9621,9203,8795,8380,7971,7555,7128,6707,6287,5863,5441,5003,4577,4146,3723,3284,2856,2419,1994,1561,1131,702,274,-155,-586,-1010,-1433,-1858,-2285,-2715,-3130,-3554,-3974,-4400,-4822,-5234,-5653,-6068,-6485,-6893,-7302,-7707,-8113,-8515,-8907,-9303,-9695,-10088,-10471,-10852,-11231,-11607,-11986,-12356,-12717,-13087,-13443,-13808,-14158,-14505,-14852,-15198,-15534,-15872,-16195,-16530,-16848,-17174,-17484,-17796,-18105,-18411,-18711,-19011,-19296,-19592,-19869,-20159,-20432,-20702,-20973,-21231,-21499,-21750,-22002,-22245,-22495,-22731,-22974,-23200,-23434,-23659,-23886,-24103,-24321,-24535,-24743,-24951,-25149,-25351,-25545,-25735,-25919,-26103,-26282,-26458,-26629,-26795,-26962,-27125,-27284,-27442,-27601,-27746,-27904,-28040,-28196,-28331,-28475,-28601,-28741,-28866,-29002,-29113,-29237,-29349,-29471,-29581,-29691,-29795,-29908,-30013,-30119,-30218,-30323,-30425,-30524,-30616,-30706,-30796,-30880,-30952,-31017,-31074,-31129,-31168,-31198,-31214,-31228,-31225,-31218,-31192,-31172,-31131,-31094,-31037,-30990,-30935,-30871,-30806,-30733,-30672,-30603,-30524,-30450,-30367,-30300,-30212,-30129,-30037,-29950,-29860,-29762,-29664,-29563,-29460,-29353,-29240,-29129,-29020,-28898,-28778,-28653,-28528,-28407,-28266,-28135,-27993,-27859,-27711,-27561,-27409,-27252,-27100,-26933,-26773,-26599,-26437,-26263,-26086,-25910,-25723,-25551,-25359,-25173,-24973,-24781,-24586,-24383,-24168,-23957,-23745,-23528,-23305,-23067,-22839,-22608,-22371,-22127,-21881,-21631,-21384,-21125,-20870,-20610,-20346,-20075,-19803,-19530,-19254,-18968,-18677,-18386,-18097,-17792,-17488,-17184,-16869,-16563,-16234,-15916,-15595,-15272,-14935,-14606,-14268,-13934,-13591,-13245,-12902,-12553,-12192,-11839,-11481,-11122,-10755,-10376,-10010,-9641,-9256,-8877,-8486,-8112,-7717,-7326,-6935,-6544,-6149,-5738,-5347,-4957,-4551,-4132,-3729,-3343,-2948,-2502,-2087,-1708,-1337,-867,-365};

extern 		SRAM1DATA o_recbuf 		recbuf;
uint32_t 							sector_addr;
extern const uint32_t 				WT_SIZE;

uint8_t	stage_enter_wtediting_flag = 0;



void init_wt_edit_settings(void)
{
	uint8_t i,j,k;
	
	spherebuf.position			= WT_TABLELEN/2;
	spherebuf.stretch_ratio		= 1;
	spherebuf.spread_amount		= DEFAULT_WTBUF_SPREAD;

	for (i=0; i<WT_DIM_SIZE; i++){
		for (j=0; j<WT_DIM_SIZE; j++){
			for (k=0; k<WT_DIM_SIZE; k++){
				spherebuf.fx[FX_WAVEFOLDING][i][j][k]	= 0;
				spherebuf.fx[FX_LPF][i][j][k] 			= 0;
				spherebuf.fx[FX_DECIMATING][i][j][k] 	= 0;
				spherebuf.fx[FX_METALIZE][i][j][k] 		= 0;
				spherebuf.fx[FX_NORMALIZE][i][j][k]		= (spherebuf.data_source==SPHERESRC_RECBUFF)? 1 : 0.5;
				spherebuf.fx[FX_SMOOTHING][i][j][k]		= (spherebuf.data_source==SPHERESRC_RECBUFF)? 1 : 0;
			}
		}
	}
}

void set_params_for_editing(void)
{
	uint8_t i;

	params.dispersion_enc = 0;
	update_wt_disp(CLEAR_LPF);

	for (i=0;i<NUM_CHANNELS;i++)
	{
		params.wtsel_enc[i] = params.wtsel_enc[0];
		params.wtsel_spread_enc[i] = 0;

		params.wt_nav_enc[0][i] = 0;
		params.wt_nav_enc[1][i] = 0;
		params.wt_nav_enc[2][i] = 0;
		params.wt_browse_step_pos_enc[i] = 0;
		reset_wbrowse_morph(i);
	}
	
	update_wtsel();
	update_all_wt_pos_interp_params();
	force_all_wt_interp_update();
}

void enter_wtrendering(void){
	ui_mode = WTRENDERING;
	set_audio_callback(&process_audio_block_codec);
}

void stage_enter_wtediting(void){
	//Using a flag system to avoid FLASH chip access collisions
	stage_enter_wtediting_flag = 1;	
}

void enter_wtediting(void)
{
	if (ui_mode==PLAY)
	{
		cache_uncache_keymodes(CACHE);
		apply_all_keymodes(ksw_MUTE);
		cache_uncache_all_lfo_to_vca(CACHE);
		set_all_lfo_to_vca(0);
		cache_uncache_locks(CACHE);
		unlock_all();
		cache_uncache_all_lfomodes(CACHE);
		set_all_lfo_mode(lfot_SHAPE);
		cache_uncache_nav_params(CACHE);

		init_user_sphere_mgr(params.wt_bank[0]);

		set_params_for_editing();
	}

	spherebuf.data_source = SPHERESRC_SPHERE;
	init_wt_edit_settings();
	copy_current_sphere_to_recbuf(params.wt_bank[0]);
	enter_wtrendering();
}

void enter_wtmonitoring(void){
	ui_mode = WTMONITORING;
	set_audio_callback(&process_audio_block_codec);
}

void enter_wtttone(void){
	ui_mode = WTTTONE;	
	set_audio_callback(&process_audio_block_codec);

	set_pitch_params_to_ttone();
	force_all_wt_interp_update();
}


void exit_wtediting(void){
	cache_uncache_all_lfomodes(UNCACHE);
	cache_uncache_locks(UNCACHE);
	cache_uncache_all_lfo_to_vca(UNCACHE);
	cache_uncache_keymodes(UNCACHE);
	cache_uncache_nav_params(UNCACHE);

	if (!is_sphere_enabled(params.wt_bank[0]))
		set_wtsel(0);

	set_audio_callback(&process_audio_block_codec);

	ui_mode = PLAY;

	update_all_wt_pos_interp_params();
	force_all_wt_interp_update();
}


// Copies sphere to recbuf for wt editing w/o recording
// Todo: Fix and re-enable copying the spheres from waveform[], giving a speed boost since they are already loaded from flash 
void copy_current_sphere_to_recbuf(uint8_t sphere_index){

	uint32_t i;
	uint32_t j;
	uint32_t pos;
	uint8_t	wt_browse;
	int16_t *ptr;
	static o_waveform tmp_waveform;

	// uint8_t already_loaded_from_flash;
	// uint8_t dim1=0;
	// uint8_t dim2=0;
	// uint8_t dim3=0;
	// uint8_t x0=0, y0=0, z0=0, x1=0, y1=0, z1=0, x,y,z;
	// extern o_waveform waveform[NUM_CHANNELS][2][2][2];


	// dim1 = 0;
	// dim2 = 0;
	// dim3 = 0;

	wt_browse=0;
	pos=0;

	// x0 = (uint8_t)(wt_osc.m0[0][0]);
	// y0 = (uint8_t)(wt_osc.m0[1][0]);
	// z0 = (uint8_t)(wt_osc.m0[2][0]);
	// x1 = (uint8_t)(wt_osc.m1[0][0]);
	// y1 = (uint8_t)(wt_osc.m1[1][0]);
	// z1 = (uint8_t)(wt_osc.m1[2][0]);

	while(wt_browse < (WT_DIM_SIZE * WT_DIM_SIZE * WT_DIM_SIZE)) {

		// already_loaded_from_flash = ((dim1 == x0) || (dim1 == x1)) &&
		// 							((dim2 == y0) || (dim2 == y1)) &&
		// 							((dim3 == z0) || (dim3 == z1));

		// if (already_loaded_from_flash) {
		// 	x = (dim1!=x0);
		// 	y = (dim2!=y0);
		// 	z = (dim3!=z0);
		// 	ptr = waveform[0][x][y][z].wave;
		// }
		
		// else {
			load_extflash_wavetable(sphere_index, &tmp_waveform, BROWSE_TABLE[wt_browse][0],BROWSE_TABLE[wt_browse][1],BROWSE_TABLE[wt_browse][2]);
			while (get_flash_state() != sFLASH_NOTBUSY) 
				{;}
			ptr = tmp_waveform.wave;
		// }

		for (j=0; j<NUM_SPHERES_IN_RECBUF; j++){
			for (i=0; i<WT_TABLELEN; i++){
				recbuf.data[pos++] = ptr[i];
			}
		}

		wt_browse++;

		// if (++dim1>=WT_DIM_SIZE){
		// 	dim1 = 0;
		// 	if (++dim2>=WT_DIM_SIZE){
		// 		dim2 = 0;
		// 		dim3++; 
		// 	}
		// }
	}
}

void render_full_sphere(void){
	float 		start_sample;
	uint8_t		wt_browse=0;

	if (stage_enter_wtediting_flag) {
		if (ui_mode==PLAY)
			enter_wtediting();
		else
		{
			ui_mode = WTEDITING;
			set_audio_callback(&process_audio_block_codec);
		}

		stage_enter_wtediting_flag = 0;
	}

	if (ui_mode == WTRENDERING) {
		start_sample = spherebuf.position;

		while(wt_browse<NUM_WAVEFORMS_IN_SPHERE){
			spherebuf.start_pos[wt_browse] = start_sample;
			start_sample = render_recbuf_to_spherebuf(BROWSE_TABLE[wt_browse][0],BROWSE_TABLE[wt_browse][1],BROWSE_TABLE[wt_browse][2], start_sample);
 			wt_browse++;
 		}

		ui_mode = WTEDITING;	
		force_all_wt_interp_update();
	}
}

//Render waveform from recbuf.data[] starting at [start_sample], to spherebuf.data[dim1][dim2][dim3]
float render_recbuf_to_spherebuf(uint8_t dim1, uint8_t dim2, uint8_t dim3, float start_sample){
	start_sample = put_waveform_in_sphere		(dim1,dim2,dim3, start_sample);

	normalize_waveform   						(dim1,dim2,dim3);
	chebyshev_wavefold							(dim1,dim2,dim3);
	decimate_waveform 							(dim1,dim2,dim3);
	metalizer 									(dim1,dim2,dim3);
	lowpass_fft									(dim1,dim2,dim3);
	normalize_waveform   						(dim1,dim2,dim3);

	return start_sample;
}

float put_waveform_in_sphere(uint8_t dim1, uint8_t dim2, uint8_t dim3, float start_sample){
	int16_t unsmoothed_buf[WT_TABLELEN + (SPHERE_REC_MAX_OVERLAP_SIZE*MAX_STRETCH)];
	uint32_t next_wave_offset;

	uint32_t unsmoothed_size = WT_TABLELEN + (spherebuf.fx[FX_SMOOTHING][dim1][dim2][dim3] * (float)SPHERE_REC_MAX_OVERLAP_SIZE);

	resample_hermitian(spherebuf.stretch_ratio, start_sample, NUM_SAMPLES_IN_RECBUF_SMOOTHED, recbuf.data, unsmoothed_size, unsmoothed_buf, 1);

	overlap_smooth_wave(unsmoothed_buf, spherebuf.data[dim1][dim2][dim3].wave, unsmoothed_size, WT_TABLELEN);

	next_wave_offset = ((float)spherebuf.spread_amount);// * spherebuf.stretch_ratio);

	return _WRAP_U32(start_sample + next_wave_offset, 0, NUM_SAMPLES_IN_RECBUF);
}



void update_sphere_stretch_position(int16_t encoder_in){
	float amt;

	if (encoder_in) {
		if (rotary_pressed(rotm_OCT)) {
			amt = switch_pressed(FINE_BUTTON) ? F_SCALING_FINE_WTBUF_STRETCH : F_SCALING_WTBUF_STRETCH;
			spherebuf.stretch_ratio = _CLAMP_F(spherebuf.stretch_ratio + (encoder_in * amt), F_SCALING_WTBUF_STRETCH, MAX_STRETCH);
		}
		else if (rotary_pressed(rotm_TRANSPOSE)) {
			amt = switch_pressed(FINE_BUTTON) ? I_SCALING_FINE_WTBUF_SPREAD : I_SCALING_WTBUF_SPREAD;
			spherebuf.spread_amount = _CLAMP_I32(spherebuf.spread_amount + (encoder_in * amt), MIN_WTBUF_SPREAD, MAX_WTBUF_SPREAD);
		}
		else {
			amt = switch_pressed(FINE_BUTTON) ? I_SCALING_FINE_WTBUF_POSITION : I_SCALING_WTBUF_POSITION;
			//spherebuf.position = wrap_wtbuf_position((int32_t)spherebuf.position + (encoder_in * amt));
			spherebuf.position = _WRAP_I32((int32_t)spherebuf.position + (encoder_in * amt), 0, NUM_SAMPLES_IN_RECBUF);
		}
		enter_wtrendering();
	}
}


//Handle all cases, whether or not I_SCALING_WTBUF_POSITION divides evenly into NUM_SAMPLES_IN_RECBUF
//[currently not used]
//If we turn negatively past 0, the find the highest postion we would have
//reached before the maximum, as if we had turned positively from 0
uint32_t wrap_wtbuf_position(int32_t new_position) {
	if (new_position < 0) {
		new_position = 0;
		while (new_position<NUM_SAMPLES_IN_RECBUF) new_position += I_SCALING_WTBUF_POSITION;
		return new_position - I_SCALING_WTBUF_POSITION;
	}
	//Always snap to 0 when wrapping in the positive direction
	else if (new_position >= NUM_SAMPLES_IN_RECBUF)
		return 0;
	else 
		return new_position;
}



void display_wt_recbuf_sel_outring(void){

	uint8_t i;
	float led_pos;
	float brightness_scaled;
	float brightness_unscaled;
	uint8_t led_pos_i, led_pos_end_ctr;

	for (i =0; i< NUM_LED_OUTRING; i++){
		set_rgb_color_brightness(&led_cont.outring[i], ledc_OFF, 0); 
	}

	const enum colorCodes waveform_colors[6] = {ledc_RED, ledc_GOLD, ledc_GREEN, ledc_AQUA, ledc_BLUE, ledc_DEEP_BLUE};
	const uint8_t display_waveforms[6] = {0, 4, 10, 15, 21, 25};

	for (i=0; i<6; i++){
		led_pos = (float)(spherebuf.start_pos[ display_waveforms[i] ] * NUM_LED_OUTRING) / (float)(NUM_SAMPLES_IN_RECBUF);
		led_pos_i = (uint8_t)led_pos;
		if ((led_pos - led_pos_i) >= 0.5) led_pos_i++; //round to closest int
		led_pos_i = _WRAP_U8((uint8_t)led_pos_i, 0, NUM_LED_OUTRING);

		add_rgb_color_brightness(&led_cont.outring[led_pos_i], waveform_colors[i], 1);

		led_pos_end_ctr = 1;
		while (led_pos_end_ctr < spherebuf.stretch_ratio){
			led_pos_i = _WRAP_U8(led_pos_i + 1, 0, NUM_LED_OUTRING);

			brightness_unscaled = _CLAMP_F(spherebuf.stretch_ratio - led_pos_end_ctr, 0.0, 1.0);
			brightness_scaled = exp_1voct_10_41V[ _SCALE_F2U16(brightness_unscaled, 0, 1.0, 128, 4095)] / 1370.0;
			add_rgb_color_brightness(&led_cont.outring[led_pos_i], waveform_colors[i], brightness_scaled);

			led_pos_end_ctr++;
		}
	}
}
