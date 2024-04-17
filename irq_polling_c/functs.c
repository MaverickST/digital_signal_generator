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
#include "keypad.h"
#include "gpio_led.h"
#include "gpio_button.h"
#include "signal_generator.h"
#include "dac.h"


volatile uint8_t gKeyCnt = 0;
volatile uint8_t gSeqCnt = 0;
volatile bool gDZero = false;

key_pad_t gKeyPad;
signal_t gSignal;
gpio_button_t gButton;
dac_t gDac;

volatile flags_t gFlags;

void initGlobalVariables(void)
{
    kp_init(&gKeyPad,6,2,true);
    signal_gen_init(&gSignal, 10, 1000, 500, true);
    button_init(&gButton, 0);
    dac_init(&gDac, 10, true);
}

void initPWMasPIT(uint8_t slice, uint16_t milis, bool enable)
{
    assert(milis<=262);                  // PWM can manage interrupt periods greater than 262 milis
    float prescaler = (float)SYS_CLK_KHZ/500;
    assert(prescaler<256); // the integer part of the clock divider can be greater than 255 
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
    case 0x01UL: // PWM slice 0 ISR used as a PIT to generate row sequence
        kp_gen_seq(&gKeyPad);
        pwm_clear_irq(0);         // Acknowledge slice 0 PWM IRQ
        break;

    case 0x02UL: // PWM slice 1 ISR used as a PIT to implement the keypad debouncer
        gFlags.B.keyDbnc = true;
        cols = gpio_get_all() & 0x000003C0; // Get columns gpio values
        pwm_clear_irq(1); // Acknowledge slice 1 PWM IRQ
        break;

    case 0x04UL: // PWM slice 2 ISR used as a PIT to implement the button debouncer
        button = gpio_get(gButton.KEY.gpio_num);
        if(button_is_2nd_zero(&gButton)){
            if(!button){
                signal_set_state(&gSignal, (gSignal.STATE.ss + 1)%4);
                button_set_irq_enabled(&gButton, true); // Disable the GPIO IRQs
                pwm_set_enabled(2, false);    // Disable the button debouncer
                gButton.KEY.dbnc = 0;
            }
            else
                button_clr_zflag(&gButton);
        }
        else{
            if(!button)
                button_set_zflag(&gButton);
        }
        pwm_clear_irq(2); // Acknowledge slice 2 PWM IRQ
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
    
    /// claim alarm0 for signal value calculation
    if(!hardware_alarm_is_claimed (1))
        hardware_alarm_claim(1);
    else
        printf("Tenemos un problemaj alarm 1\n");

    /// Set callback for each alarm. TODO: replace with an exclusive handler
    hardware_alarm_set_callback(0, timerSignalCallback);
    hardware_alarm_set_callback(1, timerPrintCallback);

    timer_hw->intr = 0x00000003; // Clear/enable alarm0, alarm1 interrupt
    timer_hw->alarm[0] = (uint32_t)(time_us_64() + gSignal.t_sample); // Set alarm0 to trigger in t_sample
    timer_hw->alarm[1] = (uint32_t)(time_us_64() + 1000000); // Set alarm1 to trigger in 1s
 }

 void keypadCallback(uint num, uint32_t mask)
 {
    gFlags.B.keyFlag = true;
    printf("// Capture the key pressed");
    // Capture the key pressed
    uint32_t cols = gpio_get_all() & 0x000003C0; // Get columns gpio values
    kp_capture(&gKeyPad, cols);
    pwm_set_enabled(0,false);                                           ///< Froze the row sequence
    pwm_set_enabled(1,true);
    gpio_set_irq_enabled(6,GPIO_IRQ_EDGE_RISE,false);  
    gpio_set_irq_enabled(7,GPIO_IRQ_EDGE_RISE,false);  
    gpio_set_irq_enabled(8,GPIO_IRQ_EDGE_RISE,false);  
    gpio_set_irq_enabled(9,GPIO_IRQ_EDGE_RISE,false);
    gpio_acknowledge_irq(num, mask); // gpio IRQ acknowledge
 }

 void buttonCallback(uint num, uint32_t mask)
 {
    pwm_set_enabled(2, true); // Enable the button debouncer
    button_set_irq_enabled(&gButton, false); // Disable the button IRQs

    button_set_zflag(&gButton); // Set the flag that indicates that a zero was detected on button
    gButton.KEY.dbnc = 1;

    gpio_acknowledge_irq(num, mask); // gpio IRQ acknowledge
 }


 void timerSignalCallback(uint num)
 {
    // Perform the signal value calculation and output to the DAC
    signal_calculate_next_value(&gSignal);
    dac_calculate(&gDac,gSignal.value);

    timer_hw->intr = 0x00000001; // Clear/enable alarm0 interruption
    timer_hw->alarm[0] = (uint32_t)(time_us_64() + gSignal.t_sample); // Set alarm0 to trigger in t_sample
 }

 void timerPrintCallback(uint num)
 {
    // Print the signal characteristics
    switch (gSignal.STATE.ss){
        case 0:
            printf("Sinusoidal: ");
            break;
        case 1:
            printf("Triangular: ");
            break;
        case 2:
            printf("Sawtooth: ");
            break;
        case 3:
            printf("Square: ");
            break;
    }
    printf("Amp: %d, Offset: %d, Freq: %d\n", gSignal.amp, gSignal.offset, gSignal.freq);

    gFlags.B.printFlag = 0; // Clear the print flag interrup

    timer_hw->intr = 0x00000002; // Clear/enable alarm1 interruption
    timer_hw->alarm[1] = (uint32_t)(time_us_64() + 1000000); // Set alarm1 to trigger in 1s
 }

void program(){
    if(gFlags.B.keyFlag){
        pwm_set_enabled(0, false);  // Disable the row sequence
        pwm_set_enabled(1, true);   // Enable the keypad debouncer
        kp_set_irq_enabled(&gKeyPad, false); // Disable the keypad IRQs

        kp_set_zflag(&gKeyPad); // Set the flag that indicates that a zero was detected on keypad
        gKeyPad.KEY.dbnc = 1;

        // Auxiliar variables
        static uint8_t in_param_state = 0x00; // 0: Nothing, 1: Entering amp (A), 2: Entering offset (B), 3: Entering freq (C)
        static uint32_t param = 0; // This variable will store the value of any parameter that is being entered
        static uint8_t key_cont = 0;


        // Process the key pressed 

        // To accept a number, in_param_state must be different of 0
        if(checkNumber(gKeyPad.KEY.dkey) && in_param_state){
            param = (!key_cont)? gKeyPad.KEY.dkey : param*10 + gKeyPad.KEY.dkey;
            key_cont++;
            printf("El valor es %d.\n", param);
        }
        // To accept a letter different of 0x0D, in_param_state must be 0
        else if(checkLetter(gKeyPad.KEY.dkey) && !in_param_state){
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            // led_on(my_led);
            switch (gKeyPad.KEY.dkey)
            {
            case 0x0A:
                in_param_state = 1;
                break;
            case 0x0B:
                in_param_state = 2;
                break;
            case 0x0C:
                in_param_state = 3;
                break;
            default:
                printf("Invalid letter\n");
                break;
            }
        }
        // To accept a 0x0D, in_param_state must be different of 0
        else if(gKeyPad.KEY.dkey == 0x0D && in_param_state){
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            // led_off(my_led);
            switch (in_param_state)
            {
            case 1:
                if(checkAmp(param)){
                    signal_set_amp(&gSignal,param);
                }
                break;
            case 2:
                if(checkOffset(param)){
                    signal_set_offset(&gSignal,param);
                }
                break;
            case 3:
                if(checkFreq(param)){
                    signal_set_freq(&gSignal,param);
                }
                break;
            default:
                printf("Invalid state\n");
                break;
            }
            in_param_state = 0;
            param = 0;
            key_cont = 0;
        }
        gKeyPad.KEY.nkey = 0;
        gFlags.B.keyFlag = false;
    }
    if(gFlags.B.keyDbnc){
        gFlags.B.keyDbnc = false;
    }
    if(gFlags.B.buttonFlag){
        gFlags.B.buttonFlag = false;
    }
    if(gFlags.B.buttonDbnc){
        gFlags.B.buttonDbnc = false;
    }
    if(gFlags.B.signalFlag){
        gFlags.B.signalFlag = false;
    }
    if(gFlags.B.printFlag){
        gFlags.B.printFlag = false;
    }
}

bool check(){
    if(gFlags.W){
        return true;
    }
    return false;
}