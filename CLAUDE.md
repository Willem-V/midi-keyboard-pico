# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

USB MIDI keyboard controller for Raspberry Pi Pico. Reads keyboard matrix input (11x11 matrix) and sends USB MIDI messages via TinyUSB.

Uses Pico SDK 2.2.0 with TinyUSB for USB MIDI device functionality.

## Directory Structure

```
midi-keyboard-pico/
├── src/                    # Main source code
│   ├── keyboard.c         # Keyboard matrix scanning (11x11 GPIO matrix)
│   └── usb_midi.c         # USB MIDI implementation
├── include/
│   ├── note_map.h         # Key-to-MIDI-note mapping (DEBUG/FUNCTIONAL modes)
│   └── usb_midi.h         # USB MIDI interface
├── build/                 # Build output (generated)
├── tools/                 # Python tools for key mapping
│   ├── map_keys.py        # Interactive MIDI key mapper
│   ├── requirements.txt   # Python dependencies
│   └── README.md          # Tool documentation
├── test_results/          # Mapping test results
├── .vscode/               # VSCode configuration
├── examples/              # Reference examples from development
│   ├── gpio-test/        # Simple GPIO hardware validation
│   └── initial-midi/     # Early USB MIDI template
├── pico/
│   ├── pico-sdk/         # Raspberry Pi Pico SDK (v2.2.0)
│   └── pico-examples/    # SDK examples
├── environment.yml        # Conda environment for Python tools
├── CMakeLists.txt        # Main project build configuration
├── pico_sdk_import.cmake # SDK locator
└── .gitignore
```

## Building the Project

The main project is at the repository root. Build using CMake + Ninja:

```bash
# Configure (first time or after CMakeLists.txt changes):
cmake -B build -G Ninja

# Build:
ninja -C build
```

### VSCode Integration
The VSCode Pico extension provides these paths (configured in `.vscode/settings.json`):
- CMake: `$HOME/.pico-sdk/cmake/v3.31.5/bin/cmake`
- Ninja: `$HOME/.pico-sdk/ninja/v1.12.1/ninja`
- Toolchain: `$HOME/.pico-sdk/toolchain/14_2_Rel1` (arm-none-eabi-gcc)
- Picotool: `$HOME/.pico-sdk/picotool/2.2.0-a4/picotool`

You can build directly from VSCode using the "Compile Project" task.

### Build Outputs

Generated in `build/`:
- `midi_keyboard.uf2` - Firmware for drag-and-drop flashing
- `midi_keyboard.elf` - Executable with debug symbols
- `midi_keyboard.bin`, `midi_keyboard.hex` - Alternative firmware formats
- `midi_keyboard.dis` - Disassembly listing
- `midi_keyboard.elf.map` - Linker map

## Flashing the Pico

1. **Enter bootloader mode**: Hold BOOTSEL button while plugging in USB
2. **Flash**: Copy `build/midi_keyboard.uf2` to the RPI-RP2 drive
3. **Alternative**: Use `picotool load -x build/midi_keyboard.uf2` (requires BOOTSEL mode)

## Hardware Configuration

### Keyboard Matrix (src/keyboard.c)
- **Drive pins**: GPIO 0-10 (outputs, default LOW)
- **Read pins**: GPIO 11-21 (inputs with pull-down resistors)
- **LED**: GPIO 25 (onboard LED)
- **Matrix size**: 11×11 = up to 121 keys

**Scanning pattern**: Set one drive pin HIGH at a time, read all read pins to detect which keys in that row/column are pressed.

## Code Architecture

### USB MIDI Implementation
- `src/usb_midi.c` / `include/usb_midi.h` - Simple MIDI message API
  - `usb_midi_task()` - Must be called in main loop to service USB stack
  - `usb_midi_send_note(note, on)` - Send Note On/Off messages
- `src/usb_descriptors.c` - USB device identity (VID/PID, device name, endpoints)
- `include/tusb_config.h` - TinyUSB configuration (enables MIDI, sets buffer sizes)

**See USB_EXPLANATION.md for detailed explanation of how USB MIDI works.**

Uses TinyUSB library (`tinyusb_device`, `tinyusb_board`).

### Keyboard Matrix Scanning (src/keyboard.c)
Currently sets up the 11×11 GPIO matrix but main loop is empty. To implement:
1. Call `usb_midi_task()` in main loop
2. Scan matrix: iterate through drive pins, read input pins
3. Detect key state changes (pressed/released)
4. Call `usb_midi_send_note()` for each state change

## SDK Configuration

- **Target board**: `pico` (Raspberry Pi Pico with RP2040)
- **C standard**: C11
- **C++ standard**: C++17
- **Stdio**: UART and USB stdio disabled to avoid conflicts with USB MIDI

## Examples

Reference code in `examples/` directory:
- **gpio-test**: Simple GPIO validation with LED feedback
- **initial-midi**: Early USB MIDI prototype with basic GPIO testing

These are kept as development references but not actively maintained.

## Python Tools for Key Mapping

The `tools/` directory contains Python utilities for mapping physical keys to the matrix.

### Setup Python Environment

**Using Conda (Recommended):**
```bash
conda env create -f environment.yml
conda activate midi-keyboard-pico
```

**Using pip:**
```bash
pip install -r tools/requirements.txt
```

### Key Mapping Tool (tools/map_keys.py)

Interactive tool to discover which matrix positions each physical key triggers:

1. Enable `#define DEBUG_MAPPING` in `include/note_map.h`
2. Build and flash firmware
3. Run: `python tools/map_keys.py`
4. Press each key as prompted (C2 → C7)
5. Tool generates:
   - `test_results/key_mapping.json` - Raw mapping data
   - `test_results/key_mapping.md` - Human-readable reference
   - `generated_note_map.c` - C code for functional mapping
6. Copy generated array into `note_map.h` and rebuild

See `tools/README.md` for detailed instructions.

### Note Mapping (include/note_map.h)

Two mapping modes controlled by `#define DEBUG_MAPPING`:

- **DEBUG_MAPPING**: Sequential notes C_1 (0) → C9 (120) for discovering matrix positions
- **FUNCTIONAL**: Maps matrix positions to actual piano keys (C2 → C7)

The Python tool uses DEBUG mode to discover the mapping, then generates the FUNCTIONAL mapping.
