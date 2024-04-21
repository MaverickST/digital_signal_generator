/**
 * \file        signal_generator.c
 * \brief
 * \details
 * \author      MST_CDA
 * \version     0.0.1
 * \date        07/04/2024
 * \copyright   Unlicensed
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "signal_generator.h"


void signal_gen_init(signal_t *signal, uint32_t freq, uint16_t amp, uint16_t offset, bool en)
{
    signal->freq = freq;
    signal->amp = amp;
    signal->offset = offset;
    signal->value = 0;
    signal->STATE.en = en;
    signal->STATE.ss = 0;
    signal->cnt = 0;
    signal->t_sample = S_TO_US/(SAMPLE*freq);
}

void signal_calculate(signal_t *signal)
{
    if(!signal->STATE.en) return; 

    switch(signal->STATE.ss){ // Calculate next signal value
        case 0: // Sinusoidal
            for (uint8_t i = 1; i <= SAMPLE; i++){
                signal_gen_sin(signal, i);
                signal->arrayV[i - 1] = signal->value + DAC_BIAS;
            }
            break;
        case 1: // Triangular
            for (uint8_t i = 1; i <= SAMPLE; i++){
                signal_gen_tri(signal, i);
                signal->arrayV[i - 1] = signal->value + DAC_BIAS;
            }
            break;
        case 2: // Saw tooth
            for (uint8_t i = 1; i <= SAMPLE; i++){
                signal_gen_saw(signal, i);
                signal->arrayV[i - 1] = signal->value + DAC_BIAS;
            }
            break;
        case 3: // Square
            for (uint8_t i = 1; i <= SAMPLE; i++){
                signal_gen_sqr(signal, i);
                signal->arrayV[i - 1] = signal->value + DAC_BIAS;
            }
            break;
    }
}