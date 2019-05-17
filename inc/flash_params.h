/*
 * flash_params.h
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
#include "led_cont.h"				// For NUM_LED_IDs
#include "analog_conditioning.h" 	// For NUM_ANALOG_ELEMENTS

#define FLASH_ADDR_userparams 				0x08008000
#define FLASH_ADDR_systemsettings_offset 	0x00000400

#define FLASH_SYMBOL_firmwareoffset 0xAA550000

typedef struct SystemCalibrations
{
	uint32_t	major_firmware_version;
	uint32_t	minor_firmware_version;

	float		rgbled_adjustments		[NUM_LED_IDs][3];
	int16_t		cv_jack_unplugged_offset[NUM_ANALOG_ELEMENTS];
	int16_t		cv_jack_plugged_offset	[NUM_ANALOG_ELEMENTS];

	float 		UNUSED;

	float		voct_tracking_adjustment	[NUM_CHANNELS+1];
	float		voct_offset_adjustment		[NUM_CHANNELS+1];

} SystemCalibrations;

uint32_t load_flash_params(void);
void save_flash_params(void);

void read_all_system_values_from_FLASH(void);
void write_all_system_values_to_FLASH(void);
void copy_system_values_into_staging(void);

void factory_reset(void);
void factory_reset_all_calibrations(void);
void firmware_upgrade_post_process(uint8_t old_major, uint8_t old_minor);

void set_firmware_version(void);
uint8_t is_valid_firmware_version(uint32_t major, uint32_t minor);
