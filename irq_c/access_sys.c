/**
 * \file        access_sys.c
 * \brief       This proyect implements an access control system with a matrix keypad
 * \details     The access control system has a database of users, where each user has an ID identifier
 * and an associated 6-digit password. To grant access to the user, they must enter a 4-digit ID and a
 * 6-digit password. The system will search its database for the ID, and if found, it will compare the 
 * ingresed password with the user's password. If the ID is found in the database and the password matches
 * the user's password, then the system grants the access.
 * 
 * The granting of access will be indicated by turning on a green LED for 10 seconds. If the user's ID 
 * does not exist or the password is incorrect, this event will be indicated by turning on a red LED for
 * 3 seconds. In any case, the system must always receive both the user's ID and their password before 
 * indicating an unsuccessful access attempt. If a user with an ID enters the password incorrectly more
 * than 4 times consecutively, not necessarily simultaneously, the user is permanently blocked from the 
 * system.
 * 
 * Other features of the system:
 * 
 * The identity verification process has a maximum time of 10 seconds to enter both the user's ID and 
 * their respective password. If the user fails to enter the required information within this time, the
 * system will return to the initial state, signaling with a red LED that the process failed.
 * 
 * Both the ID and the password must be entered without errors. Digits cannot be erased. In case of an
 * error, the user must finish entering the total of 10 required digits (4 ID + 6 password) or wait for
 * the maximum entry time to pass and the system to return to the initial state.
 * 
 * A continuously lit yellow LED will indicate that the system is ready to receive the user's ID. When
 * the user presses the first digit of their ID, the LED turns off, indicating that the identity verifi-
 * cation process has started. When the user enters the four ID digits, the yellow LED starts blinking
 * (on and off) at a frequency of 1Hz until the user enters the sixth password digit. At that point, the
 * yellow LED turns off and only turns back on when the signaling for correct password (green LED - 10 
 * sec) or incorrect password (red LED - 3 sec) has finished.
 * 
 * The system must have a database of at least 10 different users with their respective passwords.
 * 
 * \author      Ricardo Andres Velásquez
 * \version     0.0.1
 * \date        05/10/2023
 * \copyright   Unlicensed
 * 
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
#include "gpio_led.h"

#define YELLOW_LED 10
#define GREEN_LED 11
#define RED_LED 12

uint32_t yellow_t = 1000000ULL;
uint32_t green_t = 10000000ULL;
uint32_t red_t = 3000000ULL;
uint32_t out_t = 10000000ULL;

uint8_t vecIDs[] = {0x4,0x3,0x2,0x1,            // User 0 with ID 1234
                    0xA,0xB,0xC,0xD,    // User 1 with ID DCBA
                    0xE,0xB,0xE,0xB,    // User 2 with ID BEBE
                    0x4,0xC,0x4,0xC,    // User 3 with ID C4C4
                    0x1,0x2,0x3,0x4,    // User 4 with ID 4321
                    0x0,0xC,0x5,0xA,    // User 5 with ID A5C0
                    0x5,0xA,0x3,0xF,    // User 6 with ID F3A5
                    0x2,0x8,0x9,0x1,    // User 7 with ID 1982
                    0x7,0x0,0x0,0x0,    // User 8 with ID 0007
                    0xE,0x1,0x1,0x9     // User 9 with ID 911E
                    };
uint8_t vecPSWD[] = {0x6,0x5,0x4,0x3,0x2,0x1,   // User 0 with password 123456
                     0x1,0x2,0x3,0x4,0x5,0x6,   // User 1 with password 654321
                     0x0,0x0,0x0,0x0,0x0,0x0,   // User 2 with password 000000
                     0x1,0x2,0x3,0x4,0x5,0x6,   // User 3 with password 654321
                     0x1,0x2,0x3,0x4,0x5,0x6,   // User 4 with password 654321
                     0x1,0x2,0x3,0x4,0x5,0x6,   // User 5 with password 654321
                     0x1,0x2,0x3,0x4,0x5,0x6,   // User 6 with password 654321
                     0x1,0x2,0x3,0x4,0x5,0x6,   // User 7 with password 654321
                     0x1,0x2,0x3,0x4,0x5,0x6,   // User 8 with password 654321
                     0x1,0x2,0x3,0x4,0x5,0x6    // User 9 with password 654321
};

uint8_t hKeys[10]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

uint8_t missCNT[10]={0,0,0,0,0,0,0,0,0,0};

uint16_t blockIDs = 0x0000;

volatile uint8_t gKeyCnt = 0;
volatile uint8_t gSeqCnt = 0;
volatile bool gDZero = false;

/// @brief 
/// @param key 
void insertKey(uint8_t key){
    for(int i=8;i>0;i--){
        hKeys[i+i]=hKeys[i];
    }
    hKeys[0] = key;
}

uint8_t keyDecode(uint32_t keyc){
    uint8_t keyd = 0xFF;
    switch (keyc)
    {
    case 0x88:
        keyd = 0x01;
        break;
    case 0x48:
        keyd = 0x02;
        break;
    case 0x28:
        keyd = 0x03;
        break;
    case 0x18:
        keyd = 0x0A;
        break;
    case 0x84:
        keyd = 0x04;
        break;
    case 0x44:
        keyd = 0x05;
        break;
    case 0x24:
        keyd = 0x06;
        break;
    case 0x14:
        keyd = 0x0B;
        break;
    case 0x82:
        keyd = 0x07;
        break;
    case 0x42:
        keyd = 0x08;
        break;
    case 0x22:
        keyd = 0x09;
        break;
    case 0x12:
        keyd = 0x0C;
        break;
    case 0x81:
        keyd = 0x0E;
        break;
    case 0x41:
        keyd = 0x00;
        break;
    case 0x21:
        keyd = 0x0F;
        break;
    case 0x11:
        keyd = 0x0D;
        break;
    }
    return keyd;
}

/// @brief 
/// @param vecID 
/// @param ID 
/// @return 
int8_t checkID(uint8_t *vecID, uint8_t *ID){
    for(int i=0;i<10;i++){
        bool flag = true;
        for(int j=0;j<4;j++){
            if(vecID[4*i+j] != ID[j]){
                flag = false;
                break;
            }
        }
        if(flag==true)
            return i;
    }
    return -1;
}

/// @brief 
/// @param idxID 
/// @param vecPSWD 
/// @param PSWD 
/// @return 
bool checkPSWD(int8_t idxID, uint8_t *vecPSWD, uint8_t *PSWD){
    for(int j=0;j<4;j++)
        if(vecPSWD[6*idxID+j] != PSWD[j]){
            return false;
        }
    return true;
}

/// @brief 
/// @param num 
void timerCallback(uint num){
    switch (num)
    {
    case 0:
        /* Yellow Led */
        led_toggle(YELLOW_LED);
        timer_hw->alarm[0] = (uint32_t)(time_us_64() + yellow_t);
        timer_hw->intr = 0x00000001;
        break;
    case 1:
        led_off(GREEN_LED);
        led_on(YELLOW_LED);
        timer_hw->intr = 0x00000002;
        pwm_set_enabled(0,true);
        pwm_set_enabled(1,false);
        /* code */
        break;
    case 2:
        led_off(RED_LED);
        led_on(YELLOW_LED);
        timer_hw->intr = 0x00000004;
        pwm_set_enabled(0,true);
        pwm_set_enabled(1,false);
        /* code */
        break;
    case 3:
        /* code */
        led_on(RED_LED);
        timer_hw->alarm[2] = (uint32_t)(time_us_64() + red_t);
        timer_hw->intr = 0x00000008;
        pwm_set_enabled(0,false);
        pwm_set_enabled(1,false);
        gpio_clr_mask(0x0000003C);
        break;
    default:
        break;
    }
}
/// @brief 
/// @param num 
/// @param mask 
void keyboardCallback(uint num, uint32_t mask){
    pwm_set_enabled(0,false);                                           ///< Froze the row sequence
    pwm_set_enabled(1,true);
    gpio_set_irq_enabled(6,GPIO_IRQ_EDGE_RISE,false);  
    gpio_set_irq_enabled(7,GPIO_IRQ_EDGE_RISE,false);  
    gpio_set_irq_enabled(8,GPIO_IRQ_EDGE_RISE,false);  
    gpio_set_irq_enabled(9,GPIO_IRQ_EDGE_RISE,false);                                            ///< Enable the debouncer PIT
    uint32_t KeyData = (gpio_get_all()>>2) & 0x000000FF;
    uint8_t keyd = keyDecode(KeyData);
    if(keyd!=0xFF){
        insertKey(keyd);
    }
    gKeyCnt++;
    if(gKeyCnt==1){
        led_off(YELLOW_LED);
        timer_hw->alarm[3] = (uint32_t)(time_us_64()+out_t);
    }
    else if(gKeyCnt==4){
        led_on(YELLOW_LED);
        timer_hw->alarm[0] = (uint32_t)(time_us_64()+yellow_t);
    }
    else if(gKeyCnt==10){
        led_off(YELLOW_LED);
        timer_hw->armed = 0x00000009;
        int8_t idxID = checkID(vecIDs,&hKeys[6]);
        if(idxID!=-1){
            bool OK = checkPSWD(idxID,vecPSWD,hKeys);
            if (OK){
                if(!(blockIDs&(0x0001<<idxID))){
                    led_on(GREEN_LED);
                    timer_hw->alarm[1] = (uint32_t)(time_us_64()+green_t);
                    missCNT[idxID] = 0;
                }
                else{
                    led_on(RED_LED);
                    timer_hw->alarm[2] = (uint32_t)(time_us_64()+red_t);

                }
            }
            else{
                missCNT[idxID]++;
                if(missCNT[idxID]>4)
                    blockIDs |= 0x0001<<idxID;
                led_on(RED_LED);
                timer_hw->alarm[2] = (uint32_t)(time_us_64()+red_t);
            }
        }
        else{
            led_on(RED_LED);
            timer_hw->alarm[2] = (uint32_t)(time_us_64()+red_t);
        }
    }
    ///gMainFlags.bits.keyFlag = true;
    gpio_acknowledge_irq(num,mask);                                     ///< gpio IRQ acknowledge
 }

/// @brief 
/// @param  
void pwmIRQ(void){
    uint32_t gpioValue;
    uint32_t keyc;
    switch (pwm_get_irq_status_mask())
    {
    case 0x01UL:                                                          ///< PWM slice 0 ISR used as a PIT to generate row sequence
        gSeqCnt = (gSeqCnt + 1) % 4;
        gpio_put_masked(0x0000003C,0x00000001 << (gSeqCnt+2));
        pwm_clear_irq(0);                                               ///< Acknowledge slice 0 PWM IRQ
        break;
    case 0x02UL:                                                          ///< PWM slice 1 ISR used as a PIT to implement debouncer
        keyc = gpio_get_all() & 0x000003C0; ///< Get raw gpio values
        if(gDZero){                                 
            if(!keyc){               
                pwm_set_enabled(0,true);                                           ///< Froze the row sequence
                pwm_set_enabled(1,false);
                 gpio_set_irq_enabled(6,GPIO_IRQ_EDGE_RISE,true);  
                gpio_set_irq_enabled(7,GPIO_IRQ_EDGE_RISE,true);  
                gpio_set_irq_enabled(8,GPIO_IRQ_EDGE_RISE,true);  
                gpio_set_irq_enabled(9,GPIO_IRQ_EDGE_RISE,true);
            }
            gDZero = false;
        }
        else{
            gDZero = true;
        }

        pwm_clear_irq(1);                                               ///< Acknowledge slice 1 PWM IRQ
        break;
    default:
        printf("Paso lo que no debería pasar en PWM IRQ\n");
        break;
    }
 }

/// @brief 
/// @param slice 
/// @param milis 
/// @param enable 
void initPWMasPIT(uint8_t slice, uint16_t milis, bool enable){
    assert(milis<=262);                                             ///< PWM can manage interrupt periods greater than 262 milis
    float prescaler = (float)SYS_CLK_KHZ/500;
    assert(prescaler<256); ///< the integer part of the clock divider can be greater than 255 
    uint32_t wrap = 500000*milis/2000;
    assert(wrap<((1UL<<17)-1));
    pwm_config cfg =  pwm_get_default_config();
    pwm_config_set_phase_correct(&cfg,true);
    pwm_config_set_clkdiv(&cfg,prescaler);
    pwm_config_set_clkdiv_mode(&cfg,PWM_DIV_FREE_RUNNING);
    pwm_config_set_wrap(&cfg,wrap);
    pwm_set_irq_enabled(slice,true);
    irq_set_enabled(PWM_IRQ_WRAP,true);
    pwm_init(slice,&cfg,enable);
 }

/// @brief 
/// @param  
void initMatrixKeyboard4x4(void){
    /// GPIOs 5 to 2 control keyboard rows (one hot sequence)
    /// GPIOS 9 to 6 control keyboard columns (GPIO IRQs)
    /// Lets configure who controls the GPIO PAD
    gpio_set_function(2,GPIO_FUNC_SIO);
    gpio_set_function(3,GPIO_FUNC_SIO);
    gpio_set_function(4,GPIO_FUNC_SIO);
    gpio_set_function(5,GPIO_FUNC_SIO);
    gpio_set_function(6,GPIO_FUNC_SIO);
    gpio_set_function(7,GPIO_FUNC_SIO);
    gpio_set_function(8,GPIO_FUNC_SIO);
    gpio_set_function(9,GPIO_FUNC_SIO);

    gpio_set_dir_in_masked(0x000003C0);                                 ///< Set gpios 6 to 9 as inputs (columns)
    gpio_set_dir_out_masked(0x0000003C);                                ///< Set gpios 2 to 5 as outputs (rows)
    gpio_put_masked(0x0000003C,0);                                      ///< Write 0 to rows

    gpio_set_irq_enabled_with_callback(6,GPIO_IRQ_EDGE_RISE,true,keyboardCallback);
    gpio_set_irq_enabled_with_callback(7,GPIO_IRQ_EDGE_RISE,true,keyboardCallback);
    gpio_set_irq_enabled_with_callback(8,GPIO_IRQ_EDGE_RISE,true,keyboardCallback);
    gpio_set_irq_enabled_with_callback(9,GPIO_IRQ_EDGE_RISE,true,keyboardCallback);
 }

/// @brief 
/// @param  
void initTimer(void){
    /// claim alarm0 for yellow LED
    if(!hardware_alarm_is_claimed (0))
        hardware_alarm_claim(0);
    else
        printf("Tenemos un problemaj alarm 0\n");

    /// Claim alarm 1 for Green LED
    if(!hardware_alarm_is_claimed (1))
        hardware_alarm_claim(1);
    else
        printf("Tenemos un problema alarm 1\n");
    
    /// Claim alarm 2 for Red LED
    if(!hardware_alarm_is_claimed (2))
        hardware_alarm_claim(2);
    else
        printf("Tenemos un problema alarm 2\n");

    /// Claim alarm 3 for Time Out
    if(!hardware_alarm_is_claimed (3))
        hardware_alarm_claim(3);
    else
        printf("Tenemos un problema alarm3\n");

    /// Set callback for each alarm. TODO: replace with an exclusive handler
    hardware_alarm_set_callback(0,timerCallback);
    hardware_alarm_set_callback(1,timerCallback);
    hardware_alarm_set_callback(2,timerCallback);
    hardware_alarm_set_callback(3,timerCallback);

    
    timer_hw->intr = 0x0000000F;    ///< Clear interrupt flags that maybe pendant
    //timer_hw->inte = 0x0000000F;    ///< Activate interrupts for all 4 alarms

}

int main() {
    stdio_init_all();

    sleep_ms(5000);
    printf("Hola!!!");

    initPWMasPIT(0,2,true);
    initPWMasPIT(1,100,false);
    irq_set_exclusive_handler(PWM_IRQ_WRAP,pwmIRQ);
    irq_set_priority(PWM_IRQ_WRAP, 0xC0);

    initMatrixKeyboard4x4();
    /// Init Polling base Periodic Time Base
    
    led_init(YELLOW_LED);
    led_init(RED_LED);
    led_init(GREEN_LED);
    gpio_put(YELLOW_LED,1);


    while(1){
        __wfi();
    }
}

