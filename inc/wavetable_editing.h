/*
 * wavetable_editing.h
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



#pragma once

#include <stm32f7xx.h>
#include "sphere.h" // sphere typedef
#include "led_colors.h"
#include "wavetable_effects.h"

#define	NUM_SPHERES_IN_RECBUF			8
#define SPHERE_REC_MAX_OVERLAP_SIZE	 	208
#define RESAMPLING_LOOKAHEAD_SAMPLES	4
#define NUM_SAMPLES_IN_RECBUF			(NUM_SAMPLES_IN_SPHERE * NUM_SPHERES_IN_RECBUF ) 																				/*110592*/
#define NUM_SAMPLES_IN_RECBUF_SMOOTHED 	(NUM_SAMPLES_IN_RECBUF + ((WT_TABLELEN + SPHERE_REC_MAX_OVERLAP_SIZE) * MAX_STRETCH) + RESAMPLING_LOOKAHEAD_SAMPLES) 	/*116352*/

#define F_SCALING_WTBUF_STRETCH			0.5
#define F_SCALING_FINE_WTBUF_STRETCH	0.05
#define	MAX_STRETCH						NUM_SPHERES_IN_RECBUF

#define I_SCALING_WTBUF_POSITION 		(WT_TABLELEN * 6)		/*Each rotary turn shifts 6 waveforms in the buffer*/
#define I_SCALING_FINE_WTBUF_POSITION 	(WT_TABLELEN / 2)			

#define I_SCALING_WTBUF_SPREAD			(WT_TABLELEN / 2)
#define I_SCALING_FINE_WTBUF_SPREAD		(WT_TABLELEN / 16)
#define	MIN_WTBUF_SPREAD				WT_TABLELEN
#define DEFAULT_WTBUF_SPREAD			(WT_TABLELEN * 8)
#define MAX_WTBUF_SPREAD				(WT_TABLELEN * NUM_SPHERES_IN_RECBUF)

#define F_TABLE_DISTORTMAX 4.0
#define F_TABLE_DECIMATEMAX 3.0
#define F_TABLE_FOLDMAX 20.0

enum SphereDataSources {
	SPHERESRC_RECBUFF,
	SPHERESRC_SPHERE
};


typedef struct o_spherebuf{
	float 					stretch_ratio;
	uint32_t				spread_amount;
	uint32_t				position; 				
	enum SphereDataSources	data_source;
	float 					fx 			[NUM_FX][WT_DIM_SIZE][WT_DIM_SIZE][WT_DIM_SIZE];
	uint32_t 				maxvalbuf 	[WT_DIM_SIZE][WT_DIM_SIZE][WT_DIM_SIZE];
	o_waveform 				data		[WT_DIM_SIZE][WT_DIM_SIZE][WT_DIM_SIZE];
	
	uint32_t 				start_pos	[NUM_WAVEFORMS_IN_SPHERE];
} o_spherebuf;




void init_sphere_editor(void);
void set_params_for_editing(void);

void enter_wtrendering(void);
void enter_wtediting(void);
void stage_enter_wtediting(void);
void enter_wtmonitoring(void);
void enter_wtttone(void);
void exit_wtediting(void);

void copy_current_sphere_to_recbuf(uint8_t sphere_index);
void render_full_sphere(void);
float render_recbuf_to_spherebuf(uint8_t dim1, uint8_t dim2, uint8_t dim3, float start_sample);
float put_waveform_in_sphere(uint8_t dim1, uint8_t dim2, uint8_t dim3, float start_sample);

void update_sphere_stretch_position(int16_t encoder_in);
void init_wt_edit_settings(void);
uint32_t wrap_wtbuf_position(int32_t new_position);

void display_wt_recbuf_sel_outring(void);

