#include <stdio.h>
#include "pico/stdlib.h"


#define LED_PIN 25   // onboard LED
#define GPIO_ENABLE  18   // extra GPIOs
#define GPIO_READ  19

int main() {
    stdio_init_all();

    // GPIO setup
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true);

    gpio_init(GPIO_ENABLE);
    gpio_set_dir(GPIO_ENABLE, true);

    gpio_init(GPIO_READ);
    gpio_set_dir(GPIO_READ, false);

    uint32_t last_toggle = 0;
    bool led_state = false;
    bool read_key = false;

    gpio_put(GPIO_ENABLE, true); // enable the keyboard

    while (true) {
        // usb_midi_task(); // Handle USB stack

        uint32_t now = to_ms_since_boot(get_absolute_time());

        // // Blink every 500 ms
        // if (now - last_toggle > 2000) {
        //     led_state = !led_state;
        //     gpio_put(LED_PIN, led_state);

        //     last_toggle = now;
        // }

        // Read keyboard key
        read_key = gpio_get(GPIO_READ);
        if (read_key == true) { //key pressed
            gpio_put(LED_PIN, true);
        }
        else {
            gpio_put(LED_PIN, false);
        }
    }
}