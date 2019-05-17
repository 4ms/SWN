
#include "bootloader_utils.h"
#include "hardware_controls.h"

extern o_rotary 	rotary[NUM_ROTARIES];

uint8_t check_bootloader_keys(void)
{
	uint32_t dly;
	uint32_t button_debounce=0;

	dly=32000;
	while(dly--){
		if (rotary[rotm_WAVETABLE].hwswitch.gpio->IDR & rotary[rotm_WAVETABLE].hwswitch.pin) 
			button_debounce++;
		else button_debounce=0;
	}
	return (button_debounce>15000) ? 1 : 0;

}

typedef void (*EntryPoint)(void);

void JumpTo(uint32_t address) {
  uint32_t application_address = *(__IO uint32_t*)(address + 4);
  EntryPoint application = (EntryPoint)(application_address);
  __set_MSP(*(__IO uint32_t*)address);
  application();
}

void SetVectorTable(uint32_t reset_address)
{ 
	SCB->VTOR = reset_address & (uint32_t)0x1FFFFF80;
}
