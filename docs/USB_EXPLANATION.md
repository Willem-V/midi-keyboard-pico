# USB MIDI Implementation Explained

This document explains the USB-related files in this project for someone new to USB development.

## Overview: How USB Devices Work

When you plug a USB device into your computer, the computer asks it "What are you?" The device responds with **descriptors** - structured data that describes:
- What type of device it is (keyboard, mouse, MIDI controller, etc.)
- Who made it (Vendor ID)
- What model it is (Product ID)
- What capabilities it has

Think of descriptors as a device's "resume" that it shows to the computer.

## The USB Files in This Project

### 1. `include/tusb_config.h` - Configuration Settings

This file configures the TinyUSB library for your specific needs.

```c
#define CFG_TUD_MIDI              1    // Enable MIDI device class
#define CFG_TUD_CDC               0    // Disable serial port (CDC)
#define CFG_TUD_MSC               0    // Disable mass storage
#define CFG_TUD_HID               0    // Disable HID (keyboard/mouse)
```

**What this means:**
- We're telling TinyUSB "this device is a MIDI controller, nothing else"
- Other device types (serial ports, storage devices, keyboards) are disabled
- This keeps the code small and focused

**MIDI Buffer Sizes:**
```c
#define CFG_TUD_MIDI_RX_BUFSIZE   64   // Receive buffer (computer → Pico)
#define CFG_TUD_MIDI_TX_BUFSIZE   64   // Transmit buffer (Pico → computer)
```
- 64 bytes is plenty for MIDI messages (each MIDI note message is only 3 bytes)
- These buffers temporarily store messages while USB is busy

### 2. `src/usb_descriptors.c` - Device Identity and Capabilities

This file contains three types of descriptors that answer the computer's questions:

#### A. Device Descriptor - "Who are you?"

```c
tusb_desc_device_t const desc_device = {
    .bcdUSB             = 0x0200,        // USB 2.0
    .bDeviceClass       = 0x00,          // Device class defined in interface

    .idVendor           = 0xCafe,        // Vendor ID (should be registered)
    .idProduct          = USB_PID,       // Product ID (auto-generated)
    .bcdDevice          = 0x0100,        // Device version 1.0

    .iManufacturer      = 0x01,          // String #1 = manufacturer name
    .iProduct           = 0x02,          // String #2 = product name
    .iSerialNumber      = 0x03,          // String #3 = serial number

    .bNumConfigurations = 0x01           // One configuration
};
```

**What this means:**
- **Vendor ID (0xCafe)**: This is a test ID. In production, you'd register for a real VID from usb.org (~$6000) or use a shared VID
- **Product ID**: Auto-generated based on enabled features (MIDI in this case)
- **Version**: Your device's version number (1.0)
- **Strings**: References to human-readable names (defined below)

#### B. Configuration Descriptor - "What can you do?"

```c
enum {
  ITF_NUM_MIDI = 0,              // Interface 0: MIDI control
  ITF_NUM_MIDI_STREAMING,        // Interface 1: MIDI streaming
  ITF_NUM_TOTAL
};

uint8_t const desc_fs_configuration[] = {
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
  TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64)
};
```

**What this means:**
- **Interfaces**: A USB device can have multiple functions (interfaces). MIDI requires 2:
  - Interface 0: MIDI Control (handles device setup)
  - Interface 1: MIDI Streaming (actual MIDI data)
- **Endpoints**: Communication channels for data transfer
  - `EPNUM_MIDI_OUT` (0x01): Computer → Pico (you won't use this for a keyboard)
  - `EPNUM_MIDI_IN` (0x81): Pico → Computer (where you send note messages)
  - The `0x80` bit indicates "IN" direction (towards computer)
- **Power**: Requests 100mA from USB bus

**Key Concept - Endpoints:**
Think of endpoints like phone lines:
- Endpoint 0 is always the "control line" for setup/configuration
- Endpoint 1 OUT is your "incoming data line" (computer sends to Pico)
- Endpoint 1 IN is your "outgoing data line" (Pico sends to computer)

When you call `usb_midi_send_note()`, it writes to endpoint 1 IN.

#### C. String Descriptors - "What's your name?"

```c
char const *string_desc_arr[] = {
  (const char[]) { 0x09, 0x04 },  // 0: Language (English)
  "Pico MIDI",                    // 1: Manufacturer
  "MIDI Keyboard Controller",     // 2: Product name
  NULL,                           // 3: Serial (generated from unique ID)
};
```

**What this means:**
- String 0: Language identifier (0x0409 = US English)
- String 1: Manufacturer name - appears in device manager as "made by Pico MIDI"
- String 2: Product name - appears as "MIDI Keyboard Controller"
- String 3: Serial number - auto-generated from Pico's unique hardware ID

**How serial number works:**
```c
case STRID_SERIAL: {
  pico_unique_board_id_t id;
  pico_get_unique_board_id(&id);  // Read Pico's unique 64-bit ID

  // Convert to hex string (e.g., "E6614103E7594D35")
  for (size_t i = 0; i < 8; i++) {
    uint8_t byte = id.id[i];
    _desc_str[1 + i*2    ] = "0123456789ABCDEF"[byte >> 4];
    _desc_str[1 + i*2 + 1] = "0123456789ABCDEF"[byte & 0x0F];
  }
}
```
Every Pico has a unique ID burned in at the factory. This code converts it to a hex string so your computer can tell different keyboards apart.

## How USB MIDI Messages Work

### MIDI Message Format

A MIDI Note On/Off message is 3 bytes:

```
Byte 0: Status Byte
  - 0x90 = Note On, channel 1
  - 0x80 = Note Off, channel 1
  - Lower 4 bits = MIDI channel (0-15 for channels 1-16)

Byte 1: Note Number (0-127)
  - Middle C = 60
  - A440 = 69
  - Piano range: 21-108

Byte 2: Velocity (0-127)
  - How hard the key was pressed
  - 0x7F = maximum (127)
  - 0x00 = silent
```

### Example: Playing Middle C

```c
// In your keyboard scanning code:
if (key_just_pressed) {
    usb_midi_send_note(60, true);   // Note On: Middle C
}
if (key_just_released) {
    usb_midi_send_note(60, false);  // Note Off: Middle C
}
```

**What happens internally** (in `src/usb_midi.c`):

```c
void usb_midi_send_note(uint8_t note, bool on) {
    uint8_t cable_num = 0;  // Virtual cable (MIDI has 16 "cables")
    uint8_t channel   = 0;  // MIDI channel 1 (0-indexed)

    uint8_t msg[3];
    if (on) {
        msg[0] = 0x90 | channel;  // 0x90 = Note On
        msg[1] = note;            // Note number
        msg[2] = 0x7F;            // Max velocity
    } else {
        msg[0] = 0x80 | channel;  // 0x80 = Note Off
        msg[1] = note;
        msg[2] = 0x00;            // Release velocity (usually 0)
    }

    tud_midi_stream_write(cable_num, msg, 3);  // Send via USB
}
```

The `tud_midi_stream_write()` function:
1. Puts the 3-byte message in the transmit buffer
2. Signals the USB hardware that data is ready
3. USB hardware sends it to the computer via endpoint 1 IN

## The USB Task Loop

```c
void usb_midi_task(void) {
    tud_task();  // Handle USB events
}
```

**Why you need this:**
USB communication requires constant attention to handle:
- Computer requests (descriptor requests, configuration changes)
- Buffer management (moving data from your buffers to USB hardware)
- Connection events (device plugged/unplugged)

**You must call this in your main loop:**
```c
while (1) {
    usb_midi_task();        // Service USB stack (10-100 times per second minimum)
    scan_keyboard();        // Check for key presses
    // ... rest of your code
}
```

If you don't call `usb_midi_task()` frequently enough:
- The computer might think the device is unresponsive
- MIDI messages won't be sent
- Configuration requests will timeout

## Putting It All Together

### Device Enumeration Flow (What happens when you plug it in)

1. **Pico is plugged in**
   - USB hardware initializes
   - Computer sends "Get Device Descriptor" request

2. **Computer reads device descriptor**
   ```
   Computer: "What are you?"
   Pico: "I'm a USB 2.0 device, VID=0xCafe, PID=0x4008, version 1.0"
   ```

3. **Computer assigns an address**
   ```
   Computer: "Your address is 5"
   Pico: "OK, I'm now device #5"
   ```

4. **Computer reads configuration descriptor**
   ```
   Computer: "What can you do?"
   Pico: "I have 2 interfaces for MIDI, endpoint 1 for data"
   ```

5. **Computer reads string descriptors**
   ```
   Computer: "What's your name?"
   Pico: "I'm a 'MIDI Keyboard Controller' by 'Pico MIDI'"
   ```

6. **Computer loads MIDI driver**
   - Operating system recognizes the MIDI interface descriptor
   - Loads generic USB MIDI driver
   - Device appears as "MIDI Keyboard Controller" in DAW software

7. **Ready to send MIDI**
   - Your code calls `usb_midi_send_note()`
   - Messages flow through endpoint 1 IN to the computer
   - DAW receives and plays the notes

## Summary

**tusb_config.h**: "I want to be a MIDI device with these buffer sizes"

**usb_descriptors.c**: "Here's my identity card that tells the computer I'm a MIDI controller called 'MIDI Keyboard Controller'"

**usb_midi.c**: "Here's how to send MIDI notes using the USB connection"

**Your keyboard.c**: Scans keys and calls `usb_midi_send_note()` when keys are pressed/released

The USB complexity is handled by TinyUSB - you just need to:
1. Call `usb_midi_task()` regularly
2. Call `usb_midi_send_note(note_number, pressed)` when keys change state

That's it! The descriptors make your device appear as a MIDI controller, and the rest happens automatically.
