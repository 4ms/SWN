#include "sel_bus.h"
#include "drivers/uart_driver.h"
#include "preset_manager_selbus.h"

UART_HandleTypeDef *midiUART;

enum { recallPreset,
	   savePreset };

static uint8_t midiBuffer[3] = {0, 0, 0};
static uint8_t byteCount = 0;
static uint8_t saveRecall = recallPreset;
static uint8_t midiByte = 0;

void selBus_Init(void)
{
	midiUART = UART_Init(31250);
}

void selBus_Start(void)
{
	UART_Start(&midiByte, 1);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	(void)huart;
}

void UART5_IRQHandler(void)
{
	if (midiUART == (UART_HandleTypeDef *)0)
		return;

	HAL_UART_IRQHandler(midiUART);

	if (midiByte > 127) // Command byte
	{
		midiBuffer[0] = midiByte;
		byteCount = 1;
	}
	else // Data byte
	{
		midiBuffer[byteCount] = midiByte;
		byteCount++;
	}

	if (byteCount == 3 && midiBuffer[0] == 0b10110000) //control change received
	{
		if (midiBuffer[1] == 16) // CC 16 received, save/recall assignment
		{
			if (midiBuffer[2] == 127) {
				saveRecall = savePreset;
			}
			else {
				saveRecall = recallPreset;
			}
		}
	}

	if (byteCount == 2 && midiBuffer[0] == 0b11000000) // program change received
	{
		uint8_t presetNum = midiBuffer[1];
		if (saveRecall == savePreset) {
			sel_bus_queue_save_preset(presetNum);
		}
		else if (saveRecall == recallPreset) {
			sel_bus_queue_recall_preset(presetNum);
		}
	}
	UART_Start(&midiByte, 1);
}
