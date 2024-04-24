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

#include <stdint.h>
#include "hardware/timer.h"

#define RESOLUTION  255         // 8 bits
#define DAC_RANGE   9400        // 0 to 9.3V
#define DAC_BIAS    500         // DAC bias

/**
 * @typedef dac_t 
 *
 * @brief Structure to manage a 8-bit DAC
 * 
 */
typedef struct{
    struct {
        uint8_t bit0     : 1;
        uint8_t bit1     : 1;
        uint8_t bit2     : 1;
        uint8_t bit3     : 1;
        uint8_t bit4     : 1;
        uint8_t bit5     : 1;
        uint8_t bit6     : 1;
        uint8_t bit7     : 1;
    }BITS;
    bool en;                        ///< Enable DAC
    uint8_t gpio_lsb;                ///< The LSB position of the GPIOs used to output the DAC signal
    uint16_t digit_v;                 ///< Value to be outputed
}dac_t;

/**
 * @brief Initialize a 8-bit DAC. A DAC0808 is used as reference. 
 * 
 * @param dac 
 * @param pin_lsb   The LSB position of the GPIOs used to output the DAC signal
 * @param period    Period of the output signal
 * @param en        Enable DAC
 */
void dac_init(dac_t *dac, uint8_t gpio_lsb, bool en);

/**
 * @brief Generate BITS(8-bits) from the input value
 * 
 * @param dac 
 * @param decim_v 
 */
void dac_calculate(dac_t *dac, int16_t decim_v);

/**
 * @brief With BITS, output the signal.
 * 
 * @param dac 
 */
void dac_output(dac_t *dac);

#endif // __DAC_