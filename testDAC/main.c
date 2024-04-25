#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pico/binary_info.h"

int main() {
    stdio_init_all();
    
    // Configurar pines GPIO
    for (int i = 10; i <= 17; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
    }
    
    while (true) {
        // Leer dato del puerto serie
        uint8_t data = getchar();
        printf("Dato recibido: %d\n", data);
        
        // Colocar el dato en los pines GPIO 10 al 17
        for (int i = 0; i < 8; i++) {
            gpio_put(10 + i, (data >> i) & 0x01);
        }
    }
    return 0;
}
