/**
 * \file        main.c
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


int main() {
    stdio_init_all();

    sleep_ms(5000);
    printf("Hola!!!");


    while(1){
        __wfi();
    }
}

