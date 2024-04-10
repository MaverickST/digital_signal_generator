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
#include <math.h>
#include "signal_generator.h"
#include "time_base.h"
#include "hardware/gpio.h"

void signal_gen_init(signal_t *signal, uint32_t freq, uint16_t amp, uint16_t offset, bool en)
{
    signal->freq = freq;
    signal->amp = amp;
    signal->offset = offset;
    signal->gen_time = gen_time;
    signal->en = en;
    tb_init(&signal->tb_gen, MAX_TIME/freq, true);
}
