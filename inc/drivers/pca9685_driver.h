 /*
 * pca9685_driver.h - Driver for PCA9685 16-channel LED driver chips over I2C bus
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


/* I2C peripheral configuration defines (control interface of the audio codec) */
#define LEDDRIVER_I2C                      I2C1
#define LEDDRIVER_I2C_CLK_ENABLE			__HAL_RCC_I2C1_CLK_ENABLE
#define LEDDRIVER_I2C_GPIO_CLOCK_ENABLE		__HAL_RCC_GPIOB_CLK_ENABLE
#define LEDDRIVER_I2C_GPIO_AF              GPIO_AF4_I2C1
#define LEDDRIVER_I2C_GPIO                 GPIOB
#define LEDDRIVER_I2C_SCL_PIN              GPIO_PIN_8
#define LEDDRIVER_I2C_SDA_PIN              GPIO_PIN_9

#define LEDDRIVER_I2C_DMA_CLK_ENABLE		__HAL_RCC_DMA1_CLK_ENABLE
#define LEDDRIVER_I2C_DMA					DMA1   
#define LEDDRIVER_I2C_DMA_INSTANCE_TX		DMA1_Stream6
#define LEDDRIVER_I2C_DMA_CHANNEL_TX		DMA_CHANNEL_1
#define LEDDRIVER_I2C_DMA_TX_IRQn			DMA1_Stream6_IRQn
#define LEDDRIVER_I2C_DMA_TX_IRQHandler		DMA1_Stream6_IRQHandler

#define LEDDRIVER_I2C_EV_IRQn               I2C1_EV_IRQn
#define LEDDRIVER_I2C_ER_IRQn               I2C1_ER_IRQn
#define LEDDRIVER_I2C_EV_IRQHandler         I2C1_EV_IRQHandler
#define LEDDRIVER_I2C_ER_IRQHandler         I2C1_ER_IRQHandler

#define I2C1_SPEED                        800000

#define PCA9685_MODE1 0x00 // location for Mode1 register address
#define PCA9685_MODE2 0x01 // location for Mode2 reigster address
#define PCA9685_LED0 0x06 // location for start of LED0 registers
#define PRE_SCALE_MODE 0xFE //location for setting prescale (clock speed)


#define PCA9685_I2C_BASE_ADDRESS 0b10000000

#define NUM_LEDS_PER_CHIP				16

enum LEDDriverErrors {
	LEDDRIVER_NO_ERR			= 0,
	LEDDRIVER_HAL_INIT_ERR		= 1,
	LEDDRIVER_I2C_XMIT_TIMEOUT	= 2,
	LEDDRIVER_I2C_XMIT_ERR		= 3,
	LEDDRIVER_SET_LED_ERR		= 4,
	LEDDRIVER_BAD_LED_PARAM	    = 5,
	LEDDRIVER_DMA_XMIT_ERR		= 6,
	LEDDRIVER_IT_XMIT_ERR		= 7
};

uint8_t LEDDriver_get_cur_buf(void);
uint8_t LEDDriver_get_cur_chip(void);

uint32_t 				LEDDriver_init_dma(uint8_t numdrivers, uint8_t *led_image1, uint8_t *led_image2);
uint32_t 				LEDDriver_init_direct(uint8_t numdrivers);
enum LEDDriverErrors 	LEDDriver_setRGBLED_RGB(uint8_t led_number, uint16_t c_red, uint16_t c_green, uint16_t c_blue);
enum LEDDriverErrors 	LEDDriver_set_single_LED(uint8_t led_element_number, uint16_t brightness);

uint8_t 				get_red_led_element_id(uint8_t rgb_led_id);
uint8_t 				get_chip_num(uint8_t rgb_led_id);

void 					HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c);

