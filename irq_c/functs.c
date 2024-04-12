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
    pwm_config_set_phase_correct(&cfg,true);
    pwm_config_set_clkdiv(&cfg,prescaler);
    pwm_config_set_clkdiv_mode(&cfg,PWM_DIV_FREE_RUNNING);
    pwm_config_set_wrap(&cfg,wrap);
    pwm_set_irq_enabled(slice,true);
    irq_set_enabled(PWM_IRQ_WRAP,true);
    pwm_init(slice,&cfg,enable);
 }

 void pwmIRQ(void)
 {
    uint32_t gpioValue;
    uint32_t keyc;
    switch (pwm_get_irq_status_mask())
    {
    case 0x01UL: ///< PWM slice 0 ISR used as a PIT to generate row sequence
        gSeqCnt = (gSeqCnt + 1) % 4;
        gpio_put_masked(0x0000003C,0x00000001 << (gSeqCnt+2));
        pwm_clear_irq(0);         ///< Acknowledge slice 0 PWM IRQ
        break;
    case 0x02UL: ///< PWM slice 1 ISR used as a PIT to implement the keypad debouncer
        keyc = gpio_get_all() & 0x000003C0; ///< Get columns gpio values
        if(gDZero){
            if(!keyc){
                pwm_set_enabled(0,true); ///< Froze the row sequence
                pwm_set_enabled(1,false);
                gpio_set_irq_enabled(6,GPIO_IRQ_EDGE_RISE,true);  
                gpio_set_irq_enabled(7,GPIO_IRQ_EDGE_RISE,true);  
                gpio_set_irq_enabled(8,GPIO_IRQ_EDGE_RISE,true);  
                gpio_set_irq_enabled(9,GPIO_IRQ_EDGE_RISE,true);
            }
            gDZero = false;
        }
        else{
            if(!keyc)
                gDZero = true;
        }

        pwm_clear_irq(1);                                               ///< Acknowledge slice 1 PWM IRQ
        break;
    case 0x04UL: ///< PWM slice 2 ISR used as a PIT to implement the button debouncer
    default:
        printf("Paso lo que no debería pasar en PWM IRQ\n");
        break;
    }
 }