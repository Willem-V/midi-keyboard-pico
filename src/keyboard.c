/*
 * MIDI Keyboard Controller - Matrix Scanning + USB MIDI
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "hardware/gpio.h"
#include "note_map.h"

// Hardware pins
#define LED_PIN 25
#define DRIVE0  0
#define READ0   11

// Scanning config
#define DEBOUNCE_TIME_US   5000
#define SCAN_SETTLE_US     50  // Increased from 2 to 50 for better signal stability

// Key state tracking
typedef struct {
    bool pressed;
    uint64_t last_change_time;
} key_state_t;

static key_state_t key_states[NUM_DRIVE_PINS][NUM_READ_PINS];
static const uint32_t READ_PIN_MASK = 0x003FF800; // Bits 11-21

// Initialize GPIO for matrix
static void init_matrix_pins(void) {
    // Drive pins: outputs, default LOW
    for (int pin = DRIVE0; pin <= DRIVE0 + NUM_DRIVE_PINS - 1; ++pin) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
    }

    // Read pins: inputs with pull-down
    for (int pin = READ0; pin <= READ0 + NUM_READ_PINS - 1; ++pin) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);
    }

    // LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

// Scan one row efficiently
static inline uint16_t scan_row(uint8_t drive_pin) {
    gpio_put(drive_pin, 1);
    busy_wait_us_32(SCAN_SETTLE_US);
    uint32_t gpio_state = gpio_get_all();
    gpio_put(drive_pin, 0);
    return (gpio_state & READ_PIN_MASK) >> READ0;
}

// Send MIDI note on/off
static void send_midi_note(uint8_t note, bool on) {
    uint8_t msg[3];
    msg[0] = on ? 0x90 : 0x80; // Note On/Off on channel 1
    msg[1] = note;
    msg[2] = on ? 0x7F : 0x00; // Velocity
    tud_midi_stream_write(0, msg, 3);
}

// Handle key change
static void handle_key_change(uint8_t drive, uint8_t read, bool is_pressed, uint64_t now) {
    uint8_t note = get_note(drive, read);
    if (note == NOTE_NONE) return;

    key_states[drive][read].pressed = is_pressed;
    key_states[drive][read].last_change_time = now;

    send_midi_note(note, is_pressed);
}

// Scan entire matrix
static void scan_matrix(void) {
    uint64_t now = time_us_64();

    for (uint8_t drive = 0; drive < NUM_DRIVE_PINS; drive++) {
        uint16_t row_state = scan_row(DRIVE0 + drive);

        for (uint8_t read = 0; read < NUM_READ_PINS; read++) {
            bool is_pressed = (row_state >> read) & 1;
            bool was_pressed = key_states[drive][read].pressed;

            if (is_pressed != was_pressed) {
                uint64_t time_since_change = now - key_states[drive][read].last_change_time;
                if (time_since_change >= DEBOUNCE_TIME_US) {
                    handle_key_change(drive, read, is_pressed, now);
                }
            }
        }
    }
}

// Update LED based on any key pressed
static void update_led(void) {
    static uint32_t last_update = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_update < 100) return;
    last_update = now;

    bool any_key_pressed = false;
    for (uint8_t drive = 0; drive < NUM_DRIVE_PINS && !any_key_pressed; drive++) {
        for (uint8_t read = 0; read < NUM_READ_PINS; read++) {
            if (key_states[drive][read].pressed) {
                any_key_pressed = true;
                break;
            }
        }
    }
    gpio_put(LED_PIN, any_key_pressed);
}

int main() {
    // Initialize TinyUSB
    tusb_init();

    // Initialize GPIO
    init_matrix_pins();

    // Clear key states
    memset(key_states, 0, sizeof(key_states));

    while (true) {
        // Service USB
        tud_task();

        // Scan keyboard
        scan_matrix();

        // Update LED
        update_led();

        // Small delay
        sleep_us(1000);
    }
}
