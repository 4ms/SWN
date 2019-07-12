/*
 * gpio_pins.h - setup digital input and output pins
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


#include "globals.h"
#include "gpio_pins.h"
#include "drivers/rotary_driver.h"
#include "drivers/switch_driver.h"
#include "hardware_controls.h"

// GPIO map
GPIO_TypeDef 	*HW_ROTARY_SW_GPIO[NUM_ROTARIES];					// Rotaries
uint16_t 	  	HW_ROTARY_SW_PIN[NUM_ROTARIES]; 
enum PinTypes 	HW_ROTARY_SW_PINTYPE[NUM_ROTARIES]; 
enum StepTypes 	HW_ROTARY_TURN_STEP[NUM_ROTARIES];
GPIO_TypeDef 	*HW_ROTARY_A_GPIO[NUM_ROTARIES];
uint16_t 	  	HW_ROTARY_A_PIN[NUM_ROTARIES]; 
GPIO_TypeDef    *HW_ROTARY_B_GPIO[NUM_ROTARIES];
uint16_t	    HW_ROTARY_B_PIN[NUM_ROTARIES]; 						// FixMe: PRELOAD_ROTARY_ADDR <--- what does this mean? Is it obselete? Is it for the LED preload?
GPIO_TypeDef    *HW_BUTTON_SW_GPIO[NUM_BUTTONS];					// Buttons
uint16_t 	    HW_BUTTON_SW_PIN[NUM_BUTTONS]; 
GPIO_TypeDef    *HW_SW_GPIO[NUM_SWITCHES];							// Switches
uint16_t 	    HW_SW_PIN[NUM_SWITCHES]; 
GPIO_TypeDef    *HW_MONO_LED_GPIO[NUM_MONO_LED];					// Mono LEDs
uint16_t 	    HW_MONO_LED_PIN[NUM_MONO_LED]; 

// Hardware objects
// FixMe: can we avoid using "extern" somehow
extern	o_rotary 		rotary[NUM_ROTARIES];
extern	o_button 		button[NUM_BUTTONS];
extern	o_switch 		hwSwitch[NUM_SWITCHES];						//switch position = note when mutes_sw and keys_sw state is both off
extern 	o_monoLed   	monoLed[NUM_MONO_LED];


//FixMe: this will be done using const initializations in gpio_pins.h
void set_gpio_map(void){
	// ROTARIES

	HW_ROTARY_SW_GPIO[rotm_WAVETABLE] 		= GPIOD; 
	HW_ROTARY_SW_PIN[rotm_WAVETABLE]		= GPIO_PIN_9; 
	HW_ROTARY_SW_PINTYPE[rotm_WAVETABLE]	= PULLUP; 
	HW_ROTARY_TURN_STEP[rotm_WAVETABLE]		= HALFSTEP; 
	#if (PCB_VERSION>=100)
	HW_ROTARY_A_GPIO[rotm_WAVETABLE] 		= GPIOD;
	HW_ROTARY_A_PIN[rotm_WAVETABLE]			= GPIO_PIN_10; 
	HW_ROTARY_B_GPIO[rotm_WAVETABLE] 		= GPIOC;
	HW_ROTARY_B_PIN[rotm_WAVETABLE]			= GPIO_PIN_6; 
	#else
	HW_ROTARY_A_GPIO[rotm_WAVETABLE] 		= GPIOC;
	HW_ROTARY_A_PIN[rotm_WAVETABLE]			= GPIO_PIN_6; 
	HW_ROTARY_B_GPIO[rotm_WAVETABLE] 		= GPIOD;
	HW_ROTARY_B_PIN[rotm_WAVETABLE]			= GPIO_PIN_10; 
	#endif

	HW_ROTARY_SW_GPIO[rotm_OCT] 			= GPIOD; 
	HW_ROTARY_SW_PIN[rotm_OCT]				= GPIO_PIN_3; 
	HW_ROTARY_SW_PINTYPE[rotm_OCT] 			= OCTAVETRANSPOSE_ROTARY_PULL; 
	HW_ROTARY_TURN_STEP[rotm_OCT]			= NON_WT_ROTARY_STEPSIZE; 
	HW_ROTARY_A_GPIO[rotm_OCT] 				= GPIOD;
	HW_ROTARY_B_GPIO[rotm_OCT] 				= GPIOD;
	#ifdef OCTAVETRANSPOSE_DIR_REV
	HW_ROTARY_A_PIN[rotm_OCT]				= GPIO_PIN_5; 
	HW_ROTARY_B_PIN[rotm_OCT]				= GPIO_PIN_4; 
	#else
	HW_ROTARY_A_PIN[rotm_OCT]				= GPIO_PIN_4; 
	HW_ROTARY_B_PIN[rotm_OCT]				= GPIO_PIN_5; 
	#endif

	HW_ROTARY_SW_GPIO[rotm_LFOSPEED] 		= GPIOG;
	HW_ROTARY_SW_PIN[rotm_LFOSPEED]			= GPIO_PIN_9; 
	HW_ROTARY_SW_PINTYPE[rotm_LFOSPEED] 	= LFOANDPRESET_ROTARY_PULL; 
	HW_ROTARY_TURN_STEP[rotm_LFOSPEED]		= NON_WT_ROTARY_STEPSIZE; 
	HW_ROTARY_A_GPIO[rotm_LFOSPEED] 		= GPIOG;
	HW_ROTARY_A_PIN[rotm_LFOSPEED]			= GPIO_PIN_10; 
	HW_ROTARY_B_GPIO[rotm_LFOSPEED] 		= GPIOG;
	HW_ROTARY_B_PIN[rotm_LFOSPEED]			= GPIO_PIN_11; 

	HW_ROTARY_SW_GPIO[rotm_PRESET] 			= GPIOD; 
	HW_ROTARY_SW_PIN[rotm_PRESET]			= GPIO_PIN_15; 
	HW_ROTARY_TURN_STEP[rotm_PRESET]		= NON_WT_ROTARY_STEPSIZE; 
	HW_ROTARY_SW_PINTYPE[rotm_PRESET] 		= LFOANDPRESET_ROTARY_PULL; 
	HW_ROTARY_A_GPIO[rotm_PRESET] 			= GPIOG;
	HW_ROTARY_A_PIN[rotm_PRESET]			= GPIO_PIN_5; 
	HW_ROTARY_B_GPIO[rotm_PRESET] 			= GPIOG;
	HW_ROTARY_B_PIN[rotm_PRESET]			= GPIO_PIN_6; 

	HW_ROTARY_SW_GPIO[rotm_TRANSPOSE] 		= GPIOC; 
	HW_ROTARY_SW_PIN[rotm_TRANSPOSE]		= GPIO_PIN_13; 
	HW_ROTARY_SW_PINTYPE[rotm_TRANSPOSE] 	= OCTAVETRANSPOSE_ROTARY_PULL; 
	HW_ROTARY_TURN_STEP[rotm_TRANSPOSE]		= NON_WT_ROTARY_STEPSIZE; 
	HW_ROTARY_A_GPIO[rotm_TRANSPOSE] 		= GPIOC;
	HW_ROTARY_B_GPIO[rotm_TRANSPOSE] 		= GPIOC;
	#ifdef OCTAVETRANSPOSE_DIR_REV
	HW_ROTARY_A_PIN[rotm_TRANSPOSE]			= GPIO_PIN_15; 
	HW_ROTARY_B_PIN[rotm_TRANSPOSE]			= GPIO_PIN_14; 
	#else
	HW_ROTARY_A_PIN[rotm_TRANSPOSE]			= GPIO_PIN_14; 
	HW_ROTARY_B_PIN[rotm_TRANSPOSE]			= GPIO_PIN_15; 
	#endif

	HW_ROTARY_SW_GPIO[rotm_LFOSHAPE] 		= GPIOB; 
	HW_ROTARY_SW_PIN[rotm_LFOSHAPE]			= GPIO_PIN_4; 
	HW_ROTARY_SW_PINTYPE[rotm_LFOSHAPE] 	= LFOANDPRESET_ROTARY_PULL; 
	HW_ROTARY_TURN_STEP[rotm_LFOSHAPE]		= NON_WT_ROTARY_STEPSIZE; 
	HW_ROTARY_A_GPIO[rotm_LFOSHAPE] 		= GPIOB;
	HW_ROTARY_A_PIN[rotm_LFOSHAPE]			= GPIO_PIN_5; 
	HW_ROTARY_B_GPIO[rotm_LFOSHAPE] 		= GPIOB;
	HW_ROTARY_B_PIN[rotm_LFOSHAPE]			= GPIO_PIN_6; 

	HW_ROTARY_SW_GPIO[rotm_DEPTH] 			= GPIOF; 
	HW_ROTARY_SW_PIN[rotm_DEPTH]			= GPIO_PIN_15;
	HW_ROTARY_SW_PINTYPE[rotm_DEPTH] 		= PULLDOWN; 
	HW_ROTARY_TURN_STEP[rotm_DEPTH]			= FULLSTEP; 
	HW_ROTARY_A_GPIO[rotm_DEPTH] 			= GPIOG;
	HW_ROTARY_A_PIN[rotm_DEPTH]				= GPIO_PIN_0; 
	HW_ROTARY_B_GPIO[rotm_DEPTH] 			= GPIOG;
	HW_ROTARY_B_PIN[rotm_DEPTH]				= GPIO_PIN_1; 

	HW_ROTARY_SW_GPIO[rotm_LATITUDE] 		= GPIOF; 
	HW_ROTARY_SW_PIN[rotm_LATITUDE]			= GPIO_PIN_0; 
	HW_ROTARY_SW_PINTYPE[rotm_LATITUDE] 	= PULLDOWN; 
	HW_ROTARY_TURN_STEP[rotm_LATITUDE]		= FULLSTEP; 
	HW_ROTARY_A_GPIO[rotm_LATITUDE]  		= GPIOF;
	HW_ROTARY_A_PIN[rotm_LATITUDE]			= GPIO_PIN_13; 
	HW_ROTARY_B_GPIO[rotm_LATITUDE] 		= GPIOF;
	HW_ROTARY_B_PIN[rotm_LATITUDE]			= GPIO_PIN_14; 

	HW_ROTARY_SW_GPIO[rotm_LONGITUDE] 		= GPIOG; 
	HW_ROTARY_SW_PIN[rotm_LONGITUDE]		= GPIO_PIN_15; 
	HW_ROTARY_SW_PINTYPE[rotm_LONGITUDE]	= PULLDOWN; 
	HW_ROTARY_TURN_STEP[rotm_LONGITUDE]		= FULLSTEP; 
	HW_ROTARY_A_GPIO[rotm_LONGITUDE] 		= GPIOG;
	HW_ROTARY_A_PIN[rotm_LONGITUDE]			= GPIO_PIN_13; 
	HW_ROTARY_B_GPIO[rotm_LONGITUDE] 		= GPIOG;
	HW_ROTARY_B_PIN[rotm_LONGITUDE]			= GPIO_PIN_14;


	// BUTTONS

	HW_BUTTON_SW_GPIO[butm_A_BUTTON]	   	= GPIOC; 
	HW_BUTTON_SW_PIN[ butm_A_BUTTON]	   	= GPIO_PIN_9; 
	HW_BUTTON_SW_GPIO[butm_B_BUTTON]	   	= GPIOD; 
	HW_BUTTON_SW_PIN[ butm_B_BUTTON]	   	= GPIO_PIN_0; 
	HW_BUTTON_SW_GPIO[butm_C_BUTTON]	   	= GPIOD; 
	HW_BUTTON_SW_PIN[ butm_C_BUTTON]	   	= GPIO_PIN_1; 
	HW_BUTTON_SW_GPIO[butm_D_BUTTON]	   	= GPIOD; 
	HW_BUTTON_SW_PIN[ butm_D_BUTTON]	   	= BUTTON_D_PIN;  
	HW_BUTTON_SW_GPIO[butm_E_BUTTON]	   	= GPIOE; 
	HW_BUTTON_SW_PIN[ butm_E_BUTTON]	   	= GPIO_PIN_0; 
	HW_BUTTON_SW_GPIO[butm_F_BUTTON]	   	= GPIOE; 
	HW_BUTTON_SW_PIN[ butm_F_BUTTON]	   	= GPIO_PIN_1; 
	HW_BUTTON_SW_GPIO[butm_LFOMODE_BUTTON]  = GPIOG; 
	HW_BUTTON_SW_PIN[ butm_LFOMODE_BUTTON]  = GPIO_PIN_7; 
	HW_BUTTON_SW_GPIO[butm_LFOVCA_BUTTON] 	= GPIOG; 
	HW_BUTTON_SW_PIN[ butm_LFOVCA_BUTTON] 	= GPIO_PIN_8;


	// SWITCHES 

	HW_SW_GPIO[FINE_BUTTON] 				= GPIOD; 
	HW_SW_PIN[ FINE_BUTTON]					= GPIO_PIN_6;
	HW_SW_GPIO[VOCTSW] 						= GPIOE; 
	HW_SW_PIN[ VOCTSW]						= GPIO_PIN_15;

	HW_SW_GPIO[CLK_SENSE] 					= GPIOA; 
	HW_SW_PIN[ CLK_SENSE]					= GPIO_PIN_10;
	HW_SW_GPIO[WAVEFORMIN_SENSE] 			= GPIOC; 
	HW_SW_PIN[ WAVEFORMIN_SENSE]			= GPIO_PIN_7;

	

	// MONO LEDS
	
	HW_MONO_LED_GPIO[mledm_SLIDER_A] 		= GPIOD;
	HW_MONO_LED_PIN[ mledm_SLIDER_A] 		= GPIO_PIN_14;
	HW_MONO_LED_GPIO[mledm_SLIDER_B] 		= GPIOD;
	HW_MONO_LED_PIN[ mledm_SLIDER_B] 		= GPIO_PIN_12;
	HW_MONO_LED_GPIO[mledm_SLIDER_C] 		= GPIOD;
	HW_MONO_LED_PIN[ mledm_SLIDER_C] 		= GPIO_PIN_8;
	HW_MONO_LED_GPIO[mledm_SLIDER_D] 		= GPIOE;
	HW_MONO_LED_PIN[ mledm_SLIDER_D] 		= GPIO_PIN_10;
	HW_MONO_LED_GPIO[mledm_SLIDER_E] 		= GPIOB;
	HW_MONO_LED_PIN[ mledm_SLIDER_E] 		= GPIO_PIN_2;
	HW_MONO_LED_GPIO[mledm_SLIDER_F] 		= GPIOF;
	HW_MONO_LED_PIN[ mledm_SLIDER_F] 		= GPIO_PIN_2;

	// INITIALISATIONS

	init_rotaries();
	init_buttons();
	init_switches();
	init_mono_leds();
}



void init_gpio_pins(void){

	int i;
	GPIO_InitTypeDef gpio;

	ALL_GPIO_RCC_ENABLE;

	// -------------------------------
	//    GPIO INPUTS INITIALIZATION 
	// -------------------------------

	gpio.Mode 	= GPIO_MODE_INPUT;
	gpio.Speed 	= GPIO_SPEED_FREQ_LOW;

	// ROTARIES
	for (i = 0; i < NUM_ROTARIES; i++)
	{
		// rotary switch
		init_switch_gpio(&rotary[i].hwswitch);

		// turn pins
		gpio.Pull 	= GPIO_PULLUP;
		gpio.Pin 	= rotary[i].turn.A_pin;		HAL_GPIO_Init(rotary[i].turn.A_gpio, &gpio);
		gpio.Pin  	= rotary[i].turn.B_pin;		HAL_GPIO_Init(rotary[i].turn.B_gpio, &gpio);
	}

	// BUTTONS
	for (i = 0; i < NUM_BUTTONS; i++)	init_switch_gpio(&button[i].hwswitch);

	// SWITCHES
	for (i = 0; i < NUM_SWITCHES; i++)	init_switch_gpio(&hwSwitch[i]);
	

	//Todo: implement digital pin input module
	// CLOCK INPUT JACK
	gpio.Pull 	= GPIO_NOPULL;
	gpio.Pin 	= CLK_IN_pin;	HAL_GPIO_Init(CLK_IN_GPIO, &gpio);

	// BUS CLOCK INPUT
	gpio.Pull 	= GPIO_PULLDOWN;
	gpio.Pin 	= BUS_CLK_IN_pin;	HAL_GPIO_Init(BUS_CLK_IN_GPIO, &gpio);

	#if (PCB_VERSION>=25)
	// BUS SEL INPUT
	gpio.Pull 	= GPIO_PULLDOWN;
	gpio.Pin 	= BUS_SEL_IN_pin;	HAL_GPIO_Init(BUS_SEL_IN_GPIO, &gpio);
	#endif

	// ----------------------------------
	//     GPIO OUTPUTS INITIALIZATON 
	// ----------------------------------

	gpio.Mode 	= GPIO_MODE_OUTPUT_PP;
	gpio.Speed 	= GPIO_SPEED_FREQ_HIGH;
	gpio.Pull 	= GPIO_NOPULL;

	// 
	// MONO LEDs
	for (i = 0; i < NUM_MONO_LED; i++){
		gpio.Pin = monoLed[i].pin;	HAL_GPIO_Init(monoLed[i].gpio, &gpio);
	}

	// LED RING
    gpio.Pin = LED_RING_OE_pin;	HAL_GPIO_Init(LED_RING_OE_GPIO, &gpio);
    
    //DEBUG pins
    gpio.Pin = DEBUG0_pin;   	HAL_GPIO_Init(DEBUG0_GPIO, &gpio);
    gpio.Pin = DEBUG1_pin;   	HAL_GPIO_Init(DEBUG1_GPIO, &gpio);

#ifdef DEBUG_2_3_ENABLED
    gpio.Pin = DEBUG2_pin;   	HAL_GPIO_Init(DEBUG2_GPIO, &gpio);
    gpio.Pin = DEBUG3_pin;   	HAL_GPIO_Init(DEBUG3_GPIO, &gpio);
#endif 
}
