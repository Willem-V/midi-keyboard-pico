#include "pico/stdlib.h"
// #include "usb_midi.h"

#define LED_PIN 25   // onboard LED
#define GPIO_A  16   // Minus
#define GPIO_B  0 // Plus

int main() {
    stdio_init_all();

    // GPIO setup
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, true); // Set as output

    gpio_init(GPIO_A);
    gpio_set_dir(GPIO_A, false); // set to reading

    gpio_init(GPIO_B);
    gpio_set_dir(GPIO_B, true); // set as output

    uint32_t last_toggle = 0;
    bool led_state = false;
    
    // Write 1 to Plus pin
    gpio_put(GPIO_B, 1);

    while (true) {
        usb_midi_task(); // Handle USB stack

        // Read Minus pin
        int minus_state = gpio_get(GPIO_A);
        if (minus_state == 0) {
            // Toggle LED every 500ms when Minus is pressed
            uint32_t current_time = to_ms_since_boot(get_absolute_time());
            if (current_time - last_toggle >= 500) {
                led_state = !led_state;
                gpio_put(LED_PIN, led_state);
                last_toggle = current_time;
            }
        } else {
            // Ensure LED is off when Minus is not pressed
            gpio_put(LED_PIN, 0);
            led_state = false;
    }
    }
}
