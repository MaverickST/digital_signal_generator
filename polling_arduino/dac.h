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

#include "api/Common.h"
#include "api/Compat.h"

#define RESOLUTION  255         // 8 bits
#define DAC_RANGE   10120        // 0 to 9.3V
#define DAC_BIAS    -60         // DAC bias

// typedef unsigned short uint16_t;

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
  uint8_t gpio_lsb;               ///< The LSB position of the GPIOs used to output the DAC signal
  uint16_t digit_v;               ///< Value to be outputed
}dac_t;

/**
 * @brief Initialize a 8-bit DAC. A DAC0808 is used as reference. 
 * 
 * @param dac 
 * @param pin_lsb   The LSB position of the GPIOs used to output the DAC signal
 * @param en        Enable DAC
 */
void dac_init(dac_t *dac, uint8_t gpio_lsb, bool en)
{
  dac->gpio_lsb = gpio_lsb;
  dac->digit_v = 0;
  dac->en = en;

  for (int i = 0; i < 8; i++) {
    pinMode(dac->gpio_lsb + i, OUTPUT);
  }
}

/**
 * @brief With BITS, output the signal.
 * 
 * @param dac 
 */
void dac_output(dac_t *dac)
{
  if(!dac->en) return;

  digitalWrite(dac->gpio_lsb + 0, dac->BITS.bit0);
  digitalWrite(dac->gpio_lsb + 1, dac->BITS.bit1);
  digitalWrite(dac->gpio_lsb + 2, dac->BITS.bit2);
  digitalWrite(dac->gpio_lsb + 3, dac->BITS.bit3);
  digitalWrite(dac->gpio_lsb + 4, dac->BITS.bit4);
  digitalWrite(dac->gpio_lsb + 5, dac->BITS.bit5);
  digitalWrite(dac->gpio_lsb + 6, dac->BITS.bit6);
  digitalWrite(dac->gpio_lsb + 7, dac->BITS.bit7);
    
}

/**
 * @brief Generate BITS(8-bits) from the input value
 * 
 * @param dac 
 * @param decim_v 
 */
void dac_calculate(dac_t *dac, int16_t decim_v)
{
    
  dac->digit_v = (decim_v + (int16_t)DAC_BIAS + 5000)*RESOLUTION/DAC_RANGE ; // normalize to 8 bits
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



#endif // __DAC