/*
 * MIDI Keyboard Controller - Matrix Scanning Implementation
 *
 * Scans an 11x11 key matrix efficiently and sends USB MIDI messages
 * when keys are pressed or released.
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "usb_midi.h"
#include "note_map.h"

// Hardware pin definitions
#define LED_PIN 25   // Onboard LED

#define DRIVE0  0
#define DRIVE1  1
#define DRIVE2  2
#define DRIVE3  3
#define DRIVE4  4
#define DRIVE5  5
#define DRIVE6  6
#define DRIVE7  7
#define DRIVE8  8
#define DRIVE9  9
#define DRIVE10 10

#define READ0  11
#define READ1  12
#define READ2  13
#define READ3  14
#define READ4  15
#define READ5  16
#define READ6  17
#define READ7  18
#define READ8  19
#define READ9  20
#define READ10 21

// Scanning configuration
#define DEBOUNCE_TIME_US   5000    // 5ms debounce time
#define SCAN_SETTLE_US     2       // 2us settling time after setting drive pin

// Key state tracking
typedef struct {
    bool pressed;                  // Current debounced state
    uint64_t last_change_time;     // Time of last state change (for debouncing)
} key_state_t;

static key_state_t key_states[NUM_DRIVE_PINS][NUM_READ_PINS];

// Bitmask for extracting read pins from gpio_get_all()
// Pins 11-21 = bits 11-21
static const uint32_t READ_PIN_MASK = 0x003FF800;  // Bits 11-21 set

/*
 * Initialize GPIO pins for matrix scanning
 */
static void init_matrix_pins(void) {
    // Initialize drive pins (outputs, default LOW)
    for (int pin = DRIVE0; pin <= DRIVE10; ++pin) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
    }

    // Initialize read pins (inputs with pull-down)
    for (int pin = READ0; pin <= READ10; ++pin) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);
    }

    // Initialize LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

/*
 * Scan one row of the matrix
 *
 * Sets the specified drive pin HIGH, reads all read pins at once,
 * then sets the drive pin back LOW.
 *
 * Returns: 11-bit value where bit N indicates state of READ_PIN[N]
 */
static inline uint16_t scan_row(uint8_t drive_pin) {
    // Set drive pin HIGH
    gpio_put(drive_pin, 1);

    // Wait for signal to settle (capacitance, etc.)
    busy_wait_us_32(SCAN_SETTLE_US);

    // Read all GPIO pins at once
    uint32_t gpio_state = gpio_get_all();

    // Set drive pin back LOW immediately
    gpio_put(drive_pin, 0);

    // Extract and return only the read pins (bits 11-21)
    // Shift down by 11 to get bits 0-10
    return (gpio_state & READ_PIN_MASK) >> READ0;
}

/*
 * Process key state change and send MIDI message
 */
static void handle_key_change(uint8_t drive, uint8_t read, bool is_pressed, uint64_t now) {
    // Get MIDI note for this key position
    uint8_t note = get_note(drive, read);

    // Skip unmapped keys
    if (note == NOTE_NONE) {
        return;
    }

    // Update state
    key_states[drive][read].pressed = is_pressed;
    key_states[drive][read].last_change_time = now;

    // Send MIDI message
    usb_midi_send_note(note, is_pressed);
}

/*
 * Scan entire keyboard matrix and detect changes
 */
static void scan_matrix(void) {
    uint64_t now = time_us_64();

    // Scan each row
    for (uint8_t drive = 0; drive < NUM_DRIVE_PINS; drive++) {
        // Read all keys in this row at once
        uint16_t row_state = scan_row(DRIVE0 + drive);

        // Check each key in the row
        for (uint8_t read = 0; read < NUM_READ_PINS; read++) {
            // Extract current physical state
            bool is_pressed = (row_state >> read) & 1;

            // Get previous debounced state
            bool was_pressed = key_states[drive][read].pressed;

            // Check if state changed
            if (is_pressed != was_pressed) {
                // Check if enough time has passed for debouncing
                uint64_t time_since_change = now - key_states[drive][read].last_change_time;

                if (time_since_change >= DEBOUNCE_TIME_US) {
                    // Valid state change - handle it
                    handle_key_change(drive, read, is_pressed, now);
                }
            }
        }
    }
}

/*
 * Update activity LED based on any key being pressed
 */
static void update_led(void) {
    static uint32_t last_update = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Update LED every 100ms
    if (now - last_update < 100) {
        return;
    }
    last_update = now;

    // Check if any key is pressed
    bool any_key_pressed = false;
    for (uint8_t drive = 0; drive < NUM_DRIVE_PINS && !any_key_pressed; drive++) {
        for (uint8_t read = 0; read < NUM_READ_PINS; read++) {
            if (key_states[drive][read].pressed) {
                any_key_pressed = true;
                break;
            }
        }
    }

    // LED on if any key pressed
    gpio_put(LED_PIN, any_key_pressed);
}

int main() {
    // Initialize GPIO pins
    init_matrix_pins();

    // Initialize key states (all unpressed)
    memset(key_states, 0, sizeof(key_states));

    // Main loop
    while (1) {
        // Service USB MIDI stack (MUST be called regularly)
        usb_midi_task();

        // Scan keyboard matrix
        scan_matrix();

        // Update activity LED
        update_led();

        // Small delay to prevent CPU spinning at 100%
        // (1000 scans/second is plenty for keyboard response)
        sleep_us(1000);
    }
}
