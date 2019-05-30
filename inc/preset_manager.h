/*
 * preset_manager.h
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
#include "external_flash_layout.h"
#include "params_update.h"
#include "params_lfo.h"

enum PresetMgrStates{
	PM_INACTIVE,
	PM_SELECTING,

	PM_LOAD_CONFIRM,
	PM_PRESSED_TO_DO_LOAD,
	PM_DOING_LOAD,

	PM_SAVE_HELD,
	PM_SAVE_CONFIRM,
	PM_PRESSED_TO_DO_SAVE,
	PM_DOING_SAVE,
	PM_DOING_SAVE_OVER_EMPTY,

	PM_CLEAR_HELD,
	PM_CLEAR_CONFIRM,
	PM_PRESSED_TO_DO_CLEAR,
	PM_DOING_CLEAR,

	PM_UNDO_HELD,
	PM_DOING_UNDO,

	NUM_PRESET_MODES
};

typedef struct o_preset_manager
{
	uint16_t				hover_num;
	enum PresetMgrStates 	mode;
	enum PresetMgrStates	last_action;
	uint8_t					last_action_slot;

	uint8_t					filled[MAX_PRESETS]; //todo: use a bitfield
	uint32_t				animation_ctr;
	uint32_t				activity_tmr;

} o_preset_manager;

void init_preset_manager(void);
void exit_preset_manager(void);

void recall_preset_into_active(uint32_t preset_num);
void store_preset_from_active(uint32_t preset_num);
void clear_preset(uint32_t preset_num);
void clear_all_presets(void);

void store_preset(uint32_t preset_num, o_params *t_params, o_lfos *t_lfos);
void recall_preset(uint32_t preset_num, o_params *t_params, o_lfos *t_lfos);
void update_preset_version(char version, o_params *t_params, o_lfos *t_lfos);

void recalc_active_params(void);

uint8_t check_preset_filled(uint32_t preset_num, char *version);
uint32_t get_preset_addr(uint32_t preset_num);
uint32_t get_preset_size(void);
