#include "usb_midi.h"
#include "tusb_config.h"

// Called in main loop
void usb_midi_task(void) {
    tud_task(); // Run TinyUSB background tasks
}

// Send a single Note On/Off message (legacy - uses fixed velocity)
void usb_midi_send_note(uint8_t note, bool on) {
    usb_midi_send_note_with_velocity(note, on, on ? 0x7F : 0x00);
}

// Send a Note On/Off message with specified velocity
void usb_midi_send_note_with_velocity(uint8_t note, bool on, uint8_t velocity) {
#if CFG_TUD_MIDI
    uint8_t cable_num = 0; // Virtual cable
    uint8_t channel   = 0; // MIDI channel 1

    uint8_t msg[3];
    msg[0] = (on ? 0x90 : 0x80) | channel; // Note On/Off
    msg[1] = note;                          // Note number
    msg[2] = velocity;                      // Velocity (0-127)

    tud_midi_stream_write(cable_num, msg, 3);
#else
    // MIDI disabled - no-op
    (void)note;
    (void)on;
    (void)velocity;
#endif
}
