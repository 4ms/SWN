/*
 * ads8634_driver.c - driver for ADS8634 ADC chip from TI
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

/*
 * How to Use:
 * 
 *
*/

//
// Modes:
// SPI using interrupts
// DMA continuous
//

#include "gpio_pins.h"
#include "drivers/ads8634_driver.h"
#include "timekeeper.h"
#include "hal_handlers.h"

//delays about 32ns for every value of x
#define delay_32ns(x) do {  register unsigned int i;  for (i = 0; i < x; ++i)   __asm__ __volatile__ ("nop\n\t":::"memory"); } while (0)

ads8634Chip			chip[ NUMBER_OF_ADS8634_CHIPS ];

SPI_HandleTypeDef	spi_ads8634[ NUMBER_OF_ADS8634_CHIPS ];
// DMA_InitTypeDef 	spi_dma_tx[ NUMBER_OF_ADS8634_CHIPS ];
// DMA_InitTypeDef 	spi_dma_rx[ NUMBER_OF_ADS8634_CHIPS ];

#define				REG_INIT_BUFFER_SIZE 	8
uint16_t			reg_init_buffer[ REG_INIT_BUFFER_SIZE ];

// Pointer to location where ADC data should be put
float 				*g_adc_buffer_addr[ NUMBER_OF_ADS8634_CHIPS ];
uint8_t				g_adc_buffer_numchans[ NUMBER_OF_ADS8634_CHIPS ];


uint8_t				g_oversample_amt[ NUMBER_OF_ADS8634_CHIPS ][ MAX_ADCS_PER_CHIP ];
uint16_t			g_oversample_buff[ NUMBER_OF_ADS8634_CHIPS ][ MAX_ADCS_PER_CHIP ][ MAX_OVERSAMPLE_BUFF_SIZE ];
uint16_t			g_os_i[ NUMBER_OF_ADS8634_CHIPS ][ MAX_ADCS_PER_CHIP ];

// errors
enum ADS863xErrors 	hiresadc_error;

//Private:
void ADS8634_A_update(void);
void ADS8634_B_update(void);
void ads8634_IRQ_init(uint8_t chipnum, float *adc_buffer, uint8_t adc_buffer_chans);
void init_oversampling(uint8_t chipnum, uint8_t chan, uint8_t os_amt);


//*************************//
// Bit Bang
//*************************//

void ads8634_GPIO_bitbang_init(uint8_t chipnum)
{
	GPIO_InitTypeDef gpio;

	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull  = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	// SPI SCK pin configuration
	gpio.Pin = chip[chipnum].SCK.pin;
	HAL_GPIO_Init(chip[chipnum].SCK.gpio, &gpio);

	// SPI  MOSI pin configuration
	gpio.Pin =  chip[chipnum].MOSI.pin;
	HAL_GPIO_Init(chip[chipnum].MOSI.gpio, &gpio);

	// SPI  CS pin configuration:
	gpio.Pin =  chip[chipnum].CS.pin;
	HAL_GPIO_Init(chip[chipnum].CS.gpio, &gpio);

	// SPI  MISO pin configuration
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull  = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	gpio.Pin =  chip[chipnum].MISO.pin;
	HAL_GPIO_Init(chip[chipnum].MISO.gpio, &gpio);  
}

uint16_t ads8634_bitbang_send_receive(uint8_t chipnum, uint16_t cmd_word)
{
	uint16_t recv=0;
	uint8_t bit_read=0;
	uint8_t i;

	//Setup: CS High, Clock low
	PIN_HIGH(chip[chipnum].CS.gpio, chip[chipnum].CS.pin);
	PIN_LOW(chip[chipnum].SCK.gpio, chip[chipnum].SCK.pin);

	delay_32ns(100);

	//Select the chip: CS low
	PIN_LOW(chip[chipnum].CS.gpio, chip[chipnum].CS.pin);

	for(i=0;i<16;i++)
	{
		//Setup DIN with next bit to send, MSB first
		if (cmd_word & (1<<15)) 	PIN_HIGH(chip[chipnum].MOSI.gpio, chip[chipnum].MOSI.pin);
		else						PIN_LOW(chip[chipnum].MOSI.gpio, chip[chipnum].MOSI.pin);
		cmd_word <<= 1;

		//tSU(CS-SCLK) = min 18.5ns (18.5 - tSU(DIN-SCLK)  = 12.5)
		delay_32ns(13);
		//tSU(DIN-SCLK) = min 6ns
		delay_32ns(6);

		//Clock high
		PIN_HIGH(chip[chipnum].SCK.gpio, chip[chipnum].SCK.pin);

		//tH(SCLK-DIN) = min 8ns 
		delay_32ns(8);
		//tW(SCLK_H) = min 20ns clock high
		delay_32ns(12);

		//Clock low
		PIN_LOW(chip[chipnum].SCK.gpio, chip[chipnum].SCK.pin);

		//tH(SCLK-DOUT) SCLK falling to DOUT valid min 5ns
		delay_32ns(5);

		//Read DOUT, MSB first
		bit_read = PIN_READ(chip[chipnum].MISO.gpio, chip[chipnum].MISO.pin);
		recv = (recv << 1) | bit_read;

		delay_32ns(5);
	}

	//Deselect the chip: CS high
	PIN_HIGH(chip[chipnum].CS.gpio, chip[chipnum].CS.pin);

	return recv;
}

void ads8634_bitbang_test(uint8_t chipnum, uint8_t buffer_size)
{
	// uint16_t recv_data[4];
	// uint32_t i,j;

	// if (buffer_size>4) buffer_size=4;

	// for (i=0;i<50000;i++)
	// {
	// 	for (j=0;j<buffer_size;j++)
	// 		recv_data[j] = ads8634_bitbang_send_receive(chipnum, 0);
	// }

}



//*************************//
// GPIO
//*************************//


void ads8634_SPI_GPIO_init(uint8_t chipnum)
{
	GPIO_InitTypeDef gpio;

	#ifdef SPI1
		if (chip[chipnum].SPIx == SPI1)	 
			__HAL_RCC_SPI1_CLK_ENABLE();
	#endif
	#ifdef SPI2
		if (chip[chipnum].SPIx == SPI2)	 
			__HAL_RCC_SPI2_CLK_ENABLE();
	#endif
	#ifdef SPI3
		if (chip[chipnum].SPIx == SPI3)	 
			__HAL_RCC_SPI3_CLK_ENABLE();
	#endif
	#ifdef SPI4
		if (chip[chipnum].SPIx == SPI4)	 
			__HAL_RCC_SPI4_CLK_ENABLE();
	#endif
	#ifdef SPI5
		if (chip[chipnum].SPIx == SPI5)	
		 __HAL_RCC_SPI5_CLK_ENABLE();
	#endif
	#ifdef SPI6
		if (chip[chipnum].SPIx == SPI6)	 
			__HAL_RCC_SPI6_CLK_ENABLE();
	#endif

	// Assume GPIO RCC is enabled already, or if not do it here:
	//
	//		__HAL_RCC_GPIOx_CLK_ENABLE()
	// etc...

	gpio.Mode 	= GPIO_MODE_AF_PP;
	gpio.Speed 	= GPIO_SPEED_FREQ_VERY_HIGH;
	gpio.Pull  	= GPIO_PULLDOWN;


	// SPI SCK pin configuration
	gpio.Pin 		= chip[chipnum].SCK.pin;
	gpio.Alternate 	= chip[chipnum].SCK.af;
	HAL_GPIO_Init(chip[chipnum].SCK.gpio, &gpio);

	// SPI  MOSI pin configuration
	gpio.Pin 		= chip[chipnum].MOSI.pin;
	gpio.Alternate 	= chip[chipnum].MOSI.af;
	HAL_GPIO_Init(chip[chipnum].MOSI.gpio, &gpio);

	// SPI  MISO pin configuration
	gpio.Pin 		= chip[chipnum].MISO.pin;
	gpio.Alternate 	= chip[chipnum].MISO.af;
	HAL_GPIO_Init(chip[chipnum].MISO.gpio, &gpio);  

	// SPI  CS pin configuration
	gpio.Pin 		= chip[chipnum].CS.pin;
	gpio.Alternate 	= chip[chipnum].CS.af;
	HAL_GPIO_Init(chip[chipnum].CS.gpio, &gpio);
}


//*************************//
// SPI
//*************************//


void ads8634_SPI_init(uint8_t chipnum)
{
	ads8634_SPI_GPIO_init(chipnum);

	spi_ads8634[chipnum].Instance               = chip[chipnum].SPIx;
	spi_ads8634[chipnum].Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	spi_ads8634[chipnum].Init.Direction         = SPI_DIRECTION_2LINES;
	spi_ads8634[chipnum].Init.CLKPhase          = SPI_PHASE_1EDGE;
	spi_ads8634[chipnum].Init.CLKPolarity       = SPI_POLARITY_LOW;
	spi_ads8634[chipnum].Init.DataSize          = SPI_DATASIZE_16BIT;
	spi_ads8634[chipnum].Init.FirstBit          = SPI_FIRSTBIT_MSB;
	spi_ads8634[chipnum].Init.TIMode            = SPI_TIMODE_DISABLE;
	spi_ads8634[chipnum].Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	spi_ads8634[chipnum].Init.CRCPolynomial     = 7;
	spi_ads8634[chipnum].Init.NSS               = SPI_NSS_HARD_OUTPUT;
	spi_ads8634[chipnum].Init.NSSPMode			= SPI_NSS_PULSE_ENABLE;
	spi_ads8634[chipnum].Init.Mode 				= SPI_MODE_MASTER;

	if (HAL_SPI_Init(&spi_ads8634[chipnum]) != HAL_OK)
		hiresadc_error = (ADS_SPI_INIT_ERR<<1) | chipnum;
}

void ads8634_SPI_stop(uint8_t chipnum)
{
	// Disable the SPI peripheral 
	chip[chipnum].SPIx->CR1 &= ~SPI_CR1_SPE;
}

void ads8634_SPI_start(uint8_t chipnum)
{
	// Enable the SPI peripheral 
	chip[chipnum].SPIx->CR1 |= SPI_CR1_SPE;
}



void ads8634_IRQ_init(uint8_t chipnum, float *adc_buffer, uint8_t adc_buffer_chans)
{
	g_adc_buffer_addr[chipnum] = adc_buffer;
	g_adc_buffer_numchans[chipnum] = adc_buffer_chans;

	// Configure the SPI interrupt priority
	HAL_NVIC_SetPriority(chip[chipnum].SPI_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(chip[chipnum].SPI_IRQn);

	// Enable the Rx buffer not empty interrupt
	__HAL_SPI_ENABLE_IT(&spi_ads8634[chipnum], SPI_IT_RXNE);

	// Disable the TX empty interrupt
	__HAL_SPI_DISABLE_IT(&spi_ads8634[chipnum], SPI_IT_TXE);

}

uint16_t MAKE_CMD_WORD(uint8_t reg, uint8_t read_write_mode, uint8_t val);
uint16_t MAKE_CMD_WORD(uint8_t reg, uint8_t read_write_mode, uint8_t val) { return ( (reg<<9) | (read_write_mode<<8) | val ); }

//It's assumed that ADC0 to ADCN are used, where N=num_adcs-1
void ads8634_create_reg_init_buffer(uint8_t num_adcs, enum RangeSel *ranges)
{
	uint8_t channels=0;
	if (num_adcs>3) channels |= CH_SEL_3;
	if (num_adcs>2) channels |= CH_SEL_2;
	if (num_adcs>1) channels |= CH_SEL_1;
	if (num_adcs>0) channels |= CH_SEL_0;

	reg_init_buffer[0] = MAKE_CMD_WORD(RESET_DEVICE, REG_WRITE, B0_RESET);
	reg_init_buffer[1] = MAKE_CMD_WORD(AUX_CONFIG, REG_WRITE, AUX_ALPD_PD);
	reg_init_buffer[2] = MAKE_CMD_WORD(AUTO_MD_CH_SEL, REG_WRITE, channels);
	reg_init_buffer[3] = MAKE_CMD_WORD(CH0_RANGE, REG_WRITE, ranges[0] << SHIFT_CH_HIGH);
	reg_init_buffer[4] = MAKE_CMD_WORD(CH1_RANGE, REG_WRITE, ranges[1] << SHIFT_CH_HIGH);
	reg_init_buffer[5] = MAKE_CMD_WORD(CH2_RANGE, REG_WRITE, ranges[2] << SHIFT_CH_HIGH);
	reg_init_buffer[6] = MAKE_CMD_WORD(CH3_RANGE, REG_WRITE, ranges[3] << SHIFT_CH_HIGH);
	reg_init_buffer[7] = MAKE_CMD_WORD(SEQ_AUTO, REG_WRITE, RESET_SEQ);
}

void ads8634_create_vrange_buffer(enum RangeSel *ranges)
{
	reg_init_buffer[1] = MAKE_CMD_WORD(CH0_RANGE, REG_WRITE, ranges[0] << SHIFT_CH_HIGH);
	reg_init_buffer[0] = MAKE_CMD_WORD(CH1_RANGE, REG_WRITE, ranges[1] << SHIFT_CH_HIGH);
	reg_init_buffer[2] = MAKE_CMD_WORD(CH2_RANGE, REG_WRITE, ranges[2] << SHIFT_CH_HIGH);
	reg_init_buffer[3] = MAKE_CMD_WORD(CH3_RANGE, REG_WRITE, ranges[3] << SHIFT_CH_HIGH);
}

//*************************//
// DMA
// FixMe: Entire DMA section is not working
//*************************//


void ads8634_DMA_IRQ_init(uint8_t chipnum)
{
	// NVIC_InitTypeDef nvic;

	// // Enable the RX Transfer Complete DMA interrupt
	// DMA_ITConfig(chip[chipnum].rx_DMA_stream, DMA_IT_TC, ENABLE);

	// // SPI RX DMA IRQ Channel configuration
	// nvic.NVIC_IRQChannel = chip[chipnum].rx_DMA_irqn;
	// nvic.NVIC_IRQChannelPreemptionPriority = 3;
	// nvic.NVIC_IRQChannelSubPriority = 0;
	// nvic.NVIC_IRQChannelCmd = ENABLE;
	// NVIC_Init(&nvic);

	// NVIC_EnableIRQ(chip[chipnum].rx_DMA_irqn);
}

void ads8634_DMA_init(uint32_t DMA_mode, uint16_t *adc_buffer, uint32_t adc_buffer_size, uint16_t *tx_buffer, uint16_t tx_buffer_size, uint8_t chipnum)
{
	// RCC_AHB1PeriphClockCmd(chip[chipnum].DMA_clk, ENABLE);
 // 	DMA_DeInit(chip[chipnum].tx_DMA_stream);
	// DMA_DeInit(chip[chipnum].rx_DMA_stream);

	// // Configure TX DMA
	// spi_dma_tx[chipnum].DMA_BufferSize = tx_buffer_size ;
	// spi_dma_tx[chipnum].DMA_FIFOMode = DMA_FIFOMode_Disable ;
	// spi_dma_tx[chipnum].DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
	// spi_dma_tx[chipnum].DMA_MemoryBurst = DMA_MemoryBurst_Single ;
	// spi_dma_tx[chipnum].DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	// spi_dma_tx[chipnum].DMA_MemoryInc = DMA_MemoryInc_Enable;
	// spi_dma_tx[chipnum].DMA_Mode = DMA_mode;
	// spi_dma_tx[chipnum].DMA_PeripheralBaseAddr =(uint32_t) (&(chip[chipnum].SPIx->DR)) ;
	// spi_dma_tx[chipnum].DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	// spi_dma_tx[chipnum].DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	// spi_dma_tx[chipnum].DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	// spi_dma_tx[chipnum].DMA_Priority = DMA_Priority_High;
	// spi_dma_tx[chipnum].DMA_Channel = chip[chipnum].tx_DMA_Channel;
	// spi_dma_tx[chipnum].DMA_DIR = DMA_DIR_MemoryToPeripheral ;
	// spi_dma_tx[chipnum].DMA_Memory0BaseAddr =(uint32_t)tx_buffer ;

	// DMA_Init(chip[chipnum].tx_DMA_stream, &spi_dma_tx[chipnum]);

	// // Configure RX DMA 
	// spi_dma_rx[chipnum].DMA_Channel = chip[chipnum].rx_DMA_Channel ;
	// spi_dma_rx[chipnum].DMA_PeripheralBaseAddr =(uint32_t) (&(chip[chipnum].SPIx->DR)) ;
	// spi_dma_rx[chipnum].DMA_Memory0BaseAddr =(uint32_t)adc_buffer ; 
	// spi_dma_rx[chipnum].DMA_DIR = DMA_DIR_PeripheralToMemory ;
	// spi_dma_rx[chipnum].DMA_BufferSize = adc_buffer_size ;
	// spi_dma_rx[chipnum].DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	// spi_dma_rx[chipnum].DMA_MemoryInc = DMA_MemoryInc_Enable;
	// spi_dma_rx[chipnum].DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	// spi_dma_rx[chipnum].DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	// spi_dma_rx[chipnum].DMA_Mode = DMA_mode; //circular or normal
	// spi_dma_rx[chipnum].DMA_Priority = DMA_Priority_High;
	// spi_dma_rx[chipnum].DMA_FIFOMode = DMA_FIFOMode_Disable ;
	// spi_dma_rx[chipnum].DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
	// spi_dma_rx[chipnum].DMA_MemoryBurst = DMA_MemoryBurst_Single ;
	// spi_dma_rx[chipnum].DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	// DMA_Init(chip[chipnum].rx_DMA_stream, &spi_dma_rx[chipnum]);
}
void ads8634_DMA_stop(uint8_t chipnum)
{
	// // Enable DMA SPI TX Stream
	// DMA_Cmd(chip[chipnum].tx_DMA_stream, DISABLE);

	// // Enable DMA SPI RX Stream
	// DMA_Cmd(chip[chipnum].rx_DMA_stream, DISABLE);

	// // Enable SPI DMA TX Requsts
	// SPI_I2S_DMACmd(chip[chipnum].SPIx, SPI_I2S_DMAReq_Tx, DISABLE);

	// // Enable SPI DMA RX Requsts
	// SPI_I2S_DMACmd(chip[chipnum].SPIx, SPI_I2S_DMAReq_Rx, DISABLE);
}


void ads8634_DMA_start(uint8_t chipnum)
{
	// // Enable DMA SPI TX Stream
	// DMA_Cmd(chip[chipnum].tx_DMA_stream,ENABLE);

	// // Enable DMA SPI RX Stream
	// DMA_Cmd(chip[chipnum].rx_DMA_stream,ENABLE);

	// // Enable SPI DMA TX Requsts
	// SPI_I2S_DMACmd(chip[chipnum].SPIx, SPI_I2S_DMAReq_Tx, ENABLE);

	// // Enable SPI DMA RX Requsts
	// SPI_I2S_DMACmd(chip[chipnum].SPIx, SPI_I2S_DMAReq_Rx, ENABLE);
}

void ads8634_init_with_DMA(uint16_t *adc_buffer, uint8_t adc_buffer_size, uint16_t *tx_buffer, uint16_t tx_buffer_size, enum RangeSel *v_ranges, uint8_t chipnum)
{
	// uint8_t i;

	// //BitBang setup:
	// ads8634_GPIO_bitbang_init(chipnum);
	// ads8634_create_reg_init_buffer(adc_buffer_size, v_ranges);
	// for (i=0; i<REG_INIT_BUFFER_SIZE; i++)
	// 	ads8634_bitbang_send_receive(chipnum, reg_init_buffer[i]);

	// // //Bitbang test:
	// ads8634_bitbang_test(chipnum, adc_buffer_size);

	// //Set up for DMA SPI
	// ads8634_SPI_stop(chipnum);
	// ads8634_SPI_GPIO_init(chipnum);

	// ads8634_DMA_init(DMA_Mode_Normal, adc_buffer, adc_buffer_size, tx_buffer, tx_buffer_size, chipnum);
	// ads8634_SPI_init(chipnum);

	// ads8634_DMA_IRQ_init(chipnum);

	// ads8634_DMA_start(chipnum);
	
	// ads8634_SPI_start(chipnum);

	// test loop
	// while (1)
	// {
	// 	while (DMA_GetFlagStatus(chip[chipnum].tx_DMA_stream, chip[chipnum].tx_DMA_flag_tcif)==RESET);
	// 	DMA_ClearFlag(chip[chipnum].tx_DMA_stream, chip[chipnum].tx_DMA_flag_tcif);
	// }

}

//*************************//
// Init for SPI/IRQ
//*************************//

// adc_buffer: base address of an array where the final values are stored
//    adc_buffer[channel_number] is where the adc data goes after it's oversampled
//
// adc_buffer_chans: the number of channels
//
// chipnum: which chip we are using
//
// oversample_amts: a pointer to an array with adc_buffer_chans elements
//    oversample_amts[channel_number] sets the amount of oversampling for that adc channel
//
// v_ranges: a pointer to an array with adc_buffer_chans elements
//    v_ranges[channel_number] sets the voltage range for that adc channel
//
void ads8634_init_with_SPIIRQ(float *adc_buffer, uint8_t adc_buffer_chans, uint8_t chipnum, uint8_t *oversample_amts, enum RangeSel *v_ranges)
{
	uint8_t i;
	uint8_t chan;

	//Create the chip stucture
	create_chip(&chip[chipnum], chipnum);

	//FixMe: Use SPI instead of bitbang to initialize the chip
	ads8634_GPIO_bitbang_init(chipnum);
	ads8634_create_reg_init_buffer(adc_buffer_chans, v_ranges);
	for (i=0; i<REG_INIT_BUFFER_SIZE; i++)	ads8634_bitbang_send_receive(chipnum, reg_init_buffer[i]);

	//Set up for SPI
	ads8634_SPI_stop(chipnum);

	ads8634_SPI_init(chipnum);
	ads8634_IRQ_init(chipnum, adc_buffer, adc_buffer_chans);

	for (chan=0;chan<MAX_ADCS_PER_CHIP;chan++)
		init_oversampling(chipnum, chan, oversample_amts[chan]);

	ads8634_SPI_start(chipnum);

	//Initiate first transfer by writing zeros
	chip[chipnum].SPIx->DR = 0x1;
}

//Todo: not tested yet!
void ads8634_set_vrange(uint8_t chipnum, enum RangeSel *v_ranges, uint8_t number_adcs)
{
	uint8_t i;
	enum RangeSel padded_ranges[MAX_ADCS_PER_CHIP];

	__HAL_SPI_DISABLE_IT(&spi_ads8634[chipnum], SPI_IT_RXNE);

	for (i=0;i<MAX_ADCS_PER_CHIP;i++)
	{
		if (i<number_adcs) padded_ranges[i]=v_ranges[i];
		else padded_ranges[i]=RANGE_pm10V;
	}

	ads8634_create_vrange_buffer(padded_ranges);

	while (chip[chipnum].status != READY_TO_TX) {;}
	chip[chipnum].status = NOT_READY_TO_TX;

	for (i=0;i<MAX_ADCS_PER_CHIP;i++) 
	{
		while (!(chip[chipnum].SPIx->SR & SPI_FLAG_TXE)) {;}
		chip[chipnum].SPIx->DR = reg_init_buffer[i];
	}


	__HAL_SPI_ENABLE_IT(&spi_ads8634[chipnum], SPI_IT_RXNE);
}

//*************************//
// Oversampling
//*************************//

void init_oversampling(uint8_t chipnum, uint8_t chan, uint8_t os_amt)
{
	uint8_t i;

	g_oversample_amt[chipnum][chan] 	= os_amt;
	g_os_i[chipnum][chan]				= 0;
	for (i=0;i<MAX_OVERSAMPLE_BUFF_SIZE;i++) g_oversample_buff[chipnum][chan][i] = 0;
}

// Returns the average of the oversampling buffer, rejecting the max and min values
// 
float resolve_oversampling(uint8_t chipnum, uint8_t chan)
{
	uint8_t i, num_elements;
	uint32_t max_i=0, min_i=0, sum=0;

	num_elements = g_oversample_amt[chipnum][chan];
	for (i=0;i<num_elements;i++)
	{
		if (g_oversample_buff[chipnum][chan][i] > g_oversample_buff[chipnum][chan][max_i]) max_i=i;
		if (g_oversample_buff[chipnum][chan][i] < g_oversample_buff[chipnum][chan][min_i]) min_i=i;
		sum+=g_oversample_buff[chipnum][chan][i];
	}
	sum = sum - g_oversample_buff[chipnum][chan][max_i] - g_oversample_buff[chipnum][chan][min_i];
	num_elements-=2;

	return (float)sum/(float)num_elements;
}

//*************************//
// IRQ Handlers
//*************************//

// SPI Peripheral call this IRQ when it's received a value from the ADC chip
//
// Fill the oversampler buffer with raw adc values
// When the oversampler buffer is full, set the g_adc_buffer to the resolved value (average value excluding max and min)
// This IRQ takes average of 240ns (was 125ns without oversampling)
void ADS8634_A_SPI_IRQHANDLER(void)
{
	uint8_t chan;
	uint16_t recv_data;
	uint32_t itflag	= chip[0].SPIx->SR;
	uint32_t itsource = chip[0].SPIx->CR2;

	// SPI Interrupt triggered when Receive buffer Not Empty (RXNE)
	if ((itflag & SPI_FLAG_RXNE) && (itsource & SPI_IT_RXNE))
	{
		chip[0].status = NOT_READY_TO_TX;

		recv_data = chip[0].SPIx->DR;

		// Extract the channel number (channel # is top 3 bits)
		chan = (recv_data >> 13); 

		// Range-check the channel
		// ADC value is bottom 12 bits
		if (chan < g_adc_buffer_numchans[0])
		{
			g_oversample_buff[0][chan][ g_os_i[0][chan] ]  = (recv_data & 0xFFF); //ADC value is bottom 12 bits
			g_os_i[0][chan]++;
			if (g_os_i[0][chan]>=g_oversample_amt[0][chan])
			{
				g_os_i[0][chan] = 0;
				g_adc_buffer_addr[0][chan] = resolve_oversampling(0, chan);
			}
		}

		chip[0].SPIx->DR = 0x00;

		chip[0].status = READY_TO_TX;
	}
}

void ADS8634_B_SPI_IRQHANDLER(void)
{
	uint8_t chan;
	uint16_t recv_data;
	uint32_t itflag	= chip[1].SPIx->SR;
	uint32_t itsource = chip[1].SPIx->CR2;

	// SPI Interrupt triggered when Receive buffer Not Empty (RXNE)
	if ((itflag & SPI_FLAG_RXNE) && (itsource & SPI_IT_RXNE))
	{
		chip[1].status = NOT_READY_TO_TX;

		// Grab the received data
		recv_data = chip[1].SPIx->DR;

		// Extract the channel number (channel # is top 3 bits)
		chan = (recv_data >> 13); 

		// Range-check the channel
		// ADC value is bottom 12 bits
		if (chan < g_adc_buffer_numchans[1])
		{
			g_oversample_buff[1][chan][ g_os_i[1][chan] ]  = (recv_data & 0xFFF); //ADC value is bottom 12 bits
			g_os_i[1][chan]++;
			if (g_os_i[1][chan]>=g_oversample_amt[1][chan])
			{
				g_os_i[1][chan] = 0;
				g_adc_buffer_addr[1][chan] = resolve_oversampling(1, chan);
			}
		}
		chip[1].SPIx->DR = 0x00;

		chip[1].status = READY_TO_TX;
	}
}

//DMA not working yet
// void ADS8634_A_RX_DMA_IRQHandler(void)
// {
	// Transfer complete interrupt
	// if (DMA_GetFlagStatus(chip[0].rx_DMA_stream, chip[0].rx_DMA_flag_tcif) != RESET)
	// {
	// 	ads8634_SPI_init(0);

	// 	chip[0].rx_DMA_stream->NDTR = g_adc_buffer_numchans[0];

	// 	DMA_ITConfig(chip[0].rx_DMA_stream, DMA_IT_TC, ENABLE);
	// 	ads8634_DMA_start(0);
	// 	ads8634_SPI_start(0);

	// 	DMA_ClearFlag(chip[0].rx_DMA_stream, chip[0].rx_DMA_flag_tcif);
	// }
// }



