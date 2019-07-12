/*
 * analog_conditioning.c - Filters and brackets ADC values to produce conditioned output
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

/*
	To use:
	Set the enum elements in analog_conditioning.h to be the names of the adc sources
	The elements must be in the same order as the adc enums, or else you'll need to edit the mapping in process_analog_conditioning()

	// Extern the analog[] array into anywhere you need to use it
	extern o_analog	analog[NUM_ANALOG_ELEMENTS];

	// Assign the setup values into analog[]:
	analog[ MY_CV_JACK1 ].fir_lpf_size = 100;
	analog[ MY_CV_JACK1 ].iir_lpf_size = 0;
	analog[ MY_CV_JACK1 ].bracket_size = 8;
	analog[ MY_CV_JACK1 ].polarity = AP_UNIPOLAR;

	// Call setup (which copies these values into the internal filter coefficient stuctures)

	setup_analog_conditioning();

	// Then setup and start a timer to run an interrupt which calls process_analog_conditioning()
	// Or, you can just call process_analog_conditioning() in the main loop
	// ---> The conditioned ADC values will be automatically put in the analog[] structure
	
*/


#include "analog_conditioning.h"
#include "adc_interface.h"
#include "drivers/switch_driver.h"
#include "flash_params.h"
#include "timekeeper.h"
#include "math_util.h"
#include "ui_modes.h"
#include "gpio_pins.h"
#include "drivers/ads8634_driver.h"

extern float				hires_adc_raw	[ NUM_HIRES_ADCS ];
extern DMABUFFER uint16_t	builtin_adc1_raw[ NUM_BUILTIN_ADC1 ];
extern DMABUFFER uint16_t	builtin_adc3_raw[ NUM_BUILTIN_ADC3 ];

extern SystemCalibrations *system_calibrations;

o_analog analog[NUM_ANALOG_ELEMENTS];

// UI
extern enum UI_Modes ui_mode;

//Private:
void process_analog_conditioning(void);
uint8_t AUTO_ZERO_WHEN_UNPLUGGED=0;


//
// Config: set the fir, iir, bracket, and polarity settings below:
//

#define 	MAX_FIR_LPF_SIZE 10

// Allows us to enable/disable auto-zeroing of switched jacks
// Only useful if we don't want to handle detecting the switch
// in the params level
// Note: in the SWN we handle this in params_pitch.c, so this
// function is not used 
void set_auto_zero_1voct_jacks(uint8_t enabled)
{
	AUTO_ZERO_WHEN_UNPLUGGED = (enabled) ? 1 : 0;
}

void setup_analog_conditioning(void)
{
	uint8_t i;

	//Set default values (to be overriden below)
	for (i=0;i<NUM_ANALOG_ELEMENTS;i++){
		analog[ i ].bracket_size = 12;

		analog[ i ].plug_sense_switch.ptype              = DISABLED;
		analog[ i ].plug_sense_switch.pin                = 0;
		analog[ i ].plug_sense_switch.gpio 	             = 0;
		analog[ i ].plug_sense_switch.pressed            = UNKNOWN_PRESS;
	}


	for (i=0;i<6;i++)
	{ // each channel has the same values
		analog[ A_VOCT + i ].fir_lpf_size                = 0;
		analog[ A_VOCT + i ].iir_lpf_size                = 0;
		analog[ A_VOCT + i ].bracket_size                = 17;	

		analog[ A_VOCT + i ].polarity                    = (hires_adc_is_bipolar(A_VOCT + i )) ? AP_BIPOLAR : AP_UNIPOLAR;
		analog[ A_VOCT + i ].plug_sense_switch.ptype     = PULLUP;
		analog[ A_VOCT + i ].plug_sense_switch.pressed   = RELEASED;

		analog[ A_SLIDER + i ].fir_lpf_size              = 0;
		analog[ A_SLIDER + i ].iir_lpf_size              = 10;
		analog[ A_SLIDER + i ].bracket_size              = 16;
		analog[ A_SLIDER + i ].polarity                  = AP_UNIPOLAR;
	}

	analog[ A_VOCT ].plug_sense_switch.pin 	         	 = GPIO_PIN_4;
	analog[ A_VOCT ].plug_sense_switch.gpio          	 = GPIOG;

	analog[ B_VOCT ].plug_sense_switch.pin 	         	 = GPIO_PIN_3;
	analog[ B_VOCT ].plug_sense_switch.gpio          	 = GPIOG;

	analog[ C_VOCT ].plug_sense_switch.pin 	         	 = GPIO_PIN_2;
	analog[ C_VOCT ].plug_sense_switch.gpio          	 = GPIOG;

	analog[ D_VOCT ].plug_sense_switch.pin 	         	 = GPIO_PIN_9;
	analog[ D_VOCT ].plug_sense_switch.gpio          	 = GPIOE;

	analog[ E_VOCT ].plug_sense_switch.pin 	         	 = GPIO_PIN_12;
	analog[ E_VOCT ].plug_sense_switch.gpio          	 = GPIOF;

	analog[ F_VOCT ].plug_sense_switch.pin 	         	 = GPIO_PIN_11;
	analog[ F_VOCT ].plug_sense_switch.gpio          	 = GPIOF;

	for (i=0;i<6;i++)	init_switch_gpio(&(analog[ A_VOCT + i ].plug_sense_switch));

	analog[ TRANSPOSE_CV ].fir_lpf_size              = 2;
	analog[ TRANSPOSE_CV ].iir_lpf_size              = 0;
	analog[ TRANSPOSE_CV ].bracket_size              = 17;
	analog[ TRANSPOSE_CV ].polarity                  = (hires_adc_is_bipolar(A_VOCT + i )) ? AP_BIPOLAR : AP_UNIPOLAR;

	analog[ TRANSPOSE_CV ].plug_sense_switch.pin     = GPIO_PIN_1;
	analog[ TRANSPOSE_CV ].plug_sense_switch.gpio    = GPIOF;
	analog[ TRANSPOSE_CV ].plug_sense_switch.ptype   = PULLUP;
	analog[ TRANSPOSE_CV ].plug_sense_switch.pressed = RELEASED;

	init_switch_gpio(&(analog[ TRANSPOSE_CV ].plug_sense_switch));
	
	analog[ CHORD_CV ].fir_lpf_size                  	 = 10;
	analog[ CHORD_CV ].iir_lpf_size                  	 = 0;
	analog[ CHORD_CV ].polarity                      	 = AP_UNIPOLAR;

	analog[ LATITUDE_CV ].fir_lpf_size               	 = 10;
	analog[ LATITUDE_CV ].iir_lpf_size               	 = 0;
	analog[ LATITUDE_CV ].polarity                   	 = AP_UNIPOLAR;

	analog[ WTSEL_SPREAD_CV ].fir_lpf_size           	 = 10;
	analog[ WTSEL_SPREAD_CV ].iir_lpf_size           	 = 0;
	analog[ WTSEL_SPREAD_CV ].polarity               	 = AP_UNIPOLAR;

	analog[ DEPTH_CV ].fir_lpf_size                  	 = 10;
	analog[ DEPTH_CV ].iir_lpf_size                  	 = 0;
	analog[ DEPTH_CV ].polarity                      	 = AP_UNIPOLAR;

	analog[ WBROWSE_CV ].fir_lpf_size                	 = 10;
	analog[ WBROWSE_CV ].iir_lpf_size                	 = 0;
	analog[ WBROWSE_CV ].polarity                    	 = AP_UNIPOLAR;

	analog[ WTSEL_CV ].fir_lpf_size                  	 = 10;
	analog[ WTSEL_CV ].iir_lpf_size                  	 = 0;
	analog[ WTSEL_CV ].polarity                      	 = AP_UNIPOLAR;

	analog[ DISP_CV ].fir_lpf_size                   	 = 10;
	analog[ DISP_CV ].iir_lpf_size                   	 = 0;
	analog[ DISP_CV ].polarity                       	 = AP_UNIPOLAR;

	analog[ DISPPAT_CV ].fir_lpf_size                	 = 10;
	analog[ DISPPAT_CV ].iir_lpf_size                	 = 0;
	analog[ DISPPAT_CV ].polarity                    	 = AP_UNIPOLAR;

	analog[ LFO_CV ].fir_lpf_size                    	 = 10;
	analog[ LFO_CV ].iir_lpf_size                    	 = 0;
	analog[ LFO_CV ].polarity                        	 = AP_UNIPOLAR;

	analog[ RANDOM_CV ].fir_lpf_size = 4;
	analog[ RANDOM_CV ].iir_lpf_size = 0;
	analog[ RANDOM_CV ].bracket_size = 0;
	analog[ RANDOM_CV ].polarity = AP_UNIPOLAR;

	setup_fir_filters();
	setup_iir_filters();
	setup_brackets();
}

void start_analog_conditioning(void)
{
	start_timer_IRQ(ANALOG_CONDITIONING_TIM_number, &process_analog_conditioning);
}


//
// IIR
//

#define NUM_IIR_FILTERS 	NUM_ANALOG_ELEMENTS

float IIR_LPF_COEF1[NUM_IIR_FILTERS];
float IIR_LPF_COEF2[NUM_IIR_FILTERS];

void setup_iir_filters(void)
{
	uint8_t filter_id;
	uint8_t analog_id;

	for (filter_id=0; filter_id<NUM_IIR_FILTERS; filter_id++)
	{
		analog_id = filter_id; //Do some other sort of mapping if NUM_IIR_FILTERS != NUM_ANALOG_ELEMENTS

		IIR_LPF_COEF1[filter_id] = 1.0/(float)(analog[analog_id].iir_lpf_size); //size=1000 ==> 0.001
		IIR_LPF_COEF2[filter_id] = 1.0 - IIR_LPF_COEF1[filter_id];				//size=1000 ==> 0.999
	}

}

static inline float apply_iir_lpf(uint8_t filter_id, float current_value, float new_value);
static inline float apply_iir_lpf(uint8_t filter_id, float current_value, float new_value)
{
	return (current_value * IIR_LPF_COEF2[ filter_id ]) + (new_value * IIR_LPF_COEF1[ filter_id ]);
}


//
// FIR
//

#define NUM_FIR_FILTERS 	NUM_ANALOG_ELEMENTS		//This can be reduced to save memory, if we map analog_id to filter_id in process_fir_filters() and setup_fir_filters()

//Todo: set these with malloc() as each FIR analog filter is created

uint32_t 	FIR_LPF_SIZE[ NUM_FIR_FILTERS ];
float	 	fir_lpf		[ NUM_FIR_FILTERS ][ MAX_FIR_LPF_SIZE ];
uint32_t 	fir_lpf_i	[ NUM_FIR_FILTERS ];

void setup_fir_filters(void)
{
	uint8_t analog_id, filter_id, i;
	float initial_value;

	for (filter_id=0; filter_id<NUM_FIR_FILTERS; filter_id++)
	{
		analog_id = filter_id; // Re-map here if NUM_FIR_FILTERS != NUM_ANALOG_ELEMENTS

		FIR_LPF_SIZE[ filter_id ] = analog[ analog_id ].fir_lpf_size;
		if (FIR_LPF_SIZE[ filter_id ] > MAX_FIR_LPF_SIZE) FIR_LPF_SIZE[filter_id] = MAX_FIR_LPF_SIZE;

			if (analog[ analog_id ].polarity == AP_BIPOLAR)
			initial_value = 2048;
		else
			initial_value = 0;

		for (i=0; i<MAX_FIR_LPF_SIZE; i++)
			fir_lpf[ filter_id ][i] = initial_value;

		analog[ analog_id ].lpf_val = initial_value;
	}
}

//Replaces the highest and lowest values with the average value, and returns the average
static inline float average_excluding_extremes(uint8_t filter_id);
static inline float average_excluding_extremes(uint8_t filter_id)
{
	uint8_t i;
	uint16_t max=0, min=0xFFFF, max_i=0, min_i=0, sum=0, num_elements;

	num_elements = FIR_LPF_SIZE[ filter_id ];
	for (i=0;i<num_elements;i++)
	{
		if (fir_lpf[filter_id][i] > max) {max = fir_lpf[filter_id][i]; max_i=i;}
		if (fir_lpf[filter_id][i] < min) {min = fir_lpf[filter_id][i]; min_i=i;}
		sum+=fir_lpf[filter_id][i];
	}
	sum = sum - max - min;
	num_elements-=2;

	fir_lpf[filter_id][max_i] = sum/num_elements;
	fir_lpf[filter_id][min_i] = sum/num_elements;

	return ((float)sum)/((float)num_elements);
}


static inline float apply_fir_lpf(uint8_t filter_id, float current_value, float new_value);
static inline float apply_fir_lpf(uint8_t filter_id, float current_value, float new_value)
{
	float old_val;

	old_val = fir_lpf[ filter_id ][ fir_lpf_i[filter_id] ];

	fir_lpf[ filter_id ][ fir_lpf_i[filter_id] ] = new_value;

	//Increment the index, wrapping around the whole buffer
	if (++fir_lpf_i[ filter_id ] >= FIR_LPF_SIZE[ filter_id ]) fir_lpf_i[ filter_id ] = 0;

	//Calculate the arithmetic average (FIR LPF)
	current_value = ((current_value * (float)FIR_LPF_SIZE[ filter_id ]) - old_val + new_value) / (float)FIR_LPF_SIZE[ filter_id ];

	//if (fir_lpf_i[filter_id] == 0)	current_value = average_excluding_extremes(filter_id);

	current_value=_CLAMP_F(current_value, 0, 4095);

	return current_value;	
}



//
// Bracketing
//

#define NUM_BRACKETS 	NUM_ANALOG_ELEMENTS

int16_t	BRACKET_SIZE[ NUM_BRACKETS ];

void setup_brackets(void)
{
	uint8_t analog_id, filter_id;

	for (filter_id=0; filter_id<NUM_BRACKETS; filter_id++)
	{
		analog_id = filter_id; //remap here if necessary

		BRACKET_SIZE[ filter_id ] = analog[ analog_id ].bracket_size; 
	}

}

static inline uint16_t apply_bracket(uint8_t filter_id, uint16_t current_value, float new_value);
static inline uint16_t apply_bracket(uint8_t filter_id, uint16_t current_value, float new_value)
{
	int16_t t;

	t = (int16_t)new_value - current_value;

	if (t > BRACKET_SIZE[filter_id])
	{
		// current_value = (uint16_t)new_value - BRACKET_SIZE[filter_id];
		current_value = (uint16_t)new_value;
	}

	else if (t < -BRACKET_SIZE[filter_id])
	{
		// current_value = (uint16_t)new_value + BRACKET_SIZE[filter_id];
		current_value = (uint16_t)new_value;
	}

	return current_value;
}


void process_analog_conditioning(void)
{
	uint8_t i; //analog element ID

	for (i=0; i<NUM_ANALOG_ELEMENTS; i++)
	{
		//Check if the adc has a sense pin and if there's no cable patched.
		//If no cable, then set the output values all to 0 if it's unipolar, or 2048 if it's bipolar
		if (analog[i].plug_sense_switch.ptype != DISABLED)	analog[i].plug_sense_switch.pressed = read_switch_state(&(analog[i].plug_sense_switch)) ? PRESSED : RELEASED;
		else 												analog[i].plug_sense_switch.pressed = UNKNOWN_PRESS;

		// Map raw data from adc arrays into analog[] raw data
		//
		if (i < NUM_HIRES_ADCS) 							analog[i].raw_val = hires_adc_raw[i];
		else if (i < (NUM_HIRES_ADCS + NUM_BUILTIN_ADC1))	analog[i].raw_val = builtin_adc1_raw[ i - NUM_HIRES_ADCS ];
		else												analog[i].raw_val = builtin_adc3_raw[ i - (NUM_HIRES_ADCS + NUM_BUILTIN_ADC1) ];


		// Apply calibration offset to jack in play mode (only in play mode, not in calibration mode)
		// 
		
		if ((ui_mode != SELECT_PARAMS) && (ui_mode != RGB_COLOR_ADJUST))
		{
			// Auto-zero unplugged jacks with sense pins (only in PLAY mode, not in calibration mode)
			//
			if ((AUTO_ZERO_WHEN_UNPLUGGED) && (analog[i].plug_sense_switch.pressed == RELEASED))
			{
				if (analog[i].polarity == AP_UNIPOLAR) 
				{
					analog[i].lpf_val 		= 0;
					analog[i].raw_val 		= 0;
					analog[i].bracketed_val = 0;
				} else
				{
					analog[i].lpf_val 		= 2048;
					analog[i].raw_val 		= 2048;
					analog[i].bracketed_val = 2048;
				}
			}
			
			// else if (analog[i].plug_sense_switch.pressed == RELEASED)
			// 	analog[i].raw_val = _CLAMP_F(analog[i].raw_val - system_calibrations->cv_jack_unplugged_offset[i], 0 ,4095);
			
			// Plugged jacks with sense pins:
			else if (analog[i].plug_sense_switch.pressed == PRESSED)
				analog[i].raw_val = _CLAMP_F(analog[i].raw_val - system_calibrations->cv_jack_plugged_offset[i], 0, 4095);

			// Unplugged jacks with sense pins and jacks with no sense pin
			else 
				analog[i].raw_val = _CLAMP_F(analog[i].raw_val - system_calibrations->cv_jack_unplugged_offset[i], 0, 4095);
		}

		// Apply LPFs
		//
		if (analog[i].fir_lpf_size) 	analog[i].lpf_val = apply_fir_lpf(i, analog[i].lpf_val, analog[i].raw_val);
		else
		if (analog[i].iir_lpf_size) 	analog[i].lpf_val = apply_iir_lpf(i, analog[i].lpf_val, analog[i].raw_val);
		else							analog[i].lpf_val = analog[i].raw_val;

		// Apply Brackets
		//
		if (analog[i].bracket_size)		analog[i].bracketed_val = apply_bracket(i, analog[i].bracketed_val, analog[i].lpf_val);
		else							analog[i].bracketed_val = analog[i].raw_val;
	}

}

uint8_t analog_jack_plugged(enum AnalogElements jacknum) {
	return analog[jacknum].plug_sense_switch.pressed;
}

void read_cv_jack_calibration_unplugged_offsets(void)
{
	uint8_t i;

	for(i=0;i<NUM_ANALOG_ELEMENTS;i++)
	{
		//Don't calibrate sliders or RANDOMCV
		if ((i<A_SLIDER || i>F_SLIDER) && (i != RANDOM_CV)) 
			system_calibrations->cv_jack_unplugged_offset[i] = (int16_t)analog[i].lpf_val;
	}
	system_calibrations->cv_jack_unplugged_offset[RANDOM_CV] = 0;
	system_calibrations->cv_jack_unplugged_offset[A_SLIDER] = 0;
	system_calibrations->cv_jack_unplugged_offset[B_SLIDER] = 0;
	system_calibrations->cv_jack_unplugged_offset[C_SLIDER] = 0;
	system_calibrations->cv_jack_unplugged_offset[D_SLIDER] = 0;
	system_calibrations->cv_jack_unplugged_offset[E_SLIDER] = 0;
	system_calibrations->cv_jack_unplugged_offset[F_SLIDER] = 0;
}

void set_default_cv_jack_calibration_offsets(void)
{
	uint8_t i;

	for(i=0;i<NUM_ANALOG_ELEMENTS;i++)
	{
		system_calibrations->cv_jack_unplugged_offset[i] = 0;

		if (i<A_VOCT || i>TRANSPOSE_CV)
			system_calibrations->cv_jack_plugged_offset[i] = 0;
		else
		{
			//Todo: is this necessary? 
			if (hires_adc_is_bipolar(i))
				system_calibrations->cv_jack_plugged_offset[i] = 0;
			else
				system_calibrations->cv_jack_plugged_offset[i] = 0;

		}
	}
}
