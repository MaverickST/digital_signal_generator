
#include <Arduino.h>
#include <stdint.h>

#include "time_base.h"
#include "dac.h"
#include "gpio_button.h"
#include "signal_generator.h"
#include "keypad_polling.h"

static inline bool checkNumber(uint8_t number){
    return (number >= 0 && number <= 9);
}

static inline bool checkLetter(uint8_t letter){
    return (letter >= 0x0A && letter <= 0x0C);
}

static inline bool checkFreq(uint32_t freq){
    return (freq >= 1 && freq <= 12000000);
}

static inline bool checkAmp(uint32_t amp){
    return (amp >= 100 && amp <= 2500);
}

static inline bool checkOffset(uint32_t offset){
    return (offset >= 50 && offset <= 1250);
}

// Objects initialization
time_base_t tb_print;
dac_t my_dac;
gpio_button_t my_button;
signal_t my_signal;
key_pad_t my_keypad;
int my_led = 18;

// Auxiliar variables
uint8_t in_param_state = 0x00; // 0: Nothing, 1: Entering amp (A), 2: Entering offset (B), 3: Entering freq (C)
uint32_t param = 0; // This variable will store the value of any parameter that is being entered
uint8_t key_cont = 0;

void setup (){
  Serial.begin(9600);

  tb_init(&tb_print, 1000000, true);
  dac_init(&my_dac, 10, true);
  button_init(&my_button, 0, 100000, true);
  signal_gen_init(&my_signal, 10, 1000, 500, true);
  signal_calculate_next_value(&my_signal);
  kp_init(&my_keypad,2,6,100000,true);
  pinMode(my_led, OUTPUT);
}

void loop(){
  // Process keypad
  kp_captureCols(&my_keypad);
  if(my_keypad.KEY.cols && !my_keypad.KEY.dbnc){
      tb_disable(&my_keypad.tb_seq);
      kp_capture(&my_keypad);
      tb_update(&my_keypad.tb_dbnce);
      tb_enable(&my_keypad.tb_dbnce);
      kp_set_zflag(&my_keypad);
      my_keypad.KEY.dbnc = 1;
  }
  if(tb_check(&my_keypad.tb_seq)){
      tb_next(&my_keypad.tb_seq);
      kp_gen_seq(&my_keypad);
      // Serial.println(my_keypad.KEY.cols);
  }
  if(tb_check(&my_keypad.tb_dbnce)){
      tb_next(&my_keypad.tb_dbnce);
      if(kp_is_2nd_zero(&my_keypad)){
          if(!my_keypad.KEY.cols){
              // This able to generate seq only when a button is not pressed
              tb_update(&my_keypad.tb_seq); // It could happens that a button was pressed for a long time
              tb_enable(&my_keypad.tb_seq);
              tb_disable(&my_keypad.tb_dbnce);
              my_keypad.KEY.dbnc = 0;
          }
          else {
            kp_clr_zflag(&my_keypad);
          }
      }
      else{
          if(!my_keypad.KEY.cols)
              kp_set_zflag(&my_keypad);
      }
  }
  // Process button
  bool button = digitalRead(my_button.KEY.gpio_num);
  if(button && !my_button.KEY.dbnc){
      my_button.KEY.nkey = true; // This is a flag that indicates that a key was pressed
      tb_update(&my_button.tb_dbnce); 
      tb_enable(&my_button.tb_dbnce);
      button_set_zflag(&my_button);
      my_button.KEY.dbnc = 1;
  }
  if(tb_check(&my_button.tb_dbnce)){
      tb_next(&my_button.tb_dbnce);
      if(button_is_2nd_zero(&my_button)){
          if(!button){
              // Serial.println("Button pressed\n");
              signal_set_state(&my_signal, (my_signal.STATE.ss + 1)%4);
              signal_calculate_next_value(&my_signal);
              tb_disable(&my_button.tb_dbnce);
              my_button.KEY.dbnc = 0;
          }
          else {
              button_clr_zflag(&my_button);
          }
      }
      else{
          if(!button)
              button_set_zflag(&my_button);
      }
  }
  // Process signal
  if (tb_check(&my_signal.tb_gen)){
      tb_next(&my_signal.tb_gen);
      dac_calculate(&my_dac,my_signal.arrayV[my_signal.cnt]);
      my_signal.cnt = (my_signal.cnt + 1) % SAMPLE;
  }

  // Process printing
  if(tb_check(&tb_print)){
      tb_next(&tb_print);
      switch (my_signal.STATE.ss){
          case 0:
              Serial.print("Sinusoidal: ");
              break;
          case 1:
              Serial.print("Triangular: ");
              break;
          case 2:
              Serial.print("Saw tooth: ");
              break;
          case 3:
              Serial.print("Square: ");
              break;
      }
      Serial.print("Amp: ");
      Serial.print(my_signal.amp);
      Serial.print("  Offset: ");
      Serial.print(my_signal.offset);
      Serial.print("  Freq: ");
      Serial.println(my_signal.freq);
  }

  // Process entering parameters
  if(my_keypad.KEY.nkey && !my_keypad.KEY.dbnc){
      
      // To accept a number, in_param_state must be different of 0
      if(checkNumber(my_keypad.KEY.dkey) && in_param_state){
          param = (!key_cont)? my_keypad.KEY.dkey : param*10 + my_keypad.KEY.dkey;
          key_cont++;
      }
      // To accept a letter different of 0x0D, in_param_state must be 0
      else if(checkLetter(my_keypad.KEY.dkey) && !in_param_state){
          // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
          digitalWrite(my_led, HIGH);
          switch (my_keypad.KEY.dkey)
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
              Serial.println("Invalid letter\n");
              break;
          }
      }
      // To accept a 0x0D, in_param_state must be different of 0
      else if(my_keypad.KEY.dkey == 0x0D && in_param_state){
          // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
          digitalWrite(my_led, LOW);
          switch (in_param_state)
          {
          case 1:
              if(checkAmp(param)){
                  signal_set_amp(&my_signal,param);
              }
              break;
          case 2:
              if(checkOffset(param)){
                  signal_set_offset(&my_signal,param);
              }
              break;
          case 3:
              if(checkFreq(param)){
                  signal_set_freq(&my_signal,param);
              }
              break;
          default:
              Serial.println("Invalid state\n");
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

