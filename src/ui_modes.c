/*
 * ui_modes.c - handles ui mode selection
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

#include "ui_modes.h"
#include "wavetable_recording.h"
#include "key_combos.h"
#include "params_update.h"
#include "led_cont.h"
#include "wavetable_editing.h" 
#include "wavetable_saveload.h" 
#include "UI_conditioning.h"
#include "calibrate_voct.h"
#include "flash_params.h"

enum 	UI_Modes ui_mode;

void check_ui_mode_requests(void){
	
	static enum 	UI_Modes arm_ui;
	static uint8_t	flag=0;

	if (ui_mode == PLAY){
		if 		(key_combo_enter_editing())			{ arm_ui = WTEDITING;}
		else if (key_combo_enter_voct_calibrate())	{stop_all_displays(); arm_ui = VOCT_CALIBRATE;} 
		else if (key_combo_reset_to_factory())		{stop_all_displays(); arm_ui = FACTORY_RESET;}
	}

	else if (ui_mode == WTREC_WAIT){
		if 		(key_combo_enter_recording())		{arm_ui = WTEDITING;}
	}

	else if (ui_mode == WTEDITING){
		if 		(key_combo_enter_recording())		{arm_ui = WTREC_WAIT; flag=RECORD_ALL;}
		else if (key_combo_enter_record_one())		{arm_ui = WTREC_WAIT; flag=RECORD_CURRENT;}
		else if (key_combo_enter_ttone())			{arm_ui = WTTTONE;}
		else if (key_combo_enter_monitoring())		{arm_ui = WTMONITORING;}
		else if (key_combo_exit_request())			{arm_ui = WTREC_EXIT;}
	}
	
	else if (ui_mode == WTMONITORING){
		if 		(key_combo_enter_recording())		{arm_ui = WTREC_WAIT; flag=RECORD_ALL;}
		else if (key_combo_enter_record_one())		{arm_ui = WTREC_WAIT; flag=RECORD_CURRENT;}
		else if (key_combo_enter_ttone())			{arm_ui = WTTTONE;}
		else if (key_combo_exit_monitoring())		{arm_ui = WTEDITING;}
		else if (key_combo_load_request())			{arm_ui = WTLOAD_SELECTING;} //FixMe: is this used?
		else if (key_combo_exit_request())			{arm_ui = WTREC_EXIT;}
	}

	else if (ui_mode == WTTTONE){
		if 		(key_combo_enter_recording())		{arm_ui = WTREC_WAIT; flag=RECORD_ALL;}
		else if (key_combo_enter_record_one())		{arm_ui = WTREC_WAIT; flag=RECORD_CURRENT;}
		else if (key_combo_enter_monitoring())		{arm_ui = WTMONITORING;}
		else if (key_combo_load_request())			{arm_ui = WTLOAD_SELECTING;}  //FixMe: is this used?
		else if (key_combo_exit_request())			{arm_ui = WTREC_EXIT;}
	}
	
	// else if (ui_mode == WTRENDERING){
	// 	if 		(key_combo_enter_recording())		{arm_ui = WTREC_WAIT;}
	// }

	else if (ui_mode == VOCT_CALIBRATE){
		if (key_combo_exit_voct_calibrate())	{arm_ui = VOCT_CALIBRATE_EXIT;}
		if (key_combo_cancel_voct_calibrate())	{arm_ui = VOCT_CALIBRATE_CANCEL;}
	}

	else {arm_ui = UI_NONE;}

	if ((!key_combo_exit_request()) && (arm_ui == WTREC_EXIT)) {
		exit_wtediting();
		arm_ui = UI_NONE;
	} 
	
	else if ((!key_combo_enter_recording()) && (!key_combo_enter_record_one()) && arm_ui==WTREC_WAIT){
		enter_wtrecording(flag);
		arm_ui = UI_NONE;
	}

	else if ((!key_combo_enter_monitoring()) && (arm_ui == WTMONITORING)){
		if(ui_mode == WTTTONE){flag =1;}
		enter_wtmonitoring();
		if (flag){	force_all_wt_interp_update();flag = 0;}
		arm_ui = UI_NONE;
	}

	else if ((!key_combo_enter_ttone()) && (arm_ui == WTTTONE)){
		enter_wtttone();
		arm_ui = UI_NONE;
	}

	else if (key_combo_enter_editing_released() && (!key_combo_exit_monitoring()) && (!key_combo_enter_recording()) && (!key_combo_enter_record_one()) && (arm_ui == WTEDITING)){
		stage_enter_wtediting();
		arm_ui = UI_NONE;
	}

	else if (!key_combo_enter_voct_calibrate() && (arm_ui == VOCT_CALIBRATE)){
		enter_voct_calibrate_mode();
		arm_ui = UI_NONE;
	} 
	else if (arm_ui == VOCT_CALIBRATE_EXIT){
		save_exit_voct_calibrate_mode();
		arm_ui = UI_NONE;
	} 
	else if (!key_combo_cancel_voct_calibrate() && (arm_ui == VOCT_CALIBRATE_CANCEL)){
		cancel_voct_calibrate_mode();
		arm_ui = UI_NONE;
	} 
	else if (!key_combo_enter_voct_calibrate() && (arm_ui == FACTORY_RESET)){
		factory_reset();
		arm_ui = UI_NONE;
	} 
}

