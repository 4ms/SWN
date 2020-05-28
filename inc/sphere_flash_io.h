/*
 * sphere_flash_io.h - Spherical Wavetable Navigator I/O with external FLASH memory
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
#include "sphere.h"

enum SphereTypes {
	SPHERE_TYPE_EMPTY = 0,
	SPHERE_TYPE_USER = 1,
	SPHERE_TYPE_FACTORY = 2,
	SPHERE_TYPE_CLEARED = 3,

	NUM_SPHERE_TYPES
};

void init_sphere_flash(void);
void write_factory_spheres_to_extflash(void);
void restore_factory_spheres_to_extflash(void);

void load_extflash_wavetable(uint8_t wt_num, o_waveform *waveform, uint8_t x, uint8_t y, uint8_t z);
void load_extflash_wave_raw(uint8_t wt_num, int16_t *waveform, uint8_t x, uint8_t y, uint8_t z);
uint32_t get_wt_addr(uint16_t wt_num);

void save_sphere_to_flash(uint8_t wt_num, enum SphereTypes sphere_type, int16_t *sphere_data);
void save_unformatted_sphere_to_flash(uint8_t wt_num, enum SphereTypes sphere_type, o_waveform sphere_data[WT_DIM_SIZE][WT_DIM_SIZE][WT_DIM_SIZE]);

enum SphereTypes read_spheretype(uint32_t wt_num);

void empty_all_user_spheres(void);
enum SphereTypes clear_user_sphere(uint8_t wt_num);
enum SphereTypes unclear_user_sphere(uint8_t wt_num);
uint8_t all_factory_spheres_present(void);

enum SphereTypes get_spheretype(uint32_t wt_num);
void read_all_spheretypes(void);
uint8_t is_sphere_filled(uint8_t wt_num);
