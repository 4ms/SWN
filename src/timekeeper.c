/*
 * timekeeper.c - controls all interrupt timers
 * For running functions at designated intervals with designated priorities
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

#include "timekeeper.h"
#include "hal_handlers.h"

#include "led_cont.h"
#include "analog_conditioning.h"
#include "drivers/ads8634_driver.h"


#define USE_HAL_TIM_REGISTER_CALLBACKS 0

#define NUM_TIMERS 14
typedef void (*voidfunc_type)(void);

voidfunc_type tim_callbacks			[NUM_TIMERS+1];
voidfunc_type cached_tim_callbacks	[NUM_TIMERS+1];

TimerITInitStruct tim_timing[NUM_TIMERS+1];

//Private:
void deinit_timekeeper(void);
void init_interrupt_timer(uint8_t TIM_periph_number, TimerITInitStruct *timinit);

void start_timer_IRQ(uint8_t tim_number, void *callbackfunc)
{
	init_interrupt_timer(tim_number, &(tim_timing[tim_number]));
	tim_callbacks[tim_number] = callbackfunc;
}

void pause_timer_IRQ(uint8_t tim_number)
{
	cached_tim_callbacks[tim_number] = tim_callbacks[tim_number];
	tim_callbacks[tim_number] = NULL;
}
void resume_timer_IRQ(uint8_t tim_number)
{
	if (cached_tim_callbacks[tim_number] != NULL) 
		tim_callbacks[tim_number] = cached_tim_callbacks[tim_number];
}
//
// Prepares timing values for all timer IRQs
// Configure your timing values here
//
void init_timekeeper(void)
{
	uint8_t i;

	deinit_timekeeper();

	//Peripheral clocks run at /2 of system clock = 180MHz/2 = 90MHz

	//Analog Conditioning
	//Prescale --> 216MHz / (7+1) = 27MHz
	//Period --> 27MHz / 9000 = 3kHz
	i = ANALOG_CONDITIONING_TIM_number;
	tim_timing[i].priority1 		= 3;
	tim_timing[i].priority2 		= 2;
	tim_timing[i].period 			= 9000;
	tim_timing[i].prescaler			= 7;
	tim_timing[i].clock_division 	= 0;

	//UI Param update (encoders, switches, buttons)
	//Run every 1kHz (1ms)
	//Prescale = 3 --> 108MHz / 4 = 27MHz
	//Period = 27000 --> 27MHz / 27000 = 1kHz
	i = UI_CONDITIONING_UPDATE_TIM_number;
	tim_timing[i].priority1 		= 1;
	tim_timing[i].priority2 		= 2;
	tim_timing[i].period 			= 27000;
	tim_timing[i].prescaler			= 3;
	tim_timing[i].clock_division 	= 0;


	//PWM OUTS (Env/LFO outputs) update
	//Run every 3kHz (333us)
	//Prescale = 0 --> 216MHz / 1 = 216MHz
	//Period = 30000 --> 216MHz / 30000 = 7.2kHz (138.9us)
	i = PWM_OUTS_TIM_number;
	tim_timing[i].priority1 		= 0;
	tim_timing[i].priority2 		= 3;
	tim_timing[i].period 			= 30000;
	tim_timing[i].prescaler			= 0;
	tim_timing[i].clock_division 	= 0;


	//Mono LED PWM update
	//Run every 1.4kHz (711us)
	//Prescale = 1 --> 108MHz / 2 = 54MHz
	//Period = 18000 --> 54MHz / 18000 = 3kHz
	//PWM steps = 32 ---> 3kHz / 32 = 94Hz refresh rate 
	i = MONO_LED_TIM_number;
	tim_timing[i].priority1 		= 0;
	tim_timing[i].priority2 		= 3;
	tim_timing[i].period 			= 18000;
	tim_timing[i].prescaler			= 1;
	tim_timing[i].clock_division 	= 0;

	//WT update
	//Run every 1.8kHz (555us)
	//Prescale = 1 --> 108MHz / 2 = 54MHz
	//Period = 30000 --> 54MHz / 30000 = 1.8kHz
	i = WT_INTERP_TIM_number;
	tim_timing[i].priority1 		= 1;
	tim_timing[i].priority2 		= 1;
	tim_timing[i].period 			= 30000;
	tim_timing[i].prescaler			= 1;
	tim_timing[i].clock_division 	= 0;

	//OSC param update
	//Run every 1.8kHz (555us)
	//Prescale = 1 --> 108MHz / 2 = 54MHz
	//Period = 30000 --> 54MHz / 30000 = 1.8kHz
	i = OSC_TIM_number;
	tim_timing[i].priority1 		= 1;
	tim_timing[i].priority2 		= 1;
	tim_timing[i].period 			= 30000;
	tim_timing[i].prescaler			= 1;
	tim_timing[i].clock_division 	= 0;	

	//LED Update
	//Prescale = 120 --> 216MHz / 120 = 1800kHz
	//Period = 30000 --> 60Hz
	i = LED_UPDATE_TIM_number;
	tim_timing[i].priority1 		= 1;
	tim_timing[i].priority2 		= 1;
	tim_timing[i].period 			= 30000;
	tim_timing[i].prescaler			= 119;
	tim_timing[i].clock_division 	= 0;	


	//Other interrupts:
	//ADS8634 IRQ: priority 3, 0
	//Codec SAI: priority 0, 0
}

void deinit_timekeeper(void)
{
	uint8_t i;

	// Clear all timer callbacks
	for (i=0;i<NUM_TIMERS;i++)
	{
		tim_callbacks[i+1] = (voidfunc_type)NULL;

		tim_timing[i+1].priority1 		= 0;
		tim_timing[i+1].priority2 		= 0;
		tim_timing[i+1].period 			= 0;
		tim_timing[i+1].prescaler 		= 0;
		tim_timing[i+1].clock_division 	= 0;
	}
}


void init_interrupt_timer(uint8_t TIM_periph_number, TimerITInitStruct *timinit)
{
	TIM_HandleTypeDef	tim;
	uint8_t				IRQn=0;

	//Using TIM1 means we can't use TIM10, and using TIM8 means we can't use TIM13

	#ifdef TIM1
		if (TIM_periph_number == 1) { __HAL_RCC_TIM1_CLK_ENABLE();	tim.Instance = TIM1;	IRQn = TIM1_UP_TIM10_IRQn; }
	#endif
	#ifdef TIM2
		if (TIM_periph_number == 2) { __HAL_RCC_TIM2_CLK_ENABLE();	tim.Instance = TIM2;	IRQn = TIM2_IRQn; }
	#endif
	#ifdef TIM3
		if (TIM_periph_number == 3) { __HAL_RCC_TIM3_CLK_ENABLE();	tim.Instance = TIM3;	IRQn = TIM3_IRQn; }
	#endif
	#ifdef TIM4
		if (TIM_periph_number == 4) { __HAL_RCC_TIM4_CLK_ENABLE();	tim.Instance = TIM4;	IRQn = TIM4_IRQn; }
	#endif
	#ifdef TIM5
		if (TIM_periph_number == 5) { __HAL_RCC_TIM5_CLK_ENABLE();	tim.Instance = TIM5;	IRQn = TIM5_IRQn; }
	#endif
	#ifdef TIM6
		if (TIM_periph_number == 6) { __HAL_RCC_TIM6_CLK_ENABLE();	tim.Instance = TIM6;	IRQn = TIM6_DAC_IRQn; }
	#endif
	#ifdef TIM7
		if (TIM_periph_number == 7) { __HAL_RCC_TIM7_CLK_ENABLE();	tim.Instance = TIM7;	IRQn = TIM7_IRQn; }
	#endif
	#ifdef TIM8
		if (TIM_periph_number == 8) { __HAL_RCC_TIM8_CLK_ENABLE();	tim.Instance = TIM8;	IRQn = TIM8_UP_TIM13_IRQn; }
	#endif
	#ifdef TIM9
		if (TIM_periph_number == 9) { __HAL_RCC_TIM9_CLK_ENABLE();	tim.Instance = TIM9;	IRQn = TIM1_BRK_TIM9_IRQn; }
	#endif
	#ifdef TIM10
		if (TIM_periph_number == 10) { __HAL_RCC_TIM10_CLK_ENABLE(); tim.Instance = TIM10;	IRQn = TIM1_UP_TIM10_IRQn; }
	#endif
	#ifdef TIM11
		if (TIM_periph_number == 11) { __HAL_RCC_TIM11_CLK_ENABLE(); tim.Instance = TIM11;	IRQn = TIM1_TRG_COM_TIM11_IRQn; }
	#endif
	#ifdef TIM12
		if (TIM_periph_number == 12) { __HAL_RCC_TIM12_CLK_ENABLE(); tim.Instance = TIM12;	IRQn = TIM8_BRK_TIM12_IRQn; }
	#endif
	#ifdef TIM13
		if (TIM_periph_number == 13) { __HAL_RCC_TIM13_CLK_ENABLE(); tim.Instance = TIM13;	IRQn = TIM8_UP_TIM13_IRQn; }
	#endif
	#ifdef TIM14
		if (TIM_periph_number == 14) { __HAL_RCC_TIM14_CLK_ENABLE(); tim.Instance = TIM14;	IRQn = TIM8_TRG_COM_TIM14_IRQn; }
	#endif


	if (!IRQn) return; //error: Timer number not valid

	HAL_NVIC_SetPriority(IRQn, timinit->priority1, timinit->priority2);
	HAL_NVIC_EnableIRQ(IRQn);

	tim.Init.Period 			= timinit->period;
	tim.Init.Prescaler 			= timinit->prescaler;
	tim.Init.ClockDivision 		= timinit->clock_division;
	tim.Init.CounterMode 		= TIM_COUNTERMODE_UP;
	tim.Init.RepetitionCounter 	= 0;
	tim.Init.AutoReloadPreload 	= TIM_AUTORELOAD_PRELOAD_DISABLE;

	//Start the Timer
	if (HAL_TIM_Base_Init(&tim) != HAL_OK)			_Error_Handler(__FILE__, __LINE__);

	//Enable the interrupt
	if (HAL_TIM_Base_Start_IT(&tim) != HAL_OK)		_Error_Handler(__FILE__, __LINE__);
}


void TIM1_UP_TIM10_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM10, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM10, TIM_IT_UPDATE))
		{
			if (tim_callbacks[10] != NULL) tim_callbacks[10]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM10, TIM_IT_UPDATE);
	}

	if (TIM_IT_IS_SET(TIM1, TIM_IT_UPDATE))	{
		if (TIM_IT_IS_SOURCE(TIM1, TIM_IT_UPDATE))
		{
			if (tim_callbacks[1] != NULL) tim_callbacks[1]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM1, TIM_IT_UPDATE);
	}
}

void TIM2_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM2, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM2, TIM_IT_UPDATE))
		{
			if (tim_callbacks[2] != NULL) tim_callbacks[2]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM2, TIM_IT_UPDATE);
	}
}


void TIM3_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM3, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM3, TIM_IT_UPDATE))
		{
			if (tim_callbacks[3] != NULL) tim_callbacks[3]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM3, TIM_IT_UPDATE);
	}
}


void TIM4_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM4, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM4, TIM_IT_UPDATE))
		{
			if (tim_callbacks[4] != NULL) tim_callbacks[4]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM4, TIM_IT_UPDATE);
	}
}


void TIM5_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM5, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM5, TIM_IT_UPDATE))
		{
			if (tim_callbacks[5] != NULL) tim_callbacks[5]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM5, TIM_IT_UPDATE);
	}
}


void TIM6_DAC_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM6, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM6, TIM_IT_UPDATE))
		{
			if (tim_callbacks[6] != NULL) tim_callbacks[6]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM6, TIM_IT_UPDATE);
	}
}


void TIM7_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM7, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM7, TIM_IT_UPDATE))
		{
			if (tim_callbacks[7] != NULL) tim_callbacks[7]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM7, TIM_IT_UPDATE);
	}
}


void TIM8_UP_TIM13_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM8, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM8, TIM_IT_UPDATE))
		{
			if (tim_callbacks[8] != NULL) tim_callbacks[8]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM8, TIM_IT_UPDATE);
	}

	if (TIM_IT_IS_SET(TIM13, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM13, TIM_IT_UPDATE))
		{
			if (tim_callbacks[13] != NULL) tim_callbacks[13]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM13, TIM_IT_UPDATE);
	}

}

void TIM1_BRK_TIM9_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM9, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM9, TIM_IT_UPDATE))
		{
			if (tim_callbacks[9] != NULL) tim_callbacks[9]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM9, TIM_IT_UPDATE);
	}
}


void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM11, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM11, TIM_IT_UPDATE))
		{
			if (tim_callbacks[11] != NULL) tim_callbacks[11]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM11, TIM_IT_UPDATE);
	}
}


void TIM8_BRK_TIM12_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM12, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM12, TIM_IT_UPDATE))
		{
			if (tim_callbacks[12] != NULL) tim_callbacks[12]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM12, TIM_IT_UPDATE);
	}
}

void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
	if (TIM_IT_IS_SET(TIM14, TIM_IT_UPDATE)){
		if (TIM_IT_IS_SOURCE(TIM14, TIM_IT_UPDATE))
		{
			if (tim_callbacks[14] != NULL) tim_callbacks[14]();
		}
		// Clear TIM update interrupt
		TIM_IT_CLEAR(TIM14, TIM_IT_UPDATE);
	}
}

//
// SysTick_Handler() is needed for HAL_GetTick()
//
void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}
