/*
 * hardware_controls.c - hardware setup
 *
 * Author: Hugo Paris (hugoplho@gmail.com), Dan Green (danngreen1@gmail.com)
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
#include "hardware_controls.h"
#include "gpio_pins.h"
#include "led_colors.h"
#include "analog_conditioning.h"
#include "drivers/rotary_driver.h"

o_macro_states 	macro_states;
o_rotary 		rotary[NUM_ROTARIES];
o_button 		button[NUM_BUTTONS];
o_switch  		hwSwitch[NUM_SWITCHES];
o_monoLed   	monoLed[NUM_MONO_LED];

// GPIO map
extern GPIO_TypeDef 	*HW_ROTARY_SW_GPIO[NUM_ROTARIES];
extern uint16_t 	  	HW_ROTARY_SW_PIN[NUM_ROTARIES]; 
extern uint8_t 	  		HW_ROTARY_SW_PINTYPE[NUM_ROTARIES]; 
extern uint8_t 	  		HW_ROTARY_TURN_STEP[NUM_ROTARIES]; 
extern GPIO_TypeDef 	*HW_ROTARY_A_GPIO[NUM_ROTARIES];
extern uint16_t 	 	HW_ROTARY_A_PIN[NUM_ROTARIES]; 
extern GPIO_TypeDef 	*HW_ROTARY_B_GPIO[NUM_ROTARIES];
extern uint16_t	  		HW_ROTARY_B_PIN[NUM_ROTARIES]; 
extern GPIO_TypeDef 	*HW_BUTTON_SW_GPIO[NUM_BUTTONS];
extern uint16_t 		HW_BUTTON_SW_PIN[NUM_BUTTONS]; 
extern GPIO_TypeDef 	*HW_SW_GPIO[NUM_SWITCHES];
extern uint16_t 	  	HW_SW_PIN[NUM_SWITCHES]; 
extern GPIO_TypeDef 	*HW_MONO_LED_GPIO[NUM_MONO_LED];
extern uint16_t 	  	HW_MONO_LED_PIN[NUM_MONO_LED]; 

// init_rotaries()
// initializes gpio/pins of rotary encoders
void init_rotaries(void){
	uint16_t i;

	for (i=0; i < NUM_ROTARIES; i++){
	
		// initialize rotary turn
		init_rotary_turn(&rotary[i].turn);

		// rotary gpio(s) and pins
		rotary[i].hwswitch.gpio 	= HW_ROTARY_SW_GPIO[i];
		rotary[i].hwswitch.pin 		= HW_ROTARY_SW_PIN[i];
		rotary[i].hwswitch.ptype 	= HW_ROTARY_SW_PINTYPE[i];
		rotary[i].turn.step_size 	= HW_ROTARY_TURN_STEP[i];
		rotary[i].turn.A_gpio 		= HW_ROTARY_A_GPIO[i];
		rotary[i].turn.A_pin 		= HW_ROTARY_A_PIN[i];
		rotary[i].turn.B_gpio 		= HW_ROTARY_B_GPIO[i];
		rotary[i].turn.B_pin 		= HW_ROTARY_B_PIN[i];
	}	
}


// init_buttons()
// initializes gpio/pins of buttons
void init_buttons(void){

	uint16_t i;

	for (i=0; i < NUM_BUTTONS; i++){

		// button press
		button[i].hwswitch.gpio 	= HW_BUTTON_SW_GPIO[i];
		button[i].hwswitch.pin 		= HW_BUTTON_SW_PIN[i]; 
		button[i].hwswitch.ptype 	= PULLUP; //Todo: add HW_BUTTON_SW_PTYPE[i];
		// button[i].hwswitch.state 	= BUT_UP;
				
	}
}


// init_switches()
// initializes gpio/pins of flip switches and sense pins on jacks
void init_switches(void){

	uint16_t i;

	for (i = 0; i < NUM_SWITCHES; i++){
		hwSwitch[i].gpio 	= HW_SW_GPIO[i];
		hwSwitch[i].pin 	= HW_SW_PIN[i]; 
		hwSwitch[i].ptype 	= PULLUP; //Todo: add HW_SW_PTYPE[i]; 
	}
}


// init_mono_leds()
// initializes gpio/pins of all monochromatic LEDs
void init_mono_leds(void){

	uint16_t i;

	for (i = 0; i < NUM_MONO_LED; i++){

		// LED
		monoLed[i].gpio = HW_MONO_LED_GPIO[i];
		monoLed[i].pin  = HW_MONO_LED_PIN[i]; 
	}

}