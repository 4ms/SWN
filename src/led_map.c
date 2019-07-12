/*
 * led_map.c - maps of RGB LEDs as organized by the hardware
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

#include "led_map.h"
#include "hardware_controls.h" //needed for NUM_ROTARIES and such
#include "globals.h"


const uint8_t led_outring_map[NUM_LED_OUTRING]={
											ledm_OUTRING1,
											ledm_OUTRING2,
											ledm_OUTRING3,
											ledm_OUTRING4,
											ledm_OUTRING5,
											ledm_OUTRING6,
											ledm_OUTRING7,
											ledm_OUTRING8,
											ledm_OUTRING9,
											ledm_OUTRING10,
											ledm_OUTRING11,
											ledm_OUTRING12,
											ledm_OUTRING13,
											ledm_OUTRING14,
											ledm_OUTRING15,
											ledm_OUTRING16,
											ledm_OUTRING17,
											ledm_OUTRING18
};

const uint8_t led_inring_map[NUM_LED_INRING]={
											ledm_INRING1,
											ledm_INRING2,
											ledm_INRING3,
											ledm_INRING4,
											ledm_INRING5,
											ledm_INRING6
};

const uint8_t af_button_map[NUM_CHANNELS] = {
											ledm_A_BUTTON, 
											ledm_B_BUTTON, 	
											ledm_C_BUTTON, 
											ledm_D_BUTTON, 
											ledm_E_BUTTON, 
											ledm_F_BUTTON
}; 

const uint8_t led_button_map[NUM_BUTTONS] = {
											ledm_A_BUTTON, 
											ledm_B_BUTTON, 	
											ledm_C_BUTTON, 
											ledm_D_BUTTON, 
											ledm_E_BUTTON, 
											ledm_F_BUTTON,
											ledm_LFOMODE_BUTTON, 
											ledm_LFOVCA_BUTTON
}; 

const uint8_t ledstring_map[NUM_CHANNELS + 1] = {
											ledm_STRING1, 
											ledm_STRING2, 	
											ledm_STRING3, 
											ledm_STRING4, 
											ledm_STRING5, 
											ledm_STRING6,
											ledm_LFOCV
}; 

const uint8_t led_rotary_map[NUM_LED_ROTARIES] = { //Todo: remove NONE1 entries that were unused RGB encoders 
											NONE1,
											NONE1,
											NONE1,
											NONE1,
											NONE1,
											ledm_DEPTH_ENC, 	
											ledm_LATITUDE_ENC,
											ledm_LONGITUDE_ENC
}; 

const uint8_t all_rgb_led_map[NUM_LED_IDs - 7] = {
											ledm_A_BUTTON, 
											ledm_B_BUTTON, 	
											ledm_C_BUTTON, 
											ledm_D_BUTTON, 
											ledm_E_BUTTON, 
											ledm_F_BUTTON,
											ledm_LFOVCA_BUTTON,
											ledm_LFOMODE_BUTTON, 
											ledm_STRING1, 
											ledm_STRING2, 	
											ledm_STRING3,
											ledm_LFOCV,
											ledm_STRING4, 
											ledm_STRING5, 
											ledm_STRING6,
											ledm_AUDIOIN,
											ledm_OUTRING1,
											ledm_OUTRING2,
											ledm_OUTRING3,
											ledm_OUTRING4,
											ledm_OUTRING5,
											ledm_OUTRING6,
											ledm_OUTRING7,
											ledm_OUTRING8,
											ledm_OUTRING9,
											ledm_OUTRING10,
											ledm_OUTRING11,
											ledm_OUTRING12,
											ledm_OUTRING13,
											ledm_OUTRING14,
											ledm_OUTRING15,
											ledm_OUTRING16,
											ledm_OUTRING17,
											ledm_OUTRING18,
											ledm_INRING1,
											ledm_INRING2,
											ledm_INRING3,
											ledm_INRING4,
											ledm_INRING5,
											ledm_INRING6,
											ledm_DEPTH_ENC, 	
											ledm_LATITUDE_ENC,
											ledm_LONGITUDE_ENC
};
