/**
 * \file        signal_generator_irq.h
 * \brief
 * \details
 * \author      MST_CDA
 * \version     0.0.1
 * \date        07/04/2024
 * \copyright   Unlicensed
 */

#ifndef __SIGNAL_GENERATOR_
#define __SIGNAL_GENERATOR_

#define M_PI		3.14159265358979323846	/* pi */
#define S_TO_US    1000000     ///< 1000000us = 1s
#define US_TO_S    0.000001    // 1us = 0.000001s
#define RESOLUTION  255         // 8 bits
#define SAMPLE  70      // Nyquist theorem

#include <stdint.h>
#include <math.h>
#include "hardware/timer.h"

/**
 * @typedef signal_t 
 * 
 * @brief Structure to manage a signal generator
 * 
 */
typedef struct{
    struct{
        uint8_t ss      : 2;    // Signal State -> 0: Sinusoidal, 1: Triangular, 2: Saw tooth, 3: Square
        uint8_t en      : 1;    // Enable signal generation
    }STATE;
    uint32_t freq;          // Signal frequency
    uint16_t amp;           // Signal amplitude
    uint16_t offset;        // Signal offset
    int16_t value;          // Signal value
    int16_t arrayV[SAMPLE]; // Array to store the signal values for the DAC
    uint8_t cnt;            // Time variable
    uint16_t t_sample;      // Sample time
}signal_t;

/**
 * @brief Initialize the signal generator
 * 
 * @param signal 
 * @param freq in Hz
 * @param amp in mV
 * @param offset in mV
 * @param en 
 */
void signal_gen_init(signal_t *signal, uint32_t freq, uint16_t amp, uint16_t offset, bool en);

// ------------------------------------------------------------------
// ---------------------- GETTING WAVE VALUES ----------------------
// ------------------------------------------------------------------


/**
 * @brief This function calculates the values of the signal and stores them in the arrayV.
 * 
 * @param signal 
 */
void signal_calculate(signal_t *signal);

/**
 * @brief This function calculates the value of a sinusoidal signal
 * 
 * @param signal 
 * @param t 
 */
static inline void signal_gen_sin(signal_t *signal, uint8_t t)
{
    signal->value = (int16_t)(signal->offset + signal->amp*sin((2*M_PI*t)/SAMPLE));
}

/**
 * @brief This function calculates the value of a triangular signal
 * 
 * @param signal 
 * @param t 
 */
static inline void signal_gen_tri(signal_t *signal, uint8_t t)
{
    if (t <= SAMPLE/2){
        signal->value = (int16_t)(signal->offset + (4*signal->amp*t)/SAMPLE - signal->amp);
    }
    else {
        signal->value = (int16_t)(signal->offset - (4*signal->amp*t)/SAMPLE + 3*signal->amp);  
    }
}

/**
 * @brief This function calculates the value of a saw tooth signal
 * 
 * @param signal 
 * @param t 
 */
static inline void signal_gen_saw(signal_t *signal, uint8_t t)
{
    signal->value = (int16_t)(signal->offset + (2*signal->amp*t)/SAMPLE  - signal->amp);
}

/**
 * @brief This function calculates the value of a square signal
 * 
 * @param signal 
 * @param t 
 */
static inline void signal_gen_sqr(signal_t *signal, uint8_t t)
{
    if (t <= SAMPLE/2){
        signal->value = (int16_t)(signal->offset + signal->amp);
    }
    else {
        signal->value = (int16_t)(signal->offset - signal->amp);
    }
}

// ------------------------------------------------------------------
// ------------------------------------------------------------------

static inline int16_t signal_get_value(signal_t *signal){
    return signal->value;
}

static inline void signal_set_state(signal_t *signal, uint8_t ss){
    signal->STATE.ss = ss;
}

static inline void signal_set_amp(signal_t *signal, uint16_t amp){
    signal->amp = amp;
}

static inline void signal_set_offset(signal_t *signal, uint16_t offset){
    signal->offset = offset;
}

static inline void signal_set_freq(signal_t *signal, uint32_t freq){
    signal->freq = freq;
    signal->t_sample = S_TO_US/(SAMPLE*freq); // Nyquist theorem
}

static inline void signal_gen_enable(signal_t *signal){
    signal->STATE.en = 1;
}

static inline void signal_gen_disable(signal_t *signal){
    signal->STATE.en = 0;
}


#endif // __SIGNAL_GENERATOR_