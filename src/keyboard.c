/*
 * Minimal MIDI USB Test
 *
 * Tests MIDI USB enumeration with TinyUSB.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "hardware/gpio.h"

int main() {
    // Initialize TinyUSB
    tusb_init();

    // Setup LED for blinking
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) {
        // Must call tud_task() regularly to handle USB events
        tud_task();

        // Blink LED every 500ms to show it's running
        static uint32_t last_blink = 0;
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_blink > 500) {
            last_blink = now;
            gpio_put(LED_PIN, !gpio_get(LED_PIN));
        }
    }
}
