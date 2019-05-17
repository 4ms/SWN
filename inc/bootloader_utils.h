
#pragma once

#include <stm32f7xx.h>


uint8_t check_bootloader_keys(void);

void JumpTo(uint32_t address);

void SetVectorTable(uint32_t reset_address);

