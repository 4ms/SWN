/*
 * wavetable_saveload.c
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


#include <stm32f7xx.h>
#include "wavetable_saveload.h"
#include "wavetable_saveload_UI.h"
#include "wavetable_editing.h"
#include "params_sphere_enable.h"
#include "math_util.h"
#include "sphere.h"
#include "sphere_flash_io.h"
#include "led_cont.h"
#include "ui_modes.h"
#include "params_update.h" 

extern enum UI_Modes ui_mode;
extern o_spherebuf spherebuf;

extern o_UserSphereManager user_sphere_mgr;
extern o_params params;
extern o_calc_params calc_params;

// Given a sphere index (the "nth" sphere), return the physical bank number (flash slot)
//
uint8_t sphere_index_to_bank(uint8_t wtsel)
{
	uint8_t i;
	uint8_t filled_sphere_count = 0;

	for (i=0; i<MAX_TOTAL_SPHERES; i++)
	{
		if (is_sphere_filled(i) && is_sphere_enabled(i))
		{
			if (filled_sphere_count==wtsel)
				return i;
			else
				filled_sphere_count++;
		}
	}
	return 0; //error: not found, return fail-safe value
}

// Given a physical bank number (flash slot), return sphere index (the "nth" sphere) 
//
uint8_t bank_to_sphere_index(uint8_t wtbank)
{
	uint8_t i;
	uint8_t filled_sphere_count = 0;

	if (!is_sphere_filled(wtbank))
		return 0; //error: bank is not filled, return fail-safe value

	for (i=0; i<=wtbank; i++)
	{
		if (is_sphere_filled(i))
			filled_sphere_count++;
	}
	if (filled_sphere_count)
		return filled_sphere_count-1;
	else
		return 0;
}

void update_number_of_user_spheres_filled(void)
{	
	uint8_t i;
	uint8_t num_filled=0;


	for (i=0; i<MAX_TOTAL_SPHERES; i++)
	{
		if (is_sphere_filled(i)	&& is_sphere_enabled(i))
			num_filled++;
	}
	set_num_sphere_filled(num_filled);
}

void save_user_sphere(uint8_t sphere_num)
{
	ui_mode = WTSAVING;
	if (sphere_num>=NUM_FACTORY_SPHERES)
		save_unformatted_sphere_to_flash(sphere_num, SPHERE_TYPE_USER, spherebuf.data);

	//Verify all sphere types in flash
	read_all_spheretypes();
	update_number_of_user_spheres_filled();
}

void load_sphere(uint8_t sphere_num)
{	
	uint8_t sphere_index;
	uint8_t i;
	for (i=0; i<NUM_CHANNELS; i++) {
		// if (!params.wtsel_lock[i]) {
			params.wt_bank[i] = sphere_num;
			sphere_index = bank_to_sphere_index(sphere_num);
			params.wtsel_enc[i] = sphere_index;
			calc_params.wtsel[i] = sphere_index;
		// }
	}

	enter_wtediting();
}
