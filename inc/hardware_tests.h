#pragma once

#include <stm32f7xx.h>
#include "switch_driver.h"

#define FORCE_HW_TEST 0

extern o_rotary 	rotary[NUM_ROTARIES];
extern o_button 	button[NUM_ROTARIES];


static inline uint8_t key_combo_enter_hardwaretest(void)
{
	return (
		read_switch_state(&button[butm_D_BUTTON].hwswitch) \
		&& read_switch_state(&rotary[rotm_TRANSPOSE].hwswitch) \
		&& read_switch_state(&rotary[rotm_LFOSHAPE].hwswitch) \
	);}
 //use GPIO pin reading, not UI-conditioned values in order to minimize sources of error other than hardware

																	// && read_switch_state(&rotary[rotm_OCT].hwswitch) 
																	// && read_switch_state(&button[butm_LFOVCA_BUTTON].hwswitch)
																	// && read_switch_state(&button[butm_LFOMODE_BUTTON].hwswitch) 

uint8_t is_hardwaretest_already_done(void);
void do_hardware_test(void);
