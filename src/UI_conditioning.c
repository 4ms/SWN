/*
 * UI_conditioning.c - Conditions encoder, switch, buttons
 *
 * Author: Dan Green (danngreen1@gmail.com),  Hugo Paris (hugoplho@gmail.com)
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

#include "UI_conditioning.h"
#include "drivers/rotary_driver.h"
#include "drivers/switch_driver.h"
#include "timekeeper.h"

//debugging:
#include "gpio_pins.h"

//
uint16_t 				encoder 	[NUM_DIGITAL_CONTROLS];
int16_t 				encoder_q 	[NUM_DIGITAL_CONTROLS];
extern o_macro_states 	macro_states;
extern o_rotary 		rotary 		[NUM_ROTARIES];
extern o_button 		button 		[NUM_BUTTONS];
extern o_switch 		hwSwitch 	[NUM_SWITCHES];

//Private:
void 	update_encoder_q(void);
void 	update_button_presses(uint32_t elapsed_time);
void 	update_rotary_presses(uint32_t elapsed_time);
void 	update_flip_switches(void);
void 	UI_conditioning_updates(void);


void init_encoders(void){

	encoder[pec_OCT]			= 0;
	encoder[pec_TRANSPOSE]		= 0;
	encoder[pec_LFOSPEED]		= 0;
	encoder[pec_LFOSHAPE]		= 0;
	encoder[pec_LOADPRESET]		= 0;
	encoder[pec_DEPTH]			= 0;
	encoder[pec_WBROWSE]		= 0;
	encoder[pec_LATITUDE]		= 0;
	encoder[pec_LONGITUDE]		= 0;

	encoder[sec_SCALE]			= 0;
	encoder[sec_OSC_SPREAD]		= 0;
	encoder[sec_LFOGAIN]		= 0;
	encoder[sec_LFOPHASE]		= 0;
	encoder[sec_SAVEPRESET]		= 0;
	encoder[sec_DISPERSION]		= 0;
	encoder[sec_WTSEL]			= 0;
	encoder[sec_DISPPATT]		= 0;
	encoder[sec_WTSEL_SPREAD]	= 0;
}

//Time is in milliseconds, because SysTick is set to milliseconds
#define	BUT_SHORT_PRESS 		500		/* 0.5sec 	*/
#define	BUT_MED_PRESS 			2000  	/* 2 sec 	*/
#define	BUT_LONG_PRESS 			6000 	/* 4 sec 	*/
#define	BUT_ALREADY_DETECTED 	0xFFFFFFFF


// updates encoder_q[] and encoder[] arrays
// rotary turn queue gets poped into encoder queue (takes care of mapping readouts 
// ... to primary and secondary functions)
// encoder_q[] is a int16_t queue that gets poped into params everytime it's read
// encoder[] separately keeps track of overall encoder "position" for debugging
void update_encoder_q(void){

	uint8_t i;
	int8_t 	tmp_q				= 0;
	int		tmp_readout			= 0;
	// uint8_t press_tracker 		= 0;

	for (i=0; i<NUM_ROTARIES; i++){

		tmp_q = read_rotary_turn(&rotary[i]); 																// read rotary turn. Update and pop current rotary queue

		if (tmp_q){																							// if rotation registered
			if (rotary[i].hwswitch.pressed){																// ... if rotary was still pressed 
				
				// encoder position <--- FixMe: Can be removed eventually
				tmp_readout	= encoder[NUM_PRIMARY_DIGITAL_CONT + i] + tmp_q;												// ... ... compute temporary read
				if(tmp_readout<0)			{tmp_readout=0;}												// ... ... boundary check
				if(tmp_readout>ENCODER_MAX)	{tmp_readout=ENCODER_MAX;}										// ... ... boundary check
				encoder[NUM_PRIMARY_DIGITAL_CONT + i] = tmp_readout;														// ... ... update secondary control
				
				// encoder queue
				encoder_q[NUM_PRIMARY_DIGITAL_CONT + i] += tmp_q; 
			}
			else{

				// encoder position <--- FixMe: Can be removed eventually
 	 			tmp_readout	= encoder[i] + tmp_q;											// ... ... compute temporary read
				if(tmp_readout<0)			{tmp_readout=0;}												// ... ... boundary check
				if(tmp_readout>ENCODER_MAX)	{tmp_readout=ENCODER_MAX;}										// ... ... boundary check
				encoder[i] = tmp_readout;													// ... ... update primary control

				// encoder queue
				encoder_q[i] += tmp_q; 				
			}
		}
	}
}


int16_t pop_encoder_q(uint8_t digital_control){

	int16_t tmp;
	tmp = encoder_q[digital_control];
	encoder_q[digital_control] = 0;
	return tmp;
}

enum PressTypes rotary_pressed(uint8_t rotary_num) 	{ return(rotary[rotary_num].hwswitch.pressed); }
uint8_t rotary_reg_pressed(uint8_t rotary_num) 		{ return(rotary[rotary_num].hwswitch.pressed == PRESSED); }
uint8_t rotary_short_pressed(uint8_t rotary_num)	{ return(rotary[rotary_num].hwswitch.pressed >= SHORT_PRESSED);}
uint8_t rotary_med_pressed(uint8_t rotary_num)		{ return(rotary[rotary_num].hwswitch.pressed >= MED_PRESSED);}
uint8_t rotary_long_pressed(uint8_t rotary_num)		{ return(rotary[rotary_num].hwswitch.pressed >= LONG_PRESSED);}
uint8_t rotary_released(uint8_t rotary_num)			{ return(rotary[rotary_num].hwswitch.pressed == RELEASED);}

enum PressTypes button_pressed(uint8_t button_num) 	{ return(button[button_num].hwswitch.pressed); }
uint8_t button_short_pressed(uint8_t button_num)	{ return(button[button_num].hwswitch.pressed >= SHORT_PRESSED);}
uint8_t button_med_pressed(uint8_t button_num)		{ return(button[button_num].hwswitch.pressed >= MED_PRESSED);}
uint8_t button_long_pressed(uint8_t button_num)		{ return(button[button_num].hwswitch.pressed >= LONG_PRESSED);}
uint8_t button_released(uint8_t button_num)			{ return(button[button_num].hwswitch.pressed == RELEASED);}

uint8_t switch_pressed(uint8_t switch_num) 			{ return(hwSwitch[switch_num].pressed == PRESSED); }
uint8_t switch_released(uint8_t switch_num)			{ return(hwSwitch[switch_num].pressed == RELEASED); }
uint8_t jack_plugged(uint8_t switch_num) 			{ return(hwSwitch[switch_num].pressed == PRESSED); }
uint8_t jack_unplugged(uint8_t switch_num)			{ return(hwSwitch[switch_num].pressed == RELEASED); }


void update_button_presses(uint32_t elapsed_time){

	uint8_t 			i;
	uint16_t 			t;
	static uint16_t 	button_state_c_buf[NUM_BUTTONS]  = {0xffff}; 										// Current debounce status
	static uint32_t 	detection_tmr[NUM_BUTTONS] = {0};											

	// BUTTON PRESSES
	for (i = 0; i < NUM_BUTTONS; i++){																		// for each button

		if (read_switch_state( &(button[i].hwswitch) )) 		{t = 0xe000;} 								// ... button down entry in circular buffer
		else 													{t = 0xe001;}								// ... button up entry in circular buffer
		button_state_c_buf[i]	  = (button_state_c_buf[i] << 1) | t;										// ... update 16 bit button state circular buffer used for timing presses

		// UP / DOWN
		button[i].hwswitch.pressed  = RELEASED;

		// LONG PRESSES
		if (button_state_c_buf[i] == 0xe000 ){																// ... if button detected down 12 times is down

			button[i].hwswitch.pressed = PRESSED;
			if(detection_tmr[i] != BUT_ALREADY_DETECTED){													// ... if long(er) press not detected
				if (detection_tmr[i] < (0xFFFFFFFF - elapsed_time))	{detection_tmr[i] += elapsed_time;}		// ... ... check headroom in detection_tmr[]
				else 												{detection_tmr[i] = 0xFFFFFFFE;}		// ... ... prevent wrapping around uint32_t max value

				// Long Press
				if (detection_tmr[i] > BUT_LONG_PRESS) {	
					button[i].hwswitch.pressed  		 = LONG_PRESSED;
				}
				// med Press
				else if (detection_tmr[i] > BUT_MED_PRESS) {	
					button[i].hwswitch.pressed  		 = MED_PRESSED;
				}

				// Short Press
				else if (detection_tmr[i] > BUT_SHORT_PRESS) {	
					button[i].hwswitch.pressed  		 = SHORT_PRESSED;
				}
			}
		}
		else {
			detection_tmr[i] = 0;  // reset timer
		}
	}
	macro_states.all_af_buttons_released = (button[butm_A_BUTTON].hwswitch.pressed == RELEASED && button[butm_B_BUTTON].hwswitch.pressed == RELEASED && button[butm_C_BUTTON].hwswitch.pressed == RELEASED && button[butm_D_BUTTON].hwswitch.pressed == RELEASED && button[butm_E_BUTTON].hwswitch.pressed == RELEASED && button[butm_F_BUTTON].hwswitch.pressed == RELEASED);
}


void update_rotary_presses(uint32_t elapsed_time){

	uint8_t 			i;
	uint16_t 			t;
	static uint16_t 	rotary_state_c_buf[NUM_ROTARIES] = {0xffff};
	static uint32_t 	detection_tmr[NUM_BUTTONS] = {0};

	for (i = 0; i < NUM_ROTARIES; i++){

		if (read_rotary_press( &rotary[i]))
			t = 0xe000; 
		else
			t = 0xe001;

		rotary_state_c_buf[i] = (rotary_state_c_buf[i] << 1) | t;

		// UP / DOWN
		rotary[i].hwswitch.pressed = RELEASED;
		
		// LONG PRESSES
		if (rotary_state_c_buf[i] == 0xe000)
		{
			rotary[i].hwswitch.pressed = PRESSED;
			if (detection_tmr[i] != BUT_ALREADY_DETECTED)
			{
				if (detection_tmr[i] < (0xFFFFFFFF - elapsed_time))
					detection_tmr[i] += elapsed_time;
				else
					detection_tmr[i] = 0xFFFFFFFE;

				// Long Press
				if (detection_tmr[i] > BUT_LONG_PRESS) {			
					rotary[i].hwswitch.pressed = LONG_PRESSED;
				}

				// med Press
				else if (detection_tmr[i] > BUT_MED_PRESS) {			
					rotary[i].hwswitch.pressed = MED_PRESSED;
				}

				// Short Press
				else if (detection_tmr[i] > BUT_SHORT_PRESS) {				
					rotary[i].hwswitch.pressed = SHORT_PRESSED; 
				}
			}
		}
		else {	
			detection_tmr[i] = 0;
		}
	}
}

void update_flip_switches(void)
{
	uint8_t i;

	for (i = 0; i < NUM_SWITCHES; i++)
	{
		hwSwitch[i].pressed = read_switch_state(&hwSwitch[i]);
	}
}

void start_UI_conditioning_updates(void)
{
	start_timer_IRQ(UI_CONDITIONING_UPDATE_TIM_number, &UI_conditioning_updates);
}


void UI_conditioning_updates(void)
{
	static uint32_t last_sys_tmr;
	uint32_t sys_tmr;
	uint32_t elapsed_time;

	sys_tmr = (HAL_GetTick()/TICKS_PER_MS);

	// Compute elapsed time
	elapsed_time = (sys_tmr - last_sys_tmr);
	last_sys_tmr = sys_tmr;

	update_encoder_q();
	update_button_presses(elapsed_time);
	update_rotary_presses(elapsed_time);
	update_flip_switches();
}
