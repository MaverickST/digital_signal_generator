#include "api/Common.h"
/**
 * @file
 * @brief
 * @details
 * @author      Ricardo Andres Velásquez
 * @version     0.0.1
 * @date        05/10/2023
 * @copyright   Unlicensed
 */

#ifndef __KEYPAD_POLLING_
#define __KEYPAD_POLLING_

// #include <Arduino.h>
// #include <stdint.h>
// #include "time_base.h"

/**
 * @typedef key_pad_t
 * @brief Data strcuture to manage a matrix keypad of 16 keys
 */
typedef struct{
    struct key{
    uint8_t cols    : 4;
    uint8_t cnt     : 2;        ///< counter to generate row sequence
    uint8_t seq     : 4;        ///< store the current sequence state
    uint8_t ckey    : 8;        ///< captured key with position coding
    uint8_t dkey    : 5;        ///< captured key with decimal coding
    uint8_t rlsb    : 4;        ///< The rows LSB position for keypad rows, the four gpios for rows must be consecutives starting in position rlsb
    uint8_t clsb    : 4;        ///< The cols LSB position for keypad cols, the four gpios for cols must be consecutives starting in position clsb
    uint8_t en      : 1;        ///< Enable keypad processing
    uint8_t dzero   : 1;        ///< Flag for double zero
    uint8_t nkey    : 1;        ///< Flag that indicates that a key was pressed
    uint8_t dbnc    : 1;        ///< Flag that indicates that debouncer is active
    }KEY;                       ///< All key related information              
    time_base_t tb_seq;         ///< Periodic time base used to generate row sequence
    time_base_t tb_dbnce;       ///< Periocic time base used to implement the key debouncer
    uint8_t history[10];        ///< The last 10 pressed keys
}key_pad_t;

/**
 * @brief This method initializes the keypad data structure
 * @param kpad          pointer to keypad data structure
 * @param rlsb          LSB position of the first row GPIO
 * @param clsb          LSB position of the first col GPIO
 * @param dbnc_time     Time base period for keypad debouncer
 * @param en            True if keypad start enabled
 */ 
void kp_init(key_pad_t *kpad, uint8_t rlsb, uint8_t clsb, uint64_t dbnc_time, bool en)
{
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
    // Initialize keypad time bases
    tb_init(&kpad->tb_seq,2000,en);
    tb_init(&kpad->tb_dbnce,100000,false);

    // Initialize keypad gpios
    for (int i = 0; i < 4; i++) {
      pinMode(rlsb + i, OUTPUT);
      pinMode(clsb + i, INPUT_PULLDOWN);
    }
}

/** 
 * @brief This method decodes the postional coding of the key to its actual value in decimal
 * @param kpad   Pointer to keypad data structure
 */
void kp_decode(key_pad_t *kpad)
{
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

/**
 * @brief This method captures the currently pressed key. It should only be called when at lest one of the columns is different from zero
 * @param kpad      pointer to keypad data structure
 */
void kp_capture(key_pad_t *kpad)
{
    kpad->KEY.ckey = (kpad->KEY.cols << 4) | kpad->KEY.seq;
    kp_decode(kpad);
    for(int i=0;i<9;i++){
        kpad->history[9-i] = kpad->history[9-i-1];
    }
    kpad->history[0] = kpad->KEY.dkey;
    kpad->KEY.nkey = 1;
}

/**
 * @brief This method captures the currently values on the columns
 * @param kpad      pointer to keypad data structure
 */
void kp_captureCols(key_pad_t *kpad){
    kpad->KEY.cols = 0;
    bool press;
    for (int i = 0; i < 4; i++){
      press = digitalRead(kpad->KEY.clsb + i);
      kpad->KEY.cols |= (press << i);
    }
}

/** 
 * @brief This method updates the sequence value that drives the keypad rows. The sequence should be updated with a frequency higher than 100Hz
 * @param kpad   Pointer to keypad data structure
 */
static inline void kp_gen_seq(key_pad_t *kpad){
    kpad->KEY.cnt += 1;
    kpad->KEY.seq = 1 << kpad->KEY.cnt;
    for (int i = 0; i < 4; i++) {
      digitalWrite(kpad->KEY.rlsb + i, (kpad->KEY.seq >> i) & 0x1);
    }
    // gpio_put_masked(0x0000000F<<kpad->KEY.rlsb,((uint32_t)kpad->KEY.seq)<<kpad->KEY.rlsb);
}

/** 
 * @brief This method returns the value of the last pressed key in decimal coding
 * @param kpad   Pointer to keypad data structure
 */
static inline uint8_t kp_get_key(key_pad_t *kpad){
    kpad->KEY.nkey = false;
    return kpad->history[0];
}

/** 
 * @brief This method allows to access the last 10 pressed keys
 * @param kpad   Pointer to keypad data structure
 */
static inline uint8_t kp_get_keyh(key_pad_t *kpad, uint8_t n){
    return kpad->history[n%10];
}

/** 
 * @brief This method returns the value of the key with its positional coding
 * @param kpad   Pointer to keypad data structure
 */
static inline uint8_t kp_get_keyc(key_pad_t *kpad){
    return kpad->KEY.ckey;
}

/** 
 * @brief This method sets to one the flag that indicates that a zero was detected on keypad columns
 * @param kpad   Pointer to keypad data structure
 */
static inline void kp_set_zflag(key_pad_t *kpad){
    kpad->KEY.dzero = 1;
}
/** 
 * @brief This method clears the flag that indicates that a zero was detected on keypad columns
 * @param kpad   Pointer to keypad data structure
 */
static inline void kp_clr_zflag(key_pad_t *kpad){
    kpad->KEY.dzero = 0;
}

/** 
 * @brief This method returns true if a first zero was detected on keypad columns
 * @param kpad   Pointer to keypad data structure
 */
static inline bool kp_is_2nd_zero(key_pad_t *kpad){
    return kpad->KEY.dzero;
}

#endif