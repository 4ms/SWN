
 /*
 * key_combos.c
 *
 * Author: Dan Green (danngreen1@gmail.com), Hugo Paris (hugoplho@gmail.com)
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
#include "hardware_controls.h"
#include "UI_conditioning.h"

// WT REC/EDIT/SAVE
static inline uint8_t key_combo_enter_editing			(void)	{ return (rotary_pressed(rotm_LATITUDE) && rotary_pressed(rotm_LONGITUDE) && rotary_pressed(rotm_DEPTH)); }
static inline uint8_t key_combo_enter_editing_released	(void)	{ return (rotary_released(rotm_LATITUDE) && rotary_released(rotm_LONGITUDE) && rotary_released(rotm_DEPTH)); }
static inline uint8_t key_combo_enter_monitoring		(void)	{ return  button_pressed(butm_LFOMODE_BUTTON); }
static inline uint8_t key_combo_enter_ttone				(void)	{ return (key_combo_enter_monitoring() && switch_pressed(FINE_BUTTON)); }
static inline uint8_t key_combo_exit_monitoring			(void)	{ return  key_combo_enter_monitoring(); }
static inline uint8_t key_combo_enter_immediate_rec		(void)	{ return (key_combo_enter_editing() && switch_pressed(FINE_BUTTON)); }
static inline uint8_t key_combo_enter_recording			(void)	{ return  button_pressed(butm_LFOVCA_BUTTON) && !switch_pressed(FINE_BUTTON); }
static inline uint8_t key_combo_enter_record_one		(void)	{ return  button_pressed(butm_LFOVCA_BUTTON) && switch_pressed(FINE_BUTTON); }

static inline uint8_t key_combo_save_request			(void)	{ return  rotary_short_pressed(rotm_PRESET) && !rotary_pressed(rotm_WAVETABLE) && !button_pressed(butm_LFOVCA_BUTTON); }
static inline uint8_t key_combo_save_confirm			(void)	{ return  rotary_pressed(rotm_PRESET); }
static inline uint8_t key_combo_load_request			(void)	{ return  rotary_reg_pressed(rotm_PRESET) && !rotary_pressed(rotm_WAVETABLE) && !button_pressed(butm_LFOVCA_BUTTON); }
static inline uint8_t key_combo_load_confirm			(void)	{ return  rotary_pressed(rotm_PRESET); }

static inline uint8_t key_combo_exit_request			(void)	{ return (rotary_pressed(rotm_LATITUDE) && rotary_pressed(rotm_LONGITUDE) && rotary_pressed(rotm_DEPTH)); }
static inline uint8_t key_combo_clear_user_spheres		(void)	{ return (rotary_pressed(rotm_WAVETABLE) &&rotary_pressed(rotm_LATITUDE)   && rotary_pressed(rotm_LONGITUDE) && rotary_pressed(rotm_DEPTH)); }

// V/Oct calibration
static inline uint8_t key_combo_enter_voct_calibrate	(void) 	{ return (!rotary_pressed(rotm_PRESET) && rotary_pressed(rotm_OCT) && rotary_pressed(rotm_TRANSPOSE) && rotary_released(rotm_WAVETABLE)); }
static inline uint8_t key_combo_exit_voct_calibrate		(void) 	{ return  rotary_med_pressed(rotm_WAVETABLE); }
static inline uint8_t key_combo_cancel_voct_calibrate	(void) 	{ return (!rotary_pressed(rotm_PRESET) && rotary_pressed(rotm_OCT) && rotary_pressed(rotm_TRANSPOSE) && rotary_released(rotm_WAVETABLE)); }

// Reset parameters
static inline uint8_t key_combo_reset_detuning			(void)	{ return (rotary_pressed(rotm_PRESET) && rotary_pressed(rotm_TRANSPOSE) && switch_pressed(FINE_BUTTON)); }
static inline uint8_t key_combo_reset_transpose			(void)	{ return (rotary_pressed(rotm_PRESET) && rotary_pressed(rotm_TRANSPOSE) && !switch_pressed(FINE_BUTTON)); }
static inline uint8_t key_combo_reset_octaves			(void)	{ return (rotary_pressed(rotm_PRESET) && rotary_pressed(rotm_OCT)); }

static inline uint8_t key_combo_reset_navigation		(void)	{ return (rotary_pressed(rotm_DEPTH) && rotary_pressed(rotm_PRESET)); }
static inline uint8_t key_combo_reset_sphere_sel		(void)	{ return (rotary_pressed(rotm_LATITUDE) && rotary_pressed(rotm_PRESET)); }

static inline uint8_t key_combo_reset_lfos_all			(void)	{ return (rotary_short_pressed(rotm_LFOSPEED) && rotary_short_pressed(rotm_LFOSHAPE) && rotary_pressed(rotm_PRESET)); }
static inline uint8_t key_combo_reset_lfos_shapes		(void)	{ return (!rotary_pressed(rotm_LFOSPEED) && rotary_pressed(rotm_LFOSHAPE) && rotary_pressed(rotm_PRESET)); }
static inline uint8_t key_combo_reset_lfos_speeds		(void)	{ return (rotary_pressed(rotm_LFOSPEED) && !rotary_pressed(rotm_LFOSHAPE) && rotary_pressed(rotm_PRESET)); }
static inline uint8_t key_combo_reset_lfos_phases		(void)	{ return (rotary_pressed(rotm_LFOSPEED) && rotary_pressed(rotm_LFOSHAPE) && !rotary_pressed(rotm_PRESET)); }
static inline uint8_t key_combo_reset_lfos_released		(void)	{ return (rotary_released(rotm_LFOSPEED) && rotary_released(rotm_LFOSHAPE)); }

static inline uint8_t key_combo_reset_to_factory		(void)	{ return (rotary_long_pressed(rotm_OCT) && rotary_long_pressed(rotm_TRANSPOSE) && rotary_long_pressed(rotm_PRESET) && rotary_long_pressed(rotm_LONGITUDE) && rotary_long_pressed(rotm_WAVETABLE)); }


// Key Mode
static inline uint8_t key_combo_keymode_pressed 		(void)	{ return ( button_pressed(butm_LFOVCA_BUTTON) 		&& button_pressed(butm_LFOMODE_BUTTON) && !rotary_pressed(rotm_PRESET)); }
// static inline uint8_t key_combo_keymode_short_pressed  	(void)	{ return ( button_short_pressed(butm_LFOVCA_BUTTON) && button_short_pressed(butm_LFOMODE_BUTTON));}
// static inline uint8_t key_combo_keymode_med_pressed 	(void)	{ return ( button_med_pressed(butm_LFOVCA_BUTTON) 	&& button_med_pressed(butm_LFOMODE_BUTTON));}
// static inline uint8_t key_combo_keymode_long_pressed 	(void)	{ return ( button_long_pressed(butm_LFOVCA_BUTTON) 	&& button_long_pressed(butm_LFOMODE_BUTTON)); }
static inline uint8_t key_combo_keymode_released 	 	(void) 	{ return ( button_released(butm_LFOVCA_BUTTON) 		&& button_released(butm_LFOMODE_BUTTON) && !rotary_pressed(rotm_PRESET)); }

// Presets
static inline uint8_t key_combo_all_but_preset_released (void)	{ return ( button_released(butm_LFOVCA_BUTTON) \
																		&& button_released(butm_LFOMODE_BUTTON) \
																		&& button_released(butm_B_BUTTON) \
																		&& rotary_released(rotm_OCT) \
																		&& rotary_released(rotm_TRANSPOSE) \
																		&& rotary_released(rotm_DEPTH) \
																		&& rotary_released(rotm_LATITUDE) \
																		&& rotary_released(rotm_LONGITUDE) \
																		&& rotary_released(rotm_WAVETABLE) \
																		&& rotary_released(rotm_LFOSPEED) \
																		&& rotary_released(rotm_LFOSHAPE));}
#ifdef ALLOW_LOCKS
static inline uint8_t key_combo_lock_channel (uint8_t chan) 	{ return ( button_pressed(butm_A_BUTTON+chan) && switch_pressed(FINE_BUTTON)); }
#else
static inline uint8_t key_combo_lock_channel (uint8_t chan) 	{ return (0); }
#endif

static inline uint8_t key_combo_show_selbus_allows(void) { return (rotary_pressed(rotm_PRESET) \
																	&& !switch_pressed(FINE_BUTTON) \
																	&& button_pressed(butm_B_BUTTON)); }

static inline uint8_t key_combo_toggle_selbus_recall(void) { return (rotary_pressed(rotm_PRESET) \
																	&& !switch_pressed(FINE_BUTTON) \
																	&& button_pressed(butm_LFOVCA_BUTTON) \
																	&& button_pressed(butm_B_BUTTON)); }

static inline uint8_t key_combo_enable_selbus_save(void) { return (rotary_pressed(rotm_PRESET) \
																	&& !switch_pressed(FINE_BUTTON) \
																	&& button_med_pressed(butm_LFOMODE_BUTTON) \
																	&& button_pressed(butm_B_BUTTON)); }

static inline uint8_t key_combo_disable_selbus_save(void) { return (rotary_pressed(rotm_PRESET) \
																	&& !switch_pressed(FINE_BUTTON) \
																	&& button_pressed(butm_LFOMODE_BUTTON) \
																	&& button_pressed(butm_B_BUTTON)); }

