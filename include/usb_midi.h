#ifndef USB_MIDI_H
#define USB_MIDI_H

#include "tusb.h"

void usb_midi_task(void);
void usb_midi_send_note(uint8_t note, bool on);
void usb_midi_send_note_with_velocity(uint8_t note, bool on, uint8_t velocity);

#endif
