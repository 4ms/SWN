/*
 * flash.h
 *
 *  Created on: Mar 8, 2016
 *      Author: Dan Green <danngreen1@gmail.com>
 */

#pragma once

#include <stm32f7xx.h>

#define NUM_FLASH_SECTORS 8


void flash_begin_open_program(void);
void flash_open_erase_sector(uint32_t address);
uint8_t flash_open_program_byte(uint8_t byte, uint32_t address);
uint8_t flash_open_program_word(uint32_t word, uint32_t address);
uint8_t flash_open_program_block_bytes(uint8_t* arr, uint32_t address, uint32_t size);
uint8_t flash_open_program_block_words(uint32_t* arr, uint32_t address, uint32_t size);
void flash_end_open_program(void);

void flash_erase_sector(uint32_t address);
void flash_read_array(uint8_t* arr, uint32_t address, uint32_t size);
uint32_t flash_read_word(uint32_t address);
uint8_t flash_read_byte(uint32_t address);
