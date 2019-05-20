
/*
 * ads8634_driver.h
 * Driver for multiple ADS8634 chips
 */

#pragma once

#include <stm32f7xx.h>

#define NUMBER_OF_ADS8634_CHIPS 2

//Set this to 8 if using ADS8638 chips, otherwise leave at 4
//FixMe: This doesn't properly work at 8
#define MAX_ADCS_PER_CHIP			4

#define MAX_OVERSAMPLE_BUFF_SIZE 	20

//
// Structures for ADS8634 chip
//

typedef struct spiPin{
	uint16_t	pin;
	GPIO_TypeDef *gpio;
	uint32_t	gpio_clk;
	uint8_t		pin_source;
	uint8_t		af;
} spiPin;

enum chipStatus {
	UNKNOWN = 0,
	READY_TO_TX = 1,
	NOT_READY_TO_TX = 2

};

typedef struct ads8634Chip
{
	SPI_TypeDef 		*SPIx;
	uint32_t			SPI_clk;
	uint32_t			SPI_clk_pbnum; //1 = RCC_APB1PeriphClockCmd, 2 = RCC_APB2PeriphClockCmd
	IRQn_Type			SPI_IRQn;

	spiPin				SCK;
	spiPin				MISO;
	spiPin				MOSI;
	spiPin				CS;

	// DMA_TypeDef 		*DMAx;
	// uint32_t			DMA_clk;

	// uint32_t			tx_DMA_Channel;
	// DMA_Stream_TypeDef	*tx_DMA_stream;
	// uint32_t			tx_DMA_flag_tcif;
	// uint32_t			tx_DMA_irqn;

	// uint32_t			rx_DMA_Channel;
	// DMA_Stream_TypeDef	*rx_DMA_stream;
	// uint32_t			rx_DMA_flag_tcif;
	// uint32_t			rx_DMA_irqn;

	enum chipStatus		status;
} ads8634Chip;

//
// Config:
// Pin assignments
//

static inline void create_chip(ads8634Chip *chip, uint8_t chipnum);
static inline void create_chip(ads8634Chip *chip, uint8_t chipnum){
	if (chipnum==0)
	{
		chip->SPIx 				= SPI2;
		chip->SPI_IRQn 			= SPI2_IRQn;

		chip->SCK.pin			= GPIO_PIN_13;
		chip->SCK.gpio 			= GPIOB;
		chip->SCK.af			= GPIO_AF5_SPI2;

		chip->MISO.pin			= GPIO_PIN_14;
		chip->MISO.gpio			= GPIOB;
		chip->MISO.af			= GPIO_AF5_SPI2;

		chip->MOSI.pin			= GPIO_PIN_15;
		chip->MOSI.gpio 		= GPIOB;
		chip->MOSI.af			= GPIO_AF5_SPI2;

		chip->CS.pin			= GPIO_PIN_12;
		chip->CS.gpio 			= GPIOB;
		chip->CS.af				= GPIO_AF5_SPI2;

		// chip->DMAx				= DMA2;
		// chip->DMA_clk			= RCC_AHB1Periph_DMA2;

		// chip->tx_DMA_Channel	= DMA_Channel_4;
		// chip->tx_DMA_stream		= DMA2_Stream1;
		// chip->tx_DMA_flag_tcif	= DMA_FLAG_TCIF1;
		// chip->tx_DMA_irqn		= DMA2_Stream1_IRQn;

		// chip->rx_DMA_Channel	= DMA_Channel_4;
		// chip->rx_DMA_stream 	= DMA2_Stream0;
		// chip->rx_DMA_flag_tcif 	= DMA_FLAG_TCIF0;
		// chip->rx_DMA_irqn		= DMA2_Stream0_IRQn;

	}
	if (chipnum==1)
	{
		chip->SPIx 					= SPI1;
		chip->SPI_IRQn 				= SPI1_IRQn;

		chip->SCK.pin				= GPIO_PIN_5;
		chip->SCK.gpio 				= GPIOA;
		chip->SCK.af				= GPIO_AF5_SPI1;

		chip->MISO.pin				= GPIO_PIN_6;
		chip->MISO.gpio 			= GPIOA;
		chip->MISO.af				= GPIO_AF5_SPI1;

		chip->MOSI.pin				= GPIO_PIN_7;
		chip->MOSI.gpio 			= GPIOA;
		chip->MOSI.af				= GPIO_AF5_SPI1;

		chip->CS.pin				= GPIO_PIN_4;
		chip->CS.gpio 				= GPIOA;
		chip->CS.af					= GPIO_AF5_SPI1;

		// chip->DMAx				= DMA2;
		// chip->DMA_clk			= RCC_AHB1Periph_DMA2;

		// chip->tx_DMA_Channel	= DMA_Channel_3;
		// chip->tx_DMA_stream		= DMA2_Stream3;
		// chip->tx_DMA_flag_tcif	= DMA_FLAG_TCIF3;
		// chip->tx_DMA_irqn		= DMA2_Stream3_IRQn;

		// chip->rx_DMA_Channel	= DMA_Channel_3;
		// chip->rx_DMA_stream 	= DMA2_Stream2;
		// chip->rx_DMA_flag_tcif 	= DMA_FLAG_TCIF2;
		// chip->rx_DMA_irqn		= DMA2_Stream2_IRQn;

	}

}
//Todo: Research if there's a way to declare a function given its address?
//The goal is to put these functions into the chip structure
#define ADS8634_A_SPI_IRQHANDLER			SPI2_IRQHandler
#define ADS8634_B_SPI_IRQHANDLER			SPI1_IRQHandler

// #define ADS8634_A_RX_DMA_IRQHandler			DMA2_Stream0_IRQHandler
// #define ADS8634_B_RX_DMA_IRQHandler			DMA2_Stream2_IRQHandler


//*************************//
// Registers
//*************************//


enum ReadWriteModes {
	REG_READ 	= 1,
	REG_WRITE 	= 0
};
enum Registers {
	SEQ_MANUAL 		= 0x04,
	SEQ_AUTO 		= 0x05,
	RESET_DEVICE	= 0x01,
	AUX_CONFIG		= 0x06,
	AUTO_MD_CH_SEL	= 0x0C,
	CH0_RANGE		= 0x10, //For ADS8438: is CH01_RANGE
	CH1_RANGE		= 0x11, //For ADS8438: is CH23_RANGE
	CH2_RANGE		= 0x12, //For ADS8438: is CH45_RANGE
	CH3_RANGE		= 0x13, //For ADS8438: is CH67_RANGE
	ALARM_TEMP		= 0x20,

	ALARM_CH0123_TRIP	= 0x21,
	ALARM_CH0123_ACTIVE	= 0x22,
	ALARM_CH4567_TRIP	= 0x23,
	ALARM_CH4567_ACTIVE	= 0x24,

	PAGE_SEL		= 0x7F
};

//RESET_DEVICE
enum ResetReg {
	B0_RESET = (1<<0)
};

//SEQ_AUTO
enum AutoScanReg {
	SEL_TEMP 		= (1<<0),
	RANGE_REG 			= (0b000 << 1), // Range set by config registeres 0x10 to 0x13
	NEXT_RANGE_pm10V 	= (0b001 << 1),	// Next sampling's range: +/-10V
	NEXT_RANGE_pm5V 	= (0b010 << 1),	// Next sampling's range: +/-5V
	NEXT_RANGE_pm2_5V 	= (0b011 << 1),	// Next sampling's range: +/-2.5V
	NEXT_RANGE_p10V 	= (0b101 << 1),	// Next sampling's range: 0 to +10V
	NEXT_RANGE_p5V 		= (0b110 << 1),	// Next sampling's range: 0 to +5V	

	RESET_SEQ			= (1 << 7)
};

//AUX_CONFIG
enum AuxConfig{
	AUX_ALPD_AL		= (0<<3),	//AL_PD pin function select (ALARM output)
	AUX_ALPD_PD		= (1<<3),	//AL_PD pin function select (POWER DOWN input)
	AUX_INT_VREF	= (1<<2),	//Internal VREF
	AUX_TEMP_SENSE	= (1<<1)	//Temp Sensor enable
};

//AUTO_MD_CH_SEL
enum ADS8638_ChannelSel {
	ADS8638_CH_SEL_0 = (1<<7),
	ADS8638_CH_SEL_1 = (1<<6),
	ADS8638_CH_SEL_2 = (1<<5),
	ADS8638_CH_SEL_3 = (1<<4),
	ADS8638_CH_SEL_4 = (1<<3),
	ADS8638_CH_SEL_5 = (1<<2),
	ADS8638_CH_SEL_6 = (1<<1),
	ADS8638_CH_SEL_7 = (1<<0)
};

enum ADS8634_ChannelSel {
	CH_SEL_0 = (1<<7),
	CH_SEL_1 = (1<<5),
	CH_SEL_2 = (1<<3),
	CH_SEL_3 = (1<<1)

};


enum RangeSel {
	RANGE_pm10V 	= (0b001),	// +/-10V
	RANGE_pm5V 		= (0b010),	// +/-5V
	RANGE_pm2_5V 	= (0b011),	// +/-2.5V
	RANGE_p10V 		= (0b101),	// 0 to +10V
	RANGE_p5V 		= (0b110)	// 0 to +5V	
};
enum RangeShift {
	SHIFT_CH_LOW	= 0, //only used in ADS8638
	SHIFT_CH_HIGH	= 4
};


enum ADS863xErrors {
	ADS_NO_ERR = 0,
	ADS_SPI_INIT_ERR,
	
};

//Public functions:
//void ads8634_init_with_DMA(uint16_t *adc_buffer, uint8_t adc_buffer_size, uint16_t *tx_buffer, uint16_t tx_buffer_size, enum RangeSel *v_ranges, uint8_t chipnum);
void ads8634_init_with_SPIIRQ(float *adc_buffer, uint8_t adc_buffer_chans, uint8_t chipnum, uint8_t *oversample_amts, enum RangeSel *v_ranges);
void ads8634_set_vrange(uint8_t chipnum, enum RangeSel *v_ranges, uint8_t number_adcs);

