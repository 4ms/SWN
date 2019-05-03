/*
 * audio_util.c - Audio processing routines
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

#include "audio_util.h"
#include "globals.h"
#include "oscillator.h"
#include "gpio_pins.h"

/* @brief	Splits a stereo 16-bit array into an array
 * 			The src array will have four times as many elements as the ldst or rdst
 *
 * @param	sz is the number of elements in the src array. Should be a multiple of 4
 * 			src is an array of 16-bit elements arranged Lmsb Llsb Rmsb Rlsb Lmsb Llsb Rmsb Rlsb
 * 			ldst and rdst are arrays of 32-bit elements that really just have 24 significant bits
 */
// void audio_convert_2x16_to_stereo24(uint16_t sz, int16_t *src, int32_t *ldst, int32_t *rdst)
// {
// 	while(sz)
// 	{
// 		*ldst = ((int32_t)(*src++))<<16;
// 		*ldst += *src++;
// 		sz-=4;
// 	}
// }

// void audio_convert_stereo24_to_2x16(uint16_t sz, int32_t *lsrc, int32_t *rsrc, int16_t *dst)
// {
// 	while(sz)
// 	{
// 		*dst++=(int16_t)(*lsrc >> 16);
// 		*dst++=(int16_t)(*lsrc++ & 0x0000FFFF);
// 		*dst++=(int16_t)(*rsrc >> 16);
// 		*dst++=(int16_t)(*rsrc++ & 0x0000FFFF);
// 		sz-=4;
// 	}
// }

void interleaved_leftchan_only(uint16_t sz, int32_t *src, int32_t *ldst)
{
	while(sz--)
	{
		*ldst++ = *src++;
		UNUSED(*src++);
	}

}

void interleaved_to_stereo(uint16_t src_sz, int32_t *src, int32_t *ldst, int32_t *rdst)
{
	while(src_sz)
	{
		*ldst++ = *src++ ;
		*rdst++ = *src++;
		src_sz-=2;
	}

}

void stereo_to_interleaved(uint16_t dst_sz, int32_t *lsrc, int32_t *rsrc, int32_t *dst)
{
	while(dst_sz)
	{
		*dst++ = (*lsrc++) >> 8; //convert 32 to 24bit
		*dst++ = (*rsrc++) >> 8; //convert 32 to 24bit
		dst_sz-=2;
	}
}

//Given a 24-bit signed integer (right-aligned in a int32_t)
//Output a 32-bit signed integer
//Essentially this sets the top 8 bits if the sign bit is set 
int32_t convert_s24_to_s32(int32_t src)
{
	uint32_t in_24bit;
	int32_t in_s24bit;

	in_24bit = src << 8;						//0x007FFFFF max, 0x00FFFFFF first negative value (-1)
	in_s24bit = (int32_t)in_24bit;
	in_s24bit >>= 8;
	return (in_s24bit);
}




