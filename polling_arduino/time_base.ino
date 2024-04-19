/**
 * \file
 * \brief
 * \details
 * \author      Ricardo Andres Velasquez Velez
 * \version     0.0.1
 * \date        05/10/2023
 * \copyright   Unlicensed
 */

#ifndef __TIME_BASE_
#define __TIME_BASE_

// #include <stdint.h>
// #include "hardware/timer.h"

/** 
 * \typedef time_base_t
 * \brief this datatype enable the management of concurrent temporal events
 */
typedef struct{
    long next;                          ///< Struct member with the time for the next temporal event
    long delta;                         ///< Struct member with the event period in us
    bool en;                                ///< Enabler of the time base
} time_base_t;

/**
 * \brief This method initialize the time base structure
 * \param t     pointer to temporal structure
 * \param us    time base period
 * \param en    true if time base start enabled and false in other case
 */ 
void tb_init(time_base_t *t, long us, bool en)
{
    t->next = micros() + us ;
    t->delta = us;
    t->en = en;
}

/**
 * \brief Return true when the last period had lapsed and false if it is still going
 * \param t    Pointer to temporal structure
 * \param en   Enable the periodic base time
 * \return     True if time base period had lapsed and False when it hasn't
 */ 
static inline bool tb_check(time_base_t *t){
    return (micros() >= t->next) && t->en;
}

/// @brief update the tb to next temporal event with respect to the current time
/// @param t time base data structure
static inline void tb_update(time_base_t *t){
    t->next = micros() + t->delta;
}

/// @brief update the tb to next temporal event with respect to the last temporal event
/// @param t time base data structure
static inline void tb_next(time_base_t *t){
    t->next = t->next + t->delta;
}

/// @brief enable time base to generate temporal events
/// @param t time base data structure
static inline void tb_enable(time_base_t *t){
    t->en = true;
}
/// @brief disable time base, no events are generated with tb_check
/// @param t time base data structure
static inline void tb_disable(time_base_t *t){
    t->en = false;
}

static inline void tb_set_delta(time_base_t *t, long delta){
    t->delta = delta;
}

#endif