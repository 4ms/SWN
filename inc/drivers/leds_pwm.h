/*
 * leds_pwm.h
 */

#pragma once

#include <stm32f7xx.h>
#include "led_cont.h"

#define NUM_PWM_LED_CHIPS 10

void init_pwm_leds(void);

void set_pwm_led_direct(uint8_t led_id, uint16_t c_red, uint16_t c_green, uint16_t c_blue);
void set_pwm_led(uint8_t led_id, const o_rgb_led *rgbled);
void set_single_pwm_led(uint8_t single_element_led_id, uint16_t brightness);

void pwm_leds_display_on(void);
void pwm_leds_display_off(void);


