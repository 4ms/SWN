/*
 * flash.c -
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
#include "flash.h"

//FixMe: Check if our chip has 1MB or 2MB FLASH
//The following assumes 1MB FLASH. Also assumes not dual bank mode (FLASH_OPTCR->nDBANK==0) for F7 chips only

// #if defined STM32F427xx

// static uint32_t FLASH_SECTOR_ADDRESSES[NUM_FLASH_SECTORS+1] = {
// 	0x08000000,
// 	0x08004000,
// 	0x08008000,
// 	0x0800C000,
// 	0x08010000,
// 	0x08020000,
// 	0x08040000,
// 	0x08060000,
// 	0x08080000,
// 	0x080A0000,
// 	0x080C0000,
// 	0x080E0000,
// 	0x08100000
// };

// #define SRAM_OFFSET  0

// #elif defined STM32F765xx

const uint32_t FLASH_SECTOR_ADDRESSES[NUM_FLASH_SECTORS+1] = {
	0x08000000, /* bootloader: 			0x08000000 - 0x08007FFF (32kB) 	*/
	0x08008000,	/* system settings:		0x08008000 - 0x0800FFFF (32kB) 	*/
	0x08010000,	/* main app: 			0x08010000 - 0x0807FFFF (448kB)	*/
	0x08018000, /* |													*/
	0x08020000, /* |													*/
	0x08040000, /* |													*/
	0x08080000, /* receiving firmware:  0x08080000 - 0x0080FFFF (512kB)	*/
	0x080C0000, /* |													*/
	0x08100000, /* end of memory + 1 */
};

#define SRAM_OFFSET  0x07E00000 /* (0x08000000 - 0x00200000) */

// #endif


void flash_erase_sector(uint32_t address)
{
	uint8_t i;

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_ERSERR);

	for (i = 0; i < NUM_FLASH_SECTORS; i++) {
		if (address == FLASH_SECTOR_ADDRESSES[i]) {
			FLASH_Erase_Sector(i, FLASH_VOLTAGE_RANGE_3);
			break;
		}
	}
	HAL_FLASH_Lock();
}

//if address is the start of a sector, erase it
//otherwise do nothing
void flash_open_erase_sector(uint32_t address)
{
	uint8_t i;

	for (i = 0; i < NUM_FLASH_SECTORS; i++) {
		if (address == FLASH_SECTOR_ADDRESSES[i]) {
		  FLASH_Erase_Sector(i, FLASH_VOLTAGE_RANGE_3);
		  break;
		}
	}
}

void flash_begin_open_program(void)
{
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_ERSERR);
}

uint8_t flash_open_program_byte(uint8_t byte, uint32_t address)
{
	return HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, byte);
}

uint8_t flash_open_program_word(uint32_t word, uint32_t address)
{
	return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, word);
}

void flash_end_open_program(void)
{
	HAL_FLASH_Lock();
}


//size is in # of bytes
uint8_t flash_open_program_block_bytes(uint8_t* arr, uint32_t address, uint32_t size)
{
	uint8_t status=0;

	while(size--) {
		status |= HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, *arr++);
		address++;
	}
	return status;
}

//size is in # of 32-bit words
uint8_t flash_open_program_block_words(uint32_t* arr, uint32_t address, uint32_t size)
{
	uint8_t status=0;

	while(size--) {
		status |= HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *arr++);
		address+=4;
	}
	return status;
}

//size in # of bytes
void flash_read_array(uint8_t* arr, uint32_t address, uint32_t size)
{

	while(size--) {
		*arr++ = (uint8_t)(*(__IO uint32_t*)address);
		address++;
	}
}

uint32_t flash_read_word(uint32_t address)
{
    return( *(__IO uint32_t*)address);
}

uint8_t flash_read_byte(uint32_t address)
{
    return((uint8_t) (*(__IO uint32_t*)address));
}

