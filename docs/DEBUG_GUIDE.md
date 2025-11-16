# Debug Output Guide

This guide explains how to use the USB serial debug output to monitor key presses.

## What Was Added

Your MIDI keyboard now has **two USB interfaces**:
1. **MIDI interface** - Sends MIDI notes to your DAW/music software
2. **CDC Serial interface** - Outputs debug information

Both work simultaneously over a single USB cable!

## How to Use

### 1. Flash the New Firmware

```bash
# Copy to your Pico in BOOTSEL mode
cp build/midi_keyboard.uf2 /path/to/RPI-RP2/
```

### 2. Connect Serial Monitor

After flashing, your computer will see two devices:
- **MIDI Keyboard Controller** (MIDI device)
- **Debug Serial Port** (Serial/COM port)

#### On Linux:
```bash
# Find the serial port
ls /dev/ttyACM*

# Connect with screen
screen /dev/ttyACM0 115200

# Or use minicom
minicom -D /dev/ttyACM0 -b 115200

# Or use Python
python -m serial.tools.miniterm /dev/ttyACM0 115200
```

#### On macOS:
```bash
# Find the serial port
ls /dev/cu.usbmodem*

# Connect with screen
screen /dev/cu.usbmodem14201 115200

# Or use Python
python3 -m serial.tools.miniterm /dev/cu.usbmodem14201 115200
```

#### On Windows:
Use any serial terminal:
- **PuTTY**: Select "Serial", enter COM port (e.g., COM3), speed 115200
- **Arduino IDE**: Tools → Serial Monitor
- **Tera Term**: File → New Connection → Serial
- **Windows Terminal** with `mode` command

Find COM port in Device Manager under "Ports (COM & LPT)"

### 3. What You'll See

#### Startup Message:
```
=== MIDI Keyboard Controller ===
Matrix: 11x11 = 121 keys
Debug output enabled
Waiting for key presses...
```

#### When You Press Keys:
```
[PRESS] Key[0,0] = Note 36
[RELEASE] Key[0,0] = Note 36
[PRESS] Key[2,4] = Note 62
[RELEASE] Key[2,4] = Note 62
[PRESS] Key[3,5] = Note 74
[PRESS] Key[3,6] = Note 75
[RELEASE] Key[3,5] = Note 74
[RELEASE] Key[3,6] = Note 75
```

### Output Format

```
[PRESS/RELEASE] Key[drive,read] = Note MIDI_NUMBER
```

Where:
- **drive** = Row number (0-10), corresponds to GPIO 0-10
- **read** = Column number (0-10), corresponds to GPIO 11-21
- **MIDI_NUMBER** = The note being sent (36-127)

## Use Cases

### 1. Testing Your Wiring

Connect a test switch and see which Key[x,y] coordinates it triggers. This helps you map your physical keyboard layout.

**Example:**
```
[PRESS] Key[0,0] = Note 36    ← Switch between GPIO 0 and GPIO 11
[PRESS] Key[2,5] = Note 63    ← Switch between GPIO 2 and GPIO 16
```

### 2. Verifying Debouncing

If you see multiple messages for a single press, you may need to adjust debounce time:
```
[PRESS] Key[0,0] = Note 36
[RELEASE] Key[0,0] = Note 36   ← Bounce!
[PRESS] Key[0,0] = Note 36     ← Same press
[RELEASE] Key[0,0] = Note 36
```

**Fix:** Increase `DEBOUNCE_TIME_US` in `src/keyboard.c`

### 3. Finding Ghosting/Crosstalk

If pressing one key triggers multiple coordinates, you have electrical issues:
```
[PRESS] Key[0,0] = Note 36
[PRESS] Key[0,1] = Note 37     ← Crosstalk!
```

**Causes:**
- Missing diodes in your matrix
- Wiring too close together
- Poor connections

### 4. Mapping Your Layout

Use the debug output to build your note map:

1. Press each physical key
2. Note the Key[drive,read] coordinates
3. Update `include/note_map.h` to match

**Example workflow:**
```
Physical Key 1: [PRESS] Key[0,5] = Note 41
  → Map: note_map[0][5] = 60;  // Change to Middle C

Physical Key 2: [PRESS] Key[1,3] = Note 52
  → Map: note_map[1][3] = 62;  // Change to D
```

### 5. Performance Testing

Watch how fast keys respond. Each message shows real-time detection - if there's noticeable delay, check:
- USB cable quality
- CPU load
- Scan rate settings

## Troubleshooting

### No Serial Port Appears

**Check:**
1. USB cable supports data (not just power)
2. Device Manager / system shows the device
3. You're looking for the second device (MIDI is first, Serial is second)

### Serial Port Exists But No Output

**Check:**
1. Correct baud rate (115200)
2. Firmware actually flashed (check file size ~59KB)
3. Try unplugging and replugging USB

### Garbled Output

**Fix:**
- Set baud rate to 115200
- Check terminal settings (8-N-1: 8 data bits, no parity, 1 stop bit)

### Output Stops After Some Time

This is normal if:
- Terminal buffer fills up
- USB enters power save mode

**Fix:** Scroll terminal or reconnect

## Customizing Debug Output

You can modify the debug messages in `src/keyboard.c`:

### Current Implementation:
```c
printf("[%s] Key[%d,%d] = Note %d\n",
       is_pressed ? "PRESS" : "RELEASE",
       drive, read, note);
```

### Add Timestamp:
```c
printf("[%llu us] [%s] Key[%d,%d] = Note %d\n",
       now,
       is_pressed ? "PRESS" : "RELEASE",
       drive, read, note);
```

### Show Note Name:
```c
const char *note_names[] = {"C", "C#", "D", "D#", "E", "F",
                           "F#", "G", "G#", "A", "A#", "B"};
int octave = (note / 12) - 1;
const char *name = note_names[note % 12];

printf("[%s] Key[%d,%d] = %s%d (Note %d)\n",
       is_pressed ? "PRESS" : "RELEASE",
       drive, read, name, octave, note);
```

Output: `[PRESS] Key[2,2] = C4 (Note 60)`

### CSV Format for Data Logging:
```c
printf("%llu,%s,%d,%d,%d\n",
       now,
       is_pressed ? "1" : "0",
       drive, read, note);
```

Then redirect to file: `screen -L /dev/ttyACM0` and analyze in spreadsheet.

## Disabling Debug Output

If you want to disable debug output later:

**Option 1:** Keep CDC but stop printing
```c
// In keyboard.c, comment out printf:
// printf("[%s] Key[%d,%d] = Note %d\n", ...);
```

**Option 2:** Remove CDC completely
```c
// In include/tusb_config.h:
#define CFG_TUD_CDC  0

// In CMakeLists.txt:
pico_enable_stdio_usb(midi_keyboard 0)
```

Then rebuild.

## Performance Impact

**Minimal:**
- Printf takes ~50μs per message
- Only prints on key changes (not every scan)
- USB bandwidth: <1% (few messages per second)

With typical playing (10 notes/second), debug output uses:
- CPU: <1%
- USB bandwidth: <0.1%
- No impact on MIDI latency

## Summary

**What you can debug:**
- ✅ Physical key coordinates
- ✅ Note mapping
- ✅ Debounce issues
- ✅ Ghosting/crosstalk
- ✅ Response timing
- ✅ Wiring problems

**What you can't debug:**
- ❌ MIDI messages themselves (use MIDI monitor for that)
- ❌ Hardware timing (need oscilloscope)
- ❌ Deep USB issues (need USB analyzer)

For MIDI debugging, use a MIDI monitor app:
- **Windows:** MIDI-OX, MIDIberry
- **macOS:** MIDI Monitor (Snoize)
- **Linux:** `aseqdump -p "MIDI Keyboard"`

The serial debug shows you **what the keyboard sees**, MIDI monitor shows you **what the computer receives**.
