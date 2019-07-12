/*
 * pca9685_driver.c - Driver for PCA9685 16-channel LED driver chips over I2C bus
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

#include <stm32f7xx.h>
#include "gpio_pins.h"
#include "drivers/pca9685_driver.h"
#include "hal_handlers.h"
#include "i2c_util.h"

I2C_HandleTypeDef pwmleddriver_i2c;
DMA_HandleTypeDef pwmleddriver_dmatx;

const uint32_t LEDDRIVER_LONG_TIMEOUT = (4000); //4000 = 500ms
uint8_t g_num_driver_chips;
uint8_t g_cur_chip_num = 0;
uint8_t *leddriver_buffer_start[2];
uint8_t leddriver_cur_buf = 0;
uint8_t *leddriver_buffer;
enum LEDDriverErrors g_led_error;

//Private:
enum LEDDriverErrors LEDDriver_writeregister(uint8_t driverAddr, uint8_t RegisterAddr, uint8_t RegisterValue);
enum LEDDriverErrors LEDDriver_reset_chip(uint8_t driverAddr);
void LEDDriver_GPIO_Init(void);
enum LEDDriverErrors LEDDriver_I2C_Init(void);
enum LEDDriverErrors LEDDriver_I2C_DMA_Init();
enum LEDDriverErrors LEDDriver_I2C_IT_Init();
void LED_driver_tx_complete(DMA_HandleTypeDef *_hdma);

//Todo: Remove led_image2 and associated double-buffering stuff after it's 100% confirmed we 
//don't need double-buffering
uint32_t LEDDriver_init_dma(uint8_t numdrivers, uint8_t *led_image1, uint8_t *led_image2)
{
	uint8_t driverAddr;
	enum LEDDriverErrors err;

	g_num_driver_chips = numdrivers;
	g_cur_chip_num = 0;
	leddriver_buffer_start[0] = led_image1;
	leddriver_buffer_start[1] = led_image2;
	leddriver_cur_buf = 0;
	leddriver_buffer = leddriver_buffer_start[leddriver_cur_buf];

	LEDDriver_GPIO_Init();
	err = LEDDriver_I2C_Init();
	if (err) return err;

	for (driverAddr=0; driverAddr<g_num_driver_chips; driverAddr++){
		err = LEDDriver_reset_chip(driverAddr);
		if (err) return (err | ((driverAddr+1)<<4));
	}

	err = LEDDriver_I2C_Init();
	err = LEDDriver_I2C_DMA_Init();

	return err;
}

uint32_t LEDDriver_init_direct(uint8_t numdrivers)
{
	uint8_t driverAddr;
	enum LEDDriverErrors err;

	g_num_driver_chips = numdrivers;

	LEDDriver_GPIO_Init();
	err = LEDDriver_I2C_Init();
	if (err) return err;

	for (driverAddr=0; driverAddr<g_num_driver_chips; driverAddr++){
		err = LEDDriver_reset_chip(driverAddr);
		if (err) return (err | ((driverAddr+1)<<4));
	}

	return 0;
}

enum LEDDriverErrors LEDDriver_reset_chip(uint8_t driverAddr)
{
	enum LEDDriverErrors err;

	err = LEDDriver_writeregister(driverAddr, PCA9685_MODE1, 0b00000000); // clear sleep mode
	if (err) return err;
	HAL_Delay(1); //was 20 nop's

	err = LEDDriver_writeregister(driverAddr, PCA9685_MODE1, 0b10000000); //start reset mode
	if (err) return err;
	HAL_Delay(1); //was 20 nop's

	err = LEDDriver_writeregister(driverAddr, PCA9685_MODE1, 0b00100000);	//auto increment
	if (err) return err;
	err = LEDDriver_writeregister(driverAddr, PCA9685_MODE2, 0b00010001);	// INVERT=1, OUTDRV=0, OUTNE=01
	if (err) return err;

	return LEDDRIVER_NO_ERR;
}


// Sets the brightness value of a single LED
// led_element_number==0 is PWM pin 0 of PCA9685 chip with address 0
// led_element_number==1 is PWM pin 1 of PCA9685 chip with address 0
// ...
// led_element_number==15 is PWM pin 15 of PCA9685 chip with address 0
// led_element_number==16 is PWM pin 0 of PCA9685 chip with address 1
// ...
// 
enum LEDDriverErrors LEDDriver_set_single_LED(uint8_t led_element_number, uint16_t brightness)
{ 
	uint8_t driver_addr;
	uint8_t data[5]; //2 bytes for on time + 2 bytes for off time + 1 byte for LED address
	HAL_StatusTypeDef err;

	if (led_element_number < (g_num_driver_chips*16))
	{
		driver_addr = (led_element_number/16);
		led_element_number = led_element_number - (driver_addr * 16);

		driver_addr = PCA9685_I2C_BASE_ADDRESS | (driver_addr << 1);

		data[0] = PCA9685_LED0 + (led_element_number*4); 	//4 registers per LED element
		data[1] = 0; 										//on-time = 0
		data[2] = 0;
		data[3] = brightness & 0xFF; 						//off-time = brightness
		data[4] = brightness >> 8;

		while((err = HAL_I2C_Master_Transmit(&pwmleddriver_i2c, driver_addr, data, 5, LEDDRIVER_LONG_TIMEOUT)) != HAL_OK)
		{
			if (HAL_I2C_GetError(&pwmleddriver_i2c) != HAL_I2C_ERROR_AF)
				return LEDDRIVER_SET_LED_ERR;
		}
	}
	else
		return LEDDRIVER_BAD_LED_PARAM;
	
	return LEDDRIVER_NO_ERR;

}

// Sets color of an RGB LED
// Assumes 5 RGB LEDs per driver chip, in positions 0-14
// Example:
// led_number==0 refers to PWM pins 0,1,2 of PCA9685 chip with address 0
// led_number==1 refers to PWM pins 3,4,5 of PCA9685 chip with address 0
// ...
// led_number==5 refers to PWM pins 0,1,2 of PCA9685 chip with address 1
// ..
// led_number==9 refers to PWM pins 12,13,14 of PCA9685 chip with address 1
//
// Note: This function never changes PWM pin 15
//
enum LEDDriverErrors LEDDriver_setRGBLED_RGB(uint8_t led_number, uint16_t c_red, uint16_t c_green, uint16_t c_blue){ 

	uint8_t driverAddr;
	uint8_t data[13]; //(3 colors * (2 bytes for on time + 2 bytes for off time)) + 1 byte for LED address
	volatile HAL_StatusTypeDef 	err;

	if (led_number < (g_num_driver_chips*5))
	{
		driverAddr = (led_number/5);
		led_number = led_number - (driverAddr * 5);

		driverAddr = PCA9685_I2C_BASE_ADDRESS | (driverAddr << 1);

		data[0] = PCA9685_LED0 + (led_number*12); //12 registers per LED (4 registers per LED element) = 12*16 registers per driver
		data[1] = 0; 								//on-time = 0
		data[2] = 0;
		data[3] = c_red & 0xFF; 					//off-time = brightness
		data[4] = c_red >> 8;

		data[5] = 0; 								//on-time = 0 
		data[6] = 0;
		data[7] = c_green & 0xFF; 					//off-time = brightness
		data[8] = c_green >> 8;

		data[9] = 0; 								//on-time = 0
		data[10] = 0;
		data[11] = c_blue & 0xFF; 					//off-time = brightness
		data[12] = c_blue >> 8;

		while((err = HAL_I2C_Master_Transmit(&pwmleddriver_i2c, driverAddr, data, 13, LEDDRIVER_LONG_TIMEOUT)) != HAL_OK)
		{
			if (HAL_I2C_GetError(&pwmleddriver_i2c) != HAL_I2C_ERROR_AF)
				return LEDDRIVER_SET_LED_ERR;
		}
	} 
	else
		return LEDDRIVER_BAD_LED_PARAM;

	return LEDDRIVER_NO_ERR;
}

enum LEDDriverErrors LEDDriver_writeregister(uint8_t driverAddr, uint8_t RegisterAddr, uint8_t RegisterValue)
{

	//Assemble 2-byte data 
	uint8_t data[2];
	uint32_t timeout = LEDDRIVER_LONG_TIMEOUT;
	HAL_StatusTypeDef err;

	data[0] = RegisterAddr;
	data[1] = RegisterValue;

	driverAddr = PCA9685_I2C_BASE_ADDRESS | (driverAddr << 1);

	while(timeout-- && ((err = HAL_I2C_Master_Transmit(&pwmleddriver_i2c, driverAddr, data, 2, LEDDRIVER_LONG_TIMEOUT)) != HAL_OK) )		
	{
		if (HAL_I2C_GetError(&pwmleddriver_i2c) != HAL_I2C_ERROR_AF)
			return LEDDRIVER_I2C_XMIT_ERR;
	}

	//possible values for err are HAL_ERROR, HAL_BUSY, HAL_TIMEOUT, HAL_OK
	return (err==HAL_OK) ? LEDDRIVER_NO_ERR : LEDDRIVER_I2C_XMIT_TIMEOUT;
}

//returns led element number of the red element of the given RGB LED id (green is red + 1, blue = red + 2)
uint8_t get_red_led_element_id(uint8_t rgb_led_id) {
	return (rgb_led_id*3) + (rgb_led_id/5);
}
uint8_t get_chip_num(uint8_t rgb_led_id){
	return (rgb_led_id/5);
}

uint8_t LEDDriver_get_cur_buf(void) { return leddriver_cur_buf; }
uint8_t LEDDriver_get_cur_chip(void) { return g_cur_chip_num; }


void LEDDriver_GPIO_Init(void)
{
	GPIO_InitTypeDef gpio;

	LEDDRIVER_I2C_GPIO_CLOCK_ENABLE();
	LEDDRIVER_I2C_CLK_ENABLE();

	//I2C pins SDA SCL
	gpio.Mode 		= GPIO_MODE_AF_OD;
	gpio.Pull 		= GPIO_PULLUP;
	gpio.Speed 		= GPIO_SPEED_FREQ_VERY_HIGH;
	gpio.Alternate 	= LEDDRIVER_I2C_GPIO_AF;
	gpio.Pin 		= LEDDRIVER_I2C_SCL_PIN | LEDDRIVER_I2C_SDA_PIN;
	HAL_GPIO_Init(LEDDRIVER_I2C_GPIO, &gpio);
}

enum LEDDriverErrors LEDDriver_I2C_Init(void)
{
	struct i2cTimingReg timing;
	uint32_t timrg;

	HAL_I2C_DeInit(&pwmleddriver_i2c);

	timing.PRESC = (1<<4);	// tPRESC = 27MHz
	timing.SCLH = 17;		// 27MHz / 34 = 794kHz
	timing.SCLL = 17;		// 34 = 17 + 17
	timing.SCLDEL_SDADEL = 0b00010001;
	timrg = set_i2c_timing(&timing);
	pwmleddriver_i2c.Init.Timing = timrg;

	pwmleddriver_i2c.Instance = LEDDRIVER_I2C;
	pwmleddriver_i2c.Init.OwnAddress1		 	= 0x34;
	pwmleddriver_i2c.Init.AddressingMode 		= I2C_ADDRESSINGMODE_7BIT;
	pwmleddriver_i2c.Init.DualAddressMode 		= I2C_DUALADDRESS_DISABLE;
	pwmleddriver_i2c.Init.OwnAddress2 			= 0;
	pwmleddriver_i2c.Init.OwnAddress2Masks		= I2C_OA2_NOMASK;
	pwmleddriver_i2c.Init.GeneralCallMode 		= I2C_GENERALCALL_DISABLE;
	pwmleddriver_i2c.Init.NoStretchMode 		= I2C_NOSTRETCH_DISABLE;

	if (HAL_I2C_Init(&pwmleddriver_i2c) != HAL_OK)					
		return LEDDRIVER_HAL_INIT_ERR;

	return LEDDRIVER_NO_ERR;
}


enum LEDDriverErrors LEDDriver_I2C_DMA_Init(void)
{
	HAL_StatusTypeDef err;

	LEDDRIVER_I2C_DMA_CLK_ENABLE();

	pwmleddriver_dmatx.Instance                 = LEDDRIVER_I2C_DMA_INSTANCE_TX;
	pwmleddriver_dmatx.Init.Channel             = LEDDRIVER_I2C_DMA_CHANNEL_TX;               
	pwmleddriver_dmatx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	pwmleddriver_dmatx.Init.PeriphInc           = DMA_PINC_DISABLE;
	pwmleddriver_dmatx.Init.MemInc              = DMA_MINC_ENABLE;
	pwmleddriver_dmatx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	pwmleddriver_dmatx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	pwmleddriver_dmatx.Init.Mode                = DMA_NORMAL; //DMA_CIRCULAR
	pwmleddriver_dmatx.Init.Priority            = DMA_PRIORITY_LOW;
	pwmleddriver_dmatx.Init.FIFOMode 			= DMA_FIFOMODE_DISABLE;
	pwmleddriver_dmatx.Init.FIFOThreshold 		= DMA_FIFO_THRESHOLD_FULL;
	pwmleddriver_dmatx.Init.MemBurst 			= DMA_MBURST_SINGLE;
	pwmleddriver_dmatx.Init.PeriphBurst 		= DMA_PBURST_SINGLE;

	HAL_DMA_DeInit(&pwmleddriver_dmatx);
	HAL_DMA_Init(&pwmleddriver_dmatx);   

	__HAL_LINKDMA(&pwmleddriver_i2c, hdmatx, pwmleddriver_dmatx);

	HAL_NVIC_SetPriority(LEDDRIVER_I2C_DMA_TX_IRQn, 3, 1);
	HAL_NVIC_EnableIRQ(LEDDRIVER_I2C_DMA_TX_IRQn);

	HAL_NVIC_SetPriority(LEDDRIVER_I2C_ER_IRQn, 3, 2);
	HAL_NVIC_EnableIRQ(LEDDRIVER_I2C_ER_IRQn);
	HAL_NVIC_SetPriority(LEDDRIVER_I2C_EV_IRQn, 0, 3);
	HAL_NVIC_EnableIRQ(LEDDRIVER_I2C_EV_IRQn);

	err = HAL_I2C_Mem_Write_DMA(&pwmleddriver_i2c, PCA9685_I2C_BASE_ADDRESS, PCA9685_LED0, I2C_MEMADD_SIZE_8BIT, leddriver_buffer, NUM_LEDS_PER_CHIP*4);
	if (err != HAL_OK)
		return LEDDRIVER_DMA_XMIT_ERR;

    return LEDDRIVER_NO_ERR;
}


void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	HAL_StatusTypeDef err;

	uint16_t driver_addr;
	if (++g_cur_chip_num >= g_num_driver_chips)
	{
		g_cur_chip_num = 0;
		//leddriver_cur_buf = 1-leddriver_cur_buf;
	}

	leddriver_buffer = leddriver_buffer_start[leddriver_cur_buf] + g_cur_chip_num*(NUM_LEDS_PER_CHIP*4);

	driver_addr = PCA9685_I2C_BASE_ADDRESS | (g_cur_chip_num << 1);

	err = HAL_I2C_Mem_Write_DMA(&pwmleddriver_i2c, driver_addr, PCA9685_LED0, I2C_MEMADD_SIZE_8BIT, leddriver_buffer, NUM_LEDS_PER_CHIP*4);
	if (err != HAL_OK)
  		g_led_error = LEDDRIVER_DMA_XMIT_ERR;

}

void LEDDRIVER_I2C_DMA_TX_IRQHandler()
{
	HAL_DMA_IRQHandler(pwmleddriver_i2c.hdmatx);
}
void I2C1_EV_IRQHandler(void)
{
	HAL_I2C_EV_IRQHandler(&pwmleddriver_i2c);
}
void I2C1_ER_IRQHandler(void)
{
	HAL_I2C_ER_IRQHandler(&pwmleddriver_i2c);
}
