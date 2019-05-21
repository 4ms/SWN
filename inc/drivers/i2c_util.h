 /*
 * i2c_util.h - Utilities for i2c
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

#pragma once

#include <stm32f7xx.h>

struct i2cTimingReg{
	uint8_t PRESC; 			//top 4 bits: (PRESC + 1) * tI2CCLK = tPRESC
							//bottom 4 bits is ignored

	uint8_t SCLDEL_SDADEL; 	//top 4 bits: SCLDEL * tPRESC = SCL Delay between SDA edge and SCL rising edge
							//bottom 4 bits: = SDADEL * tPRESC = SDA Delay between SCL falling edge and SDA edge

	uint8_t SCLH;			//SCL high period = (SCLH+1) * tPRESC
	uint8_t SCLL;			//SCL low period = (SCLL+1) * tPRESC
};

static inline uint32_t set_i2c_timing(struct i2cTimingReg *t);
static inline uint32_t set_i2c_timing(struct i2cTimingReg *t)
{
	return ((t->PRESC) << 24) | ((t->SCLDEL_SDADEL) << 16) | ((t->SCLH) << 8) | ((t->SCLL) << 0);
}
