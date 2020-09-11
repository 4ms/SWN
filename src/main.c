/*
 * main.c - Spherical Wavetable Navigator main
 *
 * Author: Dan Green (danngreen1@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * See http://creativecommons.org/licenses/MIT/ for more information.
 *
 * -----------------------------------------------------------------------------
 */


//Set to 0 if no bootloader (must also change the .ld file to load into 0x08000000 instead of 0x08010000)
//Set to 1 if there is a bootloader
#define HAS_BOOTLOADER 1
#define HAS_INTERNAL_FLASH 1

#include <stm32f7xx.h>

#include "drivers/codec_i2c.h"
#include "drivers/codec_sai.h"
#include "audio_util.h"
#include "gpio_pins.h"
#include "globals.h"
#include "oscillator.h"
#include "envout_pwm.h"
#include "params_update.h"
#include "flash_params.h"
#include "led_cont.h"
#include "led_colors.h"
#include "adc_interface.h"
#include "drivers/leds_pwm.h"
#include "timekeeper.h"
#include "drivers/mono_led_driver.h"
#include "hardware_tests.h"
#include "hal_handlers.h"
#include "sphere_flash_io.h"
#include "system_settings.h"
#include "preset_manager.h"
#include "preset_manager_UI.h"
#include "preset_manager_selbus.h"
#include "compressor.h"
#include "ui_modes.h"
#include "quantz_scales.h"
#include "calibrate_voct.h"
#include "params_lfo.h"
#include "led_color_adjust.h"
#include "sphere.h"
#include "wavetable_recording.h"
#include "wavetable_editing.h"
#include "wavetable_saveload.h"
#include "analog_conditioning.h"
#include "UI_conditioning.h"
#include "drivers/flashram_spidma.h"
#include "sel_bus.h"



//Private functions:
void SystemClock_Config(void);
void SetVectorTable(uint32_t reset_address);

extern enum 	UI_Modes ui_mode;

extern SystemCalibrations *system_calibrations;

int main(void)
{
	uint32_t valid_fw_version;

	//SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk; // enable Usage-/Bus-/MPU Fault

	//
	// System Init: Interrupt vector table location, HAL (systick) and System Clocks
	//
	SetVectorTable(HAS_BOOTLOADER ? 0x00210000 : 0x00200000);

	HAL_Init();
	SystemClock_Config();
	//FLASH_OB_BootAddressConfig(OPTIONBYTE_BOOTADDR_0, OB_BOOTADDR_ITCM_FLASH);

	SCB_InvalidateICache();
	SCB_EnableICache();
	//SCB_DisableICache(); //not needed because we're running from FLASH on the ITCM bus, using ART and Prefetch

	SCB_InvalidateDCache();
	SCB_EnableDCache();

	__HAL_RCC_DMA2_CLK_DISABLE();

	// Setup PLL clock for codec
   	init_SAI_clock(SAMPLERATE);

   	//De-init the codec to force it to reset
    codec_deinit();
	HAL_Delay(80);

	// INITIALIZATIONS

	set_gpio_map();
	init_gpio_pins();

	if (key_combo_enter_hardwaretest() || !is_hardwaretest_already_done() || FORCE_HW_TEST)
	{
		do_hardware_test();
		while(1){;} //force reboot after hw test
	}

	init_timekeeper();

	init_pwm_leds();

	HAL_Delay(80);

	selBus_Init();

	// Initialize starting values
	init_color_palette();
	init_led_cont();
	init_led_cont_ongoing_display();
	clear_lfo_locks();
	use_internal_lfo_base();
	init_lfos();
	init_encoders();
	init_lfo_to_vc_mode();

	//External FLASH
	sFLASH_init();


	//Initialize param values (do not start updating them yet)
	init_wt_osc();
	init_params();
	init_pitch_params();
	init_quantz_scales();

	cache_uncache_keys_params_and_lfos(0, CACHE);
	cache_uncache_keys_params_and_lfos(1, CACHE);
	cache_uncache_keys_params_and_lfos(2, CACHE);
	cache_uncache_keys_params_and_lfos(3, CACHE);
	cache_uncache_keys_params_and_lfos(4, CACHE);
	cache_uncache_keys_params_and_lfos(5, CACHE);

	//Start outputing on the PWM OUT jacks
	init_envout_pwm();

	//start displaying mono leds
	start_monoled_updates();

	init_sphere_flash();

#ifdef ERASE_ALL_WAVETABLES
	for (uint8_t ww=0; ww<12; ww++)
		sFLASH_erase_sector( sFLASH_get_sector_addr(WT_SECTOR_START+ww) );
#endif

#ifdef CLEAR_USER_SPHERES_FROM_FLASH
	empty_all_user_spheres();
#endif

#ifdef FORCE_WRITE_FACTORY_SPHERES
	write_factory_spheres_to_extflash();
#endif

	// Load system_calibrations from internal flash if it's present
	if (HAS_INTERNAL_FLASH)
	{
		valid_fw_version = load_flash_params();

		if (!valid_fw_version)
		{
			factory_reset_all_calibrations();
			factory_reset();
			write_factory_spheres_to_extflash();
		}
	}
	else
		factory_reset_all_calibrations();

	restore_factory_spheres_to_extflash();

	read_all_spheretypes();
	update_number_of_user_spheres_filled();

	// Init ADC
	adc_init_all(); //starts hi-res ADC reading timers

	//Start LPF and bracketing for adcs: (must occur after loading flash, since it uses the system_calibration values)
	setup_analog_conditioning();
	start_analog_conditioning();

	//Keep this here for now until we develop a way to calibrate the plugged/unplugged jack default values
	set_default_cv_jack_calibration_offsets();

	//Start updating params (encoders, buttons, flip switches)
	start_UI_conditioning_updates();

	HAL_Delay(800);

	if (check_enter_led_adjust_mode())
	{
		enter_led_adjust_mode();

		while (!check_exit_led_adjust_mode())
		{
			process_led_color_adjust_mode();
			read_cv_jack_calibration_unplugged_offsets();
		}

		exit_led_adjust_mode();
	}

	init_preset_manager();

	//Show Firmware version
	display_firmware_version();
	HAL_Delay(4000);

	//Start updating oscillator parameters and wavetable
	start_osc_updates();
	start_osc_interp_updates();
	force_all_wt_interp_update(); 	// do first wavetabe interpolation (initializes routine's static variables)

	//Start LFO outputs
	start_envout_pwm();

	init_compressor(COMPRESS_SIGNED_24BIT, 0.90);

	ui_mode = PLAY;

	//Start Codec
	codec_GPIO_init();
	init_audio_DMA(SAMPLERATE);
	codec_I2C_init();
	codec_register_setup(SAMPLERATE);

	//init_wt_edit_settings();

	//Start audio processing
	set_audio_callback(&process_audio_block_codec);
	start_audio();

	start_led_display();

	selBus_Start();

	while(1){
		read_freq();

		read_switches();
		update_osc_param_lock();
		read_selbus_buttons();
		check_ui_mode_requests();

		read_load_save_encoder(); // Call from main loop because it can initiate a preset load/save call to sFLASH. If this is moved to an interrupt, then make sure it's lower priority than WT_INTERP
		check_sel_bus_event();	// FixMe: call from more adequate location (should be updated at about the data rate)

		if (ui_mode == VOCT_CALIBRATE) process_voct_calibrate_mode();

	} //end main loop

	return(0);

} //end main()




void SystemClock_Config(void)
{

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	//Configure the main internal regulator output voltage
	__HAL_RCC_PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	//Initializes the CPU, AHB and APB busses clocks
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 432;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		_Error_Handler(__FILE__, __LINE__);

	//Activate the OverDrive to reach the 216 MHz Frequency
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
		_Error_Handler(__FILE__, __LINE__);

	//Initializes the CPU, AHB and APB busses clocks
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
		_Error_Handler(__FILE__, __LINE__);


	//Note: Do not start the SAI clock (I2S) at this time.
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART5|RCC_PERIPHCLK_I2C2 | RCC_PERIPHCLK_I2C1;;

	PeriphClkInitStruct.PLLI2S.PLLI2SP 	= RCC_PLLP_DIV2;
	PeriphClkInitStruct.PLLI2S.PLLI2SR	= 2;

	PeriphClkInitStruct.I2c2ClockSelection 		= RCC_I2C2CLKSOURCE_PCLK1;
	PeriphClkInitStruct.I2c1ClockSelection 		= RCC_I2C1CLKSOURCE_PCLK1; //54MHz

	PeriphClkInitStruct.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	//Enables the Clock Security System
	HAL_RCC_EnableCSS();

	// Configure the Systick interrupt time for HAL_GetTick()
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/(1000*TICKS_PER_MS));

	//Configure the Systick
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	// SysTick_IRQn interrupt configuration
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void SetVectorTable(uint32_t reset_address)
{ 
	SCB->VTOR = reset_address & (uint32_t)0x1FFFFF80;
}

