/*
 * params_sphere_enable.c - Parameters for enabling/disable spheres within a preset
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

#include "params_sphere_enable.h"
#include "globals.h"
#include "params_update.h"

extern o_params params;

uint8_t is_sphere_enabled(uint8_t sphere_num)
{
	if (params.enabled_spheres[sphere_num/8] & (1<<(sphere_num & 7)) )
		return 1;
	else
		return 0;
}

void enable_sphere(uint8_t sphere_num)
{
	params.enabled_spheres[sphere_num/8] |= (1<<(sphere_num & 7));
}

void disable_sphere(uint8_t sphere_num)
{
	params.enabled_spheres[sphere_num/8] &= ~(1<<(sphere_num & 7));
}

