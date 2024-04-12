/**
 * \file        functs.c
 * \brief
 * \details
 * 
 * 
 * \author      MST_CDA
 * \version     0.0.1
 * \date        10/04/2024
 * \copyright   Unlicensed
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"

#include "functs.h"
#include "keypad_irq.h"
#include "gpio_led.h"
#include "gpio_button_irq.h"
#include "signal_generator_irq.h"
#include "dac.h"


volatile uint8_t gKeyCnt = 0;
volatile uint8_t gSeqCnt = 0;
volatile bool gDZero = false;

key_pad_t gKeyPad;
signal_t gSignal;
gpio_button_t gButton;
dac_t gDac;


void initGlobalVariables(void)
{
    kp_init(&gKeyPad,2,6,true);
    signal_gen_init(&gSignal, 10, 1000, 500, true);
    button_init(&gButton, 0);
    dac_init(&gDac, 10, true);
}

void initPWMasPIT(uint8_t slice, uint16_t milis, bool enable)
{
    assert(milis<=262);                  ///< PWM can manage interrupt periods greater than 262 milis
    float prescaler = (float)SYS_CLK_KHZ/500;
    assert(prescaler<256); ///< the integer part of the clock divider can be greater than 255 
                 // ||   counter frecuency    ||| Period in seconds taking into account de phase correct mode |||   
    uint32_t wrap = (1000*SYS_CLK_KHZ/prescaler)*(milis/(2*1000)); // 500000*milis/2000
    assert(wrap<((1UL<<17)-1));
    // Configuring the PWM
    pwm_config cfg =  pwm_get_default_config();
    pwm_config_set_phase_correct(&cfg, true);
    pwm_config_set_clkdiv(&cfg, prescaler);
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_FREE_RUNNING);
    pwm_config_set_wrap(&cfg, wrap);
    pwm_set_irq_enabled(slice, true);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_init(slice, &cfg, enable);
 }

 void pwmIRQ(void)
 {
    uint32_t cols;
    bool button;
    switch (pwm_get_irq_status_mask())
    {
    case 0x01UL: ///< PWM slice 0 ISR used as a PIT to generate row sequence
        kp_gen_seq(&gKeyPad);
        pwm_clear_irq(0);         ///< Acknowledge slice 0 PWM IRQ
        break;

    case 0x02UL: ///< PWM slice 1 ISR used as a PIT to implement the keypad debouncer
        cols = gpio_get_all() & 0x000003C0; ///< Get columns gpio values
        if(kp_is_2nd_zero(&gKeyPad)){
            if(!cols){
                kp_set_irq_enabled(&gKeyPad, true); ///< Enable the GPIO IRQs
                pwm_set_enabled(0, true);    ///< Enable the row sequence
                pwm_set_enabled(1, false);   ///< Disable the keypad debouncer
                gKeyPad.KEY.dbnc = 0;
            }
            else
                kp_clr_zflag(&gKeyPad);
        }
        else{
            if(!cols)
                kp_set_zflag(&gKeyPad);
        }

        pwm_clear_irq(1); ///< Acknowledge slice 1 PWM IRQ
        break;

    case 0x04UL: ///< PWM slice 2 ISR used as a PIT to implement the button debouncer
        button = gpio_get(gButton.KEY.gpio_num);
        if(button_is_2nd_zero(&gButton)){
            if(!button){
                signal_set_state(&gSignal, (gSignal.STATE.ss + 1)%4);
                button_set_irq_enabled(&gButton, true); ///< Disable the GPIO IRQs
                pwm_set_enabled(2, false);    ///< Disable the button debouncer
                gButton.KEY.dbnc = 0;
            }
            else
                button_clr_zflag(&gButton);
        }
        else{
            if(!button)
                button_set_zflag(&gButton);
        }
        pwm_clear_irq(2); ///< Acknowledge slice 2 PWM IRQ
        break;

    default:
        printf("Happend what should not happens on PWM IRQ\n");
        break;
    }
 }

 void initTimer(void)
 {
    /// claim alarm0 for signal value calculation
    if(!hardware_alarm_is_claimed (0))
        hardware_alarm_claim(0);
    else
        printf("Tenemos un problemaj alarm 0\n");

    /// Set callback for each alarm. TODO: replace with an exclusive handler
    hardware_alarm_set_callback(0,timerCallback);

    timer_hw->intr = 0x00000001; // Clear/enable alarm0 interrupt
    timer_hw->alarm[0] = (uint32_t)(time_us_64() + gSignal.t_sample); // Set alarm0 to trigger in t_sample
 }

 void keypadCallback(uint num, uint32_t mask)
 {
 }

 void buttonCallback(uint num, uint32_t mask)
 {
 }


 void timerCallback(uint num)
 {
    // Perform the signal value calculation and output to the DAC
    signal_calculate_next_value(&gSignal);
    dac_calculate(&gDac,gSignal.value);

    timer_hw->intr = 0x00000001; // Clear/enable alarm0 interruption
    timer_hw->alarm[0] = (uint32_t)(time_us_64() + gSignal.t_sample); // Set alarm0 to trigger in t_sample
 }
