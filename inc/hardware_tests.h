/*
 * hardware_tests.c - test procedure for procuction runs
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

 #pragma once

#include <stm32f7xx.h>
#include "switch_driver.h"

#define FORCE_HW_TEST 0

extern o_rotary 	rotary[NUM_ROTARIES];
extern o_button 	button[NUM_ROTARIES];


static inline uint8_t key_combo_enter_hardwaretest(void)
{
	return (
		read_switch_state(&button[butm_D_BUTTON].hwswitch) \
		&& read_switch_state(&rotary[rotm_TRANSPOSE].hwswitch) \
		&& read_switch_state(&rotary[rotm_LFOSHAPE].hwswitch) \
	);}

static inline uint8_t key_combo_reload_factory_spheres(void)
{
	return (
		read_switch_state(&button[butm_C_BUTTON].hwswitch) \
		&& read_switch_state(&rotary[rotm_TRANSPOSE].hwswitch) \
		&& read_switch_state(&rotary[rotm_LFOSHAPE].hwswitch) \
	);}
 //use GPIO pin reading, not UI-conditioned values in order to minimize sources of error other than hardware
uint8_t is_hardwaretest_already_done(void);
void do_hardware_test(void);

