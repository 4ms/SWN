/*
 * presets.c - Saving and recalling presets
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
#include "UI_conditioning.h"
#include "drivers/flashram_spidma.h"
#include "globals.h"
#include "gpio_pins.h"
#include "hardware_controls.h"
#include "math_util.h"
#include "params_lfo_period.h"
#include "params_wt_browse.h"
#include "preset_manager_undo.h"
#include "preset_manager_UI.h"
#include "timekeeper.h"
#include "wavetable_saveload.h"
#include "startup_preset_storage.h"

extern o_params params;
extern o_lfos lfos;

o_preset_manager preset_mgr;

char	preset_signature_v1_0[4] = {'P', 'R', '9', '\0'};
char	preset_signature_v1_2[4] = {'P', 'R', 'A', '\0'};
char	preset_signature_vLatest[4] = {'P', 'R', 'B', '\0'};

static uint8_t cached_preset[sizeof(preset_signature_vLatest) + sizeof(o_params) + sizeof(o_lfos)];
static uint8_t animation_enabled = 1;
static char read_data[4];

//Todo: pack presets in more tightly:
//each sector contains 28 presets. next sector is blank for double buffering
//1) erase inactive buffer
//2) read each preset in active buffer into cached_preset, and then write it to inactive buffer
//3) --instead of reading the preset matching preset_num, just write current params/lfos to inactive buffer
//4) set word 1 of former active sector to DOUBLE_BUFFER_INACTIVE (0x01020304) (byte has never been written since it was erased, so it's ok to write w/o immediately erasing)
//5) word 1 of new active sector will be erased (0xFFFFFFFF == DOUBLE_BUFFER_ACTIVE)
// When reading a preset, or checking if preset is filled, always read word 1 to see if it's active or inactive. Skip inactive sectors

static inline void wait_for_flash_ready(void)
{
	while (get_flash_state() != sFLASH_NOTBUSY) {}
}

void init_preset_manager(void)
{
	char dummy;

	preset_mgr.hover_num = 0;
	preset_mgr.mode = PM_INACTIVE;
	preset_mgr.last_action = PM_INACTIVE;
	preset_mgr.animation_ctr = 0;

	uint16_t i;
	for (i = 0; i < MAX_PRESETS; i++) {
		if (check_preset_filled(i, &dummy))
			preset_mgr.filled[i] = 1;
		else
			preset_mgr.filled[i] = 0;
	}

	init_startup_preset_storage();
	uint16_t preset_num = get_startup_preset();
	if (preset_num)
	{
		animation_enabled = 0;
		recall_preset_into_active(preset_num);
		animation_enabled = 1;
	}
	stash_active_into_undo_buffer();

	// if (rotary_pressed(rotm_PRESET)) LOCK_PRESET_BANK_2 = 0; //Synth-meet Mode
}

void exit_preset_manager(void)
{
	preset_mgr.mode = PM_INACTIVE;
	preset_mgr.activity_tmr = 0;
	preset_mgr.animation_ctr = 0;
}

void store_preset_from_active(uint32_t preset_num)
{
	preset_mgr.hover_num = preset_num;
	preset_start_save_animation();
	store_preset(preset_num, &params, &lfos);
	set_startup_preset(preset_num);
}

void recall_preset_into_active(uint32_t preset_num)
{
	preset_mgr.hover_num = preset_num;
	if (animation_enabled)
		preset_start_load_animation();
	stash_active_into_undo_buffer();
	recall_preset(preset_num, &params, &lfos);
	recalc_active_params();
}

void store_preset(uint32_t preset_num, o_params *t_params, o_lfos *t_lfos)
{
	uint32_t addr = get_preset_addr(preset_num);
	uint32_t other_preset_addr;
	uint32_t sz;
	char dummy;

	pause_timer_IRQ(OSC_TIM_number);
	pause_timer_IRQ(WT_INTERP_TIM_number);
	pause_timer_IRQ(PWM_OUTS_TIM_number);
	wait_for_flash_ready();

	//store other half of sector in a temp variable
	other_preset_addr = get_preset_addr((preset_num & 1) ? preset_num - 1 : preset_num + 1);
	sFLASH_read_buffer(cached_preset, other_preset_addr, get_preset_size());

	sFLASH_erase_sector(addr);

	sz = 4;
	sFLASH_write_buffer((uint8_t *)preset_signature_vLatest, addr, sz);
	addr += sz;

	sz = sizeof(o_params);
	sFLASH_write_buffer((uint8_t *)t_params, addr, sz);
	addr += sz;

	sz = sizeof(o_lfos);
	sFLASH_write_buffer((uint8_t *)t_lfos, addr, sz);

	sFLASH_write_buffer(cached_preset, other_preset_addr, get_preset_size());

	//Verify sector was written (could use a checksum to be more rigorous)
	preset_mgr.filled[preset_num] = check_preset_filled(preset_num, &dummy);

	resume_timer_IRQ(OSC_TIM_number);
	resume_timer_IRQ(WT_INTERP_TIM_number);
	resume_timer_IRQ(PWM_OUTS_TIM_number);
}

void recall_preset(uint32_t preset_num, o_params *t_params, o_lfos *t_lfos)
{
	uint32_t addr = get_preset_addr(preset_num);
	uint32_t sz;
	uint8_t preset_is_filled;
	char version;

	pause_timer_IRQ(OSC_TIM_number);
	pause_timer_IRQ(WT_INTERP_TIM_number);
	pause_timer_IRQ(PWM_OUTS_TIM_number);

	while (get_flash_state() != sFLASH_NOTBUSY) {}

	//Manually checking is not necessary, but we want to make sure this sector is readable, and we already have control of FLASH
	preset_is_filled = check_preset_filled(preset_num, &version);
	if (preset_num < MAX_PRESETS && preset_mgr.filled[preset_num] && preset_is_filled) {
		//offset for signature
		addr += 4;

		sz = sizeof(o_params);
		sFLASH_read_buffer((uint8_t *)t_params, addr, sz);
		addr += sz;

		sz = sizeof(o_lfos);
		sFLASH_read_buffer((uint8_t *)t_lfos, addr, sz);

		if (version != preset_signature_vLatest[2])
			update_preset_version(version, t_params, t_lfos);

		fix_wtsel_wtbank_offset();
	}
	else {
		//Loading an unfilled preset re-initializes params
		init_param_object(t_params);
		init_lfo_object(t_lfos);
	}

	init_wbrowse_morph();

	resume_timer_IRQ(OSC_TIM_number);
	resume_timer_IRQ(WT_INTERP_TIM_number);
	resume_timer_IRQ(PWM_OUTS_TIM_number);
}

void update_preset_version(char version, o_params *t_params, o_lfos *t_lfos)
{
	if (version==preset_signature_v1_0[2]) {
		for (uint8_t i=0; i<MAX_TOTAL_SPHERES/8; i++)
			t_params->enabled_spheres[i]=0xFF;
		for (uint8_t i=0; i<NUM_CHANNELS; i++)
			t_params->pan[i] = default_pan(i);
	}
	if (version==preset_signature_v1_2[2]) {
		for (uint8_t i=0; i<NUM_CHANNELS; i++)
			t_params->pan[i] = default_pan(i);
	}
}

void clear_preset(uint32_t preset_num)
{
	//Write over the preset
	uint32_t addr = get_preset_addr(preset_num);
	uint32_t other_preset_addr;

	pause_timer_IRQ(WT_INTERP_TIM_number);
	wait_for_flash_ready();

	//store other half of sector in a temp variable
	other_preset_addr = get_preset_addr((preset_num & 1) ? preset_num - 1 : preset_num + 1);
	sFLASH_read_buffer(cached_preset, other_preset_addr, get_preset_size());

	sFLASH_erase_sector(addr);

	sFLASH_write_buffer(cached_preset, other_preset_addr, get_preset_size());

	preset_mgr.filled[preset_num] = 0;

	resume_timer_IRQ(WT_INTERP_TIM_number);
}

void recalc_active_params(void)
{
	init_calc_params();
	update_number_of_user_spheres_filled();
	update_all_wt_pos_interp_params();
	flag_all_lfos_recalc();
	force_all_wt_interp_update();
	for (int i=0; i<NUM_CHANNELS; i++)
		compute_tuning(i);	
}

//Writes over preset version in all sectors
//Much faster than erasing sectors
void clear_all_presets(void)
{
	uint8_t preset_num;
	uint32_t addr;
	uint8_t sz;

	pause_timer_IRQ(WT_INTERP_TIM_number);
	wait_for_flash_ready();

	for (preset_num = 0; preset_num < MAX_PRESETS; preset_num++) {
		addr = get_preset_addr(preset_num);
		sz = 4;
		sFLASH_read_buffer((uint8_t *)read_data, addr, sz);
		if (   read_data[0] == preset_signature_vLatest[0]
			&& read_data[1] == preset_signature_vLatest[1]
			// && read_data[2] == preset_signature_vLatest[2]
			&& read_data[3] == preset_signature_vLatest[3] )
		{
			sz = 4;
			read_data[0] = 0x00;
			read_data[1] = 0x00;
			read_data[2] = 0x00;
			read_data[3] = 0x00;
			sFLASH_write_buffer((uint8_t *)read_data, addr, sz);
		}

		preset_mgr.filled[preset_num] = 0;
	}

	resume_timer_IRQ(WT_INTERP_TIM_number);
}

uint8_t check_preset_filled(uint32_t preset_num, char *version)
{
	uint32_t addr = get_preset_addr(preset_num);
	uint32_t sz;

	sz = 4;
	wait_for_flash_ready();

	sFLASH_read_buffer((uint8_t *)read_data, addr, sz);

	if (   read_data[0] == preset_signature_vLatest[0]
		&& read_data[1] == preset_signature_vLatest[1]
		&& (read_data[2] == preset_signature_vLatest[2] \
			|| read_data[2] == preset_signature_v1_0[2] \
			|| read_data[2] == preset_signature_v1_2[2])
		&& read_data[3] == preset_signature_vLatest[3] )
	{
		*version = read_data[2];
		return 1;
	}
	else
		return 0;
}

uint32_t get_preset_size(void)
{
	return (4 + sizeof(o_params) + sizeof(o_lfos));
}

uint32_t get_preset_addr(uint32_t preset_num)
{
	uint32_t preset_sector_offset = 0;

	if (preset_num >= MAX_PRESETS)
		preset_num = (MAX_PRESETS - 1);

	uint32_t sector_num = PRESET_SECTOR_START + (preset_num >> 1);
	uint32_t sector_start = sFLASH_get_sector_addr(sector_num);

	//odd numbered presets start in middle of a sector
	if (preset_num & 1) preset_sector_offset = sFLASH_get_sector_size(sector_num) / 2;

	return sector_start +  preset_sector_offset;
}

