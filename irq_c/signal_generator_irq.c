/**
 * \file        signal_generator_irq.c
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
#include "signal_generator_irq.h"

/**
 * @brief 
 * 
 * @param signal 
 * @param freq 
 * @param amp 
 * @param offset 
 * @param en 
 */
void signal_gen_init(signal_t *signal, uint32_t freq, uint16_t amp, uint16_t offset, bool en)
{
    signal->freq = freq;
    signal->amp = amp;
    signal->offset = offset;
    signal->value = 0;
    signal->STATE.en = en;
    signal->STATE.signal_state = 0;
    tb_init(&signal->tb_gen,MAX_TIME/(2*freq),true);
}
