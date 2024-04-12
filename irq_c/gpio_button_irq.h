/**
 * \file        gpio_button.h
 * \brief
 * \details
 * \author      MST_CDA
 * \version     0.0.1
 * \date        07/04/2024
 * \copyright   Unlicensed
 */

#ifndef __GPIO_BUTTON_
#define __GPIO_BUTTON_

#include <stdint.h>
#include "hardware/timer.h"
#include "hardware/gpio.h"

/**
 * @typedef gpio_button_t 
 *
 * @brief Structure to manage a single button connected to a GPIO.
 * 
 */
typedef struct{
    struct {
        uint8_t pin     : 6;        ///< GPIO pin number
        uint8_t en      : 1;        ///< Enable keypad processing
        uint8_t dzero   : 1;        ///< Flag for double zero
        uint8_t nkey    : 1;        ///< Flag that indicates that a key was pressed
        uint8_t dbnc    : 1;        ///< Flag that indicates that debouncer is active
    }KEY;
    time_base_t tb_dbnce;       ///< Periocic time base used to implement the key debouncer
}gpio_button_t;

/**
 * @brief 
 * 
 * @param button 
 * @param pin 
 * @param dbnc_time 
 * @param en 
 */
static inline void button_init(gpio_button_t *button, uint8_t pin, uint64_t dbnc_time, bool en){
    button->KEY.en = en;
    button->KEY.dzero = 0;
    button->KEY.nkey = 0;
    button->KEY.dbnc = 0;
    button->KEY.pin = pin;
    tb_init(&button->tb_dbnce, dbnc_time, false);

    gpio_init(button->KEY.pin);
    gpio_set_dir(button->KEY.pin, GPIO_IN);
    gpio_pull_down(button->KEY.pin);
}

/** 
 * \brief This method sets to one the flag that indicates that a zero was detected on button
 * \param button   Pointer to keypad data structure
 */
static inline void button_set_zflag(gpio_button_t *button){
    button->KEY.dzero = 1;
}
/** 
 * \brief This method clears the flag that indicates that a zero was detected on button
 * \param button   Pointer to keypad data structure
 */
static inline void button_clr_zflag(gpio_button_t *button){
    button->KEY.dzero = 0;
}

/** 
 * \brief This method returns true if a first zero was detected on button
 * \param button   Pointer to keypad data structure
 */
static inline bool button_is_2nd_zero(gpio_button_t *button){
    return button->KEY.dzero;
}


#endif