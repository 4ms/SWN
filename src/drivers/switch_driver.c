/* 
 * switch_driver.c - drivers for button switches (hardware switches, button presses, encoder presses, ...)
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

#include "drivers/switch_driver.h"
#include "hardware_controls.h"

//Note: this does not set the switch.state element, it just reads the GPIO pins and returns 0 or 1
//This allows UI conditioning to debounce, calculate short/long presses, etc.
//UI conditioning should set the state, not the driver.
//
uint32_t read_switch_state(o_switch *switch_object)
{
	uint8_t state;

	state = ((switch_object->gpio->IDR) & (switch_object->pin)) ? 0 : 1;
	if(switch_object->ptype == PULLDOWN)		state = 1 - state;
	return state;
}

// FixMe: call this function from inside init_gpio_pins() when initializing switches, buttons
// We will have to set the ptype of switches and buttons
//
void init_switch_gpio(o_switch *sw)
{
	GPIO_InitTypeDef gpio;

	gpio.Mode 	= GPIO_MODE_INPUT;
	gpio.Speed 	= GPIO_SPEED_FREQ_MEDIUM;

	if (sw->ptype == PULLDOWN)	gpio.Pull 	= GPIO_PULLDOWN;
	else
	if (sw->ptype == PULLUP)	gpio.Pull 	= GPIO_PULLUP;
	else 						gpio.Pull 	= GPIO_NOPULL;

	gpio.Pin = sw->pin;
	HAL_GPIO_Init(sw->gpio, &gpio);
}
