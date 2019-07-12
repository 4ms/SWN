/*
 * envout_pwm.c - PWM output for the channel ENV OUT jacks
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

#include "envout_pwm.h"
#include "globals.h"
#include "params_update.h"
#include "params_lfo.h"
#include "led_cont.h"
#include "gpio_pins.h"
#include "timekeeper.h"
#include "hal_handlers.h"
#include "lfo_wavetable_bank.h"
#include "oscillator.h"
#include "ui_modes.h"

extern o_params 	params;
extern o_lfos 		lfos;
extern o_led_cont 	led_cont;
extern enum UI_Modes ui_mode;

uint32_t LFOMON;
uint32_t TRIGMON;

float voltoct_pwm_tracking = 1.0f;

// Private function:
void update_envout_pwm(void);

TIM_HandleTypeDef	timAB;
TIM_HandleTypeDef	timC;
TIM_HandleTypeDef	timDEF;


void init_envout_pwm(void)
{

	uint8_t 			i;
	GPIO_InitTypeDef 	gpio;
	TIM_OC_InitTypeDef	tim_oc;

	//Initialize the values
	for (i=0;i<NUM_CHANNELS;i++)		lfos.envout_pwm[i]=0;


	ENVOUT_PWM_RCC_ENABLE();

	//
	// Setup GPIO for timer output pins
	//
	ENVOUT_PWM_TIM_GPIO_RCC_ENABLE();

	gpio.Mode 	= GPIO_MODE_AF_PP;
	gpio.Pull 	= GPIO_PULLUP;
	gpio.Speed 	= GPIO_SPEED_FREQ_HIGH;

	//Jacks A and B
	gpio.Alternate 	= ENVOUT_PWM_TIM_AB_AF;
	gpio.Pin 		= ENVOUT_PWM_pins_AB;
	HAL_GPIO_Init(ENVOUT_PWM_GPIO_AB, &gpio);

	//Jack C
	gpio.Alternate 	= ENVOUT_PWM_TIM_C_AF;
	gpio.Pin 		= ENVOUT_PWM_pins_C;
	HAL_GPIO_Init(ENVOUT_PWM_GPIO_C, &gpio);

	//Jacks D, E, and F
	gpio.Alternate 	= ENVOUT_PWM_TIM_DEF_AF;
	gpio.Pin 		= ENVOUT_PWM_pins_DEF;
	HAL_GPIO_Init(ENVOUT_PWM_GPIO_DEF, &gpio);

	// Initialize the Timer peripherals (period determines resolution and frequency)

	//This timer runs 2x as fast as the other two, because it's on the APB1 bus
	timAB.Instance 				 	= ENVOUT_PWM_TIM_AB;
	timAB.Init.Prescaler         	= 0;
	timAB.Init.Period            	= PWM_MAX; //216M / 1 / 256 = 420kHz;
	timAB.Init.ClockDivision     	= 0;
	timAB.Init.CounterMode       	= TIM_COUNTERMODE_UP;
	timAB.Init.RepetitionCounter 	= 0;
	timAB.Init.AutoReloadPreload 	= TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&timAB) != HAL_OK) _Error_Handler(__FILE__, __LINE__);


	timC.Instance 				 	= ENVOUT_PWM_TIM_C;
	timC.Init.Prescaler         	= 0;
	timC.Init.Period            	= PWM_MAX; //216M / 2 / 256 = 210kHz;
	timC.Init.ClockDivision     	= 0;
	timC.Init.CounterMode       	= TIM_COUNTERMODE_UP;
	timC.Init.RepetitionCounter 	= 0;
	timC.Init.AutoReloadPreload 	= TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&timC) != HAL_OK) _Error_Handler(__FILE__, __LINE__);

	timDEF.Instance 				= ENVOUT_PWM_TIM_DEF;
	timDEF.Init.Prescaler         	= 0;
	timDEF.Init.Period            	= PWM_MAX; //216M / 2 / 256 = 210kHz;
	timDEF.Init.ClockDivision     	= 0;
	timDEF.Init.CounterMode       	= TIM_COUNTERMODE_UP;
	timDEF.Init.RepetitionCounter 	= 0;
	timDEF.Init.AutoReloadPreload 	= TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&timDEF) != HAL_OK) _Error_Handler(__FILE__, __LINE__);


	// Configure each TIMx peripheral's Output Compare units.
	// Each channel (CCRx) needs to be enabled for each TIMx that we're using

	//Common configuration for all channels
	tim_oc.OCMode       = TIM_OCMODE_PWM1;
	tim_oc.OCPolarity   = TIM_OCPOLARITY_HIGH;
	tim_oc.OCFastMode   = TIM_OCFAST_DISABLE;
	tim_oc.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	tim_oc.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	tim_oc.OCIdleState  = TIM_OCIDLESTATE_RESET;
	tim_oc.Pulse 		= 0;

	if (HAL_TIM_PWM_ConfigChannel(&timAB, &tim_oc, 	ENVOUT_PWM_CHAN_A) != HAL_OK)		_Error_Handler(__FILE__, __LINE__);
	if (HAL_TIM_PWM_ConfigChannel(&timAB, &tim_oc, 	ENVOUT_PWM_CHAN_B) != HAL_OK)		_Error_Handler(__FILE__, __LINE__);
	if (HAL_TIM_PWM_ConfigChannel(&timC,  &tim_oc, 	ENVOUT_PWM_CHAN_C) != HAL_OK)		_Error_Handler(__FILE__, __LINE__);
	if (HAL_TIM_PWM_ConfigChannel(&timDEF, &tim_oc, ENVOUT_PWM_CHAN_D) != HAL_OK)		_Error_Handler(__FILE__, __LINE__);
	if (HAL_TIM_PWM_ConfigChannel(&timDEF, &tim_oc, ENVOUT_PWM_CHAN_E) != HAL_OK)		_Error_Handler(__FILE__, __LINE__);
	if (HAL_TIM_PWM_ConfigChannel(&timDEF, &tim_oc, ENVOUT_PWM_CHAN_F) != HAL_OK)		_Error_Handler(__FILE__, __LINE__);

	//
	// Start PWM signals generation
	//
 	if (HAL_TIM_PWM_Start(&timAB, 	ENVOUT_PWM_CHAN_A) != HAL_OK)						_Error_Handler(__FILE__, __LINE__);
 	if (HAL_TIM_PWM_Start(&timAB, 	ENVOUT_PWM_CHAN_B) != HAL_OK)						_Error_Handler(__FILE__, __LINE__);
 	if (HAL_TIM_PWM_Start(&timC, 	ENVOUT_PWM_CHAN_C) != HAL_OK)						_Error_Handler(__FILE__, __LINE__);
 	if (HAL_TIM_PWM_Start(&timDEF, 	ENVOUT_PWM_CHAN_D) != HAL_OK)						_Error_Handler(__FILE__, __LINE__);
 	if (HAL_TIM_PWM_Start(&timDEF, 	ENVOUT_PWM_CHAN_E) != HAL_OK)						_Error_Handler(__FILE__, __LINE__);
 	if (HAL_TIM_PWM_Start(&timDEF, 	ENVOUT_PWM_CHAN_F) != HAL_OK)						_Error_Handler(__FILE__, __LINE__);

}

	

void start_envout_pwm(void)
{
	start_timer_IRQ(PWM_OUTS_TIM_number, &update_envout_pwm);
}


void update_envout_pwm(void){

	uint8_t j;
	uint32_t envout_buf;
	static float vca_trig[NUM_CHANNELS] = {1.0};
	const uint8_t GATE_THRESHOLD = 127;

	update_lfos();

	for (j=0;j<NUM_CHANNELS;j++)
	{		
		envout_buf = lfos.preload[j];

		if (ui_mode == WTRECORDING && j>=3)
		{ 						
			//Do not apply gain or LFO mode to trigger outputs for WT Rec Sync
			lfos.envout_pwm[j] = envout_buf; 													
			lfos.out_lpf[j]  = (float)(lfos.envout_pwm[j]) / (float)(PWM_MAX);
		}
		else if (lfos.mode[j] == lfot_GATE)
		{

			lfos.envout_pwm[j] = envout_buf > GATE_THRESHOLD ? PWM_MAX : 0;	

			lfos.out_lpf[j] = (float)(lfos.envout_pwm[j]) / (float)(PWM_MAX);
			lfos.envout_pwm[j] *= lfos.gain[j];
		}
				
		else if (lfos.mode[j] == lfot_TRIG)
		{
			if (envout_buf <= PWM_MAX/2)
				lfos.trig_armed[j]=1;
			
			else if ((envout_buf > PWM_MAX/2) && lfos.trig_armed[j])
			{
				lfos.trigout[j] = 1;
				vca_trig[j] = F_TRIGVCADUR;
				
				lfos.trig_armed[j]++;
				if (lfos.trig_armed[j] == TRIG_DURATION)
					lfos.trig_armed[j] = 0;
			}

			else{
				lfos.trigout[j]=0;
			}
									
			if(vca_trig[j]>0) 	vca_trig[j]--;
			else				vca_trig[j]=0;
						
			lfos.out_lpf[j] *= (1-LFO_TRIG_LPF);
			lfos.out_lpf[j] += LFO_TRIG_LPF * vca_trig[j] / F_TRIGVCADUR;			
							
			lfos.envout_pwm[j] = lfos.trigout[j] * PWM_MAX * lfos.gain[j];
		} 

		else //lfos.mode[j] == lfot_SHAPE
		{
			lfos.envout_pwm[j] = envout_buf;
			lfos.out_lpf[j]  = (float)(lfos.envout_pwm[j]) / (float)(PWM_MAX);
			lfos.envout_pwm[j] *= lfos.gain[j];
		}
	}

	//ENVOUTs A,B are higher res (12 bits)
	ENVOUT_PWM_TIM_AB->ENVOUT_PWM_CC_A 	= lfos.envout_pwm[0];
	ENVOUT_PWM_TIM_AB->ENVOUT_PWM_CC_B 	= lfos.envout_pwm[1];
	ENVOUT_PWM_TIM_C->ENVOUT_PWM_CC_C 	= lfos.envout_pwm[2];
	ENVOUT_PWM_TIM_DEF->ENVOUT_PWM_CC_D = lfos.envout_pwm[3];
	ENVOUT_PWM_TIM_DEF->ENVOUT_PWM_CC_E = lfos.envout_pwm[4];
	ENVOUT_PWM_TIM_DEF->ENVOUT_PWM_CC_F = lfos.envout_pwm[5];
}
