#include "flash_storage.hh"
#include "persistent_storage.hh"
#include <stdint.h>
extern "C" {
#include "external_flash_layout.h"
#include "timekeeper.h"
}

extern "C" uint16_t get_startup_preset();
extern "C" void set_startup_preset(uint16_t preset_num);
extern "C" void init_startup_preset_storage(void);

static const uint16_t CHECK_WORD = 0xAA55;
struct StartupPreset {
	uint16_t check_word;
	uint16_t preset_num;

	bool validate()
	{
		if (check_word != CHECK_WORD)
			return false;
		if (preset_num >= MAX_PRESETS)
			return false;
		return true;
	}
};

static StartupPreset startup_preset;
static StartupPreset default_startup_preset = {CHECK_WORD, 0};
static Persistent<WearLevel<FlashStorage<STARTUP_PRESET_SETTING_SECTOR, StartupPreset>>>
	startup_preset_storage{&startup_preset};

extern "C" void init_startup_preset_storage(void)
{
	pause_timer_IRQ(OSC_TIM_number);
	pause_timer_IRQ(WT_INTERP_TIM_number);
	pause_timer_IRQ(PWM_OUTS_TIM_number);
	startup_preset_storage.init(default_startup_preset);
	resume_timer_IRQ(OSC_TIM_number);
	resume_timer_IRQ(WT_INTERP_TIM_number);
	resume_timer_IRQ(PWM_OUTS_TIM_number);
}
extern "C" void set_startup_preset(uint16_t preset_num)
{
	startup_preset.preset_num = preset_num;
	startup_preset.check_word = CHECK_WORD;

	pause_timer_IRQ(OSC_TIM_number);
	pause_timer_IRQ(WT_INTERP_TIM_number);
	pause_timer_IRQ(PWM_OUTS_TIM_number);
	startup_preset_storage.Save();
	resume_timer_IRQ(OSC_TIM_number);
	resume_timer_IRQ(WT_INTERP_TIM_number);
	resume_timer_IRQ(PWM_OUTS_TIM_number);
}
extern "C" uint16_t get_startup_preset()
{
	return startup_preset.preset_num;
}

