/**
 * \file        keypad_polling_irq.c
 * \brief
 * \details
 * \author      MST_CDA
 * \version     0.0.1
 * \date        05/10/2023
 * \copyright   Unlicensed
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "keypad_polling_irq.h"
#include "hardware/gpio.h"

void kp_init(key_pad_t *kpad, uint8_t rlsb, uint8_t clsb, uint64_t dbnc_time, bool en){
    // Initialize history buffer
    for(int i=0; i<10;i++){
        kpad->history[i] = 0xFF;
    }
    // Verify that the keypad's rows and colums gpios do not overlap
    assert(abs(rlsb-clsb)>=4);
    kpad->KEY.rlsb = rlsb;
    kpad->KEY.clsb = clsb;
    kpad->KEY.ckey = 0x00;
    kpad->KEY.dkey = 0x1F;
    kpad->KEY.cnt = 0x0;
    kpad->KEY.seq = 0x8;
    kpad->KEY.nkey = 0;
    kpad->KEY.dbnc = 0;
    kpad->KEY.dzero = 0;
    if(en)
        kpad->KEY.en = 1;

    // Initialize keypad gpios
    gpio_init_mask(0x0000000F << kpad->KEY.rlsb); // gpios for key rows 2,3,4,5
    gpio_init_mask(0x0000000F << kpad->KEY.clsb); // gpios for key cols 6,7,8,9
    gpio_set_dir_masked(0x0000000F << kpad->KEY.rlsb,0x0000000F << kpad->KEY.rlsb); // rows as outputs and cols as inputs
    gpio_set_dir_masked(0x0000000F << kpad->KEY.clsb,0x00000000); // rows as outputs and cols as inputs
    gpio_pull_down(kpad->KEY.clsb);
    gpio_pull_down(kpad->KEY.clsb + 1);
    gpio_pull_down(kpad->KEY.clsb + 2);
    gpio_pull_down(kpad->KEY.clsb + 3);
}

void kp_decode(key_pad_t *kpad){
    switch (kpad->KEY.ckey)
    {
    case 0x88:
        kpad->KEY.dkey = 0x01;
        break;
    case 0x48:
        kpad->KEY.dkey = 0x02;
        break;
    case 0x28:
        kpad->KEY.dkey = 0x03;
        break;
    case 0x18:
        kpad->KEY.dkey = 0x0A;
        break;
    case 0x84:
        kpad->KEY.dkey = 0x04;
        break;
    case 0x44:
        kpad->KEY.dkey = 0x05;
        break;
    case 0x24:
        kpad->KEY.dkey = 0x06;
        break;
    case 0x14:
        kpad->KEY.dkey = 0x0B;
        break;
    case 0x82:
        kpad->KEY.dkey = 0x07;
        break;
    case 0x42:
        kpad->KEY.dkey = 0x08;
        break;
    case 0x22:
        kpad->KEY.dkey = 0x09;
        break;
    case 0x12:
        kpad->KEY.dkey = 0x0C;
        break;
    case 0x81:
        kpad->KEY.dkey = 0x0E;
        break;
    case 0x41:
        kpad->KEY.dkey = 0x00;
        break;
    case 0x21:
        kpad->KEY.dkey = 0x0F;
        break;
    case 0x11:
        kpad->KEY.dkey = 0x0D;
        break;
    }
    
}

void kp_capture(key_pad_t *kpad, uint32_t cols){
    kpad->KEY.ckey = (cols >> 2) | kpad->KEY.seq;
    kp_decode(kpad);
    for(int i=0;i<9;i++){
        kpad->history[9-i] = kpad->history[9-i-1];
    }
    kpad->history[0] = kpad->KEY.dkey;
    kpad->KEY.nkey = 1;
}