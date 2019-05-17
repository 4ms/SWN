/*
 * pca9685_driver.c - Driver for PCA9685 16-channel LED driver chips over I2C bus
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

#include <stm32f7xx.h>
#include "gpio_pins.h"
#include "drivers/pca9685_driver.h"
#include "hal_handlers.h"

I2C_HandleTypeDef pwmleddriver_i2c;
uint8_t g_num_PCA_chips=0;

const uint32_t LEDDRIVER_LONG_TIMEOUT = (4000); //4000 = 500ms

//Private:
enum LED_Driver_Errors LEDDriver_writeregister(uint8_t driverAddr, uint8_t RegisterAddr, uint8_t RegisterValue);
enum LED_Driver_Errors LEDDriver_Reset(uint8_t driverAddr);


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


struct i2cTimingReg{
	uint8_t PRESC; 			//top 4 bits: (PRESC + 1) * tI2CCLK = tPRESC
							//bottom 4 bits is ignored

	uint8_t SCLDEL_SDADEL; 	//top 4 bits: SCLDEL * tPRESC = SCL Delay between SDA edge and SCL rising edge
							//bottom 4 bits: = SDADEL * tPRESC = SDA Delay between SCL falling edge and SDA edge

	uint8_t SCLH;			//SCL high period = (SCLH+1) * tPRESC
	uint8_t SCLL;			//SCL low period = (SCLL+1) * tPRESC
};
uint32_t set_i2c_timing(struct i2cTimingReg *t)
{
	return ((t->PRESC) << 24) | ((t->SCLDEL_SDADEL) << 16) | ((t->SCLH) << 8) | ((t->SCLL) << 0);
}

enum LED_Driver_Errors LEDDriver_I2C_Init(void)
{
	struct i2cTimingReg timing;
	uint32_t timrg;

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


uint32_t LEDDRIVER_TIMEOUT_UserCallback(void)
{
	/* nothing */
	return 1;
}

enum LED_Driver_Errors LEDDriver_writeregister(uint8_t driverAddr, uint8_t RegisterAddr, uint8_t RegisterValue){

	//Assemble 2-byte data 
	uint8_t data[2];
	uint32_t timeout = LEDDRIVER_LONG_TIMEOUT;

	data[0] = RegisterAddr;
	data[1] = RegisterValue;

	HAL_StatusTypeDef 	err;

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
uint8_t get_red_led_element_id(uint8_t rgb_led_id)
{
	return (rgb_led_id*3) + (rgb_led_id/5);
}


// Sets the brightness value of a single LED
// led_element_number==0 is PWM pin 0 of PCA9685 chip with address 0
// led_element_number==1 is PWM pin 1 of PCA9685 chip with address 0
// ...
// led_element_number==15 is PWM pin 15 of PCA9685 chip with address 0
// led_element_number==16 is PWM pin 0 of PCA9685 chip with address 1
// ...
// 
enum LED_Driver_Errors LEDDriver_set_single_LED(uint8_t led_element_number, uint16_t brightness){ 
	uint8_t driver_addr;
	uint8_t data[5]; //2 bytes for on time + 2 bytes for off time + 1 byte for LED address
	HAL_StatusTypeDef 	err;

	if (led_element_number < (g_num_PCA_chips*16))
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

//
// Sets color of an RGB LED
// Assumes 5 RGB LEDs per driver chip, in positions 0-14
// Example:
// led_number==0 refers to PWM pins 0,1,2 of PCA9685 chip with address 0
// led_number==1 refers to PWM pins 3,4,5 of PCA9685 chip with address 0
// ...
// led_number==5 refers to PWM pins 0,1,2 of PCA9685 chip with address 1
// ..
// led_number==9 refers to PWM pins 12,13,14 of PCA9685 chip with address 1
// ...
//
// Note: This function never changes PWM pin 15
//
enum LED_Driver_Errors LEDDriver_setRGBLED_RGB(uint8_t led_number, uint16_t c_red, uint16_t c_green, uint16_t c_blue){ 

	uint8_t driverAddr;
	uint8_t data[13]; //(3 colors * (2 bytes for on time + 2 bytes for off time)) + 1 byte for LED address
	volatile HAL_StatusTypeDef 	err;

	if (led_number < (g_num_PCA_chips*5))
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

enum LED_Driver_Errors LEDDriver_Reset(uint8_t driverAddr)
{
	enum LED_Driver_Errors err;

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

uint32_t LEDDriver_Init(uint8_t numdrivers){

	uint8_t driverAddr;
	enum LED_Driver_Errors err;

	LEDDriver_GPIO_Init();
	err = LEDDriver_I2C_Init();
	if (err) return err;

	for (driverAddr=0;driverAddr<numdrivers;driverAddr++){
		err = LEDDriver_Reset(driverAddr);
		if (err) return (err | ((driverAddr+1)<<4));
	}
	g_num_PCA_chips = numdrivers;

	return 0;
}
