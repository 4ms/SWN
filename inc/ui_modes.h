/*
 * ui_modes.h - handles ui mode selection
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

#pragma once

#include <stm32f7xx.h>

enum UI_Modes {

	UI_NONE, // used in check_ui_mode_requests()

	PLAY,
	SELECT_PARAMS,

	WTRECORDING,				// <---- WT rec/edit section must start with WTRECORDING 
	WTREC_WAIT,
	WTMONITORING,
	WTTTONE,
	WTRENDERING,
	WTEDITING,
	WTLOAD_SELECTING,
	WTPLAYEXPORT,
	WTPLAYEXPORT_LOAD,
	WTSAVING,					// <---- WT rec/edit section must end with WTSAVING
	WTREC_EXIT,

	RGB_COLOR_ADJUST,

	VOCT_CALIBRATE,
	VOCT_CALIBRATE_EXIT,
	VOCT_CALIBRATE_CANCEL,

	FACTORY_RESET,

	NUM_UI_MODES
};


static inline uint8_t UIMODE_IS_WT_RECORDING_EDITING(enum UI_Modes X) { return (((X) >= WTRECORDING) && ((X) <= WTSAVING)); }

void check_ui_mode_requests(void);



