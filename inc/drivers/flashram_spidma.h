/*
 * flashram_spidma.h - External FLASH RAM SPI driver using DMA
 *
 * Author: Dan Green (danngreen1@gmail.com)
 * (c) 2019
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

typedef struct SpiPin{
	uint16_t		pin;
	GPIO_TypeDef 	*gpio;
	uint32_t		gpio_clk;
	uint8_t			af;
} SpiPin;


typedef struct FlashRamChip
{
	SPI_TypeDef 			*SPIx;
	SpiPin			SCK;
	SpiPin			MISO;
	SpiPin			MOSI;
	SpiPin			CS;
	SpiPin			IO2;
	SpiPin			IO3;

} FlashRamChip;


//todo put this in a structure
#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define SPIx_TX_DMA_STREAM               DMA1_Stream5 //or 7
#define SPIx_RX_DMA_STREAM               DMA1_Stream2 //or 0
#define SPIx_TX_DMA_CHANNEL              DMA_CHANNEL_0
#define SPIx_RX_DMA_CHANNEL              DMA_CHANNEL_0
#define SPIx_DMA_TX_IRQn                 DMA1_Stream5_IRQn
#define SPIx_DMA_RX_IRQn                 DMA1_Stream2_IRQn
#define SPIx_DMA_TX_IRQHandler           DMA1_Stream5_IRQHandler
#define SPIx_DMA_RX_IRQHandler           DMA1_Stream2_IRQHandler

enum sFlashErrors{
	sFLASH_NO_ERROR			= 0,
	sFLASH_ERASE_ERROR 		= (1<<0),
	sFLASH_PROG_ERROR 		= (1<<1),
	sFLASH_SPI_INIT_ERROR 	= (1<<2),
	sFLASH_SPI_DMA_INIT_ERROR = (1<<3),
	sFLASH_SPI_DMA_RX_ERROR = (1<<4),
	sFLASH_SPI_DMA_TX_ERROR = (1<<5),
	sFLASH_SPI_PAGE_OF_WARN = (1<<6),
};

enum sFlashStates {
	sFLASH_NOTBUSY,
	sFLASH_WRITECMD,
	sFLASH_WRITING,
	sFLASH_READCMD,
	sFLASH_READING,
	sFLASH_ERASING,
	sFLASH_ERROR 
};

//Initialize
void sFLASH_init(void);

//Chip status
uint8_t sFLASH_is_chip_ready(void);
enum sFlashStates get_flash_state(void);

//Erasing
void sFLASH_erase_sector(uint32_t SectorAddr);
void sFLASH_erase_sector_background(uint32_t SectorAddr);
void sFLASH_erase_chip(void);

//Reading and writing
void sFLASH_write_buffer(uint8_t* txBuffer, uint32_t write_addr, uint16_t num_bytes);
void sFLASH_read_buffer(uint8_t* rxBuffer, uint32_t read_addr, uint16_t num_bytes);
void sFLASH_read_buffer_DMA(uint8_t* rxBuffer, uint32_t read_addr, uint16_t num_bytes);


//Testing routines
// uint32_t sFLASH_test(void);
uint32_t sFLASH_test_sector(uint32_t test_start);

