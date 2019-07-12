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

#include "globals.h"
#include "hal_handlers.h"
#include "drivers/codec_i2c.h"
#include "drivers/codec_sai.h"

I2C_HandleTypeDef codec_i2c2;


#define CODEC_IS_SLAVE 0
#define CODEC_IS_MASTER 1

#define MCLK_SRC_STM 0
#define MCLK_SRC_EXTERNAL 1

#define W8731_ADDR_0 0x1A
#define W8731_ADDR_1 0x1B
#define W8731_NUM_REGS 10


//Registers:
#define WM8731_REG_INBOTH 		8
#define WM8731_REG_INMUTE 		7
#define WM8731_REG_POWERDOWN 	6
#define WM8731_REG_INVOL 		0

#define WM8731_REG_SAMPLE_CTRL	0x08
#define WM8731_REG_RESET 		0x0F

#define VOL_p12dB	0b11111 /*+12dB*/
#define VOL_0dB		0b10111 /*0dB*/
#define VOL_n1dB	0b10110 /*-1.5dB*/
#define VOL_n3dB	0b10101 /*-3dB*/
#define VOL_n6dB	0b10011 /*-6dB*/
#define VOL_n12dB	15 		/*-12dB*/
#define VOL_n15dB	13 		/*-15dB*/
/*1.5dB steps down to..*/
#define VOL_n34dB	0b00000 /*-34.5dB*/

//Register 4: Analogue Audio Path Control
#define MICBOOST 		(1 << 0)	/* Boost Mic level */
#define MUTEMIC			(1 << 1)	/* Mute Mic to ADC */
#define INSEL_mic		(1 << 2)	/* Mic Select*/
#define INSEL_line		(0 << 2)	/* LineIn Select*/
#define BYPASS			(1 << 3)	/* Bypass Enable */
#define DACSEL			(1 << 4)	/* Select DAC */
#define SIDETONE		(1 << 5)	/* Enable Sidetone */
#define SIDEATT_neg15dB	(0b11 << 6)
#define SIDEATT_neg12dB	(0b10 << 6)
#define SIDEATT_neg9dB	(0b01 << 6)
#define SIDEATT_neg6dB	(0b00 << 6)


//Register 5: Digital Audio Path Control
#define ADCHPFDisable 1 				/* ADC High Pass Filter */
#define ADCHPFEnable 0
#define DEEMPH_48k		(0b11 << 1) 	/* De-emphasis Control */
#define DEEMPH_44k		(0b10 << 1)
#define DEEMPH_32k 		(0b01 << 1)
#define DEEMPH_disable	(0b00 << 1)
#define DACMU_enable	(1 << 3) 		/* DAC Soft Mute Control */
#define DACMU_disable	(0 << 3)
#define HPOR_store		(1 << 4) 		/* Store DC offset when HPF disabled */
#define HPOR_clear		(0 << 4)

//Register 6: Power Down Control: 1= enable power down, 0=disable power down
#define PD_LINEIN		(1<<0)
#define PD_MIC			(1<<1)
#define PD_ADC			(1<<2)
#define PD_DAC			(1<<3)
#define PD_OUT			(1<<4)
#define PD_OSC			(1<<5)
#define PD_CLKOUT		(1<<6)
#define PD_POWEROFF		(1<<7)


//Register 7: Digital Audio Interface Format
#define format_MSB_Right 0
#define format_MSB_Left 1
#define format_I2S 2
#define format_DSP 3
#define format_16b (0<<2)
#define format_20b (1<<2)
#define format_24b (2<<2)
#define format_32b (3<<2)


//Register: Sample Rate Controls
#define	SR_USB_NORM	(1<<0)		//1=USB (250/272fs), 0=Normal Mode (256/384fs)
#define SR_BOSR		(1<<1)		//Base Over-Sampling Rate: 0=250/256fs, 1=272/384fs (also 128/192)
#define SR_NORM_8K 	(0b1011 << 2)
#define SR_NORM_32K (0b0110 << 2)
#define SR_NORM_44K (0b1000 << 2)
#define	SR_NORM_48K	(0b0000 << 2)
#define SR_NORM_88K (0b1111 << 2)
#define SR_NORM_96K (0b0111 << 2)

// Oddness:
// format_I2S does not work with I2S2 on the STM32F427Z (works on the 427V) in Master TX mode (I2S2ext is RX)
// The RX data is shifted left 2 bits (x4) as it comes in, causing digital wrap-around clipping.
// Using format_MSB_Left works (I2S periph has to be set up I2S_Standard_LSB or I2S_Standard_MSB).
// Also, format_MSB_Right does not seem to work at all (with the I2S set to LSB or MSB)

uint16_t codec_init_data[] =
{
	VOL_0dB,			// Reg 00: Left Line In

	VOL_0dB,			// Reg 01: Right Line In

	0b0101111,			// Reg 02: Left Headphone out (Mute)

	0b0101111,			// Reg 03: Right Headphone out (Mute)

	(MUTEMIC 			// Reg 04: Analog Audio Path Control (maximum attenuation on sidetone, sidetone disabled, DAC selected, Mute Mic, no bypass)
	| INSEL_line
	| DACSEL
	| SIDEATT_neg6dB),

	(DEEMPH_disable			// Reg 05: Digital Audio Path Control: HPF, De-emp at 48kHz on DAC, do not soft mute dac
	| ADCHPFEnable),

	(PD_MIC
	| PD_OSC
	| PD_CLKOUT),		// Reg 06: Power Down Control (Clkout, Osc, Mic Off) 0x062

	(format_24b			// Reg 07: Digital Audio Interface Format (24-bit, slave)
	| format_I2S),

	0x000,				// Reg 08: Sampling Control (USB_NORM=Normal, BOSR=256x, default = 48k)

	0x001				// Reg 09: Active Control
};


//Set configuration here:
#define CODEC_MODE 				CODEC_IS_SLAVE
#define CODEC_MCLK_SRC 			MCLK_SRC_STM
#define CODEC_ADDRESS           (W8731_ADDR_0<<1)




//Private
uint32_t codec_reset(uint8_t master_slave, uint32_t sample_rate);
uint32_t codec_write_register(uint8_t RegisterAddr, uint16_t RegisterValue);

void codec_deinit(void)
{
	__HAL_RCC_I2C2_CLK_DISABLE();
}

uint32_t codec_power_down(void)
{
	uint32_t err=0;

	err=codec_write_register(WM8731_REG_POWERDOWN, 0xFF); //Power Down enable all
	return err;

}


uint32_t codec_register_setup(uint32_t sample_rate)
{
	uint32_t err = 0;

	err+=codec_reset(CODEC_MODE, sample_rate);

	return err;
}


uint32_t codec_reset(uint8_t master_slave, uint32_t sample_rate)
{
	uint8_t i;
	uint32_t err=0;
	

	err = codec_write_register(WM8731_REG_RESET, 0);
	

	// if (master_slave == CODEC_IS_MASTER)	codec_init_data_base[0] |= MASTER;
	// else									codec_init_data_base[0] |= SLAVE;

	if (sample_rate==48000)					codec_init_data[WM8731_REG_SAMPLE_CTRL] |= SR_NORM_48K;
	if (sample_rate==44100)					codec_init_data[WM8731_REG_SAMPLE_CTRL] |= SR_NORM_44K;
	if (sample_rate==32000)					codec_init_data[WM8731_REG_SAMPLE_CTRL] |= SR_NORM_32K;
	if (sample_rate==88200)					codec_init_data[WM8731_REG_SAMPLE_CTRL] |= SR_NORM_88K;
	if (sample_rate==96000)					codec_init_data[WM8731_REG_SAMPLE_CTRL] |= SR_NORM_96K;
	if (sample_rate==8000)					codec_init_data[WM8731_REG_SAMPLE_CTRL] |= SR_NORM_8K;


	for(i=0;i<W8731_NUM_REGS;i++)			err+=codec_write_register(i, codec_init_data[i]);

	return err;
}


uint32_t codec_write_register(uint8_t RegisterAddr, uint16_t RegisterValue)
{	
	//Assemble 2-byte data 
	uint8_t Byte1 = ((RegisterAddr<<1)&0xFE) | ((RegisterValue>>8)&0x01);
	uint8_t Byte2 = RegisterValue&0xFF;
	uint8_t data[2];
	
	data[0] = Byte1;
	data[1] = Byte2;

	HAL_StatusTypeDef 	err;

	while((err = HAL_I2C_Master_Transmit(&codec_i2c2, CODEC_ADDRESS, data, 2, CODEC_VLONG_TIMEOUT)) != HAL_OK)
	{
		if (HAL_I2C_GetError(&codec_i2c2) != HAL_I2C_ERROR_AF)
			return 2;
	}

	if (err==HAL_OK) 	return 0;
	else				return 1;
}



void codec_GPIO_init(void)
{
	GPIO_InitTypeDef gpio;

	CODEC_I2C_GPIO_CLOCK_ENABLE();
	CODEC_SAI_GPIO_CLOCK_ENABLE();
	CODEC_I2C_CLK_ENABLE();


	//I2C pins SDA SCL
	gpio.Mode 		= GPIO_MODE_AF_OD;
	gpio.Pull 		= GPIO_PULLUP;//NOPULL;
	gpio.Speed 		= GPIO_SPEED_FREQ_HIGH;
	gpio.Alternate 	= CODEC_I2C_GPIO_AF;
	gpio.Pin 		= CODEC_I2C_SCL_PIN | CODEC_I2C_SDA_PIN;
	HAL_GPIO_Init(CODEC_I2C_GPIO, &gpio);

	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio.Alternate 	= CODEC_SAI_GPIO_AF;

	gpio.Pin = CODEC_SAI_WS_PIN;	HAL_GPIO_Init(CODEC_SAI_GPIO_WS, &gpio);
	gpio.Pin = CODEC_SAI_SDI_PIN;	HAL_GPIO_Init(CODEC_SAI_GPIO_SDI, &gpio);
	gpio.Pin = CODEC_SAI_SCK_PIN;	HAL_GPIO_Init(CODEC_SAI_GPIO_SCK, &gpio);
	gpio.Pin = CODEC_SAI_SDO_PIN;	HAL_GPIO_Init(CODEC_SAI_GPIO_SDO, &gpio);

	if (CODEC_MCLK_SRC==MCLK_SRC_STM){

		gpio.Mode = GPIO_MODE_AF_PP;
		gpio.Pull = GPIO_NOPULL;
		gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		gpio.Alternate 	= CODEC_SAI_GPIO_AF;
		gpio.Pin = CODEC_SAI_MCK_PIN;
		HAL_GPIO_Init(CODEC_SAI_MCK_GPIO, &gpio);

	} else if (CODEC_MCLK_SRC==MCLK_SRC_EXTERNAL){

		gpio.Mode = GPIO_MODE_INPUT;
		gpio.Pull = GPIO_NOPULL;
		gpio.Pin = CODEC_SAI_MCK_PIN;
		HAL_GPIO_Init(CODEC_SAI_MCK_GPIO, &gpio);
	}

}

enum Codec_Errors codec_I2C_init(void)
{

	codec_i2c2.Instance = I2C2;

	codec_i2c2.Init.Timing 				= 0x20445757;
	codec_i2c2.Init.OwnAddress1		 	= 0x33;
	codec_i2c2.Init.AddressingMode 		= I2C_ADDRESSINGMODE_7BIT;
	codec_i2c2.Init.DualAddressMode 	= I2C_DUALADDRESS_DISABLE;
	codec_i2c2.Init.OwnAddress2 		= 0;
	codec_i2c2.Init.OwnAddress2Masks	= I2C_OA2_NOMASK;
	codec_i2c2.Init.GeneralCallMode 	= I2C_GENERALCALL_DISABLE;
	codec_i2c2.Init.NoStretchMode 		= I2C_NOSTRETCH_DISABLE;

	if (HAL_I2C_Init(&codec_i2c2) != HAL_OK)
		return CODEC_I2C_INIT_ERR;
	else
		return	CODEC_NO_ERR;

	//if (HAL_I2CEx_ConfigAnalogFilter(&codec_i2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)	_Error_Handler(__FILE__, __LINE__);
	//if (HAL_I2CEx_ConfigDigitalFilter(&codec_i2c2, 0) != HAL_OK)							_Error_Handler(__FILE__, __LINE__);
}


