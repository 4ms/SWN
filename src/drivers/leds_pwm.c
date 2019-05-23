/*
 * leds_pwm.c : Interfaces with the LED PWM chip driver to control all PWM LEDs
 *
 */

#include "drivers/leds_pwm.h"
#include "led_colors.h"
#include "drivers/pca9685_driver.h"
#include "flash_params.h"
#include "gpio_pins.h"
#include "math.h"
#include "system_settings.h"
#include "flash_params.h" 

extern SystemCalibrations *system_calibrations;
extern o_systemSettings system_settings;
// extern uint8_t leddriver_cur_buf;

uint32_t pwmleds[2][NUM_PWM_LED_CHIPS][NUM_LEDS_PER_CHIP];

void pwm_leds_display_off(void){	LED_RING_ON();	}
void pwm_leds_display_on(void){		LED_RING_OFF();	}

void init_pwm_leds(void)
{
	uint8_t led, chip;

	pwm_leds_display_off();

	//Turn off all PWM LEDs to start
	for (chip=0;chip<NUM_PWM_LED_CHIPS;chip++)
	{
		for (led=0;led<NUM_LEDS_PER_CHIP;led++)
		{
			if (chip==0) 	  pwmleds[0][chip][led] = 0x02000000;
			else if (chip==1) pwmleds[0][chip][led] = 0x01000000;
			else if (chip==2) pwmleds[0][chip][led] = 0x00800000;
			else if (chip==3) pwmleds[0][chip][led] = 0x00400000;
			else if (chip==4) pwmleds[0][chip][led] = 0x00200000;
			else if (chip==5) pwmleds[0][chip][led] = 0x00100000;
			else if (chip==6) pwmleds[0][chip][led] = 0x00080000;
			else if (chip==7) pwmleds[0][chip][led] = 0x00040000;
			else if (chip==8) pwmleds[0][chip][led] = 0x00020000;
			else if (chip==9) pwmleds[0][chip][led] = 0x00010000;

			pwmleds[1][chip][led] = 0x00000000;
		}
	}

	LEDDriver_init_dma(NUM_PWM_LED_CHIPS, (uint8_t *)(pwmleds[0]), (uint8_t *)(pwmleds[1]));

	pwm_leds_display_on();
}

//static inline int32_t _USAT12(int32_t x);
static inline int32_t _USAT12(int32_t x) {asm("ssat %[dst], #12, %[src]" : [dst] "=r" (x) : [src] "r" (x)); return x;}

static inline uint8_t best_write_buf(uint8_t chip_num) { 
	uint8_t leddriver_cur_buf = LEDDriver_get_cur_buf();
	uint8_t opposite_buf = 1-leddriver_cur_buf;

	if (LEDDriver_get_cur_chip() < 5)
		return opposite_buf;
	else {
		return (chip_num<5) ? leddriver_cur_buf : opposite_buf;
	}
}

void set_pwm_led(uint8_t led_id, const o_rgb_led *rgbled)
{
	uint32_t r,g,b;

	r = (float)rgbled->c_red 	* system_calibrations->rgbled_adjustments[led_id][c_RED] 	* rgbled->brightness * system_settings.global_brightness;
	g = (float)rgbled->c_green 	* system_calibrations->rgbled_adjustments[led_id][c_GREEN]	* rgbled->brightness * system_settings.global_brightness;
	b = (float)rgbled->c_blue 	* system_calibrations->rgbled_adjustments[led_id][c_BLUE]	* rgbled->brightness * system_settings.global_brightness;
	r = _USAT12(r);
	g = _USAT12(g);
	b = _USAT12(b);

	//LEDDriver_setRGBLED_RGB(led_id, r, g, b);
	uint8_t red_led_element = get_red_led_element_id(led_id) % NUM_LEDS_PER_CHIP;
	uint8_t chip_num = get_chip_num(led_id);
	uint8_t buf = best_write_buf(chip_num);

	pwmleds[buf][chip_num][red_led_element] = r<<16;
	pwmleds[buf][chip_num][red_led_element+1] = g<<16;
	pwmleds[buf][chip_num][red_led_element+2] = b<<16;

}

//Do not use this except for hardware testing, it is very inefficient!
void set_pwm_led_direct(uint8_t led_id, uint16_t c_red, uint16_t c_green, uint16_t c_blue)
{
	// uint8_t red_led_element = get_red_led_element_id(led_id) % 16;
	// uint8_t chip_num = get_chip_num(led_id);
	// pwmleds[chip_num].leds[red_led_element] = r;
	//LEDDriver_setRGBLED_RGB(led_id, c_red, c_green, c_blue);
}

void set_single_pwm_led(uint8_t single_element_led_id, uint16_t brightness)
{
	uint8_t led_element = get_red_led_element_id(single_element_led_id) % NUM_LEDS_PER_CHIP;
	uint8_t chip_num = get_chip_num(single_element_led_id);
	uint8_t buf = best_write_buf(chip_num);


	pwmleds[buf][chip_num][led_element] = _USAT12(brightness);

}