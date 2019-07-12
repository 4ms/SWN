/*
 * codec_sai.c
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
#include "codec_i2c.h"

//
// Codec SAI pins
//

#define CODEC_SAI						SAI2

#define CODEC_SAI_GPIO_AF				GPIO_AF10_SAI2
#define CODEC_SAI_GPIO_CLOCK_ENABLE		__HAL_RCC_GPIOD_CLK_ENABLE();__HAL_RCC_GPIOE_CLK_ENABLE

#define CODEC_SAI_MCK_GPIO				GPIOE
#define CODEC_SAI_MCK_PIN				GPIO_PIN_14

#define CODEC_SAI_GPIO_WS				GPIOE
#define CODEC_SAI_WS_PIN				GPIO_PIN_13

#define CODEC_SAI_GPIO_SCK				GPIOE
#define CODEC_SAI_SCK_PIN				GPIO_PIN_12

#define CODEC_SAI_GPIO_SDO				GPIOE
#define CODEC_SAI_SDO_PIN				GPIO_PIN_11

#define CODEC_SAI_GPIO_SDI				GPIOD 
#define CODEC_SAI_SDI_PIN				GPIO_PIN_11



#define CODEC_SAI_DMA_CLOCK_ENABLE		__HAL_RCC_DMA2_CLK_ENABLE

//SAI2 B: Master TX (S7C0 is an alt)
#define CODEC_SAI_TX_BLOCK				SAI2_Block_B
#define	CODEC_SAI_TX_DMA				DMA2
#define	CODEC_SAI_TX_DMA_ISR			LISR
#define	CODEC_SAI_TX_DMA_IFCR			LIFCR
#define CODEC_SAI_TX_DMA_STREAM			DMA2_Stream1
#define CODEC_SAI_TX_DMA_IRQn 			DMA2_Stream1_IRQn
#define CODEC_SAI_TX_DMA_IRQHandler		DMA2_Stream1_IRQHandler
#define CODEC_SAI_TX_DMA_CHANNEL		DMA_CHANNEL_10

#define CODEC_SAI_TX_DMA_FLAG_TC		DMA_FLAG_TCIF1_5
#define CODEC_SAI_TX_DMA_FLAG_HT		DMA_FLAG_HTIF1_5
#define CODEC_SAI_TX_DMA_FLAG_FE		DMA_FLAG_FEIF1_5
#define CODEC_SAI_TX_DMA_FLAG_TE		DMA_FLAG_TEIF1_5
#define CODEC_SAI_TX_DMA_FLAG_DME		DMA_FLAG_DMEIF1_5

//SAI2 A: RX
#define CODEC_SAI_RX_BLOCK				SAI2_Block_A
#define	CODEC_SAI_RX_DMA				DMA2
#define	CODEC_SAI_RX_DMA_ISR			LISR
#define	CODEC_SAI_RX_DMA_IFCR			LIFCR
#define CODEC_SAI_RX_DMA_STREAM			DMA2_Stream2
#define CODEC_SAI_RX_DMA_IRQn			DMA2_Stream2_IRQn
#define CODEC_SAI_RX_DMA_IRQHandler		DMA2_Stream2_IRQHandler
#define CODEC_SAI_RX_DMA_CHANNEL		DMA_CHANNEL_10

#define CODEC_SAI_RX_DMA_FLAG_TC		DMA_FLAG_TCIF2_6
#define CODEC_SAI_RX_DMA_FLAG_HT		DMA_FLAG_HTIF2_6
#define CODEC_SAI_RX_DMA_FLAG_FE		DMA_FLAG_FEIF2_6
#define CODEC_SAI_RX_DMA_FLAG_TE		DMA_FLAG_TEIF2_6
#define CODEC_SAI_RX_DMA_FLAG_DME		DMA_FLAG_DMEIF2_6


#define codec_BUFF_LEN 		64							/* DMA rx/tx buffer size, in number of DMA Periph/MemAlign-sized elements (words) */
#define codec_HT_LEN 		(codec_BUFF_LEN>>1) 		/* Half Transfer buffer size (both channels interleved)*/
#define codec_HT_CHAN_LEN 	(codec_HT_LEN>>1) 			/* Half Transfer buffer size per channel */
#define STEREO_BUFSZ 		codec_HT_LEN
#define MONO_BUFSZ 			codec_HT_CHAN_LEN

typedef void (*audio_callback_func_type)(int32_t *src, int32_t *dst);


enum Codec_Errors init_SAI_clock(uint32_t sample_rate);

void start_audio(void);
void stop_audio(void);
enum Codec_Errors init_audio_DMA(uint32_t sample_rate);
void set_audio_callback(audio_callback_func_type callback);
void reboot_codec(uint32_t sample_rate);
