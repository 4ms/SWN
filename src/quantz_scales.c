/*
 * quantz_scales.c
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

#include "quantz_scales.h"
#include "globals.h"
#include "led_colors.h"
#include "params_update.h"

//Todo: this could be a struct:
// struct quantizedScale {
//		enum colorCodes		color;
//		uint8_t				num_qtz_steps;
//		float				scale[MAX_NUM_QTZ_STEPS];
//		float				midpt[MAX_NUM_QTZ_STEPS];
//	};
//
//	
//	struct quantizedScales qtz_scales[NUM_QTZ_SCALES];
//
//	void init_(void){...
//	...
//		qtz_scales[sclm_MAJOR].color = ledc_GOLD;
//		qtz_scales[sclm_MAJOR].num_qty_steps = 7;
//		qtz_scales[sclm_MAJOR].scale[0] = 16.35;
//  	...


const enum colorCodes qtz_scale_colors[NUM_QTZ_SCALES] = {
	ledc_LIGHT_BLUE,		// NO QTZ
	ledc_BRIGHTPINK,		// MAJOR
	ledc_CORAL,				// MINOR HARMONIC
	ledc_BLUE				// SEMITONES
};

const uint8_t num_qtz_steps[NUM_QTZ_SCALES] = {
	1,					// NO QTZ
	7,					// MAJOR
	7,					// MINOR HARMONIC
	12					// SEMITONES
};

const float qtz_scales[NUM_QTZ_SCALES][MAX_NUM_QTZ_STEPS] = {

// NO QTZ
	{
		0
	},	

// MAJOR
	{
		// 16.35, // C0 2109.89
		// 18.35, // D0 1879.69
		// 20.60, // E0 1674.62
		// 21.83, // F0 1580.63
		// 24.50, // G0 1408.18
		// 27.50, // A0 1254.55
		// 30.87 // B0 1117.67
		16.3516015625,
		18.3540234375,
		20.60171875,
		21.8267578125,
		24.4997265625,
		27.50,
		30.8676953125

	},	

// MINOR HARMONIC
	{
		// 16.35, // C0	2109.89
		// 18.35, // D0	1879.69
		// 19.45, // Eb0 	1774.20
		// 21.83, // F0	1580.63
		// 24.50, // G0	1408.18
		// 25.96, // Ab0 	1329.14
		// 30.87  // B0	1117.67
		16.3516015625,
		18.3540234375,
		19.4454296875,
		21.8267578125,
		24.4997265625,
		25.9565625,
		30.8676953125
	},

// SEMITONES
	{
		// 16.35, 			// C0
		// 17.32222159,	// C#
		// 18.35225449,	// D
		// 19.44353633,	// D#
		// 20.59970917,	// E
		// 21.82463162,	// F
		// 23.12239174,	// F#
		// 24.49732071,	// G
		// 25.9540072,		// G#
		// 27.49731278,	// A
		// 29.13238808,	// A#
		// 30.86469002		// B
		16.3516015625,
		17.32390625,
		18.3540234375,
		19.4454296875,
		20.60171875,
		21.8267578125,
		23.1246484375,
		24.4997265625,
		25.9565625,
		27.50,
		29.135234375,
		30.8676953125
	}
};


float qtz_scale_midpt[NUM_QTZ_SCALES][MAX_NUM_QTZ_STEPS];

void init_quantz_scales(void)
{
	uint8_t i,j;

	for (i=0;i<NUM_QTZ_SCALES;i++)
	{
		for (j=0; j<(num_qtz_steps[i]-1); j++)
		{
			qtz_scale_midpt[i][j] = (qtz_scales[i][j] + qtz_scales[i][j+1]) / 2.0;
		}

		//last midpoint is between last note and an octave above the first note
		qtz_scale_midpt[i][num_qtz_steps[i]-1] = (qtz_scales[i][num_qtz_steps[i]-1] + (qtz_scales[i][0]*2.0)) / 2.0; 
	}

}

float quantize_to_scale(uint8_t scale_num, float unqtz_freq, uint8_t *qtz_note, int8_t *qtz_oct){
//float quantize_to_scale(uint8_t scale_num, float unqtz_freq, uint8_t *qtz_note, uint8_t *qtz_oct, uint8_t prev_qtz_note, uint8_t prev_qtz_oct){

	uint8_t note 			= 0;
	int8_t oct  			= 0;
	float 	qtz_freq		= unqtz_freq;
	float	high_note;
	float	distance_from_midpt;

	if ((scale_num == sclm_NONE) || (scale_num>=NUM_QTZ_SCALES))
		return unqtz_freq;

	// Find octave
	high_note = qtz_scale_midpt[scale_num][ num_qtz_steps[scale_num]-1 ];
 	while( (unqtz_freq >= high_note ) && (oct <= MAX_OCT) ){
 		oct++;
 		high_note *= 2;
 	}

	if (oct < MAX_OCT){

		for (note = 0; note < num_qtz_steps[scale_num]; note++){
			distance_from_midpt = (qtz_scale_midpt[scale_num][note] * (float)(1<<oct)) - unqtz_freq;
			if( distance_from_midpt > 0.0 )
			{
				if (distance_from_midpt > (0.1	 * (float)(1<<oct)))
				{
					//Set the frequency based on note and octave
					qtz_freq  = qtz_scales[scale_num][note] * (1<<oct);
					*qtz_note = note;
					*qtz_oct = oct;
					break; //found a note, so exit loop
				} else {
					break; //Todo: implement better anti-hysteresis
				}
			}
		}
	}

	return qtz_freq;
}

