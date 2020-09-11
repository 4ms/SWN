/*
 * system_settings.h
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

enum LFOCVModes {
	LFOCV_SPEED,
	LFOCV_SHAPE,
	LFOCV_GROOVE,			// FINE PHASE SPREAD
	LFOCV_SHAPE_SPREAD,
	LFOCV_RHYTHM,			// set of speeds, shapes and phase

	NUM_LFOCV_MODES
};

enum TransposeDisplayModes {
	TRANSPOSE_NOWRAP,
	TRANSPOSE_CONTINUOUS,

	NUM_TRANSPOSE_DISPLAY_MODES
};

enum SelBusRecallModes {
	SELBUS_RECALL_DISABLED,
	SELBUS_RECALL_ENABLED,
};

enum SelBusSaveModes {
	SELBUS_SAVE_DISABLED,
	SELBUS_SAVE_ENABLED,
};

typedef struct o_systemSettings
{
	enum LFOCVModes				lfo_cv_mode;
	float						master_gain;
	enum TransposeDisplayModes	transpose_display_mode;
	uint8_t						allow_bus_clock;
	float						global_brightness;
	enum SelBusRecallModes 		selbus_can_recall;
	enum SelBusSaveModes 		selbus_can_save;
} o_systemSettings;

void default_system_settings(void);
uint8_t range_check_system_settings(o_systemSettings *sys_sets);
