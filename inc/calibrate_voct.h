/*
 * calibrate_voct.h - calibration of V/oct jacks
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
#include "flash_params.h"

#define VOCTCAL_LPF_SIZE 256

enum VoctCalStates{
	VOCTCAL_NOT_CALIBRATED,
	VOCTCAL_READING_C1,
	VOCTCAL_READING_C3,
	VOCTCAL_CALCULATING,
	VOCTCAL_CALIBRATED,
	VOCTCAL_RANGE_ERROR,

	NUM_VOCTCAL_STATES
};

#define NUM_VOCT_CHANNELS (NUM_CHANNELS+1)

void 	enter_voct_calibrate_mode(void);
void 	save_exit_voct_calibrate_mode(void);
void 	cancel_voct_calibrate_mode(void);

void 	process_voct_calibrate_mode(void);

void 	set_default_voct_calibrate(void);
uint8_t range_check_calibrate_voct_values(SystemCalibrations *sys_cal);

enum VoctCalStates	get_voctcal_state(uint8_t chan);
float apply_voct_calibration(float adc_val, uint8_t chan);
