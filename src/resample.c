/*
 * resample.c - Resampling of audio buffer using a Hermite interpolation
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

#include "resample.h"
#include "math_util.h"

float resample_hermitian(float resample_rate, float start_inpos, uint32_t in_buf_size, int16_t* in, uint32_t out_samples, int16_t *out, uint16_t init_pos)
{
	static float fractional_pos = 0;
	static float xm1, x0, x1, x2;
	float a,b,c;
	uint32_t outpos;
	uint32_t inpos;

	inpos = (uint32_t)start_inpos;
	fractional_pos = start_inpos - (float)inpos;

	if (resample_rate == 1.0)		//Handle the trivial case of 1:1 resample rate (out<=in)
	{
		for(outpos=0;outpos<out_samples;outpos++)
		{
			out[outpos] = in[inpos];
			inpos++;
		}
	}
	else {

		//fill the resampling buffer with three points
		if (init_pos)
		{
			xm1		= in[inpos];
			inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);

			x0		= in[inpos];
			inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);

			x1		= in[inpos];
			inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);

			x2		= in[inpos];
			inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);

			fractional_pos = 0.0;
		}

		outpos = 0;

		while (outpos < out_samples)
		{	

			// optimize for X many spheres (currently 8)
			//Optimize for resample rates >= 4
			if (fractional_pos >= 4.0)
			{
				fractional_pos = fractional_pos - 4.0;

				//shift samples back one
				//and read a new sample
				xm1		= in[inpos];
				inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);

				x0		= in[inpos];
				inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);

				x1		= in[inpos];
				inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);

				x2		= in[inpos];
				inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);
			}
			//Optimize for resample rates >= 3
			if (fractional_pos >= 3.0)
			{
				fractional_pos = fractional_pos - 3.0;

				//shift samples back one
				//and read a new sample
				xm1 	= x2;

				x0		= in[inpos];
				inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);

				x1		= in[inpos];
				inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);

				x2		= in[inpos];
				inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);
			}
			//Optimize for resample rates >= 2
			if (fractional_pos >= 2.0)
			{
				fractional_pos = fractional_pos - 2.0;

				//shift samples back one
				//and read a new sample
				xm1 	= x1;
				x0 		= x2;

				x1		= in[inpos];
				inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);

				x2		= in[inpos];
				inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);
			}
			//Optimize for resample rates >= 1
			if (fractional_pos >= 1.0)
			{
				fractional_pos = fractional_pos - 1.0;

				//shift samples back one
				//and read a new sample
				xm1 = x0;
				x0 	= x1;
				x1 	= x2;

				x2		= in[inpos];
				inpos 	= _WRAP_U32(inpos+1, 0, in_buf_size);
			}

			//calculate coefficients
			a = (3 * (x0-x1) - xm1 + x2) / 2;
			b = 2*x1 + xm1 - (5*x0 + x2) / 2;
			c = (x1 - xm1) / 2;

			//calculate as many fractionally placed output points as we need
			while ( fractional_pos<1.0 && outpos<out_samples )
			{
				out[outpos++] = _CLAMP_I32((((a * fractional_pos) + b) * fractional_pos + c) * fractional_pos + x0, INT16_MIN, INT16_MAX);
				fractional_pos += resample_rate;
			}
		}
	}
	return (inpos + fractional_pos);
}
