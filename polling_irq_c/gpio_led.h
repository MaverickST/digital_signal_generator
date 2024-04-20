/**
 * \file
 * \brief
 * \details
 * \author      Ricardo Andres Velasquez Velez
 * \version     0.0.1
 * \date        05/10/2023
 * \copyright   Unlicensed
 */

#ifndef __GPIO_LED_H__
#define __GPIO_LED_H__

#include <stdint.h>
#include "hardware/gpio.h"

static inline void led_init(uint8_t gpio_num){
    gpio_init( gpio_num); // gpios for key rows 2,3,4,5
    gpio_set_dir(gpio_num,true); // rows as outputs and cols as inputs
    gpio_put(gpio_num,false);
}

static inline void led_on(uint8_t gpio_num){
    gpio_put(gpio_num,true);
}

static inline void led_off(uint8_t gpio_num){
    gpio_put(gpio_num,false);
}

static inline void led_toggle(uint8_t gpio_num){
    gpio_xor_mask(0x00000001 << gpio_num);
}

#endif