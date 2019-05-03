
/*


*/

#pragma once

#include <stm32f7xx.h>

typedef struct s25fl127SpiPin{
	uint16_t		pin;
	GPIO_TypeDef 	*gpio;
	uint32_t		gpio_clk;
	uint8_t			af;
} s25fl127SpiPin;


typedef struct S25FL127Chip
{
	SPI_TypeDef 			*SPIx;
	s25fl127SpiPin			SCK;
	s25fl127SpiPin			MISO;
	s25fl127SpiPin			MOSI;
	s25fl127SpiPin			CS;
	s25fl127SpiPin			IO2;
	s25fl127SpiPin			IO3;

} S25FL127Chip;


//Chip commands:

#define sFLASH_CMD_WREN			0x06  // Write enable instruction
#define sFLASH_CMD_WRDIS		0x04  // Write disable instruction

#define sFLASH_CMD_WRITE		0x02  // Page Program instruction
#define sFLASH_CMD_READ			0x03  // Read from Memory instruction
#define sFLASH_CMD_FASTREAD		0x0B  // Read fast from Memory instruction

#define sFLASH_CMD_WRSR			0x01  // Write Status Register instruction
#define sFLASH_CMD_RDSR			0x05  // Read Status Register instruction 

#define sFLASH_CMD_RDID			0x9F  // Read identification

#define sFLASH_CMD_PE			0x20  // Param Sector Erase 4kB (3-byte address)
#define sFLASH_CMD_SE			0xD8  // Sector Erase 64kB or 256kB (3-byte address)
#define sFLASH_CMD_BE			0x60  // Bulk Erase instruction 
#define sFLASH_CMD_BE_alt		0xC7  // Bulk Erase instruction 


#define sFLASH_DUMMY_BYTE		0xA5

#define sFLASH_SPI_PAGESIZE		0x100 


//Status register reads:
#define sFLASH_WIP_FLAG			0x01  // Write In Progress (WIP) flag
#define sFLASH_WLE_FLAG			0x02  // Write Enable Latch
#define sFLASH_BP0_FLAG			0x04  // Block protect
#define sFLASH_BP1_FLAG			0x08  // Block protect
#define sFLASH_BP2_FLAG			0x10  // Block protect
#define sFLASH_E_ERR_FLAG		0x20  // Erase Error Flag (E_ERR)
#define sFLASH_P_ERR_FLAG		0x40  // Program Error Flag (P_ERR)
#define sFLASH_SRWD_FLAG		0x80  // Status Register Disable

//Sectors:
#define sFLASH_SIZE						0x01000000 // 128Mbit = 16MByte 

#define sFLASH_SPI_4K_SECTOR_SIZE		0x00001000 // 4kB sectors
#define sFLASH_SPI_NUM_4K_SECTORS		16

#define sFLASH_SPI_64K_SECTOR_SIZE		0x00010000 // 64kB sectors
#define sFLASH_SPI_NUM_64K_SECTORS		255

#define sFLASH_SPI_NUM_SECTORS			(sFLASH_SPI_NUM_4K_SECTORS + sFLASH_SPI_NUM_64K_SECTORS)

#define sFLASH_SPI_FIRST_64K_ADDR       0x00010000

//4kB sectors:
//Sector 00: 0x00000000 - 0x00000FFF 
//Sector 01: 0x00001000 - 0x00001FFF 
//...
//Sector 15: 0x0000F000 - 0x0000FFFF 
//
//64kB sectors:
//Sector 16: 0x00010000 - 0x0001FFFF
//Sector 17: 0x00020000 - 0x0002FFFF
//...
//Sector269: 0x00FE0000 - 0X00FEFFFF
//Sector270: 0x00FF0000 - 0x00FFFFFF

enum sFlashErrors{
	sFLASH_NO_ERROR			= 0,
	sFLASH_ERASE_ERROR 		= (1<<0),
	sFLASH_PROG_ERROR 		= (1<<1),
	sFLASH_SPI_INIT_ERROR 	= (1<<2)
};


//Initialize
void sFLASH_init(void);

//Chip status
uint8_t sFLASH_is_chip_ready(void);

//Erasing
void sFLASH_erase_sector(uint32_t SectorAddr);
void sFLASH_erase_sector_background(uint32_t SectorAddr);
void sFLASH_erase_chip(void);

//Reading and writing
void sFLASH_write_buffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void sFLASH_read_buffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);


//Sector/address utilities
uint32_t sFLASH_get_sector_addr(uint16_t sector_num);
uint16_t sFLASH_get_sector_num(uint32_t addr);
uint32_t sFLASH_get_sector_size(uint16_t sector_num);
uint32_t sFLASH_align2sector(uint32_t addr);

//Testing routines
uint32_t sFLASH_test(void);
uint32_t sFLASH_test_sector(uint32_t test_start);
