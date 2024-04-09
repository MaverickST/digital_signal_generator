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

#define M_PI		3.14159265358979323846	/* pi */
#define MAX_TIME    1000000     // 1000000us = 1s
#define MIN_TIME    0.000001    // 1us = 0.000001s
#define RESOLUTION  255         // 8 bits

#include <stdint.h>
#include "hardware/timer.h"
#include "time_base.h"

typedef struct{
    struct{
        uint8_t signal_state    : 2; // 0: Sinusoidal, 1: Triangular, 2: Saw tooth, 3: Square
        uint8_t en              : 1; // Enable signal generation
    }STATE;
    uint32_t freq;      // Signal frequency
    uint16_t amp;       // Signal amplitude
    uint16_t offset;    // Signal offset
    int16_t value;     // Signal value
    time_base_t tb_gen; // Time base for signal generation

}signal_t;

void signal_gen_init(signal_t *signal, uint32_t freq, uint16_t amp, uint16_t offset, uint64_t gen_time, bool en);

static inline void signal_gen_sin(signal_t *signal){
    signal->value = (int16_t)(signal->offset + signal->amp*sin(2*M_PI*signal->freq*time_us_64()*MIN_TIME));
}

static inline void signal_gen_tri(signal_t *signal){
    if (time_us_64()%(MAX_TIME/signal->freq) <= MAX_TIME/(2*signal->freq)){
        signal->value = (int16_t)(signal->offset + 4*signal->amp*signal->freq*(time_us_64()%(MAX_TIME/signal->freq))*MIN_TIME);
    }
    else {
        signal->value = (int16_t)(signal->offset - 4*signal->amp*signal->freq*(time_us_64()%(MAX_TIME/signal->freq))*MIN_TIME);  
    }
}

static inline void signal_gen_saw(signal_t *signal){
    signal->value = (int16_t)(signal->offset + 2*signal->amp*signal->freq*(time_us_64()%(MAX_TIME/signal->freq))*MIN_TIME);
}

static inline void signal_gen_sqr(signal_t *signal){
    if (time_us_64()%(MAX_TIME/signal->freq) <= MAX_TIME/(2*signal->freq)){
        signal->value = (int16_t)(signal->offset + signal->amp);
    }
    else {
        signal->value = (int16_t)(signal->offset - signal->amp);
    }
}

static inline void signal_calculate_next_signal(signal_t *signal){
    switch(signal->STATE.signal_state){ // Calculate next signal value
        case 0: // Sinusoidal
            signal_gen_sin(signal);
            break;
        case 1: // Triangular
            signal_gen_tri(signal);
            break;
        case 2: // Saw tooth
            signal_gen_saw(signal);
            break;
        case 3: // Square
            signal_gen_sqr(signal);
            break;
    }
    signal->value = (signal->value/(signal->amp + signal->offset))*RESOLUTION; // Normalize to 8 bits
    tb_next(&signal->tb_gen);
}

static inline uint16_t signal_get_value(signal_t *signal){
    return signal->value;
}

static inline void signal_set_state(signal_t *signal, uint8_t signal_state){
    signal->STATE.signal_state = signal_state;
}

static inline void signal_gen_enable(signal_t *signal){
    signal->STATE.en = 1;
    tb_enable(&signal->tb_gen);
}

static inline void signal_gen_disable(signal_t *signal){
    signal->STATE.en = 0;
    tb_disable(&signal->tb_gen);
}

static inline void signal_set_amp(signal_t *signal, uint16_t amp){
    signal->amp = amp;
}

static inline void signal_set_offset(signal_t *signal, uint16_t offset){
    signal->offset = offset;
}

static inline void signal_set_freq(signal_t *signal, uint32_t freq){
    signal->freq = freq;
    tb_set_delta(&signal->tb_gen,1000000/(2*freq)); // Nyquist theorem
}

#endif // __SIGNAL_GENERATOR_