
#include <stm32f7xx.h>

#define USING_FSK

#ifndef USING_FSK
	#define USING_QPSK
#endif

extern "C"{
#include "globals.h"
#include "bootloader.h"
#include "gpio_pins.h"
#include "led_colors.h"
#include "led_map.h"
#include "drivers/codec_i2c.h"
#include "drivers/codec_sai.h"
#include "drivers/pca9685_driver.h"
#include "drivers/mono_led_driver.h"
#include "hardware_controls.h"
#include "flash.h"
#include "bl_utils.h"
#include "bl_controls.h"
#include "hal_handlers.h"
#include "hardware_tests.h"

}

#ifdef USING_QPSK
	#include "stm_audio_bootloader/qpsk/packet_decoder.h"
	#include "stm_audio_bootloader/qpsk/demodulator.h"
#else
	#include "stm_audio_bootloader/fsk/packet_decoder.h"
	#include "stm_audio_bootloader/fsk/demodulator.h"
#endif

extern "C" {

using namespace stmlib;
using namespace stm_audio_bootloader;

const uint32_t SAMPLE_RATE = 48000;

#ifdef USING_QPSK
	const float kModulationRate = 6000.0;
	const float kBitRate = 12000.0;
	const float kSampleRate = 48000.0;
#endif
uint32_t kStartExecutionAddress =		0x08010000;
// uint32_t kStartReceiveAddress = 		0x08080000;
uint32_t kStartReceiveAddress = 		0x08010000;
uint32_t EndOfMemory =					0x080FFFFC;

extern const uint32_t FLASH_SECTOR_ADDRESSES[];
const uint32_t kBlockSize = 16384;
const uint16_t kPacketsPerBlock = kBlockSize / kPacketSize;
uint8_t recv_buffer[kBlockSize];

PacketDecoder decoder;
Demodulator demodulator;

uint16_t packet_index;
uint16_t old_packet_index=0;
uint8_t	slider_i=0;

uint16_t discard_samples = 8000;

uint32_t current_address;

enum UiState {
	UI_STATE_WAITING,
	UI_STATE_RECEIVING,
	UI_STATE_ERROR,
	UI_STATE_WRITING,
	UI_STATE_DONE
};
volatile UiState ui_state;

uint16_t manual_exit_primed;
uint8_t exit_updater;



extern const uint8_t led_outring_map[NUM_LED_OUTRING];
o_rgb_led rgb_red, rgb_off, rgb_green, rgb_yellow, rgb_cyan, rgb_magenta, rgb_blue;


void set_pwm_led_rgb(uint32_t led_id, o_rgb_led *rgb);
void set_pwm_led_rgb(uint32_t led_id, o_rgb_led *rgb) { LEDDriver_setRGBLED_RGB(led_id, rgb->c_red, rgb->c_green, rgb->c_blue); }

void set_pwm_led_element(uint32_t led_element_id, uint16_t brightness);
void set_pwm_led_element(uint32_t led_element_id, uint16_t brightness) { LEDDriver_set_single_LED(led_element_id, brightness); }

void update_LEDs(void)
{
	static uint16_t dly=0;
	uint16_t fade_speed=800;

	if (ui_state == UI_STATE_RECEIVING)
	{
		if (packet_index > old_packet_index)
		{
			// Sequence the slider LEDs when receiving
			// Turn on a new outer ring LED each time the slider animation repeats (every 6 packets)
			// Use a different color each time the outer ring animation repeats 

			old_packet_index = packet_index;

			mono_led_off(mledm_SLIDER_A);
			mono_led_off(mledm_SLIDER_B);
			mono_led_off(mledm_SLIDER_C);
			mono_led_off(mledm_SLIDER_D);
			mono_led_off(mledm_SLIDER_E);
			mono_led_off(mledm_SLIDER_F);

			if (++slider_i>=6) slider_i=0;
			mono_led_on(mledm_SLIDER_A + slider_i);

			if (((packet_index/6) % (NUM_LED_OUTRING*7) ) < (NUM_LED_OUTRING*1))
				set_pwm_led_rgb(led_outring_map[(packet_index/6) % NUM_LED_OUTRING], &rgb_green);
			else
			if (((packet_index/6) % (NUM_LED_OUTRING*7) ) < (NUM_LED_OUTRING*2))
				set_pwm_led_rgb(led_outring_map[(packet_index/6) % NUM_LED_OUTRING], &rgb_blue);
			else
			if (((packet_index/6) % (NUM_LED_OUTRING*7) ) < (NUM_LED_OUTRING*3))
				set_pwm_led_rgb(led_outring_map[(packet_index/6) % NUM_LED_OUTRING], &rgb_red);
			else
			if (((packet_index/6) % (NUM_LED_OUTRING*7) ) < (NUM_LED_OUTRING*4))
				set_pwm_led_rgb(led_outring_map[(packet_index/6) % NUM_LED_OUTRING], &rgb_yellow);
			else
			if (((packet_index/6) % (NUM_LED_OUTRING*7) ) < (NUM_LED_OUTRING*5))
				set_pwm_led_rgb(led_outring_map[(packet_index/6) % NUM_LED_OUTRING], &rgb_cyan);
			else
			if (((packet_index/6) % (NUM_LED_OUTRING*7) ) < (NUM_LED_OUTRING*6))
				set_pwm_led_rgb(led_outring_map[(packet_index/6) % NUM_LED_OUTRING], &rgb_magenta);
			else
				set_pwm_led_rgb(led_outring_map[(packet_index/6) % NUM_LED_OUTRING], &rgb_off);

		}
	}
	else if (ui_state == UI_STATE_WRITING)
	{
		//Flash all sliders when writing to FLASH
		if (dly++>200)
		{
			dly=0;
			mono_led_off(mledm_SLIDER_A);
			mono_led_off(mledm_SLIDER_B);
			mono_led_off(mledm_SLIDER_C);
			mono_led_off(mledm_SLIDER_D);
			mono_led_off(mledm_SLIDER_E);
			mono_led_off(mledm_SLIDER_F);
		} else if (dly==100)
		{
			mono_led_on(mledm_SLIDER_A);
			mono_led_on(mledm_SLIDER_B);
			mono_led_on(mledm_SLIDER_C);
			mono_led_on(mledm_SLIDER_D);
			mono_led_on(mledm_SLIDER_E);
			mono_led_on(mledm_SLIDER_F);
		}
	} 
	else if (ui_state == UI_STATE_WAITING)
	{
		mono_led_off(mledm_SLIDER_A);
		mono_led_off(mledm_SLIDER_B);
		mono_led_off(mledm_SLIDER_C);
		mono_led_off(mledm_SLIDER_D);
		mono_led_off(mledm_SLIDER_E);
		mono_led_off(mledm_SLIDER_F);

		//Flash button green/off when waiting
		if (dly==(fade_speed>>1))
		{
			set_pwm_led_rgb(ledm_A_BUTTON, &rgb_off);
		}
		if (dly++>=fade_speed) {
			dly=0;
			set_pwm_led_rgb(ledm_A_BUTTON, &rgb_green);
		}
	} 
	else if (ui_state == UI_STATE_DONE)
	{
		//Flash button blue/green when done
		if (dly==(fade_speed>>1)){
			set_pwm_led_rgb(ledm_A_BUTTON, &rgb_blue);
			set_pwm_led_rgb(ledm_B_BUTTON, &rgb_blue);
			set_pwm_led_rgb(ledm_C_BUTTON, &rgb_blue);
			set_pwm_led_rgb(ledm_D_BUTTON, &rgb_blue);
			set_pwm_led_rgb(ledm_E_BUTTON, &rgb_blue);
			set_pwm_led_rgb(ledm_F_BUTTON, &rgb_blue);
		}
		if (dly++>=fade_speed) {
			dly=0;
			set_pwm_led_rgb(ledm_A_BUTTON, &rgb_green);
			set_pwm_led_rgb(ledm_B_BUTTON, &rgb_green);
			set_pwm_led_rgb(ledm_C_BUTTON, &rgb_green);
			set_pwm_led_rgb(ledm_D_BUTTON, &rgb_green);
			set_pwm_led_rgb(ledm_E_BUTTON, &rgb_green);
			set_pwm_led_rgb(ledm_F_BUTTON, &rgb_green);
		}
	}

}


void check_exit_buttons(void)
{
	uint16_t t;
	static uint16_t State[2]={0};

	//Depressed adds a 0, released adds a 1

	if (rotary_is_pressed(rotm_WAVETABLE)) t=0xe000; else t=0xe001; //1110 0000 0000 000(0|1)
	State[0]=(State[0]<<1) | t;

	if (State[0] == 0xff00)  				//Released event (depressed followed by released)
		manual_exit_primed|=1;

	if (State[0] == 0xe00f){ 				 //Depressed event (released followed by a depressed)
		if (packet_index==0 && manual_exit_primed&1)
			exit_updater=1;
	}

	if (button_is_pressed(butm_A_BUTTON)) t=0xe000; else t=0xe001; //1110 0000 0000 000(0|1)
	State[1]=(State[1]<<1) | t;

	if (State[1] == 0xff00)  				//Released event (depressed followed by released)
		manual_exit_primed|=2;

	if (State[1] == 0xe00f){ 				 //Depressed event (released followed by a depressed)
		if ((ui_state == UI_STATE_WAITING) && manual_exit_primed&2)
			exit_updater=1;
	}


}

void InitializeReception(void)
{
	#ifdef USING_QPSK
		//QPSK
		decoder.Init((uint16_t)20000);
		demodulator.Init(
		 kModulationRate / kSampleRate * 4294967296.0,
		 kSampleRate / kModulationRate,
		 2.0 * kSampleRate / kBitRate);
		demodulator.SyncCarrier(true);
		decoder.Reset();
	#else
		//FSK
		decoder.Init();
		decoder.Reset();
		demodulator.Init(16, 8, 4); //pause, one, zero. pause_thresh = 24. one_thresh = 6.
		demodulator.Sync();
	#endif
	
	current_address = kStartReceiveAddress;
	packet_index = 0;
	old_packet_index = 0;
	slider_i = 0;
	ui_state = UI_STATE_WAITING;
}

void HAL_SYSTICK_Callback(void)
{
	update_LEDs();
	check_exit_buttons();
}


void init_pwm_leds(void)
{
	uint8_t i;

	init_color_palette();
	set_rgb_color(&rgb_red, ledc_RED);
	set_rgb_color(&rgb_green, ledc_MED_GREEN);
	set_rgb_color(&rgb_blue, ledc_MED_BLUE);
	set_rgb_color(&rgb_yellow, ledc_YELLOW);
	set_rgb_color(&rgb_magenta, ledc_PURPLE);
	set_rgb_color(&rgb_cyan, ledc_LIGHT_BLUE);
	set_rgb_color(&rgb_off, ledc_OFF);

	//Turn off all PWM LEDs to start
	for (i=0;i<NUM_LED_IDs;i++)	set_pwm_led_rgb(i, &rgb_off);
}


int main(void)
{
	uint32_t symbols_processed=0;
	uint32_t dly=0, button_debounce=0;
	uint8_t do_bootloader;
	uint8_t symbol;
	PacketDecoderState state;
	bool rcv_err;
	uint32_t last_flash;
	uint8_t i;

	SetVectorTable(0x08000000);

	HAL_Init();
	SystemClock_Config();

	init_SAI_clock(SAMPLE_RATE);
    codec_deinit();

	HAL_Delay(300);

	set_gpio_map();
	init_gpio_pins();

	LEDDriver_init_direct(10);
	init_pwm_leds();

	mono_led_on(mledm_SLIDER_A);
	mono_led_on(mledm_SLIDER_B);
	mono_led_on(mledm_SLIDER_C);
	mono_led_on(mledm_SLIDER_D);
	mono_led_on(mledm_SLIDER_E);
	mono_led_on(mledm_SLIDER_F);
	set_pwm_led_element(singleledm_CLKIN, 1000);
	set_pwm_led_rgb(ledm_AUDIOIN, &rgb_red);
	set_pwm_led_rgb(ledm_AUDIOIN, &rgb_green);
	set_pwm_led_rgb(ledm_AUDIOIN, &rgb_blue);

	mono_led_off(mledm_SLIDER_A);
	mono_led_off(mledm_SLIDER_B);
	mono_led_off(mledm_SLIDER_C);
	mono_led_off(mledm_SLIDER_D);
	mono_led_off(mledm_SLIDER_E);
	mono_led_off(mledm_SLIDER_F);
	set_pwm_led_element(singleledm_CLKIN, 0);
	set_pwm_led_rgb(ledm_AUDIOIN, &rgb_off);


	dly=32000;
	while(dly--){
		if (rotary_is_pressed(rotm_WAVETABLE)) button_debounce++;
		else button_debounce=0;
	}
	do_bootloader = (button_debounce>15000) ? 1 : 0;

	if (do_bootloader)
	{
		#ifdef USING_FSK
			InitializeReception(); //FSK
		#endif

		//Initialize Codec
		codec_GPIO_init();
		init_audio_DMA(SAMPLE_RATE);

		codec_I2C_init();
		codec_register_setup(SAMPLE_RATE);

		//Begin audio DMA
		set_audio_callback(&process_audio_block_codec_bootloader);
		start_audio();

		HAL_Delay(300);

		while(rotary_is_pressed(rotm_WAVETABLE)) {;}

		//loop until check_exit_buttons() detects a button press and sets exit_updated=true
		while (!exit_updater)
		{
			rcv_err = false;

			while (demodulator.available() && !rcv_err && !exit_updater) {
				symbol = demodulator.NextSymbol();
				state = decoder.ProcessSymbol(symbol);
				symbols_processed++;

				switch (state) {
					case PACKET_DECODER_STATE_OK:
					{
						ui_state = UI_STATE_RECEIVING;
						memcpy(recv_buffer + (packet_index % kPacketsPerBlock) * kPacketSize, decoder.packet_data(), kPacketSize);
						++packet_index;
						if ((packet_index % kPacketsPerBlock) == 0) {
							ui_state = UI_STATE_WRITING;

							//Check for valid flash address before writing to flash
							if ((current_address + kBlockSize) < FLASH_SECTOR_ADDRESSES[NUM_FLASH_SECTORS])
							{
								write_flash_page(recv_buffer, current_address, kBlockSize);
								current_address += kBlockSize;
							}
							else {
								ui_state = UI_STATE_ERROR;
								set_pwm_led_rgb(ledm_A_BUTTON, &rgb_red);
								set_pwm_led_rgb(ledm_B_BUTTON, &rgb_red);
								set_pwm_led_rgb(ledm_C_BUTTON, &rgb_red);
								rcv_err = true;
							}

							decoder.Reset();

							#ifndef USING_QPSK
								demodulator.Sync(); //FSK
							#else
								demodulator.SyncCarrier(false);//QPSK
							#endif

						} else {
							#ifndef USING_QPSK
								decoder.Reset(); //FSK
							#else
								demodulator.SyncDecision();//QPSK
							#endif
						}
					}
					break;

					case PACKET_DECODER_STATE_ERROR_SYNC:
						set_pwm_led_rgb(ledm_A_BUTTON, &rgb_red);
						set_pwm_led_rgb(ledm_B_BUTTON, &rgb_red);
						set_pwm_led_rgb(ledm_C_BUTTON, &rgb_off);
						rcv_err = true;
						break;

					case PACKET_DECODER_STATE_ERROR_CRC:
						set_pwm_led_rgb(ledm_A_BUTTON, &rgb_red);
						set_pwm_led_rgb(ledm_B_BUTTON, &rgb_off);
						set_pwm_led_rgb(ledm_C_BUTTON, &rgb_red);
						rcv_err = true;
						break;

					case PACKET_DECODER_STATE_END_OF_TRANSMISSION:
						//Copy from Receive buffer to Execution memory
						//copy_flash_page(kStartReceiveAddress, kStartExecutionAddress, (current_address-kStartReceiveAddress));

						exit_updater = true;
						ui_state = UI_STATE_DONE;

						//Do a success animation

						while (!rotary_is_pressed(rotm_WAVETABLE))
						{
							set_pwm_led_rgb(led_outring_map[i%NUM_LED_OUTRING], &rgb_off);
							set_pwm_led_rgb(led_outring_map[NUM_LED_OUTRING-(i%NUM_LED_OUTRING)], &rgb_off);
							
							i++;
							set_pwm_led_rgb(led_outring_map[i%NUM_LED_OUTRING], &rgb_green);
							set_pwm_led_rgb(led_outring_map[NUM_LED_OUTRING-(i%NUM_LED_OUTRING)], &rgb_blue);

							HAL_Delay(100);
						}
						while (rotary_is_pressed(rotm_WAVETABLE)){;} //wait until button is released


						break;

					default:
						break;
				}
			}
			if (rcv_err) {
				ui_state = UI_STATE_ERROR;

				//flash button red/off until it's pressed
				last_flash = HAL_GetTick();

				while (!button_is_pressed(butm_A_BUTTON))
				{
					if (HAL_GetTick() - last_flash >= 100){ 
						set_pwm_led_rgb(ledm_A_BUTTON, &rgb_off);
					}
					if (HAL_GetTick() - last_flash >= 200){ 
						last_flash = HAL_GetTick(); 
						set_pwm_led_rgb(ledm_A_BUTTON, &rgb_red);
					}
				}
				while (button_is_pressed(butm_A_BUTTON)){;} //wait until button is released

				set_pwm_led_rgb(ledm_A_BUTTON, &rgb_off);
				set_pwm_led_rgb(ledm_B_BUTTON, &rgb_off);
				set_pwm_led_rgb(ledm_C_BUTTON, &rgb_off);

				InitializeReception();

				//do not exit bootloader if rotary is pressed after an error
				manual_exit_primed=0;
				exit_updater=false;
			}
		}

		codec_power_down();
		codec_deinit();
	}
	HAL_DeInit();
	HAL_RCC_DeInit();

	JumpTo(0x00210000);


	return 1;
}

void process_audio_block_codec_bootloader(int32_t *src, int32_t *dst)
{
	uint32_t i;
	bool sample;
	static bool last_sample=false;
	int32_t input, in_check;

	for (i=0;i<codec_HT_CHAN_LEN;i++)
	{
		input = *src; 	// Left in
		src+=2; 			// Right in
		in_check = (int32_t)((uint32_t)input<<8); //24bit to 32bit conversion
		in_check >>= 16; //back to 16bit, but now signed

		if (last_sample==true){
			if (in_check < -300)
				sample=false;
			else
				sample=true;
		} else {
			if (in_check > 400)
				sample=true;
			else
				sample=false;
		}
		last_sample=sample;

		if (!discard_samples) {
			#ifdef USING_FSK
			demodulator.PushSample(sample);
			#else
			demodulator.PushSample(in_check);
			#endif
		} else {
			--discard_samples;
		}


		if (ui_state == UI_STATE_ERROR)
		{
			*dst++=0;
			*dst++=0;
		}
		else
		{
			*dst++=input;
			*dst++=input;
		}
	}
}


} //extern "C"
