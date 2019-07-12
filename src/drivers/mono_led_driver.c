/* 
 * mono_led_driver.c - drivers for mono led(s) e.g. clipping, slider leds...
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

#include "drivers/mono_led_driver.h"
#include "timekeeper.h"

#if !IS_BOOTLOADER
	#include "led_cont.h"
#endif


//Private
void monoled_update(void);


void set_led_state(uint8_t monoled_number, uint8_t state) {
	if (state == LED_ON){mono_led_on( monoled_number);}
	else 				{mono_led_off(monoled_number);}
}
 
#if !IS_BOOTLOADER
	void start_monoled_updates(void)
	{
		start_timer_IRQ(MONO_LED_TIM_number, &monoled_update);
	}

	void monoled_update(void)
	{
		update_mono_leds();
	}
#endif
