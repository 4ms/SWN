/*
 * preset_manager_undo.c - Logic for undoing a preset save/load/clear action
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

#include "preset_manager.h"
#include "preset_manager_UI.h"
#include "preset_manager_undo.h"
#include "params_update.h"
#include "params_lfo.h"

extern	o_params 		params;				// 868 Bytes
extern	o_lfos   		lfos;				// 526 Bytes

extern	o_preset_manager		preset_mgr;

o_params 		preset_undo_params;		
o_lfos   		preset_undo_lfos;		
uint8_t 		preset_undo_buffer_valid = 0;

void undo_preset_action(void){
	preset_start_undo_animation();

	switch (preset_mgr.last_action)
	{
		case PM_DOING_LOAD:
		case PM_DOING_UNDO:
			swap_active_with_undo_buffer();
			recalc_active_params();
			preset_mgr.last_action = PM_DOING_LOAD;
			break;

		case PM_DOING_SAVE:
			swap_undo_buffer_with_slot(preset_mgr.last_action_slot);
			preset_mgr.last_action = PM_DOING_SAVE;
			break;

		case PM_DOING_CLEAR:
			swap_undo_buffer_with_slot(preset_mgr.last_action_slot);
			preset_mgr.last_action = PM_DOING_SAVE_OVER_EMPTY;
			break;

		case PM_DOING_SAVE_OVER_EMPTY:
			recall_slot_into_undo_buffer(preset_mgr.last_action_slot);
			clear_preset(preset_mgr.last_action_slot);
			preset_mgr.last_action = PM_DOING_CLEAR;
			break;

		default:
			preset_mgr.last_action = PM_DOING_UNDO;
			break;
	}

}

void recall_slot_into_undo_buffer(uint8_t preset_num)
{
	recall_preset(preset_num, &preset_undo_params, &preset_undo_lfos);
}

void swap_undo_buffer_with_slot(uint8_t preset_num)
{
	uint32_t i;
	uint8_t *src;
	uint8_t *dst;

	static o_params 	t_params;		
	static o_lfos   	t_lfos;	

	recall_preset(preset_num, &t_params, &t_lfos);
	store_preset(preset_num, &preset_undo_params, &preset_undo_lfos);

	//Copy tmp buffer to undo buffer
	dst = (uint8_t *)&preset_undo_params;
	src = (uint8_t *)&t_params;
	for (i=0;i<sizeof(o_params);i++)
		*dst++ = *src++;

	dst = (uint8_t *)&preset_undo_lfos;
	src = (uint8_t *)&t_lfos;
	for (i=0;i<sizeof(o_lfos);i++)
		*dst++ = *src++;
}


//Swap current params with undo buffer
void swap_active_with_undo_buffer(void){
	uint32_t i;
	uint8_t *src;
	uint8_t *dst;

	o_params 	t_params;		
	o_lfos   	t_lfos;		

	if (!preset_undo_buffer_valid)
	{
		stash_active_into_undo_buffer();
	} else {
		//Copy undo buffer to tmp buffer
		dst = (uint8_t *)&t_params;
		src = (uint8_t *)&preset_undo_params;
		for (i=0;i<sizeof(o_params);i++)
			*dst++ = *src++;

		dst = (uint8_t *)&t_lfos;
		src = (uint8_t *)&preset_undo_lfos;
		for (i=0;i<sizeof(o_lfos);i++)
			*dst++ = *src++;

		//Copy active params to undo buffer
		dst = (uint8_t *)&preset_undo_params;
		src = (uint8_t *)&params;
		for (i=0;i<sizeof(o_params);i++)
			*dst++ = *src++;

		dst = (uint8_t *)&preset_undo_lfos;
		src = (uint8_t *)&lfos;
		for (i=0;i<sizeof(o_lfos);i++)
			*dst++ = *src++;

		//Copy tmp buffer to active params
		dst = (uint8_t *)&params;
		src = (uint8_t *)&t_params;
		for (i=0;i<sizeof(o_params);i++)
			*dst++ = *src++;

		dst = (uint8_t *)&lfos;
		src = (uint8_t *)&t_lfos;
		for (i=0;i<sizeof(o_lfos);i++)
			*dst++ = *src++;
	} 
}


//Copy undo buffer to current params
void unstash_undo_buffer_into_active(void)
{
	uint32_t i;
	uint8_t *src;
	uint8_t *dst;

	if (preset_undo_buffer_valid){
		dst = (uint8_t *)&params;
		src = (uint8_t *)&preset_undo_params;
		for (i=0;i<sizeof(o_params);i++)
			*dst++ = *src++;

		dst = (uint8_t *)&lfos;
		src = (uint8_t *)&preset_undo_lfos;
		for (i=0;i<sizeof(o_lfos);i++)
			*dst++ = *src++;
	}
}

//Copy current params to undo buffer
void stash_active_into_undo_buffer(void)
{
	uint32_t i;
	uint8_t *src;
	uint8_t *dst;

	preset_undo_buffer_valid = 0;

	dst = (uint8_t *)&preset_undo_params;
	src = (uint8_t *)&params;
	for (i=0;i<sizeof(o_params);i++)
		*dst++ = *src++;

	dst = (uint8_t *)&preset_undo_lfos;
	src = (uint8_t *)&lfos;
	for (i=0;i<sizeof(o_lfos);i++)
		*dst++ = *src++;

	preset_undo_buffer_valid = 1;
}
