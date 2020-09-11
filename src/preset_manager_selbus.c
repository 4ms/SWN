#include "preset_manager_selbus.h"
#include "preset_manager.h"
#include "preset_manager_UI.h"
#include "system_settings.h"

extern o_systemSettings	system_settings;

uint32_t queued_preset_num = MAX_PRESETS + 1;
enum {
	NO_PRESET_ACTION,
	STORE_PRESET,
	RECALL_PRESET
} queued_preset_action = NO_PRESET_ACTION;

void check_sel_bus_event(void)
{
	if (queued_preset_action == STORE_PRESET) {
		queued_preset_action = NO_PRESET_ACTION;
		store_preset_from_active(queued_preset_num);
	}
	if (queued_preset_action == RECALL_PRESET) {
		queued_preset_action = NO_PRESET_ACTION;
		recall_preset_into_active(queued_preset_num);
	}
}

static void queue_recall_preset(uint32_t preset_num)
{
	queued_preset_action = RECALL_PRESET;
	queued_preset_num = preset_num;
}

static void queue_store_preset(uint32_t preset_num)
{
	queued_preset_action = STORE_PRESET;
	queued_preset_num = preset_num;
}

void sel_bus_queue_recall_preset(uint8_t preset_num)
{
	if (system_settings.selbus_can_recall != SELBUS_RECALL_ENABLED)
		return;

	if (preset_num >= MAX_PRESETS)
		return;

	char version;
	if (!check_preset_filled(preset_num, &version))
		return;

	queue_recall_preset(preset_num);
}

void sel_bus_queue_save_preset(uint8_t preset_num)
{
	if (system_settings.selbus_can_save != SELBUS_SAVE_ENABLED)
		return;

	if (preset_num >= MAX_PRESETS)
		return;

	queue_store_preset(preset_num);
}

void sel_bus_toggle_recall_allow(void)
{
	if (system_settings.selbus_can_recall == SELBUS_RECALL_DISABLED)
		system_settings.selbus_can_recall = SELBUS_RECALL_ENABLED;
	else
		system_settings.selbus_can_recall = SELBUS_RECALL_DISABLED;
}

void sel_bus_toggle_save_allow(void)
{
	if (system_settings.selbus_can_save == SELBUS_SAVE_DISABLED)
		system_settings.selbus_can_save = SELBUS_SAVE_ENABLED;
	else
		system_settings.selbus_can_save = SELBUS_SAVE_DISABLED;
}

