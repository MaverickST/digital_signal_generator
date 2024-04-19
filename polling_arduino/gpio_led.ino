/**
 * \file        gpio_led.h
 * \brief       Define some utility methods to control a LED through a GPIO
 * \details
 * \author      Ricardo Andres Velasquez Velez
 * \version     0.0.1
 * \date        05/10/2023
 * \copyright   Unlicensed
 */

#ifndef __GPIO_LED_H__
#define __GPIO_LED_H__

// #include <stdint.h>
// #include "hardware/gpio.h"

/// @brief Initialize a gpio to drive a LED
/// @param gpio_num gpio that drives the LED
static inline void led_init(byte gpio_num){
    gpio_init( gpio_num); // gpios for key rows 2,3,4,5
    gpio_set_dir(gpio_num,true); // rows as outputs and cols as inputs
    gpio_put(gpio_num,false);
}

/// @brief Turn on the LED
/// @param gpio_num gpio that drives the LED
static inline void led_on(byte gpio_num){
    gpio_put(gpio_num,true);
}

/// @brief Turn off the LED
/// @param gpio_num gpio that drives the LED
static inline void led_off(byte gpio_num){
    gpio_put(gpio_num,false);
}
/// @brief Toggle the LED
/// @param gpio_num gpio that drives the LED
static inline void led_toggle(byte gpio_num){
    gpio_xor_mask(0x00000001 << gpio_num);
}

#endif