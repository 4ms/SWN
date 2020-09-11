#pragma once

#include <stdint.h>

void check_sel_bus_event(void);
void sel_bus_queue_recall_preset(uint8_t preset_num);
void sel_bus_queue_save_preset(uint8_t preset_num);
void sel_bus_toggle_recall_allow(void);
void sel_bus_toggle_save_allow(void);

