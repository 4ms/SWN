/*
 * hardware_controls.h - hardware setup
 *
 * Author: Dan Green (danngreen1@gmail.com), Hugo Paris (hugoplho@gmail.com)
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

//Naming convention:
// all_lowercase: varibles
// ALL_UPPERCASE: constants/defines
// camelCase (or Camel): typedef, enum, struct type name
// o_camelCase: object (struct) definitions


#pragma once

#include <stm32f7xx.h>
#include "globals.h"


enum rotaryMap{				
	rotm_OCT,				//  0 <<not an LED encoder
	rotm_TRANSPOSE,			//  1 <<not an LED encoder
	rotm_LFOSPEED,			//  2 <<not an LED encoder
	rotm_LFOSHAPE,			//  3 <<not an LED encoder
	rotm_PRESET, 			//  4 <<not an LED encoder
	rotm_DEPTH,				//  5
	rotm_LATITUDE,			//  6
	rotm_LONGITUDE,			//  7
	rotm_WAVETABLE,			//  8 <<not an LED encoder
	
	NUM_ROTARIES
};

enum LedRotaryMap{				//Todo: fix this list to only include actual LED rotaries. Also will need to maintain <-> relation with rotaryMap (search all for NUM_ROTARIES)
	ledrotm_OCT,				//  0 removed
	ledrotm_TRANSPOSE,			//  1 removed 
	ledrotm_LFOSPEED,			//  2 removed
	ledrotm_LFOSHAPE,			//  3 removed
	ledrotm_PRESET, 			//  4 removed
	ledrotm_DEPTH,				//  5
	ledrotm_LATITUDE,			//  6
	ledrotm_LONGITUDE,			//  7

	NUM_LED_ROTARIES
};

enum buttonMap{
	butm_A_BUTTON,			//  0
	butm_B_BUTTON,			//  1
	butm_C_BUTTON,			//  2
	butm_D_BUTTON,			//  3
	butm_E_BUTTON,			//  4
	butm_F_BUTTON,			//  5
	butm_LFOVCA_BUTTON,		//  6
	butm_LFOMODE_BUTTON, 	//  7

	NUM_BUTTONS
};

enum Switches{
	FINE_BUTTON,
	VOCTSW,
	CLK_SENSE,
	WAVEFORMIN_SENSE,

	NUM_SWITCHES
};


enum VoctVcaStates {
	SW_VOCT,
	SW_VCA,

	NUM_VOCT_VCA_STATES
};

enum monoLedMap{
	mledm_SLIDER_A,
	mledm_SLIDER_B,
	mledm_SLIDER_C,
	mledm_SLIDER_D,
	mledm_SLIDER_E,
	mledm_SLIDER_F,

#if (PCB_VERSION==23)
	mledm_CLKIN,
	mledm_INCLIPL,
	mledm_INCLIPR,
#endif

	NUM_MONO_LED
};

enum monoLedState {
	LED_ON,					// 0 
	LED_OFF,				// 1
	
	NUM_MONO_LED_STATES
};



enum SwitchModes {
	MOMENTARY_RESPONSE,   	// 0
	LATCH_RESPONSE,	       	// 1

	NUM_BUTTON_MODES
};

enum PinTypes {
	PULLUP, 				// 0
	PULLDOWN,	  			// 1
	NOPULL,					// 2
	DISABLED,				// 3
	
	NUM_PIN_TYPES
};

enum StepTypes {
	HALFSTEP, 				// 0
	FULLSTEP,	  			// 1

	NUM_STEP_TYPES
};

enum PressTypes {
	RELEASED, 				// 0
	PRESSED,				// 1
	SHORT_PRESSED,  		// 2
	MED_PRESSED,  			// 3
	LONG_PRESSED,  			// 4
	UNKNOWN_PRESS,			// 5

	NUM_PRESS_TYPES
};


// HARDWARE "CLASSES" -------------------------------------------- //

typedef struct o_macro_states {
	uint8_t all_af_buttons_released;
} o_macro_states;

typedef struct o_switch {
	enum PressTypes		pressed;
	GPIO_TypeDef 	   *gpio;
	uint32_t			pin;
	enum PinTypes		ptype; 	
} o_switch;


typedef struct o_turn {
	uint8_t				state;
	int8_t				queue;
	GPIO_TypeDef 	   *A_gpio;
	uint32_t			A_pin;
	GPIO_TypeDef 	   *B_gpio;
	uint32_t			B_pin;
	enum StepTypes 		step_size; 
} o_turn;

typedef struct o_rotary {
	o_turn				turn;
	o_switch			hwswitch;
} o_rotary;

typedef struct o_button{
	o_switch		    hwswitch;
} o_button;

typedef struct o_monoLed {
	GPIO_TypeDef 	   *gpio;
	uint32_t			pin;
} o_monoLed;


// INITIALISATION FUNCTIONS -------------------------------------------- //

// Hardware
void init_rotaries(void);
void init_buttons(void);
void init_switches(void);
void init_mono_leds(void);

