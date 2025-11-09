#include <stdio.h>
#include "pico/stdlib.h"
#include "usb_midi.h"

#define LED_PIN 25   // onboard LED

#define DRIVE0 0
#define DRIVE1 1
#define DRIVE2 2
#define DRIVE3 3
#define DRIVE4 4
#define DRIVE5 5
#define DRIVE6 6
#define DRIVE7 7
#define DRIVE8 8
#define DRIVE9 9
#define DRIVE10 10

#define READ0 11
#define READ1 12
#define READ2 13
#define READ3 14
#define READ4 15
#define READ5 16
#define READ6 17
#define READ7 18
#define READ8 19
#define READ9 20
#define READ10 21


int main() {
    // stdio_init_all();

    // GPIO setup
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true); // Set as output


    for (int pin = DRIVE0; pin <= DRIVE10; ++pin) {
        gpio_init(pin);
        gpio_set_dir(pin, true); // set as output
        gpio_put(pin, 0);        // default low
    }

    // GPIO inputs
    for (int pin = READ0; pin <= READ10; ++pin) {
        gpio_init(pin);
        gpio_set_dir(pin, false); 
        gpio_pull_down(pin);      // default low
    }
    
    uint32_t last_toggle = 0;
    bool led_state = false;

    while (1){
        
    }
}
    
    
    