/*
 * flash_S25FL127.c - External FLASH RAM SPI driver for S25FL127 chip
 *
 * Author: Dan Green (danngreen1@gmail.com)
 * (c) 2018
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
 * Call sFLASH_init() in your main setup
 * 
 * To write to the Flash chip:
 * First you must erase the entire sector that you need to write, like this:
 *
 *    	sFLASH_erase_sector(SectorAddr);
 *		while (!sFLASH_is_chip_ready()) {;} //wait for chip to be ready (time typically is 150mS but datasheet says it can be up to 12 seconds)
 *
 * You can, of course, do other things before and/or inside the while loop and just check sFLASH_is_chip_ready() when it's convenient,
 * or before you need to write.
 * SectorAddr must be the start of a sector.
 * You can find the sector addresses using the sFLASH_get_sector_addr(sector_num) or sFLASH_align2sector(addr) functions.
 * The S25FL127 chip has 4kByte sectors and 64kByte sectors, see the .h for a list of sectors in the comments
 *
 * If you need to erase the whole chip, do it like this (takes about 40 seconds, up to 210 seconds worst case):
 *
 *    	sFLASH_erase_chip();
 *		while (!sFLASH_is_chip_ready()) {;} //wait for chip to be ready
 *
 * Once the sector is erased, you can write to it like this:
 *
 * 		sFLASH_write_buffer(&myDataBuffer, address, num_bytes);
 *
 * You can write any amount of data, spanning any sectors as long as the sectors it spans are already erased 
 *
 * To read data from the chip, do this:
 *
 * 		sFLASH_read_buffer(&myDataBuffer, address, num_bytes);
 *
 */


/* 
 * How to Configure:
 *
 * Choose BITBANG or SPI mode by #define'ing FLASH_S25FL127_MODE_BITBANG or FLASH_S25FL127_MODE_SPI
 * Set the pins in the create_flash_chip() function in the BITBANG or SPI section
 *
 */

#include "drivers/flash_S25FL127.h"
#include "globals.h"
#include "hal_handlers.h"
#include "gpio_pins.h"


S25FL127Chip s_flash_chip;
S25FL127Chip *flash_chip = &s_flash_chip; 

static DMA_HandleTypeDef spiflash_dma_tx;
static DMA_HandleTypeDef spiflash_dma_rx;
SPI_HandleTypeDef	spi_s25fl127;

enum sFlashErrors sflash_error=0;
volatile enum sFlashStates sflash_state=0;

//
// Config: Uncomment exactly one line below to set the mode
// #define FLASH_S25FL127_MODE_BITBANG
// #define FLASH_S25FL127_MODE_SPI
#define FLASH_S25FL127_MODE_SPIDMA

//
// Config: Comment or uncomment to enable QuadSPI mode (only available in bitbang, not in SPI)
// #define FLASH_S25FL127_MODE_BITBANG_QUAD //ToDo: implement QuadSPI mode

//
// Config: Set the pins used (and SPI# if using SPI)

#ifdef FLASH_S25FL127_MODE_BITBANG
void create_flash_chip(void);
void create_flash_chip(void){
	flash_chip = &s_flash_chip;
	flash_chip->SCK.pin				= GPIO_PIN_10;
	flash_chip->SCK.gpio 			= GPIOC;

	flash_chip->MISO.pin			= GPIO_PIN_11;
	flash_chip->MISO.gpio			= GPIOC;

	flash_chip->MOSI.pin			= GPIO_PIN_12;
	flash_chip->MOSI.gpio 			= GPIOC;

	flash_chip->CS.pin				= GPIO_PIN_15;
	flash_chip->CS.gpio 			= GPIOA;

	flash_chip->IO2.pin				= GPIO_PIN_11;
	flash_chip->IO2.gpio 			= GPIOA;

	flash_chip->IO3.pin				= GPIO_PIN_12;
	flash_chip->IO3.gpio 			= GPIOA;

}
#endif

#ifdef FLASH_S25FL127_MODE_SPI
void create_flash_chip(void);
void create_flash_chip(void){
	flash_chip = &s_flash_chip;
	flash_chip->SPIx 				= SPI3;

	flash_chip->SCK.pin				= GPIO_PIN_10;
	flash_chip->SCK.gpio 			= GPIOC;
	flash_chip->SCK.af				= GPIO_AF6_SPI3;

	flash_chip->MISO.pin			= GPIO_PIN_11;
	flash_chip->MISO.gpio			= GPIOC;
	flash_chip->MISO.af				= GPIO_AF6_SPI3;

	flash_chip->MOSI.pin			= GPIO_PIN_12;
	flash_chip->MOSI.gpio 			= GPIOC;
	flash_chip->MOSI.af				= GPIO_AF6_SPI3;

	flash_chip->CS.pin				= GPIO_PIN_15;
	flash_chip->CS.gpio 			= GPIOA;
	flash_chip->CS.af				= 0; //not used
}
#endif

#ifdef FLASH_S25FL127_MODE_SPIDMA
void create_flash_chip(void);
void create_flash_chip(void){
	flash_chip = &s_flash_chip;
	flash_chip->SPIx 				= SPI3;

	flash_chip->SCK.pin				= GPIO_PIN_10;
	flash_chip->SCK.gpio 			= GPIOC;
	flash_chip->SCK.af				= GPIO_AF6_SPI3;

	flash_chip->MISO.pin			= GPIO_PIN_11;
	flash_chip->MISO.gpio			= GPIOC;
	flash_chip->MISO.af				= GPIO_AF6_SPI3;

	flash_chip->MOSI.pin			= GPIO_PIN_12;
	flash_chip->MOSI.gpio 			= GPIOC;
	flash_chip->MOSI.af				= GPIO_AF6_SPI3;

	flash_chip->CS.pin				= GPIO_PIN_15;
	flash_chip->CS.gpio 			= GPIOA;
	flash_chip->CS.af				= GPIO_AF6_SPI3;
}
#endif




//
// Private functions:
//

uint32_t sFLASH_ReadID(void);

void sFLASH_StartReadSequence(uint32_t ReadAddr);
uint8_t sFLASH_ReadByte(void);
void sFLASH_EndReadSequence(void);

void sFLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);

uint8_t sFLASH_SendByte(uint8_t byte);
void sFLASH_WriteEnable(void);
void sFLASH_WaitForWriteEnd(void);

void sFLASH_bitbang_init(void);

void sFLASH_SPI_init(void);
void sFLASH_SPI_GPIO_init(uint32_t nss_mode);
void sFLASH_SPI_start(void);
void sFLASH_SPI_stop(void);

void sFLASH_SPIDMA_init(void);

uint32_t sFLASH_align2sector(uint32_t addr);

static inline void select_chip(void);
static inline void select_chip(void)	{  PIN_LOW(flash_chip->CS.gpio, flash_chip->CS.pin);	} //CS Low
static inline void deselect_chip(void);
static inline void deselect_chip(void)	{  PIN_HIGH(flash_chip->CS.gpio, flash_chip->CS.pin);	} //CS High


//delays about 32ns for every value of x
#define delay_32ns(x) do {  register unsigned int i;  for (i = 0; i < x; ++i)   __asm__ __volatile__ ("nop\n\t":::"memory"); } while (0)


enum sFlashStates get_flash_state(void) { return sflash_state; }

void sFLASH_init(void)
{
	create_flash_chip();

	#ifdef FLASH_S25FL127_MODE_BITBANG
		sFLASH_bitbang_init();
	#endif

	#ifdef FLASH_S25FL127_MODE_SPI
		sFLASH_SPI_stop();
		sFLASH_SPI_init();
		if (sflash_error == sFLASH_NO_ERROR)
			sFLASH_SPI_start();
	#endif

	#ifdef FLASH_S25FL127_MODE_SPIDMA
		sFLASH_SPIDMA_init();
	#endif

}

void sFLASH_bitbang_init(void)
{
	GPIO_InitTypeDef gpio;

	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull  = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH; 

	// RCC_AHB1PeriphClockCmd(flash_chip->SCK.gpio_clk | flash_chip->MISO.gpio_clk | flash_chip->MOSI.gpio_clk | flash_chip->CS.gpio_clk, ENABLE);

	// SCK pin configuration
	gpio.Pin = flash_chip->SCK.pin;
	HAL_GPIO_Init(flash_chip->SCK.gpio, &gpio);

	//  MOSI pin configuration
	gpio.Pin =  flash_chip->MOSI.pin;
	HAL_GPIO_Init(flash_chip->MOSI.gpio, &gpio);
	PIN_HIGH(flash_chip->MOSI.gpio, flash_chip->MOSI.pin);

	//  CS pin configuration:
	gpio.Pin =  flash_chip->CS.pin;
	HAL_GPIO_Init(flash_chip->CS.gpio, &gpio);
	deselect_chip();

	//  IO2 pin configuration:
	gpio.Pin =  flash_chip->IO2.pin;
	HAL_GPIO_Init(flash_chip->IO2.gpio, &gpio);
	PIN_HIGH(flash_chip->IO2.gpio, flash_chip->IO2.pin);

	//  IO3 pin configuration:
	gpio.Pin =  flash_chip->IO3.pin;
	HAL_GPIO_Init(flash_chip->IO3.gpio, &gpio);
	PIN_HIGH(flash_chip->IO3.gpio, flash_chip->IO3.pin);

	//  MISO pin configuration
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull  = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH; 

	gpio.Pin =  flash_chip->MISO.pin;
	HAL_GPIO_Init(flash_chip->MISO.gpio, &gpio);  
}

void sFLASH_SPI_GPIO_init(uint32_t nss_mode)
{
	GPIO_InitTypeDef gpio;

	#ifdef SPI1
		if (flash_chip->SPIx == SPI1)	 
			__HAL_RCC_SPI1_CLK_ENABLE();
	#endif
	#ifdef SPI2
		if (flash_chip->SPIx == SPI2)	 
			__HAL_RCC_SPI2_CLK_ENABLE();
	#endif
	#ifdef SPI3
		if (flash_chip->SPIx == SPI3)	 
			__HAL_RCC_SPI3_CLK_ENABLE();
	#endif
	#ifdef SPI4
		if (flash_chip->SPIx == SPI4)	 
			__HAL_RCC_SPI4_CLK_ENABLE();
	#endif
	#ifdef SPI5
		if (flash_chip->SPIx == SPI5)	
		 __HAL_RCC_SPI5_CLK_ENABLE();
	#endif
	#ifdef SPI6
		if (flash_chip->SPIx == SPI6)	 
			__HAL_RCC_SPI6_CLK_ENABLE();
	#endif

	// Assume GPIO RCC is enabled already, or if not do it here:
	//		__HAL_RCC_GPIOx_CLK_ENABLE()

	gpio.Speed 	= GPIO_SPEED_FREQ_VERY_HIGH;
	gpio.Pull  	= GPIO_PULLDOWN;


	// SPI SCK pin configuration
	gpio.Mode 		= GPIO_MODE_AF_PP;
	gpio.Pin 		= flash_chip->SCK.pin;
	gpio.Alternate 	= flash_chip->SCK.af;
	HAL_GPIO_Init(flash_chip->SCK.gpio, &gpio);

	// SPI  MOSI pin configuration
	gpio.Mode 		= GPIO_MODE_AF_PP;
	gpio.Pin 		= flash_chip->MOSI.pin;
	gpio.Alternate 	= flash_chip->MOSI.af;
	HAL_GPIO_Init(flash_chip->MOSI.gpio, &gpio);

	// SPI  MISO pin configuration
	gpio.Mode 		= GPIO_MODE_AF_PP;
	gpio.Pin 		= flash_chip->MISO.pin;
	gpio.Alternate 	= flash_chip->MISO.af;
	HAL_GPIO_Init(flash_chip->MISO.gpio, &gpio);  

	// SPI  CS pin configuration
	if (nss_mode == SPI_NSS_SOFT)
	{
		gpio.Mode 		= GPIO_MODE_OUTPUT_PP;
		gpio.Speed 		= GPIO_SPEED_FREQ_VERY_HIGH;
		gpio.Pull  		= GPIO_PULLUP;
		gpio.Pin 		= flash_chip->CS.pin;
		HAL_GPIO_Init(flash_chip->CS.gpio, &gpio);

		//Deselect the chip: CS high
		PIN_HIGH(flash_chip->CS.gpio, flash_chip->CS.pin);
	} 
	else if (nss_mode == SPI_NSS_HARD_OUTPUT)
	{
		gpio.Mode 		= GPIO_MODE_AF_PP;
		gpio.Pull  		= GPIO_PULLUP;
		gpio.Pin 		= flash_chip->CS.pin;
		gpio.Alternate 	= flash_chip->CS.af;
		HAL_GPIO_Init(flash_chip->CS.gpio, &gpio);  
	}
}


void sFLASH_SPI_init(void)
{
	uint32_t nss_mode = SPI_NSS_SOFT;
	
	sFLASH_SPI_GPIO_init(nss_mode);

	spi_s25fl127.Instance               = flash_chip->SPIx;
	spi_s25fl127.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	spi_s25fl127.Init.Direction         = SPI_DIRECTION_2LINES;
	spi_s25fl127.Init.CLKPhase          = SPI_PHASE_1EDGE;
	spi_s25fl127.Init.CLKPolarity       = SPI_POLARITY_LOW;
	spi_s25fl127.Init.DataSize          = SPI_DATASIZE_8BIT;
	spi_s25fl127.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	spi_s25fl127.Init.TIMode            = SPI_TIMODE_DISABLE;
	spi_s25fl127.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	spi_s25fl127.Init.CRCPolynomial     = 7;
	spi_s25fl127.Init.NSS               = nss_mode;
	spi_s25fl127.Init.Mode 				= SPI_MODE_MASTER;

	if (HAL_SPI_Init(&spi_s25fl127) != HAL_OK) 
		sflash_error |= sFLASH_SPI_INIT_ERROR;
}

void sFLASH_SPI_stop(void)
{
	// Disable the SPI peripheral 
	flash_chip->SPIx->CR1 &= ~SPI_CR1_SPE;
}

void sFLASH_SPI_start(void)
{
	// Enable the SPI peripheral 
	flash_chip->SPIx->CR1 |= SPI_CR1_SPE;
}


void sFLASH_SPIDMA_init(void)
{
	uint32_t nss_mode = SPI_NSS_SOFT;

	sFLASH_SPI_GPIO_init(nss_mode);

	spi_s25fl127.Instance               = flash_chip->SPIx;
	spi_s25fl127.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	spi_s25fl127.Init.Direction         = SPI_DIRECTION_2LINES;
	spi_s25fl127.Init.CLKPhase          = SPI_PHASE_1EDGE;
	spi_s25fl127.Init.CLKPolarity       = SPI_POLARITY_LOW;
	spi_s25fl127.Init.DataSize          = SPI_DATASIZE_8BIT;
	spi_s25fl127.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	spi_s25fl127.Init.TIMode            = SPI_TIMODE_DISABLE;
	spi_s25fl127.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	spi_s25fl127.Init.CRCPolynomial     = 7;
	spi_s25fl127.Init.NSS               = nss_mode;
	spi_s25fl127.Init.Mode 				= SPI_MODE_MASTER;

	if (HAL_SPI_Init(&spi_s25fl127) != HAL_OK)
		sflash_error |= sFLASH_SPI_DMA_INIT_ERROR;

	DMAx_CLK_ENABLE();

    spiflash_dma_tx.Instance                 = SPIx_TX_DMA_STREAM;
    spiflash_dma_tx.Init.Channel             = SPIx_TX_DMA_CHANNEL;
    spiflash_dma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    spiflash_dma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    spiflash_dma_tx.Init.MemBurst            = DMA_MBURST_SINGLE;
    spiflash_dma_tx.Init.PeriphBurst         = DMA_MBURST_SINGLE;
    spiflash_dma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    spiflash_dma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    spiflash_dma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    spiflash_dma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spiflash_dma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    spiflash_dma_tx.Init.Mode                = DMA_NORMAL;
    spiflash_dma_tx.Init.Priority            = DMA_PRIORITY_LOW;

    HAL_DMA_Init(&spiflash_dma_tx);
    __HAL_LINKDMA(&spi_s25fl127, hdmatx, spiflash_dma_tx);

    spiflash_dma_rx.Instance                 = SPIx_RX_DMA_STREAM;
    spiflash_dma_rx.Init.Channel             = SPIx_RX_DMA_CHANNEL;
    spiflash_dma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    spiflash_dma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    spiflash_dma_rx.Init.MemBurst            = DMA_MBURST_SINGLE;
    spiflash_dma_rx.Init.PeriphBurst         = DMA_MBURST_SINGLE;
    spiflash_dma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    spiflash_dma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    spiflash_dma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    spiflash_dma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    spiflash_dma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    spiflash_dma_rx.Init.Mode                = DMA_NORMAL;
    spiflash_dma_rx.Init.Priority            = DMA_PRIORITY_HIGH;

    HAL_DMA_Init(&spiflash_dma_rx);
    __HAL_LINKDMA(&spi_s25fl127, hdmarx, spiflash_dma_rx);

    HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 0, 3);
    HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);

    HAL_NVIC_SetPriority(SPIx_DMA_RX_IRQn, 0, 3);
    HAL_NVIC_EnableIRQ(SPIx_DMA_RX_IRQn);

}

#ifdef FLASH_S25FL127_MODE_SPIDMA
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	deselect_chip();
	sflash_state = sFLASH_NOTBUSY;
}
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (sflash_state == sFLASH_WRITING)
		deselect_chip();
	sflash_state = sFLASH_NOTBUSY;
}
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (sflash_state == sFLASH_READING)
		deselect_chip();
	sflash_state = sFLASH_NOTBUSY;
}
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	deselect_chip();
	sflash_state = sFLASH_ERROR;
}
void SPIx_DMA_RX_IRQHandler(void)
{
  HAL_DMA_IRQHandler(spi_s25fl127.hdmarx);
}
void SPIx_DMA_TX_IRQHandler(void)
{
  HAL_DMA_IRQHandler(spi_s25fl127.hdmatx);
}
void SPIx_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&spi_s25fl127);
}
#endif

uint8_t read_cmd[4];
void sFLASH_read_buffer_DMA(uint8_t* rxBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	HAL_StatusTypeDef err;

	read_cmd[0] = sFLASH_CMD_READ;
	read_cmd[1] = (ReadAddr & 0xFF0000) >> 16;
	read_cmd[2] = (ReadAddr& 0xFF00) >> 8;
	read_cmd[3] = ReadAddr & 0xFF;

	sflash_state = sFLASH_READCMD;
	select_chip();
	err = HAL_SPI_Transmit_DMA(&spi_s25fl127, read_cmd, 4);
	if (err != HAL_OK)
		sflash_error |= sFLASH_SPI_DMA_TX_ERROR;

	while (sflash_state != sFLASH_NOTBUSY)  { ; }

	sflash_state = sFLASH_READING;
	err = HAL_SPI_Receive_DMA(&spi_s25fl127, rxBuffer, NumByteToRead);
	if (err != HAL_OK)
		sflash_error |= sFLASH_SPI_DMA_RX_ERROR;
}



//
// Erases the specified FLASH sector.
//
void sFLASH_erase_sector(uint32_t SectorAddr)
{
	uint32_t aligned_addr;
	
	sFLASH_WriteEnable();

	select_chip();

	aligned_addr = sFLASH_align2sector(SectorAddr);

	if (aligned_addr < sFLASH_SPI_FIRST_64K_ADDR)
		sFLASH_SendByte(sFLASH_CMD_PE);
	else
		sFLASH_SendByte(sFLASH_CMD_SE);

	sFLASH_SendByte((aligned_addr & 0xFF0000) >> 16);
	sFLASH_SendByte((aligned_addr & 0xFF00) >> 8);
	sFLASH_SendByte(aligned_addr & 0xFF);

	deselect_chip();

	while (!sFLASH_is_chip_ready()) {;}
}

//
// Erases the specified FLASH sector, and returning immediately
//
// After issuing the erase command it does not wait until the flash chip is ready
// You MUST not do any more FLASH commands until sFLASH_is_chip_ready() returns true
//
// The purpose of this is to allow you to perform other tasks while the chip is erasing
// (typically 150ms per sector, but datasheet says max is 12 seconds!)
//
void sFLASH_erase_sector_background(uint32_t SectorAddr)
{
	uint32_t aligned_addr;

	sFLASH_WriteEnable();

	select_chip();

	aligned_addr = sFLASH_align2sector(SectorAddr);

	if (aligned_addr < sFLASH_SPI_FIRST_64K_ADDR)
		sFLASH_SendByte(sFLASH_CMD_PE);
	else
		sFLASH_SendByte(sFLASH_CMD_SE);

	sFLASH_SendByte((aligned_addr & 0xFF0000) >> 16);
	sFLASH_SendByte((aligned_addr & 0xFF00) >> 8);
	sFLASH_SendByte(aligned_addr & 0xFF);

	deselect_chip();
}


//
// Erases the entire FLASH.
// Must check the WIP flag before doing any reads or writes
//
void sFLASH_erase_chip(void)
{
	sFLASH_WriteEnable();
	select_chip();
	sFLASH_SendByte(sFLASH_CMD_BE);
	deselect_chip();
}

//
//  Writes more than one byte to the FLASH with a single WRITE cycle
//  The number of byte can't exceed the FLASH page size.
//
//  pBuffer: pointer to the buffer  containing the data to be written to the FLASH.
//  WriteAddr: FLASH's internal address to write to.
//  NumByteToWrite: number of bytes to write to the FLASH, must be equal or less than "sFLASH_SPI_PAGESIZE" value.
//
void sFLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	if (NumByteToWrite > sFLASH_SPI_PAGESIZE)
		NumByteToWrite = sFLASH_SPI_PAGESIZE;

	sFLASH_WriteEnable();

	select_chip();

	sFLASH_SendByte(sFLASH_CMD_WRITE);

	sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
	sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
	sFLASH_SendByte(WriteAddr & 0xFF);

	while (NumByteToWrite--)
	{
		sFLASH_SendByte(*pBuffer);
		pBuffer++;
	}

	deselect_chip();

	sFLASH_WaitForWriteEnd();
}

/**
  * @brief  Writes block of data to the FLASH. In this function, the number of
  *         WRITE cycles are reduced, using Page WRITE sequence.
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the FLASH.
  * @param  WriteAddr: FLASH's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the FLASH.
  * @retval None
  */
void sFLASH_write_buffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	Addr = WriteAddr % sFLASH_SPI_PAGESIZE;
	count = sFLASH_SPI_PAGESIZE - Addr;
	NumOfPage =  NumByteToWrite / sFLASH_SPI_PAGESIZE;
	NumOfSingle = NumByteToWrite % sFLASH_SPI_PAGESIZE;

	if (Addr == 0) // WriteAddr is sFLASH_SPI_PAGESIZE aligned
	{
		if (NumOfPage == 0) // NumByteToWrite < sFLASH_SPI_PAGESIZE
		{
			sFLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
		}
		else // NumByteToWrite > sFLASH_SPI_PAGESIZE 
		{
			while (NumOfPage--)
			{
				sFLASH_WritePage(pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
				WriteAddr +=  sFLASH_SPI_PAGESIZE;
				pBuffer += sFLASH_SPI_PAGESIZE;
			}

			sFLASH_WritePage(pBuffer, WriteAddr, NumOfSingle);
		}
	}
	else // WriteAddr is not sFLASH_SPI_PAGESIZE aligned 
	{
		if (NumOfPage == 0) // NumByteToWrite < sFLASH_SPI_PAGESIZE
		{
			if (NumOfSingle > count) // (NumByteToWrite + WriteAddr) > sFLASH_SPI_PAGESIZE
			{
				temp = NumOfSingle - count;

				sFLASH_WritePage(pBuffer, WriteAddr, count);
				WriteAddr +=  count;
				pBuffer += count;

				sFLASH_WritePage(pBuffer, WriteAddr, temp);
			}
			else
			{
				sFLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
			}
		}
		else // NumByteToWrite > sFLASH_SPI_PAGESIZE
		{
			NumByteToWrite -= count;
			NumOfPage =  NumByteToWrite / sFLASH_SPI_PAGESIZE;
			NumOfSingle = NumByteToWrite % sFLASH_SPI_PAGESIZE;

			sFLASH_WritePage(pBuffer, WriteAddr, count);
			WriteAddr +=  count;
			pBuffer += count;

			while (NumOfPage--)
			{
				sFLASH_WritePage(pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
				WriteAddr +=  sFLASH_SPI_PAGESIZE;
				pBuffer += sFLASH_SPI_PAGESIZE;
			}

			if (NumOfSingle != 0)
			{
				sFLASH_WritePage(pBuffer, WriteAddr, NumOfSingle);
			}
		}
	}
}

//
// Reads a block of data from the FLASH.
// pBuffer: pointer to the buffer that receives the data read from the FLASH.
// ReadAddr: FLASH's internal address to read from.
// NumByteToRead: number of bytes to read from the FLASH.
//
#ifdef FLASH_S25FL127_MODE_SPI
void sFLASH_read_buffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
  select_chip();

  sFLASH_SendByte(sFLASH_CMD_READ);

  sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  sFLASH_SendByte(ReadAddr & 0xFF);

  while (NumByteToRead--)
  {
    *pBuffer = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
    pBuffer++;
  }

  deselect_chip();
}
#endif

#ifdef FLASH_S25FL127_MODE_SPIDMA
void sFLASH_read_buffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	sFLASH_read_buffer_DMA(pBuffer, ReadAddr, NumByteToRead);
	while (get_flash_state() != sFLASH_NOTBUSY) {;}
}
#endif

uint32_t sFLASH_ReadID(void)
{
	uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

	select_chip();

	sFLASH_SendByte(sFLASH_CMD_RDID);

	Temp0 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
	Temp1 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
	Temp2 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

	deselect_chip();

	Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

	return Temp;
}

//
// Sends a byte through the SPI interface and returns the byte received from the SPI bus
//
#ifdef FLASH_S25FL127_MODE_SPI
uint8_t sFLASH_SendByte(uint8_t byte)
{
	uint8_t recd;

	//Send the byte
	*(__IO uint8_t *)&(flash_chip->SPIx)->DR = byte;

	
	//Wait for TX buffer to be empty
	while (!((flash_chip->SPIx->SR) & SPI_FLAG_TXE)) {;} // while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

	
	//Wait for RX buffer to be not empty
	// while (!((flash_chip->SPIx->SR) & SPI_FLAG_RXNE)) {;} // while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

	while (((flash_chip->SPIx->SR) & SPI_SR_BSY)) {;}

	recd = *(__IO uint8_t *)&(flash_chip->SPIx)->DR;

	//Return the received data
	return(recd);
}
#endif

//
// Sends a byte by bitbanging and returns the byte received
//
#ifdef FLASH_S25FL127_MODE_BITBANG
uint8_t sFLASH_SendByte(uint8_t byte)
{
	uint16_t recv=0;
	uint8_t bit_read=0;
	uint8_t i;

	//Setup: Clock low
	PIN_LOW(flash_chip->SCK.gpio, flash_chip->SCK.pin);

	//HAL_Delay(1);
	delay_32ns(100);

	for(i=0;i<8;i++)
	{
		//Setup DIN with next bit to send, MSB first
		if (byte & (1<<7)) 		PIN_HIGH(flash_chip->MOSI.gpio, flash_chip->MOSI.pin);
		else					PIN_LOW(flash_chip->MOSI.gpio, flash_chip->MOSI.pin);
		byte  <<= 1;

		//tSU = min 1.5ns 
		delay_32ns(2);
		//HAL_Delay(1);

		//Clock high
		PIN_HIGH(flash_chip->SCK.gpio, flash_chip->SCK.pin);

		//tH = min 10ns 
		delay_32ns(10);
		//HAL_Delay(1);

		//Read DOUT, MSB first
		bit_read = PIN_READ(flash_chip->MISO.gpio, flash_chip->MISO.pin);
		recv = (recv << 1) | bit_read;

		//Clock low
		PIN_LOW(flash_chip->SCK.gpio, flash_chip->SCK.pin);

		//tL Clock Low to Output Valid = 8ns max
		delay_32ns(8);
		//HAL_Delay(1);

	//	delay_32ns(2);
	}

	return recv;
}
#endif

#ifdef FLASH_S25FL127_MODE_SPIDMA
uint8_t sFLASH_SendByte(uint8_t byte)
{
	// uint8_t recd;

	// //Send the byte
	// *(__IO uint8_t *)&(flash_chip->SPIx)->DR = byte;

	
	// //Wait for TX buffer to be empty
	// while (!((flash_chip->SPIx->SR) & SPI_FLAG_TXE)) {;} // while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

	
	// //Wait for RX buffer to be not empty
	// // while (!((flash_chip->SPIx->SR) & SPI_FLAG_RXNE)) {;} // while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

	// while (((flash_chip->SPIx->SR) & SPI_SR_BSY)) {;}

	// recd = *(__IO uint8_t *)&(flash_chip->SPIx)->DR;

	// //Return the received data
	// return(recd);
}
#endif

//
// Enables the write access to the FLASH.
//
void sFLASH_WriteEnable(void)
{
	select_chip();

	sFLASH_SendByte(sFLASH_CMD_WREN);

	deselect_chip();
}

//
// Polls the status of the Write In Progress (WIP) flag in the FLASH's
// status register and loop until write operation has completed.
//
void sFLASH_WaitForWriteEnd(void)
{	
	uint8_t flashstatus = 0;

	select_chip();

	sFLASH_SendByte(sFLASH_CMD_RDSR);

	// Loop as long as the memory is busy with a write cycle
	do
	{
		// Send a dummy byte to generate the clock needed by the FLASH 
		flashstatus = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

		if (flashstatus & sFLASH_E_ERR_FLAG) 
			sflash_error |= sFLASH_ERASE_ERROR;

		if (flashstatus & sFLASH_P_ERR_FLAG) 
			sflash_error |= sFLASH_PROG_ERROR;

	}
	while ((flashstatus & sFLASH_WIP_FLAG) == SET); // Write in progress 

	deselect_chip();
}

//
// Returns 0 if chip is not ready (Write In Progress) or 1 if it's ready
//
uint8_t sFLASH_is_chip_ready(void)
{
	uint8_t flashstatus;

	select_chip();

	sFLASH_SendByte(sFLASH_CMD_RDSR);
	flashstatus = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

	deselect_chip();

	if (flashstatus & sFLASH_WIP_FLAG) return 0; //WIP is set ==> not ready
	else return 1; //WIP is cleared ==> ready
}

uint32_t sFLASH_align2sector(uint32_t addr)
{
	if (addr < sFLASH_SPI_FIRST_64K_ADDR)
		return( addr & 0xFFFFF000 );
	else if (addr < sFLASH_SIZE)
		return( addr & 0xFFFF0000 );
	else
		return 0; //error: address is past the size of the chip
}

//
// Given a sector number (0-271), Returns the start address of the sector
//
uint32_t sFLASH_get_sector_addr(uint16_t sector_num)
{
	if (sector_num < sFLASH_SPI_NUM_4K_SECTORS)
		return( sector_num * sFLASH_SPI_4K_SECTOR_SIZE );

	else if (sector_num < sFLASH_SPI_NUM_SECTORS)
		return( ((sector_num - sFLASH_SPI_NUM_4K_SECTORS) * sFLASH_SPI_64K_SECTOR_SIZE) + sFLASH_SPI_FIRST_64K_ADDR);
	else
		return 0;//invalid sector number
}

//
// Given a sector number (0-271), Returns the size sector in bytes
//
uint32_t sFLASH_get_sector_size(uint16_t sector_num)
{
	if (sector_num < sFLASH_SPI_NUM_4K_SECTORS)
		return sFLASH_SPI_4K_SECTOR_SIZE;

	else if (sector_num < sFLASH_SPI_NUM_SECTORS)
		return sFLASH_SPI_64K_SECTOR_SIZE;
	else
		return 0;//invalid sector number
}

//
// Given an address, returns the sector number which contains that address
//
uint16_t sFLASH_get_sector_num(uint32_t addr)
{

	if (addr < sFLASH_SPI_FIRST_64K_ADDR)
		return( (uint16_t)(addr / sFLASH_SPI_4K_SECTOR_SIZE) );

	else if (addr < sFLASH_SIZE)
		return( sFLASH_SPI_NUM_4K_SECTORS + (uint16_t)(addr / sFLASH_SPI_64K_SECTOR_SIZE) );
	else
		return 0;//invalid addr
}
 
uint32_t sFLASH_test_sector(uint32_t test_start)
{
	uint32_t i;
	uint8_t rd;
	uint32_t bad_bytes=0;

	//Align to sector start
	test_start = sFLASH_align2sector(test_start);

	//Erase sector
	sFLASH_erase_sector(test_start);

	//Write a page using low-level commands
	sFLASH_WriteEnable();
	select_chip();
	sFLASH_SendByte(sFLASH_CMD_WRITE);
	sFLASH_SendByte((test_start & 0xFF0000) >> 16);
	sFLASH_SendByte((test_start & 0xFF00) >> 8);
	sFLASH_SendByte(test_start & 0xFF);

	i=sFLASH_SPI_PAGESIZE;
	while (i--)
	{
		sFLASH_SendByte((0+i) & 0xFF);
	}
	deselect_chip();
	sFLASH_WaitForWriteEnd();

	//Read back the page using low-level commands

	select_chip();
	sFLASH_SendByte(sFLASH_CMD_READ);
	sFLASH_SendByte((test_start & 0xFF0000) >> 16);
	sFLASH_SendByte((test_start& 0xFF00) >> 8);
	sFLASH_SendByte(test_start & 0xFF);

	i=sFLASH_SPI_PAGESIZE;
	while (i--)
	{
		rd = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

		if (rd != ((0+i) & 0xFF))
			bad_bytes++;
	}
	deselect_chip();

	return bad_bytes;
}

//#define ENABLE_FLASH_BULK_WRITE_TEST
//#define ENABLE_FLASH_ERASE_CHIP_TEST

#ifdef ENABLE_FLASH_BULK_WRITE_TEST
	#define TDATASIZE 0x6C00
	uint8_t tdata[TDATASIZE];
#endif

uint32_t sFLASH_test(void)
{
	uint32_t i;
	uint32_t addr;
	uint32_t bad_bytes=0;

	#ifndef ENABLE_FLASH_BULK_WRITE_TEST

	//
	//test erasing all sectors, and writing/reading first page of each
	//
	for (i=0; i<sFLASH_SPI_NUM_SECTORS; i++)
	{
		addr = sFLASH_get_sector_addr(i);
		bad_bytes += sFLASH_test_sector(addr);
	}

	if (bad_bytes)
		return bad_bytes;

	#endif
	#ifdef ENABLE_FLASH_BULK_WRITE_TEST

		#ifdef ENABLE_FLASH_ERASE_CHIP_TEST
			//test erasing whole chip (30 seconds)
			sFLASH_erase_chip();
			while (!sFLASH_is_chip_ready()) {;}
		#endif

		//test writing big blocks
		// for (i=0;i<TDATASIZE;i++) tdata[i] = (i*15+0xa5) & 0xFF;
		for (i=0;i<TDATASIZE;i++) tdata[i] = (i) & 0xFF;

		addr = 0x1000;
		sFLASH_write_buffer(tdata, addr, TDATASIZE);

		for (i=0;i<TDATASIZE;i++) tdata[i] = 0;

	//bitbang @1.3MHz takes 26ms to read 4kB of data
	//SPI at prescale_4 takes 4.1ms to read 4kB of data
	//27kB (one 512 x 16bit x 3 x 3 x 3 sphere) takes 16ms to load
	// 27MHz clock (54MHz peripher clock / PRESCALE_2)
	// "off" time between bytes is 296ns, effectively making bit-rate 13.5MHz
	// 13.5Mbit/sec = 1648kBytes/sec ---> 27k in 16ms
		sFLASH_read_buffer(tdata, addr, TDATASIZE); 

		for (i=0;i<TDATASIZE;i++) 
		{
//			if ( tdata[i] != ((i*15+0xa5) & 0xFF) )
			if ( tdata[i] != ((i) & 0xFF))
				bad_bytes++;
		}

	#endif

	return bad_bytes;

}


