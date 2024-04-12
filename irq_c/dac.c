/**
 * \file        dac.c
 * \brief
 * \details
 * \author      MST_CDA
 * \version     0.0.1
 * \date        05/10/2023
 * \copyright   Unlicensed
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "hardware/gpio.h"
#include "dac.h"


void dac_init(dac_t *dac, uint8_t gpio_lsb, uint64_t period, bool en)
{
    dac->gpio_lsb = gpio_lsb;
    dac->digit_v = 0;
    dac->en = en;

    gpio_init_mask(0x000000FF << dac->gpio_lsb);
    gpio_set_dir_masked(0x000000FF << dac->gpio_lsb, 0x000000FF << dac->gpio_lsb); // Set all gpios as outputs
}

void dac_calculate(dac_t *dac, uint16_t decim_v)
{
    dac->digit_v = (decim_v + 5000)*RESOLUTION/DAC_RANGE ; // normalize to 8 bits
    dac->BITS.bit0 = (dac->digit_v & 0x01) >> 0;
    dac->BITS.bit1 = (dac->digit_v & 0x02) >> 1;
    dac->BITS.bit2 = (dac->digit_v & 0x04) >> 2;
    dac->BITS.bit3 = (dac->digit_v & 0x08) >> 3;
    dac->BITS.bit4 = (dac->digit_v & 0x10) >> 4;
    dac->BITS.bit5 = (dac->digit_v & 0x20) >> 5;
    dac->BITS.bit6 = (dac->digit_v & 0x40) >> 6;
    dac->BITS.bit7 = (dac->digit_v & 0x80) >> 7;

    dac_output(dac);
}

void dac_output(dac_t *dac)
{
    if(!dac->en) return;

    gpio_put(dac->gpio_lsb + 0, dac->BITS.bit0);
    gpio_put(dac->gpio_lsb + 1, dac->BITS.bit1);
    gpio_put(dac->gpio_lsb + 2, dac->BITS.bit2);
    gpio_put(dac->gpio_lsb + 3, dac->BITS.bit3);
    gpio_put(dac->gpio_lsb + 4, dac->BITS.bit4);
    gpio_put(dac->gpio_lsb + 5, dac->BITS.bit5);
    gpio_put(dac->gpio_lsb + 6, dac->BITS.bit6);
    gpio_put(dac->gpio_lsb + 7, dac->BITS.bit7);
    
}
