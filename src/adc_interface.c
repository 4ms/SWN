/*
 * adc_interface.c - Combines multiple ADC interfaces (internal peripherals, external chips)
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

#include "adc_interface.h"
#include "gpio_pins.h"
#include "timekeeper.h"
#include "drivers/ads8634_driver.h"
#include "drivers/adc_builtin_driver.h"

/*
 * How to Use:
 * 
 * Requires: ads8634_driver.c/h
 * Requires: adc_builtin_driver.c/h
 *
 * In ads8634_driver.h
 *   - Set NUMBER_OF_ADS8634_CHIPS to the number of ADS8634 chips
 *
 * In adc_interface.h:
 *   - Set the enum elements to the names of the ADCs you want to read.
 *
 * In this file (adc_interface.c):
 *   - Set the elements of NUM_HIRES_ADC_PER_CHIP[] equal to the number of ADCs used in each chip
 *   	-- It is assumed that if a chip uses <4 ADCs, then they are consecutive and start with channel 0
 *   - Set the voltage_ranges array elements to equal the desired initial voltage range for each ADC
 *   - Set the pins, GPIOs, channel, and sample_time for each built-in ADC in the builtin_adc_setup() function
 *
 * Then, call adc_init_all() in your main application
 */


//
// Config:
//
// Set this array to equal the number of ADC channels in each chip

const uint8_t 		NUM_HIRES_ADC_PER_CHIP [ NUMBER_OF_ADS8634_CHIPS ] = {3, 4};

//
// Config:
//
// Set the initial voltage range for the hires adcs

const enum RangeSel initial_voltage_ranges[NUM_HIRES_ADCS] = {
	RANGE_p10V,
	RANGE_p10V,
	RANGE_p10V,
	RANGE_p10V,
	RANGE_p10V,
	RANGE_p10V,
	RANGE_p10V
};

const uint8_t initial_oversampling_amts[NUM_HIRES_ADCS] = {
	10,
	10,
	10,
	10,
	10,
	10,
	10
};

//
// Config:
//
// Set the GPIO, pin, ADC_channel, and ADC_SampleTime values for every ADC pin used:

							//ToDo: make the builtinAdcSetup array a parameter
							//void builtin_adc_setup(builtinAdcSetup *adc_setup, uint8_t ADCx_number)
							//{
							//	if (ADCx_number == 1) 
							//	{
							//		adc_setup[BROWSE_ADC]...
							//	} else if (ADCx_number == 3) {
							// 		adc_setup[SLD_ADC_1]...
							// 
builtinAdcSetup	adc1_setup[NUM_BUILTIN_ADC1];
builtinAdcSetup	adc3_setup[NUM_BUILTIN_ADC3];

void builtin_adc_setup(void)
{
	//Setup and initialize the builtin ADCSs

	adc1_setup[BROWSE_ADC].gpio 			= GPIOA;
	adc1_setup[BROWSE_ADC].pin 				= GPIO_PIN_3;
	adc1_setup[BROWSE_ADC].channel 			= ADC_CHANNEL_3;
	adc1_setup[BROWSE_ADC].sample_time 		= ADC_SAMPLETIME_480CYCLES;

	adc1_setup[CHORD_ADC].gpio 			= GPIOB;
	adc1_setup[CHORD_ADC].pin 			= GPIO_PIN_1;
	adc1_setup[CHORD_ADC].channel 		= ADC_CHANNEL_9;
	adc1_setup[CHORD_ADC].sample_time 	= ADC_SAMPLETIME_480CYCLES;

	adc1_setup[LAT_ADC].gpio 			= GPIOC;
	adc1_setup[LAT_ADC].pin 			= GPIO_PIN_0;
	adc1_setup[LAT_ADC].channel 		= ADC_CHANNEL_10;
	adc1_setup[LAT_ADC].sample_time 	= ADC_SAMPLETIME_480CYCLES;

	adc1_setup[LFO_ADC].gpio 			= GPIOC;
	adc1_setup[LFO_ADC].pin 			= GPIO_PIN_1;
	adc1_setup[LFO_ADC].channel 		= ADC_CHANNEL_11;
	adc1_setup[LFO_ADC].sample_time 	= ADC_SAMPLETIME_480CYCLES;

	adc1_setup[RANDOM_ADC].gpio 		= GPIOC;
	adc1_setup[RANDOM_ADC].pin 			= GPIO_PIN_2;
	adc1_setup[RANDOM_ADC].channel 		= ADC_CHANNEL_12;
	adc1_setup[RANDOM_ADC].sample_time 	= ADC_SAMPLETIME_480CYCLES;

	adc1_setup[DEPTH_ADC].gpio 			= GPIOC;
	adc1_setup[DEPTH_ADC].pin 			= GPIO_PIN_3;
	adc1_setup[DEPTH_ADC].channel 		= ADC_CHANNEL_13;
	adc1_setup[DEPTH_ADC].sample_time 	= ADC_SAMPLETIME_480CYCLES;

	adc1_setup[SPHERE_ADC].gpio 		= GPIOC;
	adc1_setup[SPHERE_ADC].pin 			= GPIO_PIN_4;
	adc1_setup[SPHERE_ADC].channel 		= ADC_CHANNEL_14;
	adc1_setup[SPHERE_ADC].sample_time	= ADC_SAMPLETIME_480CYCLES;

	adc1_setup[WTSPREAD_ADC].gpio 		= GPIOC;
	adc1_setup[WTSPREAD_ADC].pin 		= GPIO_PIN_5;
	adc1_setup[WTSPREAD_ADC].channel 	= ADC_CHANNEL_15;
	adc1_setup[WTSPREAD_ADC].sample_time= ADC_SAMPLETIME_480CYCLES;

	//ADC3:
	adc3_setup[SLD_ADC_1].gpio 			= GPIOF;
	adc3_setup[SLD_ADC_1].pin 			= GPIO_PIN_3;
	adc3_setup[SLD_ADC_1].channel 		= ADC_CHANNEL_9;
	adc3_setup[SLD_ADC_1].sample_time 	= ADC_SAMPLETIME_480CYCLES;

	adc3_setup[SLD_ADC_2].gpio 			= GPIOF;
	adc3_setup[SLD_ADC_2].pin 			= GPIO_PIN_4;
	adc3_setup[SLD_ADC_2].channel 		= ADC_CHANNEL_14;
	adc3_setup[SLD_ADC_2].sample_time 	= ADC_SAMPLETIME_480CYCLES;

	adc3_setup[SLD_ADC_3].gpio 			= GPIOF;
	adc3_setup[SLD_ADC_3].pin 			= GPIO_PIN_5;
	adc3_setup[SLD_ADC_3].channel 		= ADC_CHANNEL_15;
	adc3_setup[SLD_ADC_3].sample_time 	= ADC_SAMPLETIME_480CYCLES;

	adc3_setup[SLD_ADC_4].gpio 			= GPIOF;
	adc3_setup[SLD_ADC_4].pin 			= GPIO_PIN_7;
	adc3_setup[SLD_ADC_4].channel 		= ADC_CHANNEL_5;
	adc3_setup[SLD_ADC_4].sample_time 	= ADC_SAMPLETIME_480CYCLES;

	adc3_setup[SLD_ADC_5].gpio 			= GPIOF;
	adc3_setup[SLD_ADC_5].pin 			= GPIO_PIN_8;
	adc3_setup[SLD_ADC_5].channel 		= ADC_CHANNEL_6;
	adc3_setup[SLD_ADC_5].sample_time 	= ADC_SAMPLETIME_480CYCLES;

	adc3_setup[SLD_ADC_6].gpio 			= GPIOF;
	adc3_setup[SLD_ADC_6].pin 			= GPIO_PIN_9;
	adc3_setup[SLD_ADC_6].channel 		= ADC_CHANNEL_7;
	adc3_setup[SLD_ADC_6].sample_time 	= ADC_SAMPLETIME_480CYCLES;

	adc3_setup[DISP_ADC].gpio 			= GPIOF;
	adc3_setup[DISP_ADC].pin 			= GPIO_PIN_6;
	adc3_setup[DISP_ADC].channel 		= ADC_CHANNEL_4;
	adc3_setup[DISP_ADC].sample_time	= ADC_SAMPLETIME_480CYCLES;

	adc3_setup[DISPPAT_ADC].gpio 		= GPIOF;
	adc3_setup[DISPPAT_ADC].pin 		= GPIO_PIN_10;
	adc3_setup[DISPPAT_ADC].channel 	= ADC_CHANNEL_8;
	adc3_setup[DISPPAT_ADC].sample_time = ADC_SAMPLETIME_480CYCLES;


}


//
// *_adc_raw[] is where raw adc data is stored
//
// extern these into any file that needs to read the adcs

float				hires_adc_raw	[ NUM_HIRES_ADCS ];
DMABUFFER uint16_t	builtin_adc1_raw[ NUM_BUILTIN_ADC1 ];
DMABUFFER uint16_t	builtin_adc3_raw[ NUM_BUILTIN_ADC3 ];


// --------------------------------------
// Below here are internal functions:
//



uint8_t hires_adc_isvalid_vrange(enum RangeSel vrange)
{
	if (vrange == RANGE_pm10V || vrange == RANGE_pm5V || vrange == RANGE_pm2_5V || vrange == RANGE_p10V || vrange == RANGE_p5V)
		return 1;
	else return 0;
}
 
hiresAdcSetup g_hires_adc_setup[ NUM_HIRES_ADCS ]; // setup variable is used internally 

void hires_adc_setup(uint8_t adc_id, hiresAdcSetup *s)
{
	if (hires_adc_isvalid_vrange(s->voltage_range))
		g_hires_adc_setup[adc_id].voltage_range = s->voltage_range;
	else
		g_hires_adc_setup[adc_id].voltage_range = RANGE_pm10V;

	if (s->oversampling_amt < MAX_OVERSAMPLE_BUFF_SIZE)
		g_hires_adc_setup[adc_id].oversampling_amt = s->oversampling_amt;
	else
		g_hires_adc_setup[adc_id].oversampling_amt = MAX_OVERSAMPLE_BUFF_SIZE;

}

//not tested yet!!!
void hires_adc_change_vrange(enum RangeSel *new_vranges)
{
	uint8_t 		adc_id;
	uint8_t			chipnum;
	uint8_t			base_adc_num;

	//Check all ranges are valid
	for (adc_id=0; adc_id<NUM_HIRES_ADCS; adc_id++) {
		if (!hires_adc_isvalid_vrange(new_vranges[adc_id]))
			new_vranges[adc_id] = RANGE_pm10V;
	}

	for (base_adc_num=0, chipnum = 0; chipnum<NUMBER_OF_ADS8634_CHIPS; chipnum++)
	{
		ads8634_set_vrange(chipnum, &(new_vranges[base_adc_num]), NUM_HIRES_ADC_PER_CHIP[chipnum]);
		base_adc_num +=  NUM_HIRES_ADC_PER_CHIP[chipnum];
	}

	for (adc_id=0; adc_id<NUM_HIRES_ADCS; adc_id++)
		g_hires_adc_setup[adc_id].voltage_range = new_vranges[adc_id];

}

enum RangeSel hires_adc_get_vrange(uint8_t adc_id)
{
	return g_hires_adc_setup[adc_id].voltage_range;
}
uint8_t hires_adc_is_bipolar(uint8_t adc_id)
{
	enum RangeSel vrange = g_hires_adc_setup[adc_id].voltage_range;
	return (vrange == RANGE_pm10V || vrange == RANGE_pm5V || vrange == RANGE_pm2_5V) ?1:0;
}


void hires_adc_init_all(float *adc_dest)
{
	enum RangeSel 	v_ranges[ NUM_HIRES_ADCS ];
	uint8_t			os_amts[ NUM_HIRES_ADCS ];
	uint8_t			chipnum;
	uint8_t			base_adc_num;
	uint8_t			i;

	base_adc_num = 0;
	for (chipnum = 0; chipnum<NUMBER_OF_ADS8634_CHIPS; chipnum++)
	{

		//Set the v_range array, filling v_ranges with the default value for the unused ADCs
		for (i=0; i<4; i++)
		{
			if (i < NUM_HIRES_ADC_PER_CHIP[chipnum])	
			{
				v_ranges[i] = g_hires_adc_setup[base_adc_num + i].voltage_range;
				os_amts[i] 	= g_hires_adc_setup[base_adc_num + i].oversampling_amt; 
			}
			else
			{
				v_ranges[i] = RANGE_pm10V;
				os_amts[i] 	= 0; 
			}
		}
		ads8634_init_with_SPIIRQ(&(adc_dest[base_adc_num]), NUM_HIRES_ADC_PER_CHIP[chipnum], chipnum, os_amts, v_ranges);

		base_adc_num +=  NUM_HIRES_ADC_PER_CHIP[chipnum];
	}
}

void adc_single_init(uint8_t adc_num)
{
	//populate the builtinAdcSetup arrays
	builtin_adc_setup();

	//Initialize and start the ADC and DMA
	if (adc_num==1) ADC1_Init(builtin_adc1_raw, NUM_BUILTIN_ADC1, adc1_setup);
	if (adc_num==3) ADC3_Init(builtin_adc3_raw, NUM_BUILTIN_ADC3, adc3_setup);
}

void adc_hires_init(void)
{
	uint8_t i;
	hiresAdcSetup hires_setup;

	//Setup and initialize hi-res ADC (ads8634 chips)
	for (i=0; i<NUM_HIRES_ADCS; i++)
	{
		hires_setup.voltage_range 		= initial_voltage_ranges[i];
		hires_setup.oversampling_amt 	= initial_oversampling_amts[i];
		hires_adc_setup(i, &hires_setup);
	}

	hires_adc_init_all(hires_adc_raw);
}

//
// adc_init_all()
//
// Initialize all ADCs, and assigns their values to raw_adc_array
//
void adc_init_all(void)
{
	adc_hires_init();
	adc_single_init(1);
	adc_single_init(3);
}
