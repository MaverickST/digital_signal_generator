/**
 * \file        signal_generator.h
 * \brief
 * \details
 * \author      MST_CDA
 * \version     0.0.1
 * \date        07/04/2024
 * \copyright   Unlicensed
 */

#ifndef __SIGNAL_GENERATOR_
#define __SIGNAL_GENERATOR_

#define M_PI		    3.14159265358979323846	/* pi */
#define S_TO_US     1000000
#define US_TO_S     0.000001
#define SAMPLE      16


// #include <stdint.h>
// #include <math.h>
// #include <stdio.h>
// #include "hardware/timer.h"
// #include "time_base.ino"

/**
 * @typedef signal_t 
 * 
 * @brief Structure to manage a signal generator
 * 
 */
typedef struct{
    struct{
        byte ss      : 2; // 0: Sinusoidal, 1: Triangular, 2: Saw tooth, 3: Square
        byte en      : 1; // Enable signal generation
    }STATE;
    unsigned int freq;          // Signal frequency
    short amp;           // Signal amplitude
    short offset;        // Signal offset
    short value;          // Signal value
    short arrayV[SAMPLE]; // Array to store the signal values for the DAC
    byte cnt;            // Time variable
    time_base_t tb_gen;     // Time base for signal generation

}signal_t;

/**
 * @brief 
 * 
 * @param signal 
 * @param freq 
 * @param amp 
 * @param offset 
 * @param en 
 */
void signal_gen_init(signal_t *signal, unsigned int freq, short amp, short offset, bool en)
{
  signal->freq = freq;
  signal->amp = amp;
  signal->offset = offset;
  signal->value = 0;
  signal->STATE.en = en;
  signal->STATE.ss = 0;
  signal->cnt = 0;
  tb_init(&signal->tb_gen,S_TO_US/(SAMPLE*freq),true);
}

// ------------------------------------------------------------------
// ---------------------- GETTING WAVE VALUES ----------------------
// ------------------------------------------------------------------

static inline void signal_gen_sin(signal_t *signal, byte t){
    signal->value = (short)(signal->offset + signal->amp*sin((2*M_PI*t)/SAMPLE));
}

static inline void signal_gen_tri(signal_t *signal, byte t){
    if (t <= SAMPLE/2){
        signal->value = (short)(signal->offset + (4*signal->amp*t)/SAMPLE - signal->amp);
    }
    else {
        signal->value = (short)(signal->offset - (4*signal->amp*t)/SAMPLE + 3*signal->amp);  
    }
}

static inline void signal_gen_saw(signal_t *signal, byte t){
    signal->value = (short)(signal->offset + (2*signal->amp*t)/SAMPLE  - signal->amp);
}

static inline void signal_gen_sqr(signal_t *signal, byte t){
    if (t <= SAMPLE/2){
        signal->value = (short)(signal->offset + signal->amp);
    }
    else {
        signal->value = (short)(signal->offset - signal->amp);
    }
}

// ------------------------------------------------------------------

static inline void signal_calculate_next_value(signal_t *signal){

    if(!signal->STATE.en) return; 

    switch(signal->STATE.ss){ // Calculate next signal value
        case 0: // Sinusoidal
            for (byte i = 1; i <= SAMPLE; i++){
                signal_gen_sin(signal, i);
                signal->arrayV[i - 1] = signal->value - 800; // The -500 is to set the bias of the DAC.
            }
            break;
        case 1: // Triangular
            for (byte i = 1; i <= SAMPLE; i++){
                signal_gen_tri(signal, i);
                signal->arrayV[i - 1] = signal->value - 800; // The -500 is to set the bias of the DAC.
            }
            break;
        case 2: // Saw tooth
            for (byte i = 1; i <= SAMPLE; i++){
                signal_gen_saw(signal, i);
                signal->arrayV[i - 1] = signal->value - 800; // The -500 is to set the bias of the DAC.
            }
            break;
        case 3: // Square
            for (byte i = 1; i <= SAMPLE; i++){
                signal_gen_sqr(signal, i);
                signal->arrayV[i - 1] = signal->value - 800; // The -500 is to set the bias of the DAC.
            }
            break;
    }

}

static inline short signal_get_value(signal_t *signal){
    return signal->value;
}

static inline void signal_set_state(signal_t *signal, byte ss){
    signal->STATE.ss = ss;
}

static inline void signal_set_amp(signal_t *signal, short amp){
    signal->amp = amp;
}

static inline void signal_set_offset(signal_t *signal, short offset){
    signal->offset = offset;
}

static inline void signal_set_freq(signal_t *signal, unsigned int freq){
    signal->freq = freq;
    tb_set_delta(&signal->tb_gen,S_TO_US/(SAMPLE*freq)); // Nyquist theorem
}

static inline void signal_gen_enable(signal_t *signal){
    signal->STATE.en = 1;
    tb_enable(&signal->tb_gen);
}

static inline void signal_gen_disable(signal_t *signal){
    signal->STATE.en = 0;
    tb_disable(&signal->tb_gen);
}

#endif // __SIGNAL_GENERATOR_