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

# define M_PI		3.14159265358979323846	/* pi */

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
    uint16_t value;     // Signal value
    time_base_t tb_gen; // Time base for signal generation

}signal_t;



#endif // __SIGNAL_GENERATOR_