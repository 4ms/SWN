#include "sel_bus.h"
#include "drivers/uart_driver.h"
#include "preset_manager_selbus.h"

UART_HandleTypeDef *midiUART;

enum { recallPreset, savePreset };

static const uint32_t kBaudRate = 31250;
static const uint8_t kMIDICommandControlChange = 0xB0;
static const uint8_t kMIDICCNumAssignSaveRecall = 16;
static const uint8_t kMIDICCValChooseSave = 127;

static const uint8_t kMIDICommandMalekkoMakeNoiseSaveRecall = 0xC0;
static const uint8_t kMIDICommandMakeNoiseSave = 0xF4; 		//https://www.makenoisemusic.com/content/manuals/tempimanual.pdf page 32, "State Save"

static uint8_t midiBuffer[3] = {0, 0, 0};
static uint8_t byteCount = 0;
static uint8_t saveRecall = recallPreset;
static uint8_t midiByte = 0;

void selBus_Init(void)
{
	midiUART = UART_Init(kBaudRate);
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

	if (byteCount == 3 && midiBuffer[0] == kMIDICommandControlChange)
	{
		if (midiBuffer[1] == kMIDICCNumAssignSaveRecall)
		{
			if (midiBuffer[2] == kMIDICCValChooseSave) {
				saveRecall = savePreset;
			}
			else {
				saveRecall = recallPreset;
			}
		}
	}

	if (byteCount == 2 && midiBuffer[0] == kMIDICommandMalekkoMakeNoiseSaveRecall)
	{
		uint8_t presetNum = midiBuffer[1];
		if (saveRecall == savePreset) {
			sel_bus_queue_save_preset(presetNum);
		}
		else if (saveRecall == recallPreset) {
			sel_bus_queue_recall_preset(presetNum);
		}
	}

	if (byteCount == 2 && midiBuffer[0] == kMIDICommandMakeNoiseSave)
	{
		uint8_t presetNum = midiBuffer[1];
		sel_bus_queue_save_preset(presetNum);
	}

	selBus_Start();
}

//Tests:
//0xC0 0x02 --> loads preset 2
//0xC0 0x02, 0xC0 0x03 --> loads preset 2 then 3
//0x02 0xC0 0xC0 0x01 --> loads preset 1
//0xB0 0x01 0xC0 0x03 --> loads preset 3
//0xB0 0x01 0xC0 0xB0 0x01 --> no loading
//0xB0 0xB0 0xC0 0x01 --> loads preset 1
//0xB0 0xB0 0x10 0xC0 0x02 --> loads preset 2
//0xB0 0x10 0x7F 0xC0 0x02 --> saves preset 2
//0xB0 0x10 0x7F, 0x05 0x05, 0xC0 0x02 --> saves preset 2
//0xB0 0x10 0x7F 0xB0 0x10 0x7E 0xC0 0x01 --> loads preset 1
//
//0xF4 0x01 --> save preset 1
//0xF4 0xB0 0x10 0x01 0xC0 0x04 -> save preset 4
//0xB0 0x10 0x01 0xF4 0x10 -> save preset 16
//0xB0 0x10 0x7F, 0xC0 0x10 -> save preset 16 (Would be a bug if the 0xB0 0x10 0x7F is a malekko requesting a save, and 0xC0 0x10 is a makenoise device requesting a load)
//
