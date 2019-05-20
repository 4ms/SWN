/*
 * timekeeper.h - controls all interrupt timers
 * For running functions at designated intervals with designated priorities

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

#define MONO_LED_TIM_number					4
// #define UNUSED				5
#define OSC_TIM_number						6
// #define UNUSED 				7
#define ANALOG_CONDITIONING_TIM_number		8
#define PWM_OUTS_TIM_number					9
#define LED_UPDATE_TIM_number				10
#define	UI_CONDITIONING_UPDATE_TIM_number	11
#define WT_INTERP_TIM_number				12
#define LFO_TIM_number						14



typedef struct TimerITInitStruct{
	uint8_t		priority1;
	uint8_t		priority2;
	uint16_t	period;
	uint8_t		prescaler;
	uint8_t		clock_division;

} TimerITInitStruct;

#define TIM_IT_IS_SET(x,y) 		(((x)->SR & (y)) == (y))
#define TIM_IT_IS_SOURCE(x,y) 	((((x)->DIER & (y)) == (y)) ? SET : RESET)
#define TIM_IT_CLEAR(x,y) 		(x)->SR &= ~(y)


void init_timekeeper(void);
void start_timer_IRQ(uint8_t tim_number, void *callbackfunc);
void pause_timer_IRQ(uint8_t tim_number);
void resume_timer_IRQ(uint8_t tim_number);


