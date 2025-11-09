#include "usb_midi.h"

// Called in main loop
void usb_midi_task(void) {
    tud_task(); // Run TinyUSB background tasks
}

// Send a single Note On/Off message
void usb_midi_send_note(uint8_t note, bool on) {
    uint8_t cable_num = 0; // Virtual cable
    uint8_t channel   = 0; // MIDI channel 1

    uint8_t msg[3];
    if (on) {
        msg[0] = 0x90 | channel; // Note On
        msg[1] = note;           // Note number
        msg[2] = 0x7F;           // Velocity
    } else {
        msg[0] = 0x80 | channel; // Note Off
        msg[1] = note;
        msg[2] = 0x00;
    }

    tud_midi_stream_write(cable_num, msg, 3);
}
