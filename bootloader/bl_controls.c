#include "bl_controls.h"
#include "hardware_controls.h"
#include "switch_driver.h"

extern o_rotary 		rotary[NUM_ROTARIES];
extern o_button 		button[NUM_BUTTONS];

uint32_t rotary_is_pressed(uint32_t rotary_num) { 
	return read_switch_state(&rotary[rotary_num].hwswitch);
}

uint32_t button_is_pressed(uint32_t button_num) { 
	return read_switch_state(&button[button_num].hwswitch);
}
