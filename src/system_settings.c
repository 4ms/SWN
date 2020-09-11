/*
 * system_settings.c - Global system settings and modes that are user-editable during runtime
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

#include <math.h>

#include "system_settings.h"
#include "globals.h"

o_systemSettings	system_settings;

void default_system_settings(void)
{
	system_settings.master_gain				= 1.0/48.0;
	system_settings.allow_bus_clock 		= 0;
	system_settings.transpose_display_mode	= TRANSPOSE_CONTINUOUS;
	system_settings.lfo_cv_mode 			= LFOCV_SPEED;
	system_settings.global_brightness 		= 0.8;
	system_settings.selbus_can_recall 		= SELBUS_RECALL_DISABLED;
	system_settings.selbus_can_save 		= SELBUS_SAVE_DISABLED;
}

uint8_t range_check_system_settings(o_systemSettings *sys_sets)
{
	uint8_t range_errors=0;

	if (isnan(sys_sets->master_gain) || (sys_sets->master_gain > 1.0) || (sys_sets->master_gain < (1.0/1000.0)))
	{
		sys_sets->master_gain = 1.0/48.0;
		range_errors++;
	}

	if (sys_sets->allow_bus_clock > 1)
	{
		sys_sets->allow_bus_clock = 0;
		range_errors++;
	}

	if (sys_sets->transpose_display_mode >= NUM_TRANSPOSE_DISPLAY_MODES)
	{
		sys_sets->transpose_display_mode = TRANSPOSE_CONTINUOUS;
		range_errors++;
	}

	if (sys_sets->lfo_cv_mode >= NUM_LFOCV_MODES)
	{
		sys_sets->lfo_cv_mode = LFOCV_SHAPE;
		range_errors++;
	}

	if (isnan(sys_sets->global_brightness) || (sys_sets->global_brightness > 2.0) || (sys_sets->global_brightness < (1.0/1000.0)))
	{
		sys_sets->global_brightness = 0.8;
		range_errors++;
	}

	if (sys_sets->selbus_can_recall != SELBUS_RECALL_ENABLED && sys_sets->selbus_can_recall != SELBUS_RECALL_DISABLED)
	{
		sys_sets->selbus_can_recall = SELBUS_RECALL_DISABLED;
		range_errors++;
	}

	if (sys_sets->selbus_can_save != SELBUS_SAVE_ENABLED && sys_sets->selbus_can_save != SELBUS_SAVE_DISABLED)
	{
		sys_sets->selbus_can_save = SELBUS_SAVE_DISABLED;
		range_errors++;
	}

	return range_errors;
}
