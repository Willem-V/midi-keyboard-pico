# Keyboard Implementation Guide

This document explains the keyboard scanning implementation and how to customize it.

## Overview

The implementation scans an 11×11 key matrix efficiently, detecting key presses/releases and sending MIDI messages via USB.

## How Matrix Scanning Works

### Hardware Setup

**Drive Pins (GPIO 0-10):** Outputs that activate one row at a time
**Read Pins (GPIO 11-21):** Inputs that detect if a key is pressed

The keys are arranged in a grid:
```
        Read0  Read1  Read2  ... Read10
Drive0   K00    K01    K02   ...  K010
Drive1   K10    K11    K12   ...  K110
Drive2   K20    K21    K22   ...  K210
  ⋮       ⋮      ⋮      ⋮           ⋮
Drive10  K100   K101   K102  ...  K1010
```

Each key connects its Drive pin to its Read pin when pressed.

### Scanning Algorithm

**For each drive pin (row):**
1. Set drive pin HIGH (all others LOW)
2. Wait 2μs for signal to settle
3. Read ALL 11 read pins at once using `gpio_get_all()`
4. Set drive pin back LOW
5. Process the results

**Key Optimization:** Instead of calling `gpio_get()` 11 times, we call `gpio_get_all()` once and extract bits 11-21:

```c
uint32_t gpio_state = gpio_get_all();      // Read all 32 GPIO pins at once
uint16_t row_state = (gpio_state & 0x003FF800) >> 11;  // Extract pins 11-21
```

This is **~10x faster** than individual reads!

### Scan Rate

- **Each row scan:** ~10μs (2μs settle + read + processing)
- **Full matrix scan:** ~110μs (11 rows × 10μs)
- **Scan frequency:** ~1000Hz (1ms loop delay)

This gives excellent response time - key presses detected within 1ms!

## Debouncing

Mechanical switches "bounce" when pressed - the contact opens/closes rapidly for a few milliseconds. Without debouncing, one key press could register as multiple notes.

**Implementation:**
```c
#define DEBOUNCE_TIME_US  5000   // 5ms

// Only register state change if:
// 1. Current state ≠ previous state
// 2. At least 5ms has passed since last change
```

This filters out bounces while keeping response time fast (5ms is imperceptible to humans).

## Note Mapping

The file `include/note_map.h` contains a 2D array mapping matrix positions to MIDI notes:

```c
static const uint8_t note_map[11][11] = {
    // Drive 0 (Row 0): C2 to Bb2
    { 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46 },

    // Drive 1 (Row 1): B2 to A3
    { 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57 },

    // ... etc
};
```

### Current Mapping

**Default:** Chromatic layout starting at C2 (MIDI note 36)
- Drive 0, Read 0 = C2 (36)
- Drive 0, Read 1 = C#2 (37)
- Drive 2, Read 0 = Bb3 (58)
- Drive 2, Read 2 = **Middle C** (60)
- Drive 3, Read 1 = A4 / A440 (69)

**Total notes mapped:** 92 notes (C2 to G9)
- MIDI notes 36-127
- Positions after note 127 are marked as `NOTE_NONE` (0xFF)

### Customizing the Note Map

You can completely customize which physical key produces which note:

#### Example 1: Piano Layout (White Keys Only)

For a layout with only white keys (C-D-E-F-G-A-B pattern):

```c
// First 7 keys = one octave of white keys (C-B)
// Repeat pattern across rows
{ 36, 38, 40, 41, 43, 45, 47,  // C2-B2
  48, 50, 52, 53 },             // C3-F3

{ 55, 57, 59, 60, 62, 64, 65,  // G3-F4
  67, 69, 71, 72 },             // G4-C5
```

#### Example 2: Drum Pads

Map specific positions to drum sounds (General MIDI drum map):

```c
// Map 4×4 grid to common drum sounds
{ 36, 38, 42, 46,  // Kick, Snare, Hi-hat closed, Hi-hat open
  41, 43, 45, 47,  // Low tom, Mid tom, High tom, Crash
  49, 51, 53, 55,  // Crash 2, Ride, Ride bell, Splash
  NOTE_NONE, NOTE_NONE, ... }  // Rest unmapped
```

#### Example 3: Button Box

Map specific buttons to specific notes:

```c
// Only certain positions mapped, rest unused
{ NOTE_NONE, 60, NOTE_NONE, 64, NOTE_NONE, 67, ... },  // C, E, G
{ NOTE_NONE, 62, NOTE_NONE, 65, NOTE_NONE, 69, ... },  // D, F, A
```

### MIDI Note Numbers

**Reference:**
- Middle C (C4) = 60
- A440 (concert pitch) = 69
- 88-key piano range: A0 (21) to C8 (108)
- MIDI range: 0-127 (G9 is the highest)

**Formula:** `note = (octave × 12) + semitone`

Where semitone: C=0, C#=1, D=2, ..., B=11

Examples:
- C2: (2 × 12) + 0 = 24... wait, actually C2 = 36
- Actually: **C-1 = 0**, so C2 = (2+1) × 12 = 36
- A4 (440Hz): (4+1) × 12 + 9 = 69 ✓

## Key State Tracking

Each key has its own state:

```c
typedef struct {
    bool pressed;               // Current debounced state
    uint64_t last_change_time;  // Timestamp for debouncing
} key_state_t;

key_state_t key_states[11][11];  // One per key position
```

**State transitions:**
1. Key physically pressed → Start debounce timer
2. Timer expires (5ms) → Update state, send MIDI Note On
3. Key physically released → Start debounce timer
4. Timer expires → Update state, send MIDI Note Off

## Main Loop Flow

```c
while (1) {
    usb_midi_task();      // 1. Service USB (send queued MIDI)
    scan_matrix();        // 2. Scan keyboard, detect changes
    update_led();         // 3. Update LED indicator
    sleep_us(1000);       // 4. Sleep 1ms (1kHz loop rate)
}
```

### Why This Order?

1. **USB first:** Ensures MIDI messages get sent promptly
2. **Scan second:** Detect new key presses
3. **LED third:** Visual feedback (can be slow)
4. **Sleep last:** Prevents CPU from spinning at 100%

## Performance Characteristics

### Latency

**From key press to MIDI sent:**
1. Scan delay: 0-1ms (depends where in 1ms cycle)
2. Debounce: 5ms
3. USB transmission: <1ms
4. **Total: ~6-7ms**

This is excellent - professional MIDI keyboards range from 5-15ms latency.

### CPU Usage

- Scanning: ~10% CPU (110μs scan / 1000μs loop)
- USB: ~5% CPU
- **Idle: ~85%** (available for future features)

### Memory Usage

- Key states: 11 × 11 × 9 bytes = 1,089 bytes (~1KB)
- Note map: 11 × 11 × 1 byte = 121 bytes
- **Total: ~1.2KB** (Pico has 264KB RAM)

Plenty of room for expansion!

## LED Indicator

The onboard LED (GPIO 25) lights up when **any key is pressed**.

This provides visual feedback that:
- The keyboard is working
- Keys are being detected
- USB MIDI is active

**Update rate:** 10Hz (checked every 100ms) - fast enough for visual feedback, slow enough not to waste CPU.

## Tuning Parameters

You can adjust these in `src/keyboard.c`:

```c
#define DEBOUNCE_TIME_US   5000    // Debounce time (5ms default)
#define SCAN_SETTLE_US     2       // Signal settling time (2μs)
```

**Debounce time:**
- Too low (<3ms): May register bounce as multiple notes
- Too high (>10ms): Feels sluggish
- Sweet spot: 5-7ms

**Settle time:**
- Depends on your wiring (capacitance, cable length)
- If you get false triggers, increase to 5-10μs
- Longer cables/breadboard = need more settling time

**Loop delay:**
```c
sleep_us(1000);  // 1ms = 1kHz scan rate
```
- Faster (500μs): Lower latency, higher CPU usage
- Slower (2000μs): Higher latency, lower CPU usage
- 1kHz is a good balance

## Testing Your Keyboard

### Without Hardware

You can test the software compiles:
```bash
ninja -C build
```

### With Hardware

1. **Connect one key** between Drive0 (GPIO 0) and Read0 (GPIO 11)
2. **Flash firmware:** Copy `build/midi_keyboard.uf2` to Pico
3. **Plug into computer** - should appear as "MIDI Keyboard Controller"
4. **Open MIDI monitor** (or DAW)
5. **Press key** - should send Note On (C2, note 36)
6. **Release key** - should send Note Off

### Troubleshooting

**No MIDI device appears:**
- Check USB cable (must be data cable, not just power)
- Try different USB port
- Check in device manager / system report

**LED doesn't light up when pressing key:**
- Verify GPIO connections
- Check pull-down resistors are working
- Measure voltage on read pin (should be ~3.3V when key pressed)

**Multiple notes for one press:**
- Increase `DEBOUNCE_TIME_US`
- Check for electrical noise/interference

**Wrong notes:**
- Verify physical wiring matches assumed matrix layout
- Customize `note_map.h` to match your wiring

## Next Steps

### Add Velocity Sensitivity

Currently all notes sent with maximum velocity (127). To add velocity:
1. Measure time between key press detect and signal peak
2. Map to velocity range (1-127)
3. Update `usb_midi_send_note()` to accept velocity parameter

### Add Pitch Bend / Modulation

Use additional GPIO pins for:
- Pitch bend wheel
- Modulation wheel
- Pedals (sustain, expression)

See `src/usb_midi.c` - add functions for sending CC (Control Change) messages.

### Multiple Configurations

Store different note maps in flash, allow switching between them:
- Piano mode
- Chromatic mode
- Drum pad mode

### LCD Display

Add an LCD to show:
- Current note being played
- Active configuration
- MIDI channel

### MIDI Learn Mode

Press a button, then press a key → assign that key to any MIDI note you want.

## Summary

**What you have:**
- ✅ Efficient 11×11 matrix scanning (1kHz rate)
- ✅ Debouncing (5ms)
- ✅ USB MIDI output
- ✅ Customizable note mapping
- ✅ ~6ms latency (excellent)
- ✅ LED activity indicator

**What to customize:**
- `include/note_map.h` - map physical positions to MIDI notes
- Debounce/scan timing if needed
- Add features (velocity, pitch bend, etc.)

The core scanning engine is solid and efficient - now it's ready for your hardware!
