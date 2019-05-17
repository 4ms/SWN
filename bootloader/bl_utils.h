#pragma once

#include <stm32f7xx.h>

void SystemClock_Config(void);
void SetVectorTable(uint32_t reset_address);
void JumpTo(uint32_t address);
void *memcpy(void *dest, const void *src, size_t n);

void write_flash_page(const uint8_t* data, uint32_t dst_addr, uint32_t bytes_to_write);
void copy_flash_page(uint32_t src_addr, uint32_t dst_addr, uint32_t bytes_to_copy);
