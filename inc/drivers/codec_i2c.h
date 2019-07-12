/*
 * codec_i2c.c
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

//
// I2C clock speed configuration (in Hz)
//
// #define I2C_SPEED                       50000

// Uncomment defines below to select standard for audio communication between Codec and I2S peripheral
//#define I2S_STANDARD_PHILLIPS

//#define USE_DEFAULT_TIMEOUT_CALLBACK


//
// Codec RESET pin
//
// #define CODEC_RESET_RCC_ENABLE	__HAL_RCC_GPIOE_CLK_ENABLE
// #define CODEC_RESET_pin 		GPIO_PIN_15
// #define CODEC_RESET_GPIO 		GPIOE
// #define CODEC_RESET_HIGH 		CODEC_RESET_GPIO->BSRR = CODEC_RESET_pin
// #define CODEC_RESET_LOW 		CODEC_RESET_GPIO->BSRR = (uint32_t)CODEC_RESET_pin << 16

//
// I2C Config
//
#define CODEC_I2C						I2C2
#define CODEC_I2C_CLK_ENABLE			__HAL_RCC_I2C2_CLK_ENABLE
#define CODEC_I2C_GPIO_CLOCK_ENABLE		__HAL_RCC_GPIOB_CLK_ENABLE
#define CODEC_I2C_GPIO_AF				GPIO_AF4_I2C2
#define CODEC_I2C_GPIO					GPIOB
#define CODEC_I2C_SCL_PIN				GPIO_PIN_10
#define CODEC_I2C_SDA_PIN				GPIO_PIN_11


#define CODEC_FLAG_TIMEOUT             ((uint32_t)1)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300))
#define CODEC_VLONG_TIMEOUT            ((uint32_t)(1000))



enum Codec_Errors {
	CODEC_NO_ERR = 0,
	CODEC_I2C_INIT_ERR,
	CODEC_SAI_CLK_INIT_ERR,
	CODEC_SAIA_INIT_ERR,
	CODEC_SAIB_INIT_ERR,
	CODEC_SAIA_DMA_INIT_ERR,
	CODEC_SAIB_DMA_INIT_ERR,
	CODEC_SAIA_XMIT_DMA_ERR,
	CODEC_SAIB_XMIT_DMA_ERR,
	CODEC_DMA_IT_FE,
	CODEC_DMA_IT_TE,
	CODEC_DMA_IT_DME,
	CODEC_INVALID_PARAM

};


void codec_deinit(void);
uint32_t codec_power_down(void);
uint32_t codec_register_setup(uint32_t sample_rate);
void codec_GPIO_init(void);
enum Codec_Errors codec_I2C_init(void);

