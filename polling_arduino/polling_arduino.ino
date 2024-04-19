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
 * TESTING: 2000mV, 500mV, 100Hz
 * 
 * \author      MST_CDA
 * \version     0.0.1
 * \date        05/10/2023
 * \copyright   Unlicensed
 * 
 */
 

// #include <stdint.h>
// #include <stdbool.h>
// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/timer.h"
// #include "pico/cyw43_arch.h"
// #include "hardware/gpio.h"

// #include "time_base.ino"
// #include "keypad_polling.ino"
// #include "gpio_led.ino"
// #include "gpio_button.ino"
// #include "signal_generator.ino"
// #include "dac.ino"

bool checkNumber(byte number) {
  return (number >= 0 && number <= 9);
}

bool checkLetter(byte letter) {
  return (letter >= 0x0A && letter <= 0x0C);
}

bool checkFreq(unsigned int freq) {
  return (freq >= 1 && freq <= 12000000);
}

bool checkAmp(unsigned int amp) {
  return (amp >= 100 && amp <= 2500);
}

bool checkOffset(unsigned int offset) {
  return (offset >= 50 && offset <= 1250);
}


// ------------------------------------------------------------------------------------
// ----------------------------------- TIME BASE --------------------------------------
// ------------------------------------------------------------------------------------

/** 
 * \typedef time_base_t
 * \brief this datatype enable the management of concurrent temporal events
 */
typedef struct{
    long next;                          ///< Struct member with the time for the next temporal event
    long delta;                         ///< Struct member with the event period in us
    bool en;                                ///< Enabler of the time base
}time_base_t;

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
bool tb_check(time_base_t *t){
    return (micros() >= t->next) && t->en;
}

/// @brief update the tb to next temporal event with respect to the current time
/// @param t time base data structure
void tb_update(time_base_t *t){
    t->next = micros() + t->delta;
}

/// @brief update the tb to next temporal event with respect to the last temporal event
/// @param t time base data structure
void tb_next(time_base_t *t){
    t->next = t->next + t->delta;
}

/// @brief enable time base to generate temporal events
/// @param t time base data structure
void tb_enable(time_base_t *t){
    t->en = true;
}
/// @brief disable time base, no events are generated with tb_check
/// @param t time base data structure
void tb_disable(time_base_t *t){
    t->en = false;
}

void tb_set_delta(time_base_t *t, long delta){
    t->delta = delta;
}


// ------------------------------------------------------------------------------------
// ------------------------------------- KEYPAD ---------------------------------------
// ------------------------------------------------------------------------------------

/**
 * \typedef key_pad_t
 * \brief Data strcuture to manage a matrix keypad of 16 keys
 */
typedef struct{
    struct key{
    byte cnt     : 2;        ///< counter to generate row sequence
    byte seq     : 4;        ///< store the current sequence state
    byte ckey    : 8;        ///< captured key with position coding
    byte dkey    : 5;        ///< captured key with decimal coding
    byte rlsb    : 4;        ///< The rows LSB position for keypad rows, the four gpios for rows must be consecutives starting in position rlsb
    byte clsb    : 4;        ///< The cols LSB position for keypad cols, the four gpios for cols must be consecutives starting in position clsb
    byte en      : 1;        ///< Enable keypad processing
    byte dzero   : 1;        ///< Flag for double zero
    byte nkey    : 1;        ///< Flag that indicates that a key was pressed
    byte dbnc    : 1;        ///< Flag that indicates that debouncer is active
    }KEY;                       ///< All key related information              
    time_base_t tb_seq;         ///< Periodic time base used to generate row sequence
    time_base_t tb_dbnce;       ///< Periocic time base used to implement the key debouncer
    byte history[10];        ///< The last 10 pressed keys
}key_pad_t;

/**
 * \brief This method initializes the keypad data structure
 * \param kpad          pointer to keypad data structure
 * \param rlsb          LSB position of the first row GPIO
 * \param clsb          LSB position of the first col GPIO
 * \param dbnc_time     Time base period for keypad debouncer
 * \param en            True if keypad start enabled
 */ 
void kp_init(key_pad_t *kpad, byte rlsb, byte clsb, unsigned int dbnc_time, bool en)
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
    for(int i = 0; i < 4; i++){
      pinMode(kpad->KEY.rlsb + i, OUTPUT)
      pinMode(kpad->KEY.clsb + i, INPUT_PULLDOWN)
    }
    // gpio_init_mask(0x0000000F << kpad->KEY.rlsb); // gpios for key rows 2,3,4,5
    // gpio_init_mask(0x0000000F << kpad->KEY.clsb); // gpios for key cols 6,7,8,9
    // gpio_set_dir_masked(0x0000000F << kpad->KEY.rlsb,0x0000000F << kpad->KEY.rlsb); // rows as outputs and cols as inputs
    // gpio_set_dir_masked(0x0000000F << kpad->KEY.clsb,0x00000000); // rows as outputs and cols as inputs
    // gpio_pull_down(kpad->KEY.clsb);
    // gpio_pull_down(kpad->KEY.clsb + 1);
    // gpio_pull_down(kpad->KEY.clsb + 2);
    // gpio_pull_down(kpad->KEY.clsb + 3);
}

/** 
 * \brief This method decodes the postional coding of the key to its actual value in decimal
 * \param kpad   Pointer to keypad data structure
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
 * \brief This method captures the currently pressed key. It should only be called when at lest one of the columns is different from zero
 * \param kpad      pointer to keypad data structure
 * \param cols      value of gpios masked to the gpios connected to the keypad columns
 */
void kp_capture(key_pad_t *kpad, uint32_t cols)
{
    kpad->KEY.ckey = (cols >> 2) | kpad->KEY.seq;
    kp_decode(kpad);
    for(int i=0;i<9;i++){
        kpad->history[9-i] = kpad->history[9-i-1];
    }
    kpad->history[0] = kpad->KEY.dkey;
    kpad->KEY.nkey = 1;
}

/** 
 * \brief This method updates the sequence value that drives the keypad rows. The sequence should be updated with a frequency higher than 100Hz
 * \param kpad   Pointer to keypad data structure
 */
void kp_gen_seq(key_pad_t *kpad){
    kpad->KEY.cnt += 1;
    kpad->KEY.seq = 1 << kpad->KEY.cnt;
    gpio_put_masked(0x0000000F<<kpad->KEY.rlsb,((uint32_t)kpad->KEY.seq)<<kpad->KEY.rlsb);
}

/** 
 * \brief This method returns the value of the last pressed key in decimal coding
 * \param kpad   Pointer to keypad data structure
 */
byte kp_get_key(key_pad_t *kpad){
    kpad->KEY.nkey = false;
    return kpad->history[0];
}

/** 
 * \brief This method allows to access the last 10 pressed keys
 * \param kpad   Pointer to keypad data structure
 */
byte kp_get_keyh(key_pad_t *kpad, byte n){
    return kpad->history[n%10];
}

/** 
 * \brief This method returns the value of the key with its positional coding
 * \param kpad   Pointer to keypad data structure
 */
byte kp_get_keyc(key_pad_t *kpad){
    return kpad->KEY.ckey;
}

/** 
 * \brief This method sets to one the flag that indicates that a zero was detected on keypad columns
 * \param kpad   Pointer to keypad data structure
 */
void kp_set_zflag(key_pad_t *kpad){
    kpad->KEY.dzero = 1;
}
/** 
 * \brief This method clears the flag that indicates that a zero was detected on keypad columns
 * \param kpad   Pointer to keypad data structure
 */
void kp_clr_zflag(key_pad_t *kpad){
    kpad->KEY.dzero = 0;
}

/** 
 * \brief This method returns true if a first zero was detected on keypad columns
 * \param kpad   Pointer to keypad data structure
 */
bool kp_is_2nd_zero(key_pad_t *kpad){
    return kpad->KEY.dzero;
}


// ------------------------------------------------------------------------------------
// ------------------------------------- LED ------------------------------------------
// ------------------------------------------------------------------------------------

/// @brief Initialize a gpio to drive a LED
/// @param gpio_num gpio that drives the LED
void led_init(byte gpio_num){
    gpio_init( gpio_num); // gpios for key rows 2,3,4,5
    gpio_set_dir(gpio_num,true); // rows as outputs and cols as inputs
    gpio_put(gpio_num,false);
}

/// @brief Turn on the LED
/// @param gpio_num gpio that drives the LED
void led_on(byte gpio_num){
    gpio_put(gpio_num,true);
}

/// @brief Turn off the LED
/// @param gpio_num gpio that drives the LED
void led_off(byte gpio_num){
    gpio_put(gpio_num,false);
}
/// @brief Toggle the LED
/// @param gpio_num gpio that drives the LED
void led_toggle(byte gpio_num){
    gpio_xor_mask(0x00000001 << gpio_num);
}


// ------------------------------------------------------------------------------------
// ------------------------------------- DAC ------------------------------------------
// ------------------------------------------------------------------------------------

#define RESOLUTION  255         // 8 bits
#define DAC_RANGE   9400        // 0 to 9.3V

/**
 * @typedef dac_t 
 *
 * @brief Structure to manage a 8-bit DAC
 * 
 */
typedef struct{
    struct {
        byte bit0     : 1;
        byte bit1     : 1;
        byte bit2     : 1;
        byte bit3     : 1;
        byte bit4     : 1;
        byte bit5     : 1;
        byte bit6     : 1;
        byte bit7     : 1;
    }BITS;
    bool en;                        ///< Enable DAC
    byte gpio_lsb;               ///< The LSB position of the GPIOs used to output the DAC signal
    short digit_v;               ///< Value to be outputed
}dac_t;

/**
 * @brief Initialize a 8-bit DAC. A DAC0808 is used as reference. 
 * 
 * @param dac 
 * @param pin_lsb   The LSB position of the GPIOs used to output the DAC signal
 * @param en        Enable DAC
 */
void dac_init(dac_t *dac, byte gpio_lsb, bool en)
{
    dac->gpio_lsb = gpio_lsb;
    dac->digit_v = 0;
    dac->en = en;

    gpio_init_mask(0x000000FF << dac->gpio_lsb);
    gpio_set_dir_masked(0x000000FF << dac->gpio_lsb, 0x000000FF << dac->gpio_lsb); // Set all gpios as outputs
}

/**
 * @brief Generate BITS(8-bits) from the input value
 * 
 * @param dac 
 * @param decim_v 
 */
void dac_calculate(dac_t *dac, int16_t decim_v)
{
    
    dac->digit_v = (decim_v + 5000)*RESOLUTION/DAC_RANGE ; // normalize to 8 bits
    dac->BITS.bit0 = (dac->digit_v & 0x01) >> 0;
    dac->BITS.bit1 = (dac->digit_v & 0x02) >> 1;
    dac->BITS.bit2 = (dac->digit_v & 0x04) >> 2;
    dac->BITS.bit3 = (dac->digit_v & 0x08) >> 3;
    dac->BITS.bit4 = (dac->digit_v & 0x10) >> 4;
    dac->BITS.bit5 = (dac->digit_v & 0x20) >> 5;
    dac->BITS.bit6 = (dac->digit_v & 0x40) >> 6;
    dac->BITS.bit7 = (dac->digit_v & 0x80) >> 7;

    dac_output(dac);
}

/**
 * @brief With BITS, output the signal.
 * 
 * @param dac 
 */
void dac_output(dac_t *dac)
{
    if(!dac->en) return;

    gpio_put(dac->gpio_lsb + 0, dac->BITS.bit0);
    gpio_put(dac->gpio_lsb + 1, dac->BITS.bit1);
    gpio_put(dac->gpio_lsb + 2, dac->BITS.bit2);
    gpio_put(dac->gpio_lsb + 3, dac->BITS.bit3);
    gpio_put(dac->gpio_lsb + 4, dac->BITS.bit4);
    gpio_put(dac->gpio_lsb + 5, dac->BITS.bit5);
    gpio_put(dac->gpio_lsb + 6, dac->BITS.bit6);
    gpio_put(dac->gpio_lsb + 7, dac->BITS.bit7);
    
}

// ------------------------------------------------------------------------------------
// ------------------------------------- BUTTON ---------------------------------------
// ------------------------------------------------------------------------------------

/**
 * @typedef gpio_button_t 
 *
 * @brief Structure to manage a single button connected to a GPIO.
 * 
 */
typedef struct{
    struct {
        byte gpio_num     : 6;        ///< GPIO gpio_num number
        byte en      : 1;        ///< Enable keypad processing
        byte dzero   : 1;        ///< Flag for double zero
        byte nkey    : 1;        ///< Flag that indicates that a key was pressed
        byte dbnc    : 1;        ///< Flag that indicates that debouncer is active
    }KEY;
    time_base_t tb_dbnce;       ///< Periocic time base used to implement the key debouncer
}gpio_button_t;

/**
 * @brief 
 * 
 * @param button 
 * @param gpio_num 
 * @param dbnc_time 
 * @param en 
 */
void button_init(gpio_button_t *button, byte gpio_num, long dbnc_time, bool en){
    button->KEY.en = en;
    button->KEY.dzero = 0;
    button->KEY.nkey = 0;
    button->KEY.dbnc = 0;
    button->KEY.gpio_num = gpio_num;
    tb_init(&button->tb_dbnce, dbnc_time, false);

    gpio_init(button->KEY.gpio_num);
    gpio_set_dir(button->KEY.gpio_num, GPIO_IN);
    gpio_pull_down(button->KEY.gpio_num);
}

/** 
 * \brief This method sets to one the flag that indicates that a zero was detected on button
 * \param button   Pointer to keypad data structure
 */
void button_set_zflag(gpio_button_t *button){
    button->KEY.dzero = 1;
}
/** 
 * \brief This method clears the flag that indicates that a zero was detected on button
 * \param button   Pointer to keypad data structure
 */
void button_clr_zflag(gpio_button_t *button){
    button->KEY.dzero = 0;
}

/** 
 * \brief This method returns true if a first zero was detected on button
 * \param button   Pointer to keypad data structure
 */
bool button_is_2nd_zero(gpio_button_t *button){
    return button->KEY.dzero;
}

// ------------------------------------------------------------------------------------
// -------------------------------- SIGNAL GENERATOR ----------------------------------
// ------------------------------------------------------------------------------------

#define M_PI		    3.14159265358979323846	/* pi */
#define S_TO_US     1000000
#define US_TO_S     0.000001
#define SAMPLE      16


// #include <stdint.h>
// #include <math.h>
// #include <stdio.h>
// #include "hardware/timer.h"
// #include "time_base.ino"

/**
 * @typedef signal_t 
 * 
 * @brief Structure to manage a signal generator
 * 
 */
typedef struct{
    struct{
        byte ss      : 2; // 0: Sinusoidal, 1: Triangular, 2: Saw tooth, 3: Square
        byte en      : 1; // Enable signal generation
    }STATE;
    unsigned int freq;          // Signal frequency
    short amp;           // Signal amplitude
    short offset;        // Signal offset
    short value;          // Signal value
    short arrayV[SAMPLE]; // Array to store the signal values for the DAC
    byte cnt;            // Time variable
    time_base_t tb_gen;     // Time base for signal generation

}signal_t;

/**
 * @brief 
 * 
 * @param signal 
 * @param freq 
 * @param amp 
 * @param offset 
 * @param en 
 */
void signal_gen_init(signal_t *signal, unsigned int freq, short amp, short offset, bool en)
{
  signal->freq = freq;
  signal->amp = amp;
  signal->offset = offset;
  signal->value = 0;
  signal->STATE.en = en;
  signal->STATE.ss = 0;
  signal->cnt = 0;
  tb_init(&signal->tb_gen,S_TO_US/(SAMPLE*freq),true);
}

// ------------------------------------------------------------------
// ---------------------- GETTING WAVE VALUES ----------------------
// ------------------------------------------------------------------

void signal_gen_sin(signal_t *signal, byte t){
    signal->value = (short)(signal->offset + signal->amp*sin((2*M_PI*t)/SAMPLE));
}

void signal_gen_tri(signal_t *signal, byte t){
    if (t <= SAMPLE/2){
        signal->value = (short)(signal->offset + (4*signal->amp*t)/SAMPLE - signal->amp);
    }
    else {
        signal->value = (short)(signal->offset - (4*signal->amp*t)/SAMPLE + 3*signal->amp);  
    }
}

void signal_gen_saw(signal_t *signal, byte t){
    signal->value = (short)(signal->offset + (2*signal->amp*t)/SAMPLE  - signal->amp);
}

void signal_gen_sqr(signal_t *signal, byte t){
    if (t <= SAMPLE/2){
        signal->value = (short)(signal->offset + signal->amp);
    }
    else {
        signal->value = (short)(signal->offset - signal->amp);
    }
}

// ------------------------------------------------------------------

void signal_calculate_next_value(signal_t *signal){

    if(!signal->STATE.en) return; 

    switch(signal->STATE.ss){ // Calculate next signal value
        case 0: // Sinusoidal
            for (byte i = 1; i <= SAMPLE; i++){
                signal_gen_sin(signal, i);
                signal->arrayV[i - 1] = signal->value - 800; // The -500 is to set the bias of the DAC.
            }
            break;
        case 1: // Triangular
            for (byte i = 1; i <= SAMPLE; i++){
                signal_gen_tri(signal, i);
                signal->arrayV[i - 1] = signal->value - 800; // The -500 is to set the bias of the DAC.
            }
            break;
        case 2: // Saw tooth
            for (byte i = 1; i <= SAMPLE; i++){
                signal_gen_saw(signal, i);
                signal->arrayV[i - 1] = signal->value - 800; // The -500 is to set the bias of the DAC.
            }
            break;
        case 3: // Square
            for (byte i = 1; i <= SAMPLE; i++){
                signal_gen_sqr(signal, i);
                signal->arrayV[i - 1] = signal->value - 800; // The -500 is to set the bias of the DAC.
            }
            break;
    }

}

short signal_get_value(signal_t *signal){
    return signal->value;
}

void signal_set_state(signal_t *signal, byte ss){
    signal->STATE.ss = ss;
}

void signal_set_amp(signal_t *signal, short amp){
    signal->amp = amp;
}

void signal_set_offset(signal_t *signal, short offset){
    signal->offset = offset;
}

void signal_set_freq(signal_t *signal, unsigned int freq){
    signal->freq = freq;
    tb_set_delta(&signal->tb_gen,S_TO_US/(SAMPLE*freq)); // Nyquist theorem
}

void signal_gen_enable(signal_t *signal){
    signal->STATE.en = 1;
    tb_enable(&signal->tb_gen);
}

void signal_gen_disable(signal_t *signal){
    signal->STATE.en = 0;
    tb_disable(&signal->tb_gen);
}

// ------------------------------------------------------------------------------------
// ------------------------------------- MAIN -----------------------------------------
// ------------------------------------------------------------------------------------


void setup() {
  // stdio_init_all();
  // // sleep_ms(5000);
  printf("Hola!!!");

  // Initialize signal generator
  byte in_param_state = 0x00;  // 0: Nothing, 1: Entering amp (A), 2: Entering offset (B), 3: Entering freq (C)
  unsigned int param = 0;             // This variable will store the value of any parameter that is being entered
  byte key_cont = 0;
  signal_t my_signal;
  signal_gen_init(&my_signal, 1, 1000, 500, true);

  // Initialize DAC
  dac_t my_dac;
  dac_init(&my_dac, 10, true);
  signal_calculate_next_value(&my_signal);

  // Initialize LED
  byte my_led = 18;
  led_init(my_led);
  // cyw43_arch_init();

  // Initialize keypad and button
  key_pad_t my_keypad;
  kp_init(&my_keypad, 2, 6, 100000, true);

  gpio_button_t my_button;
  button_init(&my_button, 0, 100000, true);

  // Initialize Printing
  time_base_t tb_print;
  tb_init(&tb_print, 1000000, true);
}


void loop() {

  // Process keypad
  unsigned int cols = gpio_get_all() & (0x0000003C0);  // 0000 0100 0000
  if (cols && !my_keypad.KEY.dbnc) {
    tb_disable(&my_keypad.tb_seq);
    kp_capture(&my_keypad, cols);
    tb_update(&my_keypad.tb_dbnce);
    tb_enable(&my_keypad.tb_dbnce);
    kp_set_zflag(&my_keypad);
    my_keypad.KEY.dbnc = 1;
  }
  if (tb_check(&my_keypad.tb_seq)) {
    tb_next(&my_keypad.tb_seq);
    kp_gen_seq(&my_keypad);
  }
  if (tb_check(&my_keypad.tb_dbnce)) {
    tb_next(&my_keypad.tb_dbnce);
    if (kp_is_2nd_zero(&my_keypad)) {
      if (!cols) {
        // This able to generate seq only when a button is not pressed
        tb_update(&my_keypad.tb_seq);  // It could happens that a button was pressed for a long time
        tb_enable(&my_keypad.tb_seq);
        tb_disable(&my_keypad.tb_dbnce);
        my_keypad.KEY.dbnc = 0;
      } else
        kp_clr_zflag(&my_keypad);
    } else {
      if (!cols)
        kp_set_zflag(&my_keypad);
    }
  }
  // Process button
  bool button = gpio_get(my_button.KEY.gpio_num);
  if (button && !my_button.KEY.dbnc) {
    my_button.KEY.nkey = true;  // This is a flag that indicates that a key was pressed
    tb_update(&my_button.tb_dbnce);
    tb_enable(&my_button.tb_dbnce);
    button_set_zflag(&my_button);
    my_button.KEY.dbnc = 1;
  }
  if (tb_check(&my_button.tb_dbnce)) {
    tb_next(&my_button.tb_dbnce);
    if (button_is_2nd_zero(&my_button)) {
      if (!button) {
        // printf("Button pressed\n");
        signal_set_state(&my_signal, (my_signal.STATE.ss + 1) % 4);
        signal_calculate_next_value(&my_signal);
        tb_disable(&my_button.tb_dbnce);
        my_button.KEY.dbnc = 0;
      } else
        button_clr_zflag(&my_button);
    } else {
      if (!button)
        button_set_zflag(&my_button);
    }
  }
  // Process signal
  if (tb_check(&my_signal.tb_gen)) {
    tb_next(&my_signal.tb_gen);
    dac_calculate(&my_dac, my_signal.arrayV[my_signal.cnt]);
    my_signal.cnt = (my_signal.cnt + 1) % SAMPLE;
  }

  // Process printing
  if (tb_check(&tb_print)) {
    tb_next(&tb_print);
    switch (my_signal.STATE.ss) {
      case 0:
        printf("Sinusoidal: ");
        break;
      case 1:
        printf("Triangular: ");
        break;
      case 2:
        printf("Saw tooth: ");
        break;
      case 3:
        printf("Square: ");
        break;
    }
    printf("Amp: %d, Offset: %d, Freq: %d\n", my_signal.amp, my_signal.offset, my_signal.freq);
  }

  // Process entering parameters
  if (my_keypad.KEY.nkey && !my_keypad.KEY.dbnc) {

    // To accept a number, in_param_state must be different of 0
    if (checkNumber(my_keypad.KEY.dkey) && in_param_state) {
      param = (!key_cont) ? my_keypad.KEY.dkey : param * 10 + my_keypad.KEY.dkey;
      key_cont++;
    }
    // To accept a letter different of 0x0D, in_param_state must be 0
    else if (checkLetter(my_keypad.KEY.dkey) && !in_param_state) {
      // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
      led_on(my_led);
      switch (my_keypad.KEY.dkey) {
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
    else if (my_keypad.KEY.dkey == 0x0D && in_param_state) {
      // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
      led_off(my_led);
      switch (in_param_state) {
        case 1:
          if (checkAmp(param)) {
            signal_set_amp(&my_signal, param);
          }
          break;
        case 2:
          if (checkOffset(param)) {
            signal_set_offset(&my_signal, param);
          }
          break;
        case 3:
          if (checkFreq(param)) {
            signal_set_freq(&my_signal, param);
          }
          break;
        default:
          printf("Invalid state\n");
          break;
      }
      signal_calculate_next_value(&my_signal);
      in_param_state = 0;
      param = 0;
      key_cont = 0;
    }
    my_keypad.KEY.nkey = 0;
  }
}