/*
 * analog_conditioning.h - Filters and brackets ADC values to produce conditioned output
 *
 */

#pragma once
#include <stm32f7xx.h>
#include "hardware_controls.h"

// The order of this is the concatenated elements of enum HiresAdcChannels, enum BuiltinAdc1Channels, and enum BuiltinAdc3Channels from adc_interface.h
// And changing _ADC to _CV or _SLIDER
// If the order doesn't line up with the adc enums, then mapping must be edited in process_analog_conditioning()
enum AnalogElements{
	A_VOCT,
	B_VOCT,
	C_VOCT,
	D_VOCT,
	E_VOCT,
	F_VOCT,
	TRANSPOSE_CV, 	//6

	WBROWSE_CV,				
	CHORD_CV,
	LATITUDE_CV,
	LFO_CV,
	RANDOM_CV,		//11
	DEPTH_CV,
	WTSEL_CV,
	WTSEL_SPREAD_CV,	//14		

	A_SLIDER, 		//15
	B_SLIDER,
	C_SLIDER,
	D_SLIDER,
	E_SLIDER,
	F_SLIDER,

	DISP_CV,
	DISPPAT_CV,

	NUM_ANALOG_ELEMENTS
};



enum AnalogPolarity{
	AP_UNIPOLAR,
	AP_BIPOLAR
};


typedef struct o_analog {
	//Value outputs:
	float				raw_val;
	float				lpf_val;
	uint16_t			bracketed_val;

	//Settings (input)
	uint16_t			iir_lpf_size;		//size of iir average. 0 = disabled. if fir_lpf_size > 0, then iir average is disabled.
	uint16_t			fir_lpf_size; 		//size of moving average (number of samples to average). 0 = disabled.
	uint16_t			bracket_size;		//size of bracket (ignore changes when old_val-bracket_size < new_val < old_val+bracket_size)
	enum AnalogPolarity	polarity;			//AP_UNIPOLAR or AP_BIPOLAR

	o_switch			plug_sense_switch;		//Used with stereo jacks, where the ring terminal is used as a sense pin to detect if a cable is plugged in

} o_analog;

uint8_t analog_jack_plugged(enum AnalogElements jacknum);

void setup_iir_filters(void);
void setup_fir_filters(void);
void setup_brackets(void);

void setup_analog_conditioning(void);
void start_analog_conditioning(void);

void set_auto_zero_1voct_jacks(uint8_t enabled);

void set_default_cv_jack_calibration_offsets(void);
void read_cv_jack_calibration_unplugged_offsets(void);

