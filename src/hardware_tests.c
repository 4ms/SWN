/*
 * hardware_tests.c - test procedure for procuction runs
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

#include "hardware_tests.h"
#include "globals.h"
#include "gpio_pins.h"
#include "analog_conditioning.h"
#include "flash.h"
#include "flash_params.h"
#include "switch_driver.h"
#include "hardware_controls.h"
#include "led_map.h"
#include "math_util.h"
#include "drivers/codec_i2c.h"
#include "drivers/codec_sai.h"
#include "drivers/pca9685_driver.h"
#include "drivers/mono_led_driver.h"
#include "drivers/flashram_spidma.h"
#include "drivers/rotary_driver.h"
#include "drivers/ads8634_driver.h"
#include "adc_interface.h"
#include "external_flash_layout.h"
#include "timekeeper.h"
#include "envout_pwm.h"
#include "sphere_flash_io.h"

#include <math.h>

//test
// #include "fft_filter.h"

//User input
extern o_switch  	hwSwitch[NUM_SWITCHES];

extern float				hires_adc_raw	[ NUM_HIRES_ADCS ];
extern DMABUFFER uint16_t	builtin_adc1_raw[ NUM_BUILTIN_ADC1 ];
extern DMABUFFER uint16_t	builtin_adc3_raw[ NUM_BUILTIN_ADC3 ];

extern o_analog analog[NUM_ANALOG_ELEMENTS];

//LED maps
extern const uint8_t all_rgb_led_map[NUM_LED_IDs];
extern const uint8_t led_outring_map[NUM_LED_OUTRING];
extern const uint8_t led_button_map[NUM_BUTTONS];
extern const uint8_t ledstring_map[NUM_CHANNELS + 1];
const uint32_t NUM_LED_ELEMENTS = (NUM_LED_IDs - 7) * 3 + 1;

//External errors
extern enum sFlashErrors sflash_error;
extern enum ADS863xErrors 	hiresadc_error;


void test_slider_leds(void);
void test_pwm_leds(void);
void test_controls(void);
void test_codec_comm(void);
void test_audio(void);
void test_env_outs(void);
void test_ext_adc(void);
void test_input_jacks(void);
void test_sflash(void);

void hardwaretest_audio_block_codec(int32_t *src, int32_t *dst);
void hardware_test_LFOs_callback(void);
void hardware_test_5VLFOs(void);
// void audio_freq_shift(int32_t *src, int32_t *dst);

void all_sliders_off(void);
void all_sliders_on(void);
void animate_success(void);

uint8_t hardwaretest_continue_button(void);
void pause_until_button_pressed(void);
void pause_until_button_released(void);

uint8_t get_ordered_led_element_number(uint8_t led_element_number);
uint8_t get_led_cv_correspondant(uint8_t cvjack_id);
// void set_env_amplitude(uint8_t chan, uint8_t amp);

void do_hardware_test(void)
{
	pause_until_button_released();

	test_slider_leds();
	test_pwm_leds();
	test_controls(); 
	test_codec_comm();
	test_audio();
	test_env_outs();
	test_ext_adc();
	test_input_jacks();
	test_sflash();

	stop_audio();
	pause_timer_IRQ(PWM_OUTS_TIM_number);

	pause_until_button_released();

	//Set default parameters and FW version into internal FLASH if they're not already
	if (!is_hardwaretest_already_done())
	{
		factory_reset_all_calibrations();
		factory_reset();
		restore_factory_spheres_to_extflash();
	}
	//Animate success and force a hard reboot
	while (1) {
		animate_success();
		HAL_Delay(160);
	}

}

void animate_success(void)
{
	static uint32_t led_element_id=0;

	LEDDriver_set_single_LED(get_ordered_led_element_number(led_element_id), 0);
	led_element_id = _WRAP_I16(led_element_id+1, 0, NUM_LED_ELEMENTS);
	LEDDriver_set_single_LED(get_ordered_led_element_number(led_element_id), 1023);
}

// Slider LED test
// The continue button is pressed to illuminate each LED, one at a time
// At the end, all 6 turn on and pressing continue exits the test
void test_slider_leds(void)
{
	all_sliders_off();
	pause_until_button_pressed();

	mono_led_on(mledm_SLIDER_A);
	pause_until_button_released();
	pause_until_button_pressed();
	mono_led_off(mledm_SLIDER_A);

	mono_led_on(mledm_SLIDER_B);
	pause_until_button_released();
	pause_until_button_pressed();
	mono_led_off(mledm_SLIDER_B);

	mono_led_on(mledm_SLIDER_C);
	pause_until_button_released();
	pause_until_button_pressed();
	mono_led_off(mledm_SLIDER_C);

	mono_led_on(mledm_SLIDER_D);
	pause_until_button_released();
	pause_until_button_pressed();
	mono_led_off(mledm_SLIDER_D);

	mono_led_on(mledm_SLIDER_E);
	pause_until_button_released();
	pause_until_button_pressed();
	mono_led_off(mledm_SLIDER_E);

	mono_led_on(mledm_SLIDER_F);
	pause_until_button_released();
	pause_until_button_pressed();
	mono_led_off(mledm_SLIDER_F);

	all_sliders_on();
	pause_until_button_released();
	pause_until_button_pressed();
	all_sliders_off();

	HAL_Delay(800);
}


// PWM LED test (RGB LEDs)
//
// First, check I2C communication with PWM LED driver chips: all sliders either on or fast flashing
// keeping binary code of chip# (1-10) steadily lit on sliders A/B/C/D (A=1, B=2, C=4, D=8)
// If there's an error, pressing button can continue
//
// Second, turn on all LEDs. User must press button to continue (gives time to inspect the LEDs)
// If we get an I2C error, or user holds button for 3 seconds
// then we enter the third test (otherwise we exit)
//
// Third, allow turning on each LED element individually by spinning the center rotary.
// 
void test_pwm_leds(void)
{
	uint8_t continue_armed=0;

	// First test: I2C
	//
	uint32_t led_pwm_chip_err = LEDDriver_init_direct(10);
	while (led_pwm_chip_err) {
		all_sliders_on();
		HAL_Delay(800);
		
		if ((led_pwm_chip_err >> 4) & 0b0001) mono_led_on(mledm_SLIDER_A); else mono_led_off(mledm_SLIDER_A);
		if ((led_pwm_chip_err >> 4) & 0b0010) mono_led_on(mledm_SLIDER_B); else mono_led_off(mledm_SLIDER_B);
		if ((led_pwm_chip_err >> 4) & 0b0100) mono_led_on(mledm_SLIDER_C); else mono_led_off(mledm_SLIDER_C);
		if ((led_pwm_chip_err >> 4) & 0b1000) mono_led_on(mledm_SLIDER_D); else mono_led_off(mledm_SLIDER_D);
		mono_led_off(mledm_SLIDER_E);
		mono_led_off(mledm_SLIDER_F);

		HAL_Delay(800);

		if (hardwaretest_continue_button()) continue_armed=1;
		if (continue_armed && !hardwaretest_continue_button()) {continue_armed=0;break;}
	}

	// Second test: Check turning on all LEDs. 
	//
	uint32_t led_err=0;

	for (uint32_t led_id=0; led_id<NUM_LED_IDs && !led_err; led_id++)
		led_err = LEDDriver_setRGBLED_RGB(led_id, 1023, 1023, 1023);
	if (!led_err)
		led_err += LEDDriver_set_single_LED(singleledm_CLKIN, 1023);
	
	HAL_Delay(2000);
	pause_until_button_pressed();

	// Third test: Individual LEDs
	// (optional: hold center rotary until sliders flash to do test)
	// Turn center rotary or Preset to select which LED element is on
	//
	uint32_t time = HAL_GetTick()/TICKS_PER_MS;
	uint32_t force_slow_led_test=0;
	while (hardwaretest_continue_button())
	{
		if (((HAL_GetTick()/TICKS_PER_MS) - time) > 3000) {
			all_sliders_on();
			force_slow_led_test = 1;
			all_sliders_off();
		}
	}

	//Turn all LEDs off, one color at a time
	for (uint32_t led_id=0; led_id<NUM_LED_IDs && !led_err; led_id++)
		led_err += LEDDriver_setRGBLED_RGB(led_id, 1023, 1023, 0);
	if (!led_err)
		led_err += LEDDriver_set_single_LED(singleledm_CLKIN, 0);
	HAL_Delay(800);
	for (uint32_t led_id=0; led_id<NUM_LED_IDs && !led_err; led_id++)
		led_err += LEDDriver_setRGBLED_RGB(led_id, 1023, 0, 0);
	HAL_Delay(800);
	for (uint32_t led_id=0; led_id<NUM_LED_IDs && !led_err; led_id++)
		led_err += LEDDriver_setRGBLED_RGB(led_id, 0, 0, 0);
	HAL_Delay(800);

	int16_t led_element_id=0;
	int8_t turn=0;

	init_rotary_turn(&rotary[rotm_WAVETABLE].turn);
	init_rotary_turn(&rotary[rotm_PRESET].turn);

	while (led_err || force_slow_led_test)
	{
		turn = read_rotary_turn(&rotary[rotm_WAVETABLE]) + read_rotary_turn(&rotary[rotm_PRESET]);

		if (turn!=0) {
			LEDDriver_set_single_LED(get_ordered_led_element_number(led_element_id), 0);
			if (turn>0) led_element_id = _WRAP_I16(led_element_id+1, 0, NUM_LED_ELEMENTS);
			if (turn<0) led_element_id = _WRAP_I16(led_element_id-1, 0, NUM_LED_ELEMENTS);
			LEDDriver_set_single_LED(get_ordered_led_element_number(led_element_id), 1023);
			turn = 0;
		}

		if (hardwaretest_continue_button()) continue_armed=1;
		if (continue_armed && !hardwaretest_continue_button()) {continue_armed=0;break;}
	}


}

uint8_t get_rotary_check_led(uint8_t rot_num)
{
	if (rot_num==rotm_LFOSPEED)  return get_red_led_element_id(ledm_STRING1);
	if (rot_num==rotm_OCT) 		 return get_red_led_element_id(ledm_STRING2);
	if (rot_num==rotm_PRESET) 	 return get_red_led_element_id(ledm_STRING3);
	if (rot_num==rotm_WAVETABLE) return get_red_led_element_id(ledm_STRING4);
	if (rot_num==rotm_TRANSPOSE) return get_red_led_element_id(ledm_STRING5);
	if (rot_num==rotm_LFOSHAPE)  return get_red_led_element_id(ledm_STRING6);
	if (rot_num==rotm_DEPTH) 	 return get_red_led_element_id(ledm_DEPTH_ENC);
	if (rot_num==rotm_LATITUDE)  return get_red_led_element_id(ledm_LATITUDE_ENC);
	if (rot_num==rotm_LONGITUDE) return get_red_led_element_id(ledm_LONGITUDE_ENC);
	return 0;//invalid
}

//
// Tester has to move each control to make all the lights turn off
// Outer LED ring shows rotary motions
void test_controls(void)
{
	uint8_t rot_num;
	uint8_t continue_armed=0;

	all_sliders_on();

	//Turn on all button LEDs
	for (uint8_t button_num=0; button_num<6; button_num++)
		LEDDriver_setRGBLED_RGB(led_button_map[button_num], 1023, 1023, 1023);
	LEDDriver_setRGBLED_RGB(ledm_LFOMODE_BUTTON, 0, 0, 1023);
	LEDDriver_setRGBLED_RGB(ledm_LFOVCA_BUTTON, 0, 0, 1023);

	//Turn on LED string
	for (uint8_t ledsttring_id=0; ledsttring_id<(NUM_CHANNELS + 1); ledsttring_id++)
		LEDDriver_setRGBLED_RGB(ledstring_map[ledsttring_id], 1023, 1023, 1023);

	//Turn on Rotary LEDs
	LEDDriver_setRGBLED_RGB(ledm_DEPTH_ENC, 1023, 1023, 1023);
	LEDDriver_setRGBLED_RGB(ledm_LATITUDE_ENC, 1023, 1023, 1023);
	LEDDriver_setRGBLED_RGB(ledm_LONGITUDE_ENC, 1023, 1023, 1023);

	//init the internal ADC3 for the sliders
	adc_single_init(3);
	HAL_Delay(2000);

	//init the rotaries
	for (rot_num=0; rot_num<NUM_ROTARIES; rot_num++)
		init_rotary_turn(&rotary[rot_num].turn);

	int8_t turn;
	int8_t ring_pos=0;
	uint8_t voct_state = 0;
	uint8_t	voct_armed = 0;
	uint8_t last_voct_state=read_switch_state(&hwSwitch[VOCTSW]);
	uint32_t press_time=0;
	uint8_t slider_armed[NUM_CHANNELS]={0};
	uint32_t last_ring_pos_timestamp=0;
	uint8_t ring_pos_active=0;

	while (1)
	{
		//Check rotary turns and presses
		for (rot_num=0; rot_num<NUM_ROTARIES; rot_num++)
		{
			//Animate outer ring to show rotary direction and number of steps
			if ((turn = read_rotary_turn(&rotary[rot_num]))) {
				LEDDriver_setRGBLED_RGB(led_outring_map[ring_pos], 0, 0, 0);
				ring_pos = _WRAP_I8(ring_pos + turn, 0, NUM_LED_OUTRING);
				LEDDriver_setRGBLED_RGB(led_outring_map[ring_pos], 1023, 1023, 1023);
				last_ring_pos_timestamp = HAL_GetTick()/TICKS_PER_MS;
				ring_pos_active = 1;
				//Turn off an LED for each direction/rotary checked: red for +, green for -
				uint8_t check_led = get_rotary_check_led(rot_num) + ((turn>0)?0:1);
				LEDDriver_set_single_LED(check_led, 0);
			}

			//Turn off blue LED when rotary pressed
			if (read_switch_state(&rotary[rot_num].hwswitch)) {
				uint8_t check_led = get_rotary_check_led(rot_num) + 2;
				LEDDriver_set_single_LED(check_led, 0);
			}
		}

		if (ring_pos_active && (((HAL_GetTick()/TICKS_PER_MS) - last_ring_pos_timestamp) > 250))
		{
			ring_pos_active = 0;
			LEDDriver_setRGBLED_RGB(led_outring_map[ring_pos], 0, 0, 0);
		}

		//Check button presses
		for (uint8_t button_num=0; button_num<NUM_BUTTONS; button_num++) {
			if (read_switch_state(&button[button_num].hwswitch))
				LEDDriver_set_single_LED(get_red_led_element_id(led_button_map[button_num])+2, 0);
		}

		//Check for Fine button
		if (read_switch_state(&hwSwitch[FINE_BUTTON]))		LEDDriver_set_single_LED(get_red_led_element_id(ledm_LFOCV), 0);

		//Check for motion on voct switch
		voct_state = read_switch_state(&hwSwitch[VOCTSW]);
		if (voct_state != last_voct_state) voct_armed=1;
		if (voct_armed) {
			if (voct_state && !last_voct_state)		LEDDriver_set_single_LED(get_red_led_element_id(ledm_LFOCV)+1, 0);
			if (!voct_state && last_voct_state)		LEDDriver_set_single_LED(get_red_led_element_id(ledm_LFOCV)+2, 0);
			last_voct_state = voct_state;
		}

		//Check sliders
		for (uint8_t slider_num=0; slider_num<NUM_CHANNELS; slider_num++) {
			if (builtin_adc3_raw[slider_num] > 2100) mono_led_on(mledm_SLIDER_A + slider_num);
			if (builtin_adc3_raw[slider_num] < 2000) mono_led_off(mledm_SLIDER_A + slider_num);
			if (builtin_adc3_raw[slider_num] > 4050) {
				if (slider_armed[slider_num]) 	LEDDriver_set_single_LED(get_red_led_element_id(led_button_map[slider_num]), 0);
			} else if (builtin_adc3_raw[slider_num] < 50) {
				if (slider_armed[slider_num])	LEDDriver_set_single_LED(get_red_led_element_id(led_button_map[slider_num])+1, 0);
			} else {
				slider_armed[slider_num] = 1;
			}
		}

		if (!continue_armed && hardwaretest_continue_button()) {continue_armed=1; press_time=HAL_GetTick()/TICKS_PER_MS;}
		if (!hardwaretest_continue_button()) continue_armed=0;

		if (continue_armed && (((HAL_GetTick()/TICKS_PER_MS) - press_time) > 2000))
		{
			while (hardwaretest_continue_button()) {
				all_sliders_on(); all_sliders_off();
			}
			break;
		}

	}
}

//
// Check the audio Codec: flash slider LED F is SAI fails, flash LED E if I2C fails
//
void test_codec_comm(void)
{
	uint8_t continue_armed=0;

	//All LEDs off
	all_sliders_off();
	for (uint32_t led_id=0; led_id<NUM_LED_IDs; led_id++)
		LEDDriver_setRGBLED_RGB(led_id, 0, 0, 0);

	// Waveform In LED turns blue for SAI init test
	LEDDriver_setRGBLED_RGB(ledm_AUDIOIN, 0, 0, 1023);
	HAL_Delay(1600);

	codec_GPIO_init();
	uint32_t codec_sai_err = init_audio_DMA(SAMPLERATE);

	while (codec_sai_err)
	{
		mono_led_on(mledm_SLIDER_F);
		HAL_Delay(1600);
		mono_led_off(mledm_SLIDER_F);
		HAL_Delay(1600);

		if (hardwaretest_continue_button()) continue_armed=1;
		if (continue_armed && !hardwaretest_continue_button()) {continue_armed=0;break;}
	}

	// Waveform In LED turns red for I2C test
	LEDDriver_setRGBLED_RGB(ledm_AUDIOIN, 1023, 0, 0);
	HAL_Delay(1600);

	uint32_t codec_i2c_err = codec_I2C_init();

	// Waveform In LED turns green for register setup test
	LEDDriver_setRGBLED_RGB(ledm_AUDIOIN, 0, 1023, 0);
	HAL_Delay(1600);

	uint32_t codec_reg_err = codec_register_setup(SAMPLERATE);

	while (codec_i2c_err || codec_reg_err)
	{
		mono_led_on(mledm_SLIDER_E);
		HAL_Delay(codec_i2c_err ? 2400 : 600);
		mono_led_off(mledm_SLIDER_E);
		HAL_Delay(codec_i2c_err ? 2400 : 600);

		if (hardwaretest_continue_button()) continue_armed=1;
		if (continue_armed && !hardwaretest_continue_button()) {continue_armed=0;break;}
	}

	LEDDriver_setRGBLED_RGB(ledm_AUDIOIN, 0, 0, 0);
}

// Turn rotary LEDs to R,G,B
// Start audio
// Turn rotary LEDs off if success (failure==hardfault)
//
void test_audio(void)
{
	LEDDriver_setRGBLED_RGB(ledm_DEPTH_ENC, 0, 0, 1024);
	LEDDriver_setRGBLED_RGB(ledm_LATITUDE_ENC, 1024, 0, 0);
	LEDDriver_setRGBLED_RGB(ledm_LONGITUDE_ENC, 0, 1024, 0);

	set_audio_callback(&hardwaretest_audio_block_codec);
	start_audio();

	LEDDriver_setRGBLED_RGB(ledm_DEPTH_ENC, 0, 0, 0);
	LEDDriver_setRGBLED_RGB(ledm_LATITUDE_ENC, 0, 0, 0);
	LEDDriver_setRGBLED_RGB(ledm_LONGITUDE_ENC, 0, 0, 0);

}

// Turn LFO LEDs yellow
// Start LFO outputs: 8.8V: 2Hz, 4Hz, 8Hz, 66Hz, 90Hz, 224Hz 
// Fail = hardfault
// Flash LFOs A/B/C to speed (red/green/blue), turn LFO LEDs D/E/F red/green/blue
// Press rotary button to continue
void test_env_outs(void)
{
	for (uint32_t led_id=0; led_id<(NUM_CHANNELS); led_id++)
		LEDDriver_setRGBLED_RGB(ledstring_map[led_id], 1023, 1023, 0);

	init_timekeeper(); 

	init_envout_pwm();

	start_timer_IRQ(PWM_OUTS_TIM_number, &hardware_test_LFOs_callback);

	LEDDriver_setRGBLED_RGB(ledstring_map[0], 1024, 0, 0);
	LEDDriver_setRGBLED_RGB(ledstring_map[1], 0, 1024, 0);
	LEDDriver_setRGBLED_RGB(ledstring_map[2], 0, 0, 1024);
	LEDDriver_setRGBLED_RGB(ledstring_map[3], 1024, 1024, 0);
	LEDDriver_setRGBLED_RGB(ledstring_map[4], 0, 1024, 1024);
	LEDDriver_setRGBLED_RGB(ledstring_map[5], 1024, 0, 1024);

	pause_until_button_pressed();
	all_sliders_off();
	all_sliders_on();
	pause_until_button_released();

	pause_timer_IRQ(PWM_OUTS_TIM_number);

}

// Turn all lights off
// Turn on LFO LEDs and inner ring 1 blue
// Init the ADC chips
// Flash the sliders A and B if there's a failure (press button to ocntinue)
// Read each jack and turn light white if reading 0, red if out of range
// If found an out of range jack, press button to continue (otherwise it automatically continues)
void test_ext_adc(void)
{
	uint8_t continue_armed=0;
	uint8_t num_failed;

	//All LEDs off
	all_sliders_off();
	for (uint32_t led_id=0; led_id<NUM_LED_IDs; led_id++)
		LEDDriver_setRGBLED_RGB(led_id, 0, 0, 0);

	//init_timekeeper(); 

	//Turn on LED string and inner ring 1
	for (uint32_t led_id=0; led_id<(NUM_CHANNELS); led_id++)
		LEDDriver_setRGBLED_RGB(ledstring_map[led_id], 0, 0, 1023);

	LEDDriver_setRGBLED_RGB(ledm_INRING1, 0, 0, 1023);
	
	adc_hires_init();

	while (hiresadc_error){
		mono_led_on(hiresadc_error & 0b1 ? mledm_SLIDER_B: mledm_SLIDER_A);
		HAL_Delay(1600);
		mono_led_off(hiresadc_error & 0b1 ? mledm_SLIDER_B: mledm_SLIDER_A);
		HAL_Delay(1600);

		if (hardwaretest_continue_button()) continue_armed=1;
		if (continue_armed && !hardwaretest_continue_button()) {continue_armed=0;break;}
	}

	//enum RangeSel new_vranges[NUM_HIRES_ADCS] = {RANGE_pm10V, RANGE_pm10V, RANGE_pm10V, RANGE_pm10V, RANGE_pm10V, RANGE_pm10V, RANGE_pm10V};
	//hires_adc_change_vrange(new_vranges);

	HAL_Delay(2000);

	num_failed = 0;
	for (uint8_t adc_id = 0; adc_id < NUM_HIRES_ADCS; adc_id++){
		//if (hires_adc_raw[adc_id] < 2000 || hires_adc_raw[adc_id] > 2100) LEDDriver_setRGBLED_RGB(ledstring_map[adc_id], 1023, 0, 0);
		if (hires_adc_raw[adc_id] > 10) {
			num_failed++;
			LEDDriver_setRGBLED_RGB(adc_id == 6 ? ledm_INRING1 : ledstring_map[adc_id], 1023, 0, 0);
		} else 
			LEDDriver_setRGBLED_RGB(adc_id == 6 ? ledm_INRING1 : ledstring_map[adc_id], 1023, 1023, 1023);
	}

	if (num_failed) {
		pause_until_button_pressed();
		pause_until_button_released();
	}

}

// Turn all LEDs green
// Set LFOs A, B, C to 5V amplitude and low speeds
// Start the internal ADC for CV jacks
// Turn all LEDs off except:
// - LFO LEDs (A-F) and button F are white
// - ... are yellow (red+green)
// - Clk In and Waveform In LEDs are blue
//
// Begin testing:
// If it detects 0V on any jack then green will turn off: Assuming all CV jacks are unplugged, if any light is green (or yellow or cyan) then this is a Fail.
// Patch LFO A into Clk In jack: Clk In light should flash, LFO->VCA butotn should turn off. (plugging into Clock turns it off, applying a gate shows a bright blue light. Stays off once it's detected a gate, and is unplugged. Stays dim if unplugged but never detected a gate.)
// Plugging into all other stereo jacks make blue go off (white->yellow)
// Patch Detecting voltage on any jack makes red light turn off
//
void test_input_jacks(void)
{
	uint8_t plug_switch_tested[21]={0};
	uint8_t range_tested[21] = {0};
	uint8_t zero_tested[21] = {0};
	uint16_t last_clock_in_brightness=0, brightness;
	uint8_t jack_switch;
	uint8_t test_id;
	uint16_t adc_val, last_adc_val[21]={0};
	uint8_t adc_led=0;
	uint8_t last_adc_test_id=0;
	uint32_t last_adc_test_timestamp=0;
	uint8_t adc_active=0;
	uint16_t adc_5V_val;
	uint8_t continue_armed=0;
	uint16_t red;
	uint16_t green;
	uint16_t blue;

	//All LEDs green
	all_sliders_off();
	for (uint32_t led_id=0; led_id<NUM_LED_IDs; led_id++)
		LEDDriver_setRGBLED_RGB(led_id, 0, 1023, 0);

	start_timer_IRQ(PWM_OUTS_TIM_number, &hardware_test_5VLFOs);

	//Todo: separate the switches from the analog conditioning 
	setup_analog_conditioning();

	//init the internal ADC1 for the jacks
	adc_single_init(1);

	HAL_Delay(2000);

	for (uint32_t led_id=0; led_id<NUM_LED_IDs; led_id++)
		LEDDriver_setRGBLED_RGB(led_id, 0, 0, 0);

	//Turn button LEDs and LFO Mode button white
	for (uint32_t led_id=0; led_id<NUM_CHANNELS; led_id++)
		LEDDriver_setRGBLED_RGB(led_button_map[led_id], 1023, 1023, 1023);

	LEDDriver_setRGBLED_RGB(ledm_LFOVCA_BUTTON, 1023, 1023, 1023);

	//Clock In off, and Waveform In and LFO VCA button LEDs = blue
	LEDDriver_setRGBLED_RGB(ledm_AUDIOIN, 0, 0, 1023);
	LEDDriver_setRGBLED_RGB(ledm_LFOMODE_BUTTON, 0, 0, 1023);
	LEDDriver_set_single_LED(singleledm_CLKIN, 4095);

	HAL_Delay(2000);

	pause_until_button_released();

	while (1) {
		//Channel and Transpose jacks
		for (test_id=0; test_id<17; test_id++)
		{
			if (test_id==RANDOM_ADC+7) //11
				continue;

			else if (test_id<=TRANSPOSE_JACK_ADC)  //0..6
			{
				adc_val = hires_adc_raw[test_id];
				adc_5V_val = 1800;
				jack_switch = read_switch_state(&(analog[test_id].plug_sense_switch));
			}
			else if (test_id<=(TRANSPOSE_JACK_ADC+NUM_BUILTIN_ADC1+NUM_BUILTIN_ADC3-NUM_CHANNELS)) //7..16, except for 11
			{ 
				adc_val = (test_id-TRANSPOSE_JACK_ADC-1)<NUM_BUILTIN_ADC1 ? builtin_adc1_raw[test_id-TRANSPOSE_JACK_ADC-1] : builtin_adc3_raw[test_id-TRANSPOSE_JACK_ADC-NUM_BUILTIN_ADC3+NUM_CHANNELS-1];
				adc_5V_val = 3800;

				jack_switch = RELEASED; //no switch on these jacks
				plug_switch_tested[test_id] = 1; 
			}

			if (!plug_switch_tested[test_id] && jack_switch==PRESSED) {
				plug_switch_tested[test_id] = 1;
				red = range_tested[test_id] ? 0 : 1023;
				green = zero_tested[test_id] ? 0 : 1023;
				blue = plug_switch_tested[test_id] ? 0 : 1023;
				LEDDriver_setRGBLED_RGB(get_led_cv_correspondant(test_id), red, green, blue);
			}

			if (!range_tested[test_id] && adc_val > adc_5V_val) {
				range_tested[test_id] = 1;
				red = range_tested[test_id] ? 0 : 1023;
				green = zero_tested[test_id] ? 0 : 1023;
				blue = plug_switch_tested[test_id] ? 0 : 1023;
				LEDDriver_setRGBLED_RGB(get_led_cv_correspondant(test_id), red, green, blue);
			}

			if (!zero_tested[test_id] && adc_val < 10) {
				zero_tested[test_id] = 1;
				red = range_tested[test_id] ? 0 : 1023;
				green = zero_tested[test_id] ? 0 : 1023;
				blue = plug_switch_tested[test_id] ? 0 : 1023;
				LEDDriver_setRGBLED_RGB(get_led_cv_correspondant(test_id), red, green, blue);
			}

			if (fabs((int16_t)adc_val - (int16_t)(last_adc_val[test_id])) > 50)
			{
				last_adc_val[test_id] = adc_val;
				LEDDriver_setRGBLED_RGB(led_outring_map[adc_led], 0, 0, 0);
				adc_led = (adc_val * NUM_LED_OUTRING)>>12;
				LEDDriver_setRGBLED_RGB(led_outring_map[adc_led], 1023, 1023, 1023);
				last_adc_test_timestamp = (HAL_GetTick()/TICKS_PER_MS);
				last_adc_test_id = test_id;
				adc_active = 1;

				red = range_tested[test_id] ? 0 : 1023;
				green = zero_tested[test_id] ? 0 : 1023;
				blue = ((HAL_GetTick()/TICKS_PER_MS) & 0x20) ? 3000 : 0;
				LEDDriver_setRGBLED_RGB(get_led_cv_correspondant(test_id), red, green, blue);
			}

		}

		//Turn off flashing jack corresponding light and adc_led if no adc activity in 250ms
		if (adc_active && ((HAL_GetTick()/TICKS_PER_MS) - last_adc_test_timestamp) > 250)
		{
			adc_active=0;
			LEDDriver_setRGBLED_RGB(led_outring_map[adc_led], 0, 0, 0);
			red = range_tested[last_adc_test_id] ? 0 : 1023;
			green = zero_tested[last_adc_test_id] ? 0 : 1023;
			blue = plug_switch_tested[last_adc_test_id] ? 0 : 1023;
			LEDDriver_setRGBLED_RGB(get_led_cv_correspondant(last_adc_test_id), red, green, blue);
		}

		//Clock In jack
		test_id = 17;
		if (read_switch_state(&(hwSwitch[CLK_SENSE]))==PRESSED) {
			zero_tested[test_id] = 0; //reset zero_tested so that we turn off the light when unplugging

			if (!plug_switch_tested[test_id]) {
				LEDDriver_setRGBLED_RGB(ledm_LFOMODE_BUTTON, 0, 0, 0);
				plug_switch_tested[test_id] = 1;
			}
			if (CLK_IN()) {
				range_tested[test_id] = 1;
				brightness = 1023;
			} else
				brightness = 0;

			if (brightness != last_clock_in_brightness) {
				LEDDriver_set_single_LED(singleledm_CLKIN, brightness);
				last_clock_in_brightness = brightness;
			}
		}
		else {
			if (CLK_IN()) {
				LEDDriver_set_single_LED(singleledm_CLKIN, 4095);
				zero_tested[test_id] = 0;
			}
			else {
				if (range_tested[test_id] && !zero_tested[test_id]) {
					zero_tested[test_id] = 1;
					LEDDriver_set_single_LED(singleledm_CLKIN, 0);
				}
			}
		}

		//Waveform In jack
		test_id = 18;
		if (!plug_switch_tested[test_id] && read_switch_state(&(hwSwitch[WAVEFORMIN_SENSE]))==PRESSED) {
			plug_switch_tested[test_id] = 1;
			LEDDriver_setRGBLED_RGB(ledm_AUDIOIN, 0, 0, 0);
		}

		if (hardwaretest_continue_button()) continue_armed=1;
		if (continue_armed && !hardwaretest_continue_button()) {continue_armed=0;break;}

		//Clock Bus
		test_id = 19;
		if (BUS_CLK() && !range_tested[test_id]) {
			range_tested[test_id] = 1;
			red = range_tested[19] ? 0 : 1023;
			green = zero_tested[19] ? 0 : 1023;
			blue = range_tested[20] ? 0 : 1023;
			LEDDriver_setRGBLED_RGB(get_led_cv_correspondant(test_id), red, green, blue);
		}
		if (!BUS_CLK() && !zero_tested[test_id]) {
			zero_tested[test_id] = 1;
			red = range_tested[19] ? 0 : 1023;
			green = zero_tested[19] ? 0 : 1023;
			blue = range_tested[20] ? 0 : 1023;
			LEDDriver_setRGBLED_RGB(get_led_cv_correspondant(test_id), red, green, blue);
		}

		//Select Bus
		test_id = 20;
		if (BUS_SEL() && !range_tested[test_id]) {
			range_tested[test_id] = 1;
			red = range_tested[19] ? 0 : 1023;
			green = zero_tested[19] ? 0 : 1023;
			blue = range_tested[20] ? 0 : 1023;
			LEDDriver_setRGBLED_RGB(get_led_cv_correspondant(test_id), red, green, blue);
		}
		if (!BUS_SEL() && !zero_tested[test_id]) {
			zero_tested[test_id] = 1;
			red = range_tested[19] ? 0 : 1023;
			green = zero_tested[19] ? 0 : 1023;
			blue = range_tested[20] ? 0 : 1023;
			LEDDriver_setRGBLED_RGB(get_led_cv_correspondant(test_id), red, green, blue);
		}

	}
}


//
// Check the external flash chip: flash slider LED D is SPI init fails, flash LED C if read/write fails
//
void test_sflash(void)
{
	uint8_t continue_armed=0;

	//
	// Check ext FLASH RAM chip
	//
	all_sliders_off();

	sFLASH_init();
	while (sflash_error)
	{
		mono_led_on(mledm_SLIDER_D);
		HAL_Delay(1600);
		mono_led_off(mledm_SLIDER_D);
		HAL_Delay(1600);

		if (hardwaretest_continue_button()) continue_armed=1;
		if (continue_armed && !hardwaretest_continue_button()) {continue_armed=0;break;}
	}

	//Don't do any more flash tests if init failed
	if (sflash_error) return;


	uint32_t bad_bytes;

	//Test first sector before Wavetables
	bad_bytes = sFLASH_test_sector( sFLASH_get_sector_addr(WT_SECTOR_START - 1) );

	//Test sector after the last Wavetable, before first preset sector
	if (!bad_bytes) bad_bytes =  sFLASH_test_sector( sFLASH_get_sector_addr(PRESET_SECTOR_START - 1) );

	while (bad_bytes)
	{
		mono_led_on(mledm_SLIDER_C);
		HAL_Delay(1600);
		mono_led_off(mledm_SLIDER_C);
		HAL_Delay(1600);

		if (hardwaretest_continue_button()) continue_armed=1;
		if (continue_armed && !hardwaretest_continue_button()) {continue_armed=0;break;}
	} 
}






//
// Generate asymmetrical triangle wave on left output, and pass audio input to right output
//
void hardwaretest_audio_block_codec(int32_t *src, int32_t *dst)
{
	uint16_t i;
	static float tri=0;
	static int32_t tri_dir=1;
	const int32_t MAX_CODEC_DAC_VAL = 8388607;
	const int32_t MIN_CODEC_DAC_VAL = -8388608;

	const int32_t period = (SAMPLERATE/100); // 441  = 44100Hz / 100Hz
	const int32_t rise_period = 100;
	const int32_t fall_period = period - rise_period;

	for (i=0; i<MONO_BUFSZ; i++)
	{
		*dst++ = *src++;
		UNUSED(*src++);

		if (tri_dir==1)	tri+=(MAX_CODEC_DAC_VAL - MIN_CODEC_DAC_VAL)/rise_period;
		else			tri-=(MAX_CODEC_DAC_VAL - MIN_CODEC_DAC_VAL)/fall_period;

		if (tri >= MAX_CODEC_DAC_VAL) 	{ tri_dir = 1 - tri_dir; tri = MAX_CODEC_DAC_VAL; }
		if (tri <= MIN_CODEC_DAC_VAL) 	{ tri_dir = 1 - tri_dir; tri = MIN_CODEC_DAC_VAL; }

		*dst++ = (int32_t)tri;
	}
}

// void audio_freq_shift(int32_t *src, int32_t *dst)
// {
// 	uint16_t i; 
// 	int16_t inbuf[MONO_BUFSZ];
// 	int16_t outbuf[MONO_BUFSZ];
// 	float tmpbuf[MONO_BUFSZ*2];
// 	int32_t insmpl;

// 	for (i=0; i<MONO_BUFSZ; i++)
// 	{
// 		insmpl = *src++;
// 		UNUSED(*src++);
// 		inbuf[i] = insmpl/(int32_t)256;
// 	}
// 	do_fft_shift_16(inbuf, outbuf, tmpbuf, 1);

// 	for (i=0; i<MONO_BUFSZ; i++)
// 	{
// 		*dst++ = inbuf[i]*256;
// 		*dst++ = outbuf[i]*256;
// 	}

// }


void hardware_test_LFOs_callback(void)
{
	static float tri[6]={0,0,0,0,0,0};
	static int32_t tri_dir[6]={1,1,1,1,1,1};
	static float env_rise_inc[6] = {42.5, 42.5, 51.0, 51.0, 63.75, 63.75};
	static float env_fall_inc[6] = {11.09, 12.14, 13.42, 14.17, 15.00, 15.94};
	static int32_t env_amplitude[6] = {255, 255, 255, 255, 255, 255};

	for (uint8_t chan=0;chan<6;chan++)
	{
		if (tri_dir[chan])	tri[chan]+=env_rise_inc[chan];
		else				tri[chan]-=env_fall_inc[chan];

		if (tri[chan] >= env_amplitude[chan]) {
			tri_dir[chan] = !tri_dir[chan];
			tri[chan] = env_amplitude[chan];
		}
		if (tri[chan] <= 0) {
			tri_dir[chan] = !tri_dir[chan];
			tri[chan] = 0;
		}
	}

	ENVOUT_PWM_TIM_AB->ENVOUT_PWM_CC_A 	= (uint32_t)tri[0];
	ENVOUT_PWM_TIM_AB->ENVOUT_PWM_CC_B 	= (uint32_t)tri[1];
	ENVOUT_PWM_TIM_C->ENVOUT_PWM_CC_C 	= (uint32_t)tri[2];
	ENVOUT_PWM_TIM_DEF->ENVOUT_PWM_CC_D = (uint32_t)tri[3];
	ENVOUT_PWM_TIM_DEF->ENVOUT_PWM_CC_E = (uint32_t)tri[4];
	ENVOUT_PWM_TIM_DEF->ENVOUT_PWM_CC_F = (uint32_t)tri[5];

}

void hardware_test_5VLFOs(void)
{
	static float tri[6]={0,0,0,0,0,0};
	static int32_t tri_dir[6]={1,1,1,1,1,1};
	static float env_rise_inc[6] = {0.1, 0.2, 0.5, 0.2, 0.2, 255};
	static float env_fall_inc[6] = {0.05, 0.1, 0.2, 0.2, 255, 0.2};
	static int32_t env_amplitude[6] = {149, 149, 149, 149, 149, 149};
	static uint32_t skip_ctr = 0;

	for (uint8_t chan=0;chan<6;chan++)
	{
		if (tri_dir[chan])	tri[chan]+=env_rise_inc[chan];
		else				tri[chan]-=env_fall_inc[chan];

		if (tri[chan] >= env_amplitude[chan]) {
			tri_dir[chan] = !tri_dir[chan];
			tri[chan] = env_amplitude[chan];
		}
		if (tri[chan] <= 0) {
			tri_dir[chan] = !tri_dir[chan];
			tri[chan] = 0;
		}
	}

	ENVOUT_PWM_TIM_AB->ENVOUT_PWM_CC_A 	= (uint32_t)tri[0];
	ENVOUT_PWM_TIM_AB->ENVOUT_PWM_CC_B 	= (uint32_t)tri[1];
	ENVOUT_PWM_TIM_C->ENVOUT_PWM_CC_C 	= (uint32_t)tri[2];
	ENVOUT_PWM_TIM_DEF->ENVOUT_PWM_CC_D = (uint32_t)tri[3];
	ENVOUT_PWM_TIM_DEF->ENVOUT_PWM_CC_E = (uint32_t)tri[4];
	ENVOUT_PWM_TIM_DEF->ENVOUT_PWM_CC_F = (uint32_t)tri[5];

	if (skip_ctr++>85){
		LEDDriver_setRGBLED_RGB(ledstring_map[0], tri[0] * 10, tri[0] * 10, tri[0] * 10);
		LEDDriver_setRGBLED_RGB(ledstring_map[1], tri[1] * 10, tri[1] * 10, tri[1] * 10);
		LEDDriver_setRGBLED_RGB(ledstring_map[2], tri[2] * 10, tri[2] * 10, tri[2] * 10);
		LEDDriver_setRGBLED_RGB(ledstring_map[3], tri[3] * 10, tri[3] * 10, tri[3] * 10);
		LEDDriver_setRGBLED_RGB(ledstring_map[4], tri[4] * 10, tri[4] * 10, tri[4] * 10);
		LEDDriver_setRGBLED_RGB(ledstring_map[5], tri[5] * 10, tri[5] * 10, tri[5] * 10);
		skip_ctr=0;
	}
}



void all_sliders_off(void)
{
	mono_led_off(mledm_SLIDER_A);
	mono_led_off(mledm_SLIDER_B);
	mono_led_off(mledm_SLIDER_C);
	mono_led_off(mledm_SLIDER_D);
	mono_led_off(mledm_SLIDER_E);
	mono_led_off(mledm_SLIDER_F);
}
void all_sliders_on(void)
{
	mono_led_on(mledm_SLIDER_A);
	mono_led_on(mledm_SLIDER_B);
	mono_led_on(mledm_SLIDER_C);
	mono_led_on(mledm_SLIDER_D);
	mono_led_on(mledm_SLIDER_E);
	mono_led_on(mledm_SLIDER_F);
}

uint8_t get_led_cv_correspondant(uint8_t cvjack_id)
{
	if (cvjack_id < TRANSPOSE_JACK_ADC) 
		return led_button_map[cvjack_id];

	if (cvjack_id == TRANSPOSE_JACK_ADC)
		return ledm_LFOVCA_BUTTON;

	if (cvjack_id == BROWSE_ADC + 7)
		return ledm_INRING6;

	if (cvjack_id == CHORD_ADC + 7)
		return ledm_INRING2;

	if (cvjack_id == LAT_ADC + 7)
		return ledm_LATITUDE_ENC;

	if (cvjack_id == LFO_ADC + 7)
		return ledm_LFOCV;

	if (cvjack_id == RANDOM_ADC + 7)
		return ledm_INRING1;

	if (cvjack_id == DEPTH_ADC + 7)
		return ledm_DEPTH_ENC;

	if (cvjack_id == SPHERE_ADC + 7)
		return ledm_INRING5;

	if (cvjack_id == WTSPREAD_ADC + 7) 
		return ledm_LONGITUDE_ENC;

	if (cvjack_id == (DISP_ADC + NUM_BUILTIN_ADC3 - NUM_CHANNELS + 7))
		return ledm_INRING4;

	if (cvjack_id == (DISPPAT_ADC + NUM_BUILTIN_ADC3 - NUM_CHANNELS + 7))
		return ledm_INRING3;

	if (cvjack_id == 19)
		return ledm_INRING1;

	if (cvjack_id == 20)
		return ledm_INRING1;

	return NONE1;
}

//returns led element numbers in order as they appear on the faceplate (top to bottom)
uint8_t get_ordered_led_element_number(uint8_t led_element_number)
{
	uint8_t ordered_led_element_number;

	if (led_element_number==0)
	{
		ordered_led_element_number = singleledm_CLKIN;
	}
	else {
		uint8_t rgb_led_id = all_rgb_led_map[(led_element_number-1)/3];
		uint8_t rgb_color = (led_element_number-1) % 3;

		ordered_led_element_number = get_red_led_element_id(rgb_led_id) + rgb_color;
	}

	return ordered_led_element_number;
}


uint8_t is_hardwaretest_already_done(void)
{
	uint32_t word0 = flash_read_word(FLASH_ADDR_userparams+0);
	uint32_t word1 = flash_read_word(FLASH_ADDR_userparams+4);

	word0 -= FLASH_SYMBOL_firmwareoffset;

	//either major or minor version has to be non-zero, AND both must be less than the max valid
	if ( ((word0 > 0) || (word1 > 0)) &&  (word0 < MAX_VALID_MAJOR_FW_VERSION) && (word1 < MAX_VALID_MAJOR_FW_VERSION) ) {
		return 1;
	} else
		return 0;
}

//use low-level GPIO reading to minimize sources of errors besides hardware
uint8_t hardwaretest_continue_button(void) 	{return read_switch_state(&rotary[rotm_WAVETABLE].hwswitch); }

void pause_until_button_pressed(void) { HAL_Delay(800); while (!hardwaretest_continue_button()) {;} }
void pause_until_button_released(void) { HAL_Delay(800); while (hardwaretest_continue_button()) {;} }
