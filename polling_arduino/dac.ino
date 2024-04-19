/**
 * \file        dac.h
 * \brief
 * \details
 * \author      MST_CDA
 * \version     0.0.1
 * \date        07/04/2024
 * \copyright   Unlicensed
 */

#ifndef __DAC_
#define __DAC_

// #include <stdint.h>
// #include "hardware/timer.h"

#define RESOLUTION  255         // 8 bits
#define DAC_RANGE   9400        // 0 to 9.3V

/**
 * @typedef dac_t 
 *
 * @brief Structure to manage a 8-bit DAC
 * 
 */
typedef struct{
    struct {
        byte bit0     : 1;
        byte bit1     : 1;
        byte bit2     : 1;
        byte bit3     : 1;
        byte bit4     : 1;
        byte bit5     : 1;
        byte bit6     : 1;
        byte bit7     : 1;
    }BITS;
    bool en;                        ///< Enable DAC
    byte gpio_lsb;               ///< The LSB position of the GPIOs used to output the DAC signal
    short digit_v;               ///< Value to be outputed
}dac_t;

/**
 * @brief Initialize a 8-bit DAC. A DAC0808 is used as reference. 
 * 
 * @param dac 
 * @param pin_lsb   The LSB position of the GPIOs used to output the DAC signal
 * @param en        Enable DAC
 */
void dac_init(dac_t *dac, byte gpio_lsb, bool en)
{
    dac->gpio_lsb = gpio_lsb;
    dac->digit_v = 0;
    dac->en = en;

    gpio_init_mask(0x000000FF << dac->gpio_lsb);
    gpio_set_dir_masked(0x000000FF << dac->gpio_lsb, 0x000000FF << dac->gpio_lsb); // Set all gpios as outputs
}

/**
 * @brief Generate BITS(8-bits) from the input value
 * 
 * @param dac 
 * @param decim_v 
 */
void dac_calculate(dac_t *dac, int16_t decim_v)
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

/**
 * @brief With BITS, output the signal.
 * 
 * @param dac 
 */
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

#endif // __DAC_