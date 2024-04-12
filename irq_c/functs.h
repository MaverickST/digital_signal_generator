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

void initGlobalVariables(void);

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

/**
 * @brief This function is the handler for the PWM interruptions.
 * 
 */
void pwmIRQ(void);

/**
 * @brief This function initializes the timer 0 to generate interruptions every 1ms.
 * 
 */
void initTimer(void);

/**
 * @brief Definition of the keypad callback function, which will be called by the handler of the GPIO interruptions.
 * 
 * @param num 
 * @param mask 
 */
void keypadCallback(uint num, uint32_t mask);

/**
 * @brief Definition of the button callback function, which will be called by the handler of the GPIO interruptions.
 * 
 * @param num 
 * @param mask 
 */
void buttonCallback(uint num, uint32_t mask);

/**
 * @brief Definition of the timer callback function, which will be called by the handler of the timer interruptions.
 * 
 * @param num Alarm number that triggered the interruption
 */
void timerCallback(uint num);

#endif // FUNTCS

