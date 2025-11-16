/*
 * MIDI Keyboard Controller - Matrix Scanning + USB MIDI
 * WITH VELOCITY-SENSITIVE DUAL-SENSOR SUPPORT
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
#define READ0   12

// Scanning config - SUPER SLOW for debugging
#define DEBOUNCE_TIME_US   500
#define SCAN_SETTLE_US     500  // 5ms = 5000μs - VERY slow to eliminate timing issues

// ============================================================================
// VELOCITY CONFIGURATION
// ============================================================================

// Enable velocity debug output (requires USB serial)
// #define VELOCITY_DEBUG

// Velocity timing constants (in microseconds)
#define VELOCITY_TIMEOUT_US     150000  // 150ms - timeout for second sensor
#define VELOCITY_MIN_TIME_US    5000    // 5ms - fastest possible press (velocity 127)
#define VELOCITY_MAX_TIME_US    100000  // 100ms - slowest press (velocity 1)
#define VELOCITY_DEFAULT        64      // Default velocity for single-sensor keys

// Velocity curve (linear mapping)
// Shorter time = faster press = higher velocity
// Time range: 5ms (fast) to 100ms (slow)
// Velocity range: 127 (fast) to 1 (slow)

// Key velocity state machine
typedef enum {
    KEY_IDLE,           // No sensors triggered
    KEY_FIRST_PRESSED,  // First sensor triggered, waiting for second
    KEY_BOTH_PRESSED,   // Both sensors triggered, note is playing
} key_velocity_state_t;

// Velocity tracking per key (indexed by MIDI note number)
typedef struct {
    key_velocity_state_t state;
    uint64_t first_trigger_time;    // When first sensor triggered
    uint64_t second_trigger_time;   // When second sensor triggered
    uint8_t calculated_velocity;    // Calculated velocity (1-127)
    bool first_sensor_active;       // Current state of first sensor
    bool second_sensor_active;      // Current state of second sensor
} velocity_state_t;

// Array to track velocity state for each MIDI note (0-127, plus extended 128-143)
#define MAX_NOTES 144
static velocity_state_t velocity_states[MAX_NOTES];

// Legacy key state tracking (for debouncing sensors)
typedef struct {
    bool pressed;
    uint64_t last_change_time;
} key_state_t;

static key_state_t key_states[NUM_DRIVE_PINS][NUM_READ_PINS];
static const uint32_t READ_PIN_MASK = 0x047FF000; // Bits 12-22 + bit 26 (12 pins)

// ============================================================================
// VELOCITY HELPER FUNCTIONS
// ============================================================================

// Initialize velocity tracking system
static void init_velocity_system(void) {
    memset(velocity_states, 0, sizeof(velocity_states));
    for (int i = 0; i < MAX_NOTES; i++) {
        velocity_states[i].state = KEY_IDLE;
    }
}

// Calculate velocity from time difference between sensors
// Returns velocity value 1-127 (linear mapping)
// Shorter time = faster press = higher velocity
static uint8_t calculate_velocity(uint64_t delta_us) {
    // Clamp delta to valid range
    if (delta_us <= VELOCITY_MIN_TIME_US) {
        return 127; // Fastest possible
    }
    if (delta_us >= VELOCITY_MAX_TIME_US) {
        return 1;   // Slowest (but not 0, which can mean Note Off)
    }

    // Linear mapping: velocity = 127 - ((delta - min) * 126 / (max - min))
    // This gives us a range of 1-127, with faster presses = higher velocity
    uint64_t range = VELOCITY_MAX_TIME_US - VELOCITY_MIN_TIME_US;
    uint64_t offset = delta_us - VELOCITY_MIN_TIME_US;
    uint8_t velocity = 127 - (uint8_t)((offset * 126) / range);

    // Ensure we stay in valid range (should never trigger, but safety first)
    if (velocity < 1) velocity = 1;
    if (velocity > 127) velocity = 127;

    return velocity;
}

// Get first sensor note at matrix position
static inline uint8_t get_first_sensor_note(uint8_t drive, uint8_t read) {
    if (drive >= NUM_DRIVE_PINS || read >= NUM_READ_PINS) {
        return NOTE_NONE;
    }
    return first_sensor_map[drive][read];
}

// Get second sensor note at matrix position
static inline uint8_t get_second_sensor_note(uint8_t drive, uint8_t read) {
    if (drive >= NUM_DRIVE_PINS || read >= NUM_READ_PINS) {
        return NOTE_NONE;
    }
    return second_sensor_map[drive][read];
}

// Initialize GPIO for matrix
static void init_matrix_pins(void) {
    // Drive pins: outputs, default LOW (GPIO 0-11)
    for (int pin = DRIVE0; pin <= DRIVE0 + NUM_DRIVE_PINS - 1; ++pin) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
    }

    // Read pins: inputs with pull-down
    // GPIO 12-22 (11 pins)
    for (int pin = 12; pin <= 22; ++pin) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);
    }
    // GPIO 26 (1 pin)
    gpio_init(26);
    gpio_set_dir(26, GPIO_IN);
    gpio_pull_down(26);

    // LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

// Scan one row efficiently
// Columns 0-10 = GPIO 12-22, Column 11 = GPIO 26
static inline uint16_t scan_row(uint8_t drive_pin) {
    gpio_put(drive_pin, 1);
    busy_wait_us_32(SCAN_SETTLE_US);
    uint32_t gpio_state = gpio_get_all();
    gpio_put(drive_pin, 0);

    // Extract GPIO 12-22 to columns 0-10 (11 bits)
    uint16_t result = (gpio_state >> 12) & 0x7FF;

    // Extract GPIO 26 to column 11
    if (gpio_state & (1 << 26)) {
        result |= (1 << 11);
    }

    return result;
}

// ============================================================================
// VELOCITY-AWARE MIDI FUNCTIONS
// ============================================================================

// Send MIDI note with velocity
// Notes 0-127: sent on channel 0
// Notes 128-143: sent as (note - 128) on channel 1 (for DEBUG mode)
static void send_midi_note_velocity(uint8_t note, bool on, uint8_t velocity) {
    if (note >= MAX_NOTES) return; // Safety check

    uint8_t msg[3];
    uint8_t channel = 0;
    uint8_t actual_note = note;

    // Handle extended notes (>127) by using channel 1
    if (note >= 128) {
        channel = 1;
        actual_note = note - 128;
    }

    msg[0] = (on ? 0x90 : 0x80) | channel; // Note On/Off with channel
    msg[1] = actual_note;
    msg[2] = velocity; // Use provided velocity
    tud_midi_stream_write(0, msg, 3);

#ifdef VELOCITY_DEBUG
    if (on) {
        printf("Note %d ON, velocity %d\n", note, velocity);
    } else {
        printf("Note %d OFF\n", note);
    }
#endif
}

// Handle first sensor state change
static void handle_first_sensor(uint8_t note, bool is_pressed, uint64_t now) {
    if (note == NOTE_NONE || note >= MAX_NOTES) return;

    velocity_state_t *vs = &velocity_states[note];
    vs->first_sensor_active = is_pressed;

    if (is_pressed && vs->state == KEY_IDLE) {
        // First sensor pressed - start velocity measurement
        vs->state = KEY_FIRST_PRESSED;
        vs->first_trigger_time = now;

#ifdef VELOCITY_DEBUG
        printf("First sensor: note %d pressed at %llu\n", note, now);
#endif
    }
    else if (!is_pressed && vs->state != KEY_IDLE) {
        // First sensor released
        if (vs->state == KEY_BOTH_PRESSED && !vs->second_sensor_active) {
            // Both sensors now released - send Note Off
            send_midi_note_velocity(note, false, 0);
            vs->state = KEY_IDLE;

#ifdef VELOCITY_DEBUG
            printf("First sensor: note %d released (both off)\n", note);
#endif
        }
        else if (vs->state == KEY_FIRST_PRESSED) {
            // First sensor released before second triggered - timeout case
            vs->state = KEY_IDLE;

#ifdef VELOCITY_DEBUG
            printf("First sensor: note %d released early\n", note);
#endif
        }
    }
}

// Handle second sensor state change
static void handle_second_sensor(uint8_t note, bool is_pressed, uint64_t now) {
    if (note == NOTE_NONE || note >= MAX_NOTES) return;

    velocity_state_t *vs = &velocity_states[note];
    vs->second_sensor_active = is_pressed;

    if (is_pressed && (vs->state == KEY_FIRST_PRESSED || vs->state == KEY_IDLE)) {
        // Second sensor pressed
        uint8_t velocity;

        if (vs->state == KEY_FIRST_PRESSED) {
            // Both sensors active - calculate velocity
            uint64_t delta = now - vs->first_trigger_time;
            velocity = calculate_velocity(delta);

#ifdef VELOCITY_DEBUG
            printf("Second sensor: note %d pressed, delta=%llu us, velocity=%d\n",
                   note, delta, velocity);
#endif
        } else {
            // Second sensor pressed without first (shouldn't happen normally, but handle it)
            velocity = VELOCITY_DEFAULT;

#ifdef VELOCITY_DEBUG
            printf("Second sensor: note %d pressed WITHOUT first sensor, using default velocity\n", note);
#endif
        }

        vs->state = KEY_BOTH_PRESSED;
        vs->second_trigger_time = now;
        vs->calculated_velocity = velocity;

        // Send Note On with calculated velocity
        send_midi_note_velocity(note, true, velocity);
    }
    else if (!is_pressed && vs->state == KEY_BOTH_PRESSED) {
        // Second sensor released
        if (!vs->first_sensor_active) {
            // Both sensors released - send Note Off
            send_midi_note_velocity(note, false, 0);
            vs->state = KEY_IDLE;

#ifdef VELOCITY_DEBUG
            printf("Second sensor: note %d released (both off)\n", note);
#endif
        }
    }
}

// Check for velocity timeout (first sensor triggered but second hasn't within timeout)
static void check_velocity_timeout(uint64_t now) {
    for (int note = 0; note < MAX_NOTES; note++) {
        velocity_state_t *vs = &velocity_states[note];

        if (vs->state == KEY_FIRST_PRESSED) {
            uint64_t time_waiting = now - vs->first_trigger_time;

            if (time_waiting >= VELOCITY_TIMEOUT_US) {
                // Timeout - send Note On with default velocity
                vs->state = KEY_BOTH_PRESSED;
                vs->calculated_velocity = VELOCITY_DEFAULT;
                send_midi_note_velocity(note, true, VELOCITY_DEFAULT);

#ifdef VELOCITY_DEBUG
                printf("Timeout: note %d, using default velocity after %llu us\n",
                       note, time_waiting);
#endif
            }
        }
    }
}

// ============================================================================
// DUAL-SENSOR MATRIX SCANNING
// ============================================================================

// Scan entire matrix for both first and second sensors
static void scan_matrix(void) {
    uint64_t now = time_us_64();

    // Scan all drive/read positions
    for (uint8_t drive = 0; drive < NUM_DRIVE_PINS; drive++) {
        uint16_t row_state = scan_row(DRIVE0 + drive);

        for (uint8_t read = 0; read < NUM_READ_PINS; read++) {
            bool is_pressed = (row_state >> read) & 1;
            bool was_pressed = key_states[drive][read].pressed;

            // Debounce: only process if state changed and enough time has passed
            if (is_pressed != was_pressed) {
                uint64_t time_since_change = now - key_states[drive][read].last_change_time;
                if (time_since_change >= DEBOUNCE_TIME_US) {
                    // Update debounce state
                    key_states[drive][read].pressed = is_pressed;
                    key_states[drive][read].last_change_time = now;

                    // Check if this position is a first sensor
                    uint8_t first_note = get_first_sensor_note(drive, read);
                    if (first_note != NOTE_NONE) {
                        handle_first_sensor(first_note, is_pressed, now);
                    }

                    // Check if this position is a second sensor
                    uint8_t second_note = get_second_sensor_note(drive, read);
                    if (second_note != NOTE_NONE) {
                        handle_second_sensor(second_note, is_pressed, now);
                    }
                }
            }
        }
    }

    // Check for timeouts (first sensor triggered but second hasn't responded)
    check_velocity_timeout(now);
}

// Update LED based on any key pressed (check velocity states)
static void update_led(void) {
    static uint32_t last_update = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_update < 100) return;
    last_update = now;

    bool any_key_pressed = false;
    for (int note = 0; note < MAX_NOTES && !any_key_pressed; note++) {
        if (velocity_states[note].state == KEY_BOTH_PRESSED) {
            any_key_pressed = true;
            break;
        }
    }
    gpio_put(LED_PIN, any_key_pressed);
}

int main() {
    // Initialize TinyUSB
    tusb_init();

    // Initialize GPIO
    init_matrix_pins();

    // Clear key states (for debouncing)
    memset(key_states, 0, sizeof(key_states));

    // Initialize velocity tracking system
    init_velocity_system();

    while (true) {
        // Service USB
        tud_task();

        // Scan keyboard (dual-sensor with velocity detection)
        scan_matrix();

        // Update LED
        update_led();

        // Small delay
        sleep_us(1000);
    }
}
