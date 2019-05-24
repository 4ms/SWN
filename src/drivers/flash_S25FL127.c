
#include "drivers/flash_S25FL127.h"


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
