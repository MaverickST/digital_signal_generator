/**
 * \file
 * \brief
 * \details
 * \author      Ricardo Andres Velásquez
 * \version     0.0.1
 * \date        05/10/2023
 * \copyright   Unlicensed
 */

#include "time_base.h"
#include <stdint.h>

/**
 * \brief This method initialize the time base structure
 * \param t     pointer to temporal structure
 * \param us    time base period
 * \param en    true if time base start enabled and false in other case
 */ 
void tb_init(time_base_t *t, uint64_t us, bool en){
    t->next = time_us_64() + us ;
    t->delta = us;
    t->en = en;
}