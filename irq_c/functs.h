/**
 * \file        functs.h
 * \brief
 * \details
 * \author      MST_CDA
 * \version     0.0.1
 * \date        10/04/2024
 * \copyright   Unlicensed
 */

#ifndef __FUNTCS_
#define __FUNTCS_

#include <stdint.h>
#include "hardware/gpio.h"


/**
 * @brief This function initializes a PWM signal using a periodic interrupt timer (PIT).
 * Each slice will generate interruptions at a period of milis miliseconds.
 * Due to each slice share clock counter (period), events with diferents periods 
 * must not be generated in the same slice, i.e, they must not share channel.
 * 
 * @param slice 
 * @param milis Period of the PWM interruption in miliseconds
 * @param enable 
 */
void initPWMasPIT(uint8_t slice, uint16_t milis, bool enable);

void pwmIRQ(void);

#endif // FUNTCS

