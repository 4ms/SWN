/*
 * envout_pwm.h
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

#define PWM_MAX 		254	// Maximum PWM amplitude

#define LFO_TRIG_LPF	0.09
#define F_TRIGVCADUR  	700.0

// #define LFO_TOVCA_LPF	0.03
#define TRIG_DURATION 	20
#define TRIGDISPDUR 	400
// #define TRIGVCADUR  	600

// PWM OUTs

// PWM Output Jacks A and B
#define ENVOUT_PWM_TIM_AB		TIM1

#define ENVOUT_PWM_CC_A 		CCR2
#define ENVOUT_PWM_CC_B 		CCR1

#define ENVOUT_PWM_CHAN_A 		TIM_CHANNEL_2
#define ENVOUT_PWM_CHAN_B 		TIM_CHANNEL_1

#define ENVOUT_PWM_TIM_AB_AF	GPIO_AF1_TIM1
#define ENVOUT_PWM_pins_AB 		(GPIO_PIN_9 | GPIO_PIN_8)
#define ENVOUT_PWM_GPIO_AB 		GPIOA


// PWM Output Jack C
#define ENVOUT_PWM_TIM_C		TIM3

#define ENVOUT_PWM_CC_C 		CCR3
#define ENVOUT_PWM_CHAN_C 		TIM_CHANNEL_3

#define ENVOUT_PWM_TIM_C_AF		GPIO_AF2_TIM3
#define ENVOUT_PWM_pins_C 		GPIO_PIN_0
#define ENVOUT_PWM_GPIO_C 		GPIOB


// PWM Output Jacks D, E, and F
#define ENVOUT_PWM_TIM_DEF		TIM2

#define ENVOUT_PWM_CC_D 		CCR3
#define ENVOUT_PWM_CC_E 		CCR2
#define ENVOUT_PWM_CC_F			CCR1
#define ENVOUT_PWM_CHAN_D	 		TIM_CHANNEL_3
#define ENVOUT_PWM_CHAN_E 		TIM_CHANNEL_2
#define ENVOUT_PWM_CHAN_F 		TIM_CHANNEL_1

#define ENVOUT_PWM_TIM_DEF_AF	GPIO_AF1_TIM2
#define ENVOUT_PWM_pins_DEF 	(GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0)
#define ENVOUT_PWM_GPIO_DEF 	GPIOA

#define ENVOUT_PWM_TIM_GPIO_RCC_ENABLE()	__HAL_RCC_GPIOA_CLK_ENABLE(); __HAL_RCC_GPIOB_CLK_ENABLE()
#define ENVOUT_PWM_RCC_ENABLE()				__HAL_RCC_TIM1_CLK_ENABLE(); __HAL_RCC_TIM2_CLK_ENABLE(); __HAL_RCC_TIM3_CLK_ENABLE();


void init_envout_pwm(void);
void start_envout_pwm(void);
