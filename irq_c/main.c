/**
 * \file        main.c
 * \brief       This proyect implements an digital signal generator. 
 * \details     In this practice, we will develop a Digital Signal Generator (DSG) device. 
 * The DSG device generates 4 different waveforms: sine, triangular, sawtooth, and square. 
 * The user can select the type of waveform generated by the DSG using a push button. 
 * Each time the button is pressed, the system switches from one waveform to the next sequentially. 
 * The amplitude, DC level (offset), and signal frequency are entered using a 4x4 matrix keypad. 
 * To enter the amplitude, the user will press the A key followed by the desired voltage value in 
 * millivolts and the D key to finalize. Similarly, to enter the DC level, the user will press 
 * the B key followed by the desired DC voltage value in millivolts and the D key to finalize. 
 * Finally, to enter the frequency, the user must press the C key followed by the frequency 
 * value in Hertz and the D key to finalize. The current values of Amplitude, DC Level, and 
 * Frequency along with the current waveform will be printed every second via the serial or USB 
 * interface to a terminal tool. 
 * 
 * The Amplitude should be adjustable between 100mV and 2500mV, the DC Level between 50mV and 1250mV. 
 * The frequency should be adjustable between 1 Hz and 12000000 Hz. 
 * Therefore, the max range for the value of the signal is: -2450mV to 3750mV.
 * 
 * By default, the DSG device starts generating a sinusoidal signal with an amplitude of 1000 mV, 
 * DC Level of 500mV, and frequency of 10Hz. The generated signal and its characteristics 
 * should be able to be verified by a measuring instrument, multimeter, or oscilloscope.  
 * 
 * ISR: Input Shift Register
 * PIT: Periodic Interrupt Timer
 * 
 * Interrupts:  button debouncer, keypad debouncer, secuence -> Slices 0,1,2 of the PWM
 *              value calculation, printing -> Timers 0, 1
 *              gpio columns -> GPIO 
 * 
 * \author      MST_CDA
 * \version     0.0.1
 * \date        05/10/2023
 * \copyright   Unlicensed
 * 
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/sync.h"

#include "functs.h"


int main() {
    stdio_init_all();
    printf("Hola!!!");

    // Initialize global variables: keypad, signal generator, button, and DAC.
    initGlobalVariables();

    // Initialize the PWM slices as PIT.
    initPWMasPIT(0,2,true);     // 2ms for the secuence generation
    initPWMasPIT(1,100,false);  // 100ms for the keypad debouncer
    initPWMasPIT(2,100, false); // 100ms for the button debouncer

    // Initialize two timers: one for the value calculation and the other for the printing.
    initTimer();

    // For the PWM interruption, it specifies the handler.
    irq_set_exclusive_handler(PWM_IRQ_WRAP,pwmIRQ);
    irq_set_priority(PWM_IRQ_WRAP, 0xC0);

    while(1){
        __wfi();
    }
}

