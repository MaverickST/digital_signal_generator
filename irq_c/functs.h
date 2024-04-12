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

/**
 * @brief This function initializes the global variables of the system: keypad, signal generator, button, and DAC.
 * 
 */
void initGlobalVariables(void);

/**
 * @brief This function initializes a PWM signal as a periodic interrupt timer (PIT).
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

// -------------------------------------------------------------
// ---------------------- Callback functions -------------------
// -------------------------------------------------------------

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
 * @brief Definition of the signal callback function, which will be called by the handler of the timer interruptions.
 * Performs the signal generation and DAC conversion.
 * 
 * @param num Alarm number that triggered the interruption
 */
void timerSignalCallback(uint num);

/**
 * @brief Definition of the printing callback function, which will be called by the handler of the timer interruptions.
 * Every second, the current values of Amplitude, DC Level, and Frequency along with 
 * the current waveform will be printed.
 * 
 * @param num Alarm number that triggered the interruption
 */
void timerPrintCallback(uint num);

// -------------------------------------------------------------
// ---------------------- Check functions ----------------------
// -------------------------------------------------------------

static inline bool checkNumber(uint8_t number){
    return (number >= 0 && number <= 9);
}

static inline bool checkLetter(uint8_t letter){
    return (letter >= 0x0A && letter <= 0x0C);
}

static inline bool checkFreq(uint32_t freq){
    return (freq >= 1 && freq <= 12000000);
}

static inline bool checkAmp(uint32_t amp){
    return (amp >= 100 && amp <= 2500);
}

static inline bool checkOffset(uint32_t offset){
    return (offset >= 50 && offset <= 1250);
}

#endif // FUNTCS

