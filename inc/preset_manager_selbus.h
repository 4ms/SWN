#pragma once

#include <stm32f7xx.h>

void check_sel_bus_event(void);
void sel_bus_queue_recall_preset(uint8_t preset_num);
void sel_bus_queue_save_preset(uint8_t preset_num);

