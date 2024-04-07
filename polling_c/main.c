
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "time_base.h"
#include "keypad_polling.h"
#include "gpio_led.h"


int main() {
    
    stdio_init_all();
    sleep_ms(5000);
    printf("Hola!!!");

    key_pad_t my_keypad;
    kp_init(&my_keypad,2,6,100000,true);

    while(1){
        // Process keypad
        uint32_t cols = gpio_get_all() & (0x000003C0);
        if(cols && !my_keypad.KEY.dbnc){
            //printf("%d\n",cols);
            tb_disable(&my_keypad.tb_seq);
            kp_capture(&my_keypad,cols);
            tb_update(&my_keypad.tb_dbnce);
            tb_enable(&my_keypad.tb_dbnce);
            kp_set_zflag(&my_keypad);
            my_keypad.KEY.dbnc = 1;
        }
        if(tb_check(&my_keypad.tb_seq)){
            tb_next(&my_keypad.tb_seq);
            kp_gen_seq(&my_keypad);
            //printf("%d\n",my_keypad.KEY.seq);
        }
        if(tb_check(&my_keypad.tb_dbnce)){
            tb_next(&my_keypad.tb_dbnce);
            if(kp_is_2nd_zero(&my_keypad)){
                if(!cols){
                    tb_update(&my_keypad.tb_seq);
                    tb_enable(&my_keypad.tb_seq);
                    tb_disable(&my_keypad.tb_dbnce);
                    my_keypad.KEY.dbnc = 0;
                }
                else
                    kp_clr_zflag(&my_keypad);
            }
            else{
                if(!cols)
                    kp_set_zflag(&my_keypad);
            }
        }
    }

    return 1;
}