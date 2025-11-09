#include <stdio.h>
#include "pico/stdlib.h"
// #include "usb_midi.h"

#define LED_PIN 25   // onboard LED

#define GPIO_16  16   // Minus
#define GPIO_0  0 // Plus

int main() {
    // stdio_init_all();

    // GPIO setup
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true); // Set as output

    gpio_init(GPIO_16);
    gpio_set_dir(GPIO_16, false); // set to reading

    gpio_init(GPIO_0);
    gpio_set_dir(GPIO_0, true); // set as output

    uint32_t last_toggle = 0;
    bool led_state = false;
    
    // Write 1 to Plus pin
    gpio_put(GPIO_0, 1);

    while (true) {
        // usb_midi_task(); // Handle USB stack

        // Read Minus pin
        int minus_state = gpio_get(GPIO_16);#include <stdio.h>
#include "pico/stdlib.h"
// #include "usb_midi.h"

#define LED_PIN 25   // onboard LED
#define GPIO_16  16   // Minus
#define GPIO_0  0 // Plus

int main() {
    // stdio_init_all();

    // GPIO setup
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true); // Set as output

    gpio_init(GPIO_16);
    gpio_set_dir(GPIO_16, false); // set to reading

    gpio_init(GPIO_0);
    gpio_set_dir(GPIO_0, true); // set as output

    uint32_t last_toggle = 0;
    bool led_state = false;
    
    // Write 1 to Plus pin
    gpio_put(GPIO_0, 1);

    while (true) {
        // usb_midi_task(); // Handle USB stack

        // Read Minus pin
        int minus_state = gpio_get(GPIO_16);
        if (minus_state == 0) {
            // Toggle LED every 500ms when Minus is pressed
            uint32_t current_time = to_ms_since_boot(get_absolute_time());
            if (current_time - last_toggle >= 500) {
                led_state = !led_state;
                gpio_put(LED_PIN, led_state);
                last_toggle = current_time;
            }
        } else {
            // Blink LED at a different rate when Minus is not pressed
            // (200 ms period here, adjust as desired)
            uint32_t current_time = to_ms_since_boot(get_absolute_time());
            static uint32_t last_other_toggle = 0;
            static bool other_led_state = false;
            if (current_time - last_other_toggle >= 2000) {
                other_led_state = !other_led_state;
                gpio_put(LED_PIN, other_led_state);
                last_other_toggle = current_time;
            }
            // Ensure the pressed-mode state flag is cleared
            led_state = false;
    }
    }
}

        if (minus_state == 0) {
            // Toggle LED every 500ms when Minus is pressed
            uint32_t current_time = to_ms_since_boot(get_absolute_time());
            if (current_time - last_toggle >= 500) {
                led_state = !led_state;
                gpio_put(LED_PIN, led_state);
                last_toggle = current_time;
            }
        } else {
            // Blink LED at a different rate when Minus is not pressed
            // (200 ms period here, adjust as desired)
            uint32_t current_time = to_ms_since_boot(get_absolute_time());
            static uint32_t last_other_toggle = 0;
            static bool other_led_state = false;
            if (current_time - last_other_toggle >= 2000) {
                other_led_state = !other_led_state;
                gpio_put(LED_PIN, other_led_state);
                last_other_toggle = current_time;
            }
            // Ensure the pressed-mode state flag is cleared
            led_state = false;
    }
    }
}
