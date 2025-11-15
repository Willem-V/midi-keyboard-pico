# MIDI Key Mapping Tool

This directory contains tools for mapping physical piano keys to the keyboard matrix.

## map_keys.py

Interactive tool that captures MIDI events from your keyboard and generates the functional note mapping.

### Prerequisites

1. **Python 3.6+** installed
2. **Firmware with DEBUG_MAPPING enabled** flashed to the Pico
3. **MIDI keyboard connected** via USB

### Installation

**Option 1: Using Conda (Recommended)**

Create and activate the conda environment from the project root:

```bash
conda env create -f environment.yml
conda activate midi-keyboard-pico
```

**Option 2: Using pip**

Install Python dependencies:

```bash
pip install -r tools/requirements.txt
```

Or manually:

```bash
pip install mido python-rtmidi
```

### Usage

1. **Enable DEBUG_MAPPING** in `include/note_map.h`:
   ```c
   #define DEBUG_MAPPING  // Make sure this is uncommented
   ```

2. **Build and flash firmware**:
   ```bash
   cmake --build build
   # Copy build/midi_keyboard.uf2 to Pico in BOOTSEL mode
   ```

3. **Connect the keyboard** - it should appear as a MIDI device

4. **Run the mapping tool**:
   ```bash
   python tools/map_keys.py
   ```

5. **Follow the prompts**:
   - Select your MIDI keyboard from the list
   - Press each key as requested (C2 → C7)
   - The tool captures which matrix positions each key triggers

6. **Review the outputs**:
   - `test_results/key_mapping.json` - Machine-readable data
   - `test_results/key_mapping.md` - Human-readable reference
   - `generated_note_map.c` - C code for note_map array

7. **Update the firmware**:
   - Open `generated_note_map.c`
   - Copy the `note_map` array
   - Paste into `include/note_map.h` (replace functional mapping section)
   - Comment out `#define DEBUG_MAPPING` in note_map.h
   - Rebuild and flash

### Features

- **Auto-detects MIDI ports** - lists available devices
- **Dual velocity sensor support** - captures both sensors per key
- **Interactive prompts** - guides you through each key
- **Error handling** - retry/skip/quit options
- **Multiple output formats** - JSON, Markdown, and C code
- **Auto-generates mapping** - ready-to-copy C array

### Troubleshooting

**No MIDI ports found:**
- Check USB cable connection
- Make sure Pico is powered and running
- Try `lsusb` to verify USB enumeration

**Wrong notes captured:**
- Make sure DEBUG_MAPPING is enabled in firmware
- Verify firmware was rebuilt after enabling DEBUG_MAPPING
- Check that you're pressing the correct key

**Only 1 note per key instead of 2:**
- This is normal for keys with one working velocity sensor
- The tool handles this automatically

**Timeout on key press:**
- Press the key more firmly
- Check if the key is physically working
- Use 'r' to retry or 's' to skip

## Example Output

```
Available MIDI input ports:
  0: MIDI Keyboard:MIDI Keyboard MIDI 1 20:0

Select port (0-0): 0
Opening port: MIDI Keyboard:MIDI Keyboard MIDI 1 20:0

============================================================
MAPPING 61 KEYS (C2 to C7)
============================================================

[1/61] Press key: C2 (MIDI note 36)
  Listening... note 41 note 19 ✓ Captured 2 note(s)
  Mapped to matrix positions: [(3, 9), (0, 7)]

[2/61] Press key: C#2 (MIDI note 37)
  Listening... note 75 note 108 ✓ Captured 2 note(s)
  Mapped to matrix positions: [(6, 9), (9, 9)]

...
```

## Files Generated

- **key_mapping.json** - Complete mapping data in JSON format
- **key_mapping.md** - Human-readable mapping reference
- **generated_note_map.c** - C code ready to copy into note_map.h
