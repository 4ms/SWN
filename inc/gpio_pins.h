/*
 * gpio_pins.h - setup digital input and output pins
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
#include "hardware_controls.h"

#define PIN_ON(x,y) 	x->BSRR = y
#define PIN_OFF(x,y) 	x->BSRR = (uint32_t)y << 16
#define PIN_READ(x, y) (((x->IDR) & y) ? 1 : 0)

#define PIN_HIGH(x,y)	PIN_ON(x,y)
#define PIN_LOW(x,y)	PIN_OFF(x,y)

//##############################################################################
// 								GPIOs and PINS 
//##############################################################################

#if (PCB_VERSION>=100)
	#define BUTTON_D_PIN 			GPIO_PIN_7
#else
	#define BUTTON_D_PIN 			GPIO_PIN_2
#endif

#if (PCB_VERSION>=25)
#define OCTAVETRANSPOSE_ROTARY_PULL PULLUP
#define OCTAVETRANSPOSE_DIR_REV
#else
	#define OCTAVETRANSPOSE_ROTARY_PULL PULLDOWN
#endif

#define NON_WT_ROTARY_STEPSIZE 		HALFSTEP
#define LFOANDPRESET_ROTARY_PULL 	PULLUP



// CLOCK IN
#define CLK_IN_pin 				GPIO_PIN_12
#define CLK_IN_GPIO 			GPIOG
#define CLK_IN()					PIN_READ(CLK_IN_GPIO, CLK_IN_pin)

#define BUS_CLK_IN_pin			GPIO_PIN_8
#define BUS_CLK_IN_GPIO			GPIOC
#define BUS_CLK() 				PIN_READ(BUS_CLK_IN_GPIO, BUS_CLK_IN_pin)

#if (PCB_VERSION>=100)
	#define BUS_SEL_IN_pin 			GPIO_PIN_2
	#define BUS_SEL_IN_GPIO 		GPIOD
	#define BUS_SEL() 				PIN_READ(BUS_SEL_IN_GPIO, BUS_SEL_IN_pin)

#elif (PCB_VERSION>=25)
	#define BUS_SEL_IN_pin			GPIO_PIN_4
	#define BUS_SEL_IN_GPIO			GPIOE
	#define BUS_SEL() 				PIN_READ(BUS_SEL_IN_GPIO, BUS_SEL_IN_pin)
#endif

// LED RING
#define LED_RING_OE_GPIO 		GPIOB
#define LED_RING_OE_pin 		GPIO_PIN_7
#define LED_RING_ON()			PIN_ON(LED_RING_OE_GPIO, LED_RING_OE_pin)
#define LED_RING_OFF()			PIN_OFF(LED_RING_OE_GPIO, LED_RING_OE_pin)

#define DEBUG0_GPIO 			GPIOE
#define DEBUG0_pin 				GPIO_PIN_8
#define DEBUG1_GPIO 			GPIOE
#define DEBUG1_pin 				GPIO_PIN_7

#define DEBUG0_ON 				PIN_ON(DEBUG0_GPIO, DEBUG0_pin)
#define DEBUG0_OFF 				PIN_OFF(DEBUG0_GPIO, DEBUG0_pin)

#define DEBUG1_ON 				PIN_ON(DEBUG1_GPIO, DEBUG1_pin)
#define DEBUG1_OFF 				PIN_OFF(DEBUG1_GPIO, DEBUG1_pin)

#ifdef DEBUG_2_3_ENABLED
	#define DEBUG2_pin 			GPIO_PIN_5
	#define DEBUG2_GPIO 		GPIOE
	#define DEBUG3_pin 			GPIO_PIN_6
	#define DEBUG3_GPIO 		GPIOE

	#define DEBUG2_ON 			PIN_ON(DEBUG2_GPIO, DEBUG2_pin)
	#define DEBUG2_OFF 			PIN_OFF(DEBUG2_GPIO, DEBUG2_pin)
	#define DEBUG3_ON 			PIN_ON(DEBUG3_GPIO, DEBUG3_pin)
	#define DEBUG3_OFF 			PIN_OFF(DEBUG3_GPIO, DEBUG3_pin)
#else 
	#define DEBUG2_pin 		
	#define DEBUG3_pin 			
	#define DEBUG2_GPIO 		
	#define DEBUG3_GPIO 	

	#define DEBUG2_OFF
	#define DEBUG3_OFF
	#define DEBUG2_ON
	#define DEBUG3_ON
#endif

#define ALL_GPIO_RCC_ENABLE		__HAL_RCC_GPIOA_CLK_ENABLE();\
								__HAL_RCC_GPIOB_CLK_ENABLE();\
								__HAL_RCC_GPIOC_CLK_ENABLE();\
								__HAL_RCC_GPIOD_CLK_ENABLE();\
								__HAL_RCC_GPIOE_CLK_ENABLE();\
								__HAL_RCC_GPIOF_CLK_ENABLE();\
								__HAL_RCC_GPIOG_CLK_ENABLE();\
								__HAL_RCC_GPIOH_CLK_ENABLE();
								
void 	init_gpio_pins(void);
void 	set_gpio_map(void);
