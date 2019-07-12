/*
 * fft_filter.c - PWM output for the channel ENV OUT jacks
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

 #include <math.h>
#include "arm_math.h"
#include "arm_const_structs.h"
#include "fft_filter.h"
#include "math_util.h"

#include "gpio_pins.h"


//inbuf and outbuf have 512 elements (can be the same array)
//cinbuf has 1024 elements
//freq is 0..1: f0 = freq/(fs/2)
//Timing analysis @ -03: 512 samples @ 44100S/sec = 11.61ms.
//Processing FFT+IFFT alone = 0.410us max. Total process 0.516 - 0.580us = 5% of load
void do_cfft_lpf_512_f32(int16_t *inbuf,  int16_t *outbuf, float *cinbuf, float freq)
{
	const uint16_t bufsize = 512;
	uint16_t i;

	//convert real floats to complex (skip every other)
	//is there an arm_math_real_to_complex()?
	for (i=0; i<bufsize; i++) { //24us loop
		cinbuf[i*2] = (float)(inbuf[i])/(float)(INT16_MAX+1);
		cinbuf[i*2+1] = 0.0;
	}

	//Do the CFFT
	arm_cfft_f32(&arm_cfft_sR_f32_len512, cinbuf, 0, 1); //144us - 188us

	//process cinbuf here
	//
	uint16_t cutoff = (uint16_t)((float)bufsize * freq);
	for (i=cutoff; i<(bufsize*2); i++)
		cinbuf[i] = 0;

	//Do the inverse CFFT
	arm_cfft_f32(&arm_cfft_sR_f32_len512, cinbuf, 1, 1); //166us - 222us

	//Convert complex values to real
	for (i=0; i<bufsize; i++) { //64us - 128us
		outbuf[i] = _CLAMP_F(cinbuf[i*2] * (INT16_MAX), INT16_MIN, INT16_MAX);
	}
}


void do_fft_shift_16(int16_t *inbuf, int16_t *outbuf, float *tmpbuf, int16_t shift)
{
	// const uint16_t bufsize = 16;
	// uint16_t i;
	// float tmp32[16];

	// for (i=0; i<bufsize; i++) { //24us loop
	// 	tmpbuf[i*2] = (float)(inbuf[i])/(float)(INT16_MAX+1);
	// 	tmpbuf[i*2+1] = 0.0;
	// }

	// arm_cfft_f32(&arm_cfft_sR_f32_len16, tmpbuf, 0, 1); //144us - 188us

	// for (i=0; i<(bufsize-1); i++)
	// {
	// 	tmpbuf[i*2] = tmpbuf[i*2+2];
	// 	tmpbuf[i*2+1] = tmpbuf[i*2+3];
	// }
	// // tmpbuf[bufsize*2-1] = 0;
	// // tmpbuf[bufsize*2-2] = 0;

	// arm_cfft_f32(&arm_cfft_sR_f32_len16, tmpbuf, 1, 1); //166us - 222us

	// arm_cmplx_mag_f32(tmpbuf, tmp32, bufsize);

	// for (i=0; i<bufsize; i++){
	// 	outbuf[i] = (int16_t)_CLAMP_F(tmp32[i] * (INT16_MAX), INT16_MIN, INT16_MAX);
	// 	if (tmpbuf[i*2]<0) outbuf[i] *= -1;
	// }

	// for (i=0; i<bufsize; i++) { //64us - 128us
	// 	outbuf[i] = _CLAMP_F(tmpbuf[i*2] * (INT16_MAX), INT16_MIN, INT16_MAX);
	// }


}


//Not working!
void do_fft_lpf_q15(q15_t *inbuf, q15_t *outbuf, q15_t *fftbuf, uint16_t bufsize, float freq)
{
	// arm_rfft_instance_q15 	rfft_instance;
	// arm_status 				status;
	// uint16_t 				i;

	// status = arm_rfft_init_q15(&rfft_instance, bufsize, 0, 0);
	// arm_rfft_q15(&rfft_instance, inbuf, fftbuf);

	// //process fftbuf[]
	// //
	// uint16_t cutoff = (uint16_t)((float)bufsize * freq);
	// for (i=cutoff; i<bufsize; i++)
	// 	fftbuf[i] = 0;

	// status = arm_rfft_init_q15(&rfft_instance, bufsize, 1, 0);
	// arm_rfft_q15(&rfft_instance, fftbuf, outbuf);

	// (void)status;
}