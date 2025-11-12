/*
 * Minimal USB Test - Based on hello_usb example
 *
 * Tests basic USB CDC enumeration by printing messages over USB serial.
 */

#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    while (true) {
        printf("Hello, USB!\n");
        sleep_ms(1000);
    }
}
