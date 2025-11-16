# USB Settings Detailed Explanation

This document explains every USB-related setting and identifies potential enumeration issues.

## 1. TinyUSB Configuration (include/tusb_config.h)

### Board Configuration
```c
#define BOARD_TUD_RHPORT      0    // Root hub port 0 (RP2040 only has one USB port)
#define BOARD_TUD_MAX_SPEED   OPT_MODE_DEFAULT_SPEED  // Full-speed USB 2.0 (12 Mbps)
```
**Why:** RP2040 has single USB controller at port 0, operates at full-speed (not high-speed)

### MCU and OS
```c
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined  // Should be OPT_MCU_RP2040 (set by build system)
#endif

#define CFG_TUSB_OS  OPT_OS_NONE  // Bare metal, no RTOS
```
**Why:** Defines target microcontroller and operating system model

### Device Configuration
```c
#define CFG_TUD_ENABLED       1    // Enable USB device mode (not host)
#define CFG_TUD_MAX_SPEED     BOARD_TUD_MAX_SPEED  // Full-speed
#define CFG_TUD_ENDPOINT0_SIZE    64  // Control endpoint EP0 packet size
```
**Why:**
- EP0 size: 64 bytes is standard for full-speed USB 2.0
- Must match bMaxPacketSize0 in device descriptor

### Enabled Classes
```c
#define CFG_TUD_CDC    1  // ← Serial/COM port
#define CFG_TUD_MSC    0  // Mass storage (disabled)
#define CFG_TUD_HID    0  // Keyboard/Mouse (disabled)
#define CFG_TUD_MIDI   1  // ← MIDI device
#define CFG_TUD_VENDOR 0  // Custom vendor class (disabled)
```
**Why:** Each enabled class adds interfaces to the device

### Buffer Sizes
```c
#define CFG_TUD_CDC_RX_BUFSIZE    64
#define CFG_TUD_CDC_TX_BUFSIZE    64
#define CFG_TUD_MIDI_RX_BUFSIZE   64  // (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_MIDI_TX_BUFSIZE   64
```
**Why:** Internal FIFO sizes for data buffering. 64 bytes is sufficient for our use.

---

## 2. Device Descriptor (usb_descriptors.c)

```c
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),     // 18 bytes
    .bDescriptorType    = TUSB_DESC_DEVICE,               // 0x01
    .bcdUSB             = 0x0200,                         // USB 2.0
    .bDeviceClass       = 0x00,      // ← POTENTIAL ISSUE!
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = 64,                             // Must match CFG_TUD_ENDPOINT0_SIZE
    .idVendor           = 0x2E8A,                         // Raspberry Pi
    .idProduct          = 0x4009,                         // Calculated: 0x4000 | CDC(1) | MIDI(8)
    .bcdDevice          = 0x0100,                         // Device version 1.0
    .iManufacturer      = 1,                              // String index
    .iProduct           = 2,
    .iSerialNumber      = 3,
    .bNumConfigurations = 1                               // One configuration
};
```

### ISSUE FOUND: Device Class for Composite Devices

**Current:**
```c
.bDeviceClass       = 0x00,  // Class defined in interface
.bDeviceSubClass    = 0x00,
.bDeviceProtocol    = 0x00,
```

**Problem:** When using CDC + MIDI (composite device), this might cause issues!

**USB Spec Says:**
- **0x00** = Class defined per-interface (works for single-class devices)
- **0xEF** = Miscellaneous Device (for multi-function devices)
- **0x02** = Communications Device Class (for CDC-based devices)

**For Composite CDC+MIDI, should be:**
```c
.bDeviceClass       = 0xEF,  // Miscellaneous
.bDeviceSubClass    = 0x02,  // Common Class
.bDeviceProtocol    = 0x01,  // Interface Association Descriptor
```

This tells the OS "this device has multiple functions, use IAD to group them."

---

## 3. Configuration Descriptor

### Interface Numbering
```c
enum {
  ITF_NUM_CDC = 0,           // CDC Control interface
  ITF_NUM_CDC_DATA,          // = 1, CDC Data interface
  ITF_NUM_MIDI,              // = 2, MIDI Control interface
  ITF_NUM_MIDI_STREAMING,    // = 3, MIDI Streaming interface
  ITF_NUM_TOTAL              // = 4
};
```
**Why:** Interfaces must be numbered consecutively starting from 0.

### Configuration Header
```c
TUD_CONFIG_DESCRIPTOR(
    1,                  // Configuration number (1)
    ITF_NUM_TOTAL,      // Number of interfaces (4)
    0,                  // String index (none)
    CONFIG_TOTAL_LEN,   // Total length in bytes
    0x00,               // Attributes (none, bus-powered)
    100                 // Max power in mA
)
```

### Endpoint Allocation
```c
// CDC endpoints
#define EPNUM_CDC_NOTIF   0x81  // Endpoint 1 IN
#define EPNUM_CDC_OUT     0x02  // Endpoint 2 OUT
#define EPNUM_CDC_IN      0x82  // Endpoint 2 IN

// MIDI endpoints
#define EPNUM_MIDI_OUT    0x03  // Endpoint 3 OUT
#define EPNUM_MIDI_IN     0x83  // Endpoint 3 IN
```

**Why this works:**
- EP0: Reserved for control (automatic)
- EP1 IN: CDC notifications only (doesn't need OUT)
- EP2: Bidirectional (CDC data)
- EP3: Bidirectional (MIDI data)

RP2040 supports up to 16 endpoints (0-15), bidirectional = 32 total endpoint addresses.

---

## 4. CDC Descriptor Details

```c
TUD_CDC_DESCRIPTOR(
    ITF_NUM_CDC,        // Interface number (0)
    4,                  // String descriptor index ("Debug Serial Port")
    EPNUM_CDC_NOTIF,    // Notification endpoint (0x81)
    8,                  // Notification size (8 bytes)
    EPNUM_CDC_OUT,      // Data OUT endpoint (0x02)
    EPNUM_CDC_IN,       // Data IN endpoint (0x82)
    64                  // Data endpoint size (64 bytes)
)
```

**This expands to multiple descriptors:**
1. Interface Association Descriptor (IAD) - groups CDC control + data
2. Interface Descriptor - CDC Control
3. CDC Functional Descriptors (Header, Call Management, ACM, Union)
4. Endpoint Descriptor - Notification
5. Interface Descriptor - CDC Data
6. Endpoint Descriptor - Data OUT
7. Endpoint Descriptor - Data IN

**ISSUE:** Without proper device class (0xEF), IAD might not work!

---

## 5. MIDI Descriptor Details

```c
TUD_MIDI_DESCRIPTOR(
    ITF_NUM_MIDI,       // Interface number (2)
    0,                  // String descriptor index (none)
    EPNUM_MIDI_OUT,     // MIDI OUT endpoint (0x03)
    EPNUM_MIDI_IN,      // MIDI IN endpoint (0x83)
    64                  // Endpoint size (64 bytes)
)
```

**This expands to:**
1. Interface Descriptor - MIDI Control (AudioControl)
2. Interface Descriptor - MIDI Streaming (MIDIStreaming)
3. MIDI Streaming descriptors
4. Endpoint Descriptor - MIDI OUT
5. Endpoint Descriptor - MIDI IN

---

## 6. Descriptor Length Calculation

```c
#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_MIDI_DESC_LEN)
```

**Let's verify:**
- TUD_CONFIG_DESC_LEN = 9 bytes (configuration header)
- TUD_CDC_DESC_LEN = 66 bytes (IAD + CDC interfaces + endpoints)
- TUD_MIDI_DESC_LEN = 101 bytes (MIDI interfaces + endpoints)
- **Total = 176 bytes**

**This must match the actual bytes sent!**

---

## 7. String Descriptors

```c
char const *string_desc_arr[] = {
  (const char[]) { 0x09, 0x04 },  // 0: Language (English US)
  "Pico MIDI",                    // 1: Manufacturer
  "MIDI Keyboard Controller",     // 2: Product
  NULL,                           // 3: Serial (generated from chip ID)
  "Debug Serial Port",            // 4: CDC interface
};
```

**Why index 3 is NULL:** Special case - generates serial number from Pico's unique ID.

---

## Potential Issues Identified

### 🔴 CRITICAL: Device Class for Composite Device

**Current (BROKEN):**
```c
.bDeviceClass       = 0x00,
.bDeviceSubClass    = 0x00,
.bDeviceProtocol    = 0x00,
```

**Should be:**
```c
.bDeviceClass       = 0xEF,  // Miscellaneous Device
.bDeviceSubClass    = 0x02,  // Common Class
.bDeviceProtocol    = 0x01,  // Interface Association Descriptor
```

### 🟡 POSSIBLE: Descriptor Length Mismatch

If TinyUSB's macro-calculated length doesn't match actual descriptor size, enumeration fails.

### 🟡 POSSIBLE: IAD (Interface Association Descriptor)

CDC requires IAD to associate Control + Data interfaces. TinyUSB should generate this automatically, but it requires proper device class!

### 🟢 LIKELY OK: Everything Else

- Endpoint numbering: ✓ Correct
- Interface numbering: ✓ Consecutive from 0
- Buffer sizes: ✓ Adequate
- String descriptors: ✓ Valid

---

## The Fix

**Change in `src/usb_descriptors.c`:**

```c
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    // FIX: Use Miscellaneous Device Class for composite device
    .bDeviceClass       = 0xEF,  // Miscellaneous Device Class
    .bDeviceSubClass    = 0x02,  // Common Class
    .bDeviceProtocol    = 0x01,  // Interface Association Descriptor

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x2E8A,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};
```

**Why this fixes it:**
1. 0xEF tells OS "I'm a multi-function device"
2. Protocol 0x01 enables IAD support
3. OS will properly enumerate CDC and MIDI as separate functions

---

## Alternative: CDC-Only Device Class

If Miscellaneous doesn't work, try CDC as primary:

```c
.bDeviceClass       = 0x02,  // CDC Communications
.bDeviceSubClass    = 0x00,
.bDeviceProtocol    = 0x00,
```

But this might hide the MIDI interface!

---

## Debugging Steps

1. **Fix device class** to 0xEF/0x02/0x01
2. **Rebuild completely:** `rm -rf build && cmake -B build -G Ninja && ninja -C build`
3. **Flash and test**
4. **Check dmesg:** `sudo dmesg | tail -30`
5. **If still fails:** Try MIDI-only (disable CDC) to isolate the issue

---

## References

- USB 2.0 Specification Chapter 9: Device Framework
- USB CDC Specification 1.1
- USB MIDI Device Specification 1.0
- TinyUSB documentation: https://docs.tinyusb.org/
- USB Class Codes: https://www.usb.org/defined-class-codes
