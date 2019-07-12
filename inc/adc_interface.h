/*
 * adc_interface.h - Combines multiple ADC interfaces (internal peripherals, external chips)
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

//
// Config:
//
// Un/comment the #defines below:
//

#define USE_HIRES_ADC
#define USE_BUILTIN_ADC

#ifdef USE_HIRES_ADC
#include "drivers/ads8634_driver.h"

typedef struct hiresAdcSetup {
	enum RangeSel	voltage_range;		//voltage range (ADS8634 chip only)
	uint8_t			oversampling_amt;	//amount of oversampling (number of samples to read and average before reporting a final value)
} hiresAdcSetup;

#endif

#ifdef USE_BUILTIN_ADC
#include "drivers/adc_builtin_driver.h"
#endif




//
// Config:
//
// Set the names of the ADC channels being used
//

//ads8634 chips:
enum HiresAdcChannels{
	CH1_JACK_ADC,
	CH2_JACK_ADC,
	CH3_JACK_ADC,
	CH4_JACK_ADC,
	CH5_JACK_ADC,
	CH6_JACK_ADC,
	TRANSPOSE_JACK_ADC,

	NUM_HIRES_ADCS
};

// ADC1
enum BuiltinAdc1Channels{
	BROWSE_ADC,		//  0
	CHORD_ADC,		//  1
	LAT_ADC,		//  2
	LFO_ADC, 		//  3
	RANDOM_ADC,		//  4
	DEPTH_ADC,		//  5
	SPHERE_ADC,		//  6
	WTSPREAD_ADC,	//  7
	
	NUM_BUILTIN_ADC1
};

// ADC2
enum BuiltinAdc2Channels{	
	NUM_BUILTIN_ADC2
};

// ADC3
enum BuiltinAdc3Channels{
	SLD_ADC_1,		//  0
	SLD_ADC_2,		//  1
	SLD_ADC_3,		//  2
	SLD_ADC_4, 		//  3
	SLD_ADC_5,		//  4
	SLD_ADC_6,		//  5
	DISP_ADC,		//  6
	DISPPAT_ADC, 	//  7
	
	NUM_BUILTIN_ADC3
};

#define NUM_BUILTIN_ADCS	(NUM_BUILTIN_ADC1 + NUM_BUILTIN_ADC2 + NUM_BUILTIN_ADC3)
#define NUM_ADCS 			(NUM_HIRES_ADCS + NUM_BUILTIN_ADCS)


void adc_init_all(void);
void adc_single_init(uint8_t adc_num);
void adc_hires_init(void);

#ifdef USE_HIRES_ADC
void hires_adc_change_vrange(enum RangeSel *new_vranges);
enum RangeSel hires_adc_get_vrange(uint8_t adc_id);
uint8_t hires_adc_is_bipolar(uint8_t adc_id);
#endif
