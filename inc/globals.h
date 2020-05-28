/*
 * globals.h
 *
 * Author: Dan Green(danngreen1@gmail.com), Hugo Paris (hugoplho@gmail.com)
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

// #define FORCE_WRITE_FACTORY_SPHERES 	// <-- Uncomment this to force re-loading of wavetables from const arrays, boot time will increase
// #define CLEAR_USER_SPHERES_FROM_FLASH 	// <-- Uncomment this to erase user spheres from flash
#define SKIP_FACTORY_SPHERES_IN_HEXFILE  // <-- Uncomment this to create a smaller ELF/HEX/BIN file, by not including the factory spheres (you must have them already loaded, of course)
// #define ERASE_ALL_WAVETABLES

// Set the PCB version here:
#define PCB_VERSION 100			//version v1.0 and v1.1
// #define PCB_VERSION 25		//In-house Proto (white PCB)
// #define PCB_VERSION 24		//Beta version

#ifndef PCB_VERSION
	#define PCB_VERSION 100
#endif

#define FW_MAJOR_VERSION 	2
#define FW_MINOR_VERSION 	1

#define ALLOW_LOCKS

//Any firmware version less than these numbers will be considered invalid and a factory reset will be forced
#define MAX_VALID_MAJOR_FW_VERSION 15
#define MAX_VALID_MINOR_FW_VERSION 15

#define DEBUG_2_3_ENABLED


#define SAMPLERATE 			44100
#define F_SAMPLERATE 		44100.0

#define NUM_INRING_LEDS		6
#define NUM_OUTRING_LEDS	18
#define NUM_LEDLINE_LEDS	6
#define NUM_CHANNELS 		6

#define TICKS_PER_MS 	8

enum CacheUncache {
	CACHE,
	UNCACHE
};


// Stuff below here works with the linker file to utilize memory in the most efficient way

#define DMABUFFER 
// #define DMABUFFER 	__attribute__ ((section (".dtcmdata"))) 

#define SRAM1DATA	__attribute__ ((section (".sram1data")))
// #define SRAM1DATA 
