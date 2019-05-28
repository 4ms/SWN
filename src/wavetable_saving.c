/*
 * wavetable_saving.c
 *
 * Author: Hugo Paris (hugoplho@gmail.com), Dan Green (danngreen1@gmail.com)
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
#include "wavetable_saving.h"
#include "wavetable_saving_UI.h"
#include "math_util.h"
#include "sphere.h"
#include "sphere_flash_io.h"
#include "led_cont.h"
#include "ui_modes.h"
#include "wavetable_editing.h"
#include "params_update.h" 

extern enum UI_Modes ui_mode;
extern o_spherebuf spherebuf;

extern o_UserSphereManager user_sphere_mgr;


// Given a sphere index (the "nth" sphere), return the physical bank number (flash slot)
//
uint8_t sphere_index_to_bank(uint8_t wtsel)
{
	uint8_t i;
	uint8_t filled_sphere_count;

	if (wtsel<NUM_FACTORY_SPHERES)
		return wtsel;

	filled_sphere_count = NUM_FACTORY_SPHERES;

	for (i=0; i<NUM_USER_SPHERES_ALLOWED; i++)
	{
		if (is_spheretype_user(i+NUM_FACTORY_SPHERES))
		{
			if (filled_sphere_count==wtsel)
				return i+NUM_FACTORY_SPHERES;
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
	uint8_t filled_sphere_count;

	if (wtbank<NUM_FACTORY_SPHERES)
		return wtbank;

	if (!is_sphere_filled(wtbank))
		return 0; //error: bank is not filled, return fail-safe value

	filled_sphere_count = NUM_FACTORY_SPHERES-1;

	for (i=NUM_FACTORY_SPHERES; i<=wtbank; i++)
	{
		if (is_spheretype_user(i))
			filled_sphere_count++;
	}

	return filled_sphere_count;
}

void update_number_of_user_spheres_filled(void){
	
	uint8_t i;
	uint8_t num_filled=0;

	read_all_spheretypes();
	
	for (i=0; i<NUM_USER_SPHERES_ALLOWED; i++)
	{
		if (is_spheretype_user(i+NUM_FACTORY_SPHERES))
			num_filled++;
	}

	update_num_sphere_filled(num_filled);
}

void save_user_sphere(uint8_t sphere_num)
{
	ui_mode = WTSAVING;
	
	save_unformatted_sphere_to_flash(NUM_FACTORY_SPHERES + sphere_num, SPHERE_TYPE_USER, spherebuf.data);

	//Verify all sphere types in flash
	update_number_of_user_spheres_filled();
}


void clear_user_spheres_from_flash(void){
	quick_clear_user_spheres();

	// uint8_t i;
	// o_waveform sphere_data[WT_DIM_SIZE][WT_DIM_SIZE][WT_DIM_SIZE]={0};

	// for (i=0; i< MAX_TOTAL_SPHERES; i++){
	// 	save_sphere_to_flash(NUM_FACTORY_SPHERES + i, SPHERE_TYPE_EMPTY, (int16_t*)sphere_data);
	// }
	// update_number_of_user_spheres_filled();
}
