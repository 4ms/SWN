/*
 * flash_params.c - Saving system calibrations
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

#include "globals.h"
#include "flash_params.h"
#include "flash.h"
#include "led_color_adjust.h"
#include "led_colors.h"
#include "calibrate_voct.h"
#include "params_update.h"
#include "system_settings.h"

#include "preset_manager.h"
#include "calibrate_voct.h"
#include "sphere_flash_io.h"


#include <math.h>

SystemCalibrations s_system_calibrations;
SystemCalibrations *system_calibrations = &s_system_calibrations;

SystemCalibrations s_staging_system_calibrations;
SystemCalibrations *staging_system_calibrations = &s_staging_system_calibrations;

extern o_systemSettings	system_settings;
o_systemSettings	staging_system_settings;


//Reads calibration data from FLASH and stores it into the system_calibrations global variable
//It also checks for valid data and sets to defaults
//It also checks for valid firmware version and updates the firmware version if found
uint32_t load_flash_params(void)
{
	uint32_t i;
	uint8_t *src;
	uint8_t *dst;

	read_all_system_values_from_FLASH(); //into staging area

	if (is_valid_firmware_version(staging_system_calibrations->major_firmware_version, staging_system_calibrations->minor_firmware_version))
	{

		//system_calibrations = staging_system_calibrations;
		dst = (uint8_t *)system_calibrations;
		src = (uint8_t *)staging_system_calibrations;
		for (i=0;i<sizeof(SystemCalibrations);i++)
			*dst++ = *src++;
		
		//system_settings = staging_system_settings;
		dst = (uint8_t *)&system_settings;
		src = (uint8_t *)&staging_system_settings;
		for (i=0;i<sizeof(o_systemSettings);i++)
			*dst++ = *src++;

		if (system_calibrations->major_firmware_version != FW_MAJOR_VERSION || system_calibrations->minor_firmware_version != FW_MINOR_VERSION)
		{
			firmware_upgrade_post_process(system_calibrations->major_firmware_version, system_calibrations->minor_firmware_version);
			
    		set_firmware_version();
			save_flash_params();
		}
		return 1; //Valid firmware version found
	} else
	{
    	set_firmware_version();

		return 0; //No calibration data found
	}
}

void firmware_upgrade_post_process(uint8_t old_major, uint8_t old_minor)
{
	if (old_major==0 && old_minor<=8)
	{
		write_fatory_spheres_to_extflash();
	}
}

void save_flash_params(void)
{
	copy_system_values_into_staging();
	write_all_system_values_to_FLASH();
}

void factory_reset_all_calibrations(void)
{
	set_default_led_color_adjust();
	set_default_cv_jack_calibration_offsets();
	set_default_voct_calibrate();
}

void factory_reset(void)
{	
	default_system_settings();
	set_default_voct_calibrate();
	empty_all_user_spheres();
	clear_all_presets();

	set_firmware_version();
	save_flash_params();
}


void range_check_calibration_values(void)
{
	uint32_t i;

	//Todo, this should be moved to a check_valid_pwm_led_adjustments(), giving staging_system_cal as an argument
	for (i=0;i<NUM_LED_IDs;i++)
	{
		if (isnan(staging_system_calibrations->rgbled_adjustments[i][c_RED]) || staging_system_calibrations->rgbled_adjustments[i][c_RED] > 5.5 || staging_system_calibrations->rgbled_adjustments[i][c_RED] < 0.05)
			staging_system_calibrations->rgbled_adjustments[i][c_RED] = 1.0;

		if (isnan(staging_system_calibrations->rgbled_adjustments[i][c_GREEN]) || staging_system_calibrations->rgbled_adjustments[i][c_GREEN] > 5.5 || staging_system_calibrations->rgbled_adjustments[i][c_GREEN] < 0.05)
			staging_system_calibrations->rgbled_adjustments[i][c_GREEN] = 1.0;

		if (isnan(staging_system_calibrations->rgbled_adjustments[i][c_BLUE]) || staging_system_calibrations->rgbled_adjustments[i][c_BLUE] > 5.5 || staging_system_calibrations->rgbled_adjustments[i][c_BLUE] < 0.05)
			staging_system_calibrations->rgbled_adjustments[i][c_BLUE] = 1.0;

	}

	//This should be moved to range_check function for cv jack unplugged offset
	for(i=0;i<NUM_ANALOG_ELEMENTS;i++)
	{
		if (staging_system_calibrations->cv_jack_unplugged_offset[i] > 2000 || staging_system_calibrations->cv_jack_unplugged_offset[i]<-2000)		
			staging_system_calibrations->cv_jack_unplugged_offset[i] = 0;

		if (staging_system_calibrations->cv_jack_plugged_offset[i] > 2000 || staging_system_calibrations->cv_jack_plugged_offset[i]<-2000)		
			staging_system_calibrations->cv_jack_plugged_offset[i] = 0;

		//FixMe: remove this temporary hard-set to a fixed value
		if (i>=A_VOCT && i<=TRANSPOSE_CV) staging_system_calibrations->cv_jack_plugged_offset[i] = 0;

	}

	range_check_calibrate_voct_values(staging_system_calibrations);
	range_check_system_settings(&staging_system_settings);
}

void read_all_system_values_from_FLASH(void)
{
	uint32_t i;
	uint8_t *ptr;
	// uint8_t invalid_fw_version=0;

	//Read FLASH and store into *staging_system_calibrations
	ptr = (uint8_t *)staging_system_calibrations;
	for (i=0;i<sizeof(SystemCalibrations);i++)
	{
		*ptr++ = flash_read_byte(FLASH_ADDR_userparams + i);
	}

	//Read FLASH and store into *staging_system_settings
	ptr = (uint8_t *)&staging_system_settings;
	for (i=0;i<sizeof(o_systemSettings);i++)
	{
		*ptr++ = flash_read_byte(FLASH_ADDR_userparams+FLASH_ADDR_systemsettings_offset + i);
	}

	staging_system_calibrations->major_firmware_version -= FLASH_SYMBOL_firmwareoffset;

	//Check for valid data, setting to default value if out-of-range data is found.
	//If invalid firmware version found, set all data to default
	if (!is_valid_firmware_version(staging_system_calibrations->major_firmware_version, staging_system_calibrations->minor_firmware_version)){
		factory_reset_all_calibrations();
		factory_reset();
	}
	else{
		range_check_calibration_values();
	}
}

void write_all_system_values_to_FLASH(void)
{
	uint32_t i;
	uint8_t *addr;

	flash_begin_open_program();

	flash_open_erase_sector(FLASH_ADDR_userparams);

	staging_system_calibrations->major_firmware_version += FLASH_SYMBOL_firmwareoffset;

	addr = (uint8_t *)staging_system_calibrations;

	for(i=0;i<sizeof(SystemCalibrations);i++)
	{
		flash_open_program_byte(*addr++, FLASH_ADDR_userparams + i);
	}

	addr = (uint8_t *)&staging_system_settings;

	for(i=0;i<sizeof(o_systemSettings);i++)
	{
		flash_open_program_byte(*addr++, FLASH_ADDR_userparams+FLASH_ADDR_systemsettings_offset + i);
	}

	flash_end_open_program();
}

void set_firmware_version(void)
{
	staging_system_calibrations->major_firmware_version = FW_MAJOR_VERSION;
	system_calibrations->major_firmware_version 		= FW_MAJOR_VERSION;

	staging_system_calibrations->minor_firmware_version	= FW_MINOR_VERSION;
	system_calibrations->minor_firmware_version 		= FW_MINOR_VERSION;
}

uint8_t is_valid_firmware_version(uint32_t major, uint32_t minor)
{
	if (major > MAX_VALID_MAJOR_FW_VERSION || minor > MAX_VALID_MINOR_FW_VERSION)
		return 0;
	else
		return 1;
}

void copy_system_values_into_staging(void)
{
	uint32_t i;
	uint8_t *src;
	uint8_t *dst;

	//copy: staging_system_calibrations = system_calibrations;
	dst = (uint8_t *)staging_system_calibrations;
	src = (uint8_t *)system_calibrations;
	for (i=0;i<sizeof(SystemCalibrations);i++)
		*dst++ = *src++;

	//staging_system_settings = system_settings
	dst = (uint8_t *)&staging_system_settings;
	src = (uint8_t *)&system_settings;
	for (i=0;i<sizeof(o_systemSettings);i++)
		*dst++ = *src++;
}

