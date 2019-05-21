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
o_rgb_led	cached_rgb_led[NUM_LED_IDs];

struct LEDRamImage {
	uint8_t start_command;
	uint32_t leds[NUM_LEDS_PER_CHIP];
};

struct LEDRamImage pwmleds[NUM_PWM_LED_CHIPS];

// uint32_t pwmleds[NUM_PWM_LED_CHIPS][NUM_LEDS_PER_CHIP+1];

void pwm_leds_display_off(void){	LED_RING_ON();	}
void pwm_leds_display_on(void){		LED_RING_OFF();	}

void init_pwm_leds(void)
{
	uint8_t i, j;

	pwm_leds_display_off();

	//Turn off all PWM LEDs to start
	for (i=0;i<NUM_LED_IDs;i++)
	{
		cached_rgb_led[i].c_red 		= 0;
		cached_rgb_led[i].c_green 		= 0;
		cached_rgb_led[i].c_blue 		= 0;
		cached_rgb_led[i].brightness 	= F_MAX_BRIGHTNESS;
	}

	for (i=0;i<NUM_PWM_LED_CHIPS;i++)
	{
		pwmleds[i].start_command = PCA9685_LED0;
		
		for (j=0;j<NUM_LEDS_PER_CHIP;j++)
			pwmleds[i].leds[j] = 0;
	}

	LEDDriver_Init(NUM_PWM_LED_CHIPS, (uint8_t *)pwmleds);

	pwm_leds_display_on();
}

static inline int32_t _USAT12(int32_t x);
static inline int32_t _USAT12(int32_t x) {asm("ssat %[dst], #12, %[src]" : [dst] "=r" (x) : [src] "r" (x)); return x;}

void set_pwm_led(uint8_t led_id, const o_rgb_led *rgbled)
{
	uint32_t r,g,b;
	static float cached_global_brightness[NUM_LED_IDs];
	// enum LED_Driver_Errors led_err;
	

	//Check the o_rgb_led value with the cached value
	if (	rgbled->c_red 						!= cached_rgb_led[led_id].c_red
		|| 	rgbled->c_green 					!= cached_rgb_led[led_id].c_green
		|| 	rgbled->c_blue 						!= cached_rgb_led[led_id].c_blue
		||	rgbled->brightness 					!= cached_rgb_led[led_id].brightness
		|| 	system_settings.global_brightness 	!= cached_global_brightness[led_id])
	{
		cached_rgb_led[led_id].c_red		= rgbled->c_red;
		cached_rgb_led[led_id].c_green		= rgbled->c_green;
		cached_rgb_led[led_id].c_blue		= rgbled->c_blue;
		cached_rgb_led[led_id].brightness 	= rgbled->brightness;
		cached_global_brightness[led_id] 	= system_settings.global_brightness;

		r = (float)cached_rgb_led[led_id].c_red 	* system_calibrations->rgbled_adjustments[led_id][c_RED] 	* cached_rgb_led[led_id].brightness * cached_global_brightness[led_id];
		g = (float)cached_rgb_led[led_id].c_green 	* system_calibrations->rgbled_adjustments[led_id][c_GREEN]	* cached_rgb_led[led_id].brightness * cached_global_brightness[led_id];
		b = (float)cached_rgb_led[led_id].c_blue 	* system_calibrations->rgbled_adjustments[led_id][c_BLUE]	* cached_rgb_led[led_id].brightness * cached_global_brightness[led_id];
		r = _USAT12(r);
		g = _USAT12(g);
		b = _USAT12(b);

		LEDDriver_setRGBLED_RGB(led_id, r, g, b);

		// DEBUG1_ON;
		// led_err = LEDDriver_setRGBLED_RGB(led_id, r, g, b);
		// if (!led_err)
		// 	DEBUG1_OFF;
	}
}

//Wrapper function, placeholder until something better is written:
//Do not use this except for hardware testing, it is very inefficient!
void set_pwm_led_direct(uint8_t led_id, uint16_t c_red, uint16_t c_green, uint16_t c_blue)
{
	LEDDriver_setRGBLED_RGB(led_id, c_red, c_green, c_blue);
}

//Todo: cached_brightness needs to be an array if there is more than one single_element_led_id allowed
void set_single_pwm_led(uint8_t single_element_led_id, uint16_t brightness)
{
	static float cached_brightness;

	if (fabs(cached_brightness - brightness)>0.01) {
		LEDDriver_set_single_LED(single_element_led_id, brightness);
		cached_brightness = brightness;
	}
}