/*
 * params_pitch.c - pitch parameters
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

#include "params_pitch.h"
#include "analog_conditioning.h"
#include "calibrate_voct.h"
#include "math_util.h"
#include <math.h>

extern const float 	exp_1voct_10_41V[4096];
extern o_analog analog[NUM_ANALOG_ELEMENTS];

//Private:
float interpolate_voct(float adc_val);


float calc_expo_pitch(uint8_t chan, float adc_val)
{

	//Unplugged jacks: no pitch change
	if (analog[A_VOCT + chan].plug_sense_switch.pressed == RELEASED)
		return 1.0;

	adc_val = apply_voct_calibration(adc_val, chan);

	if (analog[A_VOCT + chan].polarity == AP_BIPOLAR)
	{
		if (adc_val >= 2048.0)
			return interpolate_voct(adc_val - 2048.0);
		else
			return 1.0/interpolate_voct(2048.0 - adc_val);
	} else
	{
		return interpolate_voct(adc_val);
	}
}

float interpolate_voct(float adc_val)
{
	uint16_t i_val;
	float f_val;

	i_val = (uint16_t)adc_val;
	f_val = adc_val - (float)i_val;
	
	// if (i_val >= 4095) return 1024.0;
	// return powf(2.0, adc_val / 409.5);

	if (i_val >= 4095) return exp_1voct_10_41V[4095];

	return _CROSSFADE(exp_1voct_10_41V[i_val], exp_1voct_10_41V[i_val+1], f_val);
}
