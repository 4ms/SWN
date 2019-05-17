// Author: Hugo Paris, hugoplho@gmail.com
// Dan Green, danngreen1@gmail.com


#pragma once

#include <stm32f7xx.h>

#define 	LFO_TABLELEN 	256 			//Must be a power of 2
#define 	F_LFO_TABLELEN 	256.0
#define	 	F_MAX_LFO_TABLELEN				(float)(F_LFO_TABLELEN - 1.0)	
#define 	NUM_LFO_SHAPES  25 
#define 	NUM_LFO_GROUPS 	6 		
#define 	MAX_LFO_WT_VAL 	256
#define 	KEY_SHAPE		0
	
extern 		uint8_t 		LFOS_TO_BANK_END[NUM_LFO_GROUPS];
extern const uint8_t 		lfo_wavetable[NUM_LFO_SHAPES][LFO_TABLELEN];
