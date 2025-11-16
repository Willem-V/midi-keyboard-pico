#!/usr/bin/env python3
"""
MIDI Key Mapping Tool

This tool helps map physical piano keys to the keyboard matrix positions
by capturing MIDI events and correlating them with the debug note mapping.

Requirements: pip install mido python-rtmidi
"""

import json
import time
from typing import List, Tuple, Dict
import mido

# Debug mapping: matrix position to MIDI note number
# Matrix: 12×12 = 144 positions.
# Positions 0-127: sent as notes 0-127 on channel 0
# Positions 128-143: sent as notes 0-15 on channel 1
# Columns: 0-10 = GPIO 12-22, Column 11 = GPIO 26
# This matches the DEBUG_MAPPING in note_map.h
DEBUG_MAPPING = [
    # Row 0: 0-11
    [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11],
    # Row 1: 12-23
    [12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23],
    # Row 2: 24-35
    [24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35],
    # Row 3: 36-47
    [36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47],
    # Row 4: 48-59
    [48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59],
    # Row 5: 60-71
    [60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71],
    # Row 6: 72-83
    [72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83],
    # Row 7: 84-95
    [84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95],
    # Row 8: 96-107
    [96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107],
    # Row 9: 108-119
    [108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119],
    # Row 10: 120-131 (positions 120-127 on ch0, 128-131 on ch1 as notes 0-3)
    [120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131],
    # Row 11: 132-143 (on ch1 as notes 4-15)
    [132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143]
]

# MIDI note names and numbers for a 61-key keyboard (C2 to C7)
KEYS_TO_MAP = [
    # Octave 2
    ("C2", 36), ("C#2", 37), ("D2", 38), ("D#2", 39), ("E2", 40), ("F2", 41),
    ("F#2", 42), ("G2", 43), ("G#2", 44), ("A2", 45), ("A#2", 46), ("B2", 47),
    # Octave 3
    ("C3", 48), ("C#3", 49), ("D3", 50), ("D#3", 51), ("E3", 52), ("F3", 53),
    ("F#3", 54), ("G3", 55), ("G#3", 56), ("A3", 57), ("A#3", 58), ("B3", 59),
    # Octave 4
    ("C4", 60), ("C#4", 61), ("D4", 62), ("D#4", 63), ("E4", 64), ("F4", 65),
    ("F#4", 66), ("G4", 67), ("G#4", 68), ("A4", 69), ("A#4", 70), ("B4", 71),
    # Octave 5
    ("C5", 72), ("C#5", 73), ("D5", 74), ("D#5", 75), ("E5", 76), ("F5", 77),
    ("F#5", 78), ("G5", 79), ("G#5", 80), ("A5", 81), ("A#5", 82), ("B5", 83),
    # Octave 6
    ("C6", 84), ("C#6", 85), ("D6", 86), ("D#6", 87), ("E6", 88), ("F6", 89),
    ("F#6", 90), ("G6", 91), ("G#6", 92), ("A6", 93), ("A#6", 94), ("B6", 95),
    # Octave 7
    ("C7", 96)
]


def note_to_matrix_position(note: int, channel: int = 0) -> Tuple[int, int]:
    """
    Convert MIDI note number + channel to matrix [row, col] using DEBUG_MAPPING.
    Channel 0: positions 0-127 (notes 0-127)
    Channel 1: positions 128-143 (notes 0-15 on channel 1)
    """
    # For channel 1, convert to position number (128 + note)
    position = note if channel == 0 else (128 + note)

    for row in range(12):
        for col in range(12):
            if DEBUG_MAPPING[row][col] == position:
                return (row, col)
    return None


def select_midi_port() -> mido.ports.BaseInput:
    """List available MIDI input ports and let user select one."""
    ports = mido.get_input_names()

    if not ports:
        print("ERROR: No MIDI input ports found!")
        print("Make sure your MIDI keyboard is connected.")
        exit(1)

    print("\nAvailable MIDI input ports:")
    for i, port in enumerate(ports):
        print(f"  {i}: {port}")

    while True:
        try:
            choice = input(f"\nSelect port (0-{len(ports)-1}): ").strip()
            port_idx = int(choice)
            if 0 <= port_idx < len(ports):
                port_name = ports[port_idx]
                print(f"Opening port: {port_name}")
                return mido.open_input(port_name)
            else:
                print(f"Invalid choice. Enter a number between 0 and {len(ports)-1}")
        except (ValueError, KeyboardInterrupt):
            print("\nExiting...")
            exit(0)


def capture_key_press(port: mido.ports.BaseInput) -> Dict:
    """
    Capture MIDI Note On events for a single key press.
    User presses key, then presses Enter to confirm or 'r' to retry.
    Returns dict with notes, channels, and sensor order info.
    """
    import sys
    import select

    note_triggers = []  # List of (note, channel, timestamp) tuples

    print("  Press key, then Enter to confirm (or 'r' + Enter to retry): ", end="", flush=True)

    while True:
        # Check for MIDI messages
        msg = port.poll()
        if msg is not None and msg.type == 'note_on':
            # Check if this note+channel hasn't been seen yet
            seen = [(note, ch) for note, ch, _ in note_triggers]
            if (msg.note, msg.channel) not in seen:
                timestamp = time.time()
                note_triggers.append((msg.note, msg.channel, timestamp))
                ch_str = f" ch{msg.channel}" if msg.channel > 0 else ""
                print(f"\n  Detected note {msg.note}{ch_str}...", end="", flush=True)

        # Check for user input (Enter or 'r')
        # Non-blocking check for user input (Enter or 'r')
        ready, _, _ = select.select([sys.stdin], [], [], 0)
        if ready:
            user_input = sys.stdin.readline()
            if user_input is None:
                user_input = ""
            else:
                user_input = user_input.rstrip("\n")
        else:
            # No user input yet; wait a tiny bit and continue polling MIDI
            time.sleep(0.01)
            continue

        if user_input.lower().strip() == 'r':
            print("  Retrying... ", end="", flush=True)
            return {'retry': True}
        else:
            # User pressed Enter - done capturing
            break

    # Determine trigger order
    result = {
        'count': len(note_triggers)
    }

    # If dual sensors detected, record which triggered first
    if len(note_triggers) == 2:
        first_note, first_ch, _ = note_triggers[0]
        second_note, second_ch, _ = note_triggers[1]

        result['first_sensor'] = (first_note, first_ch)
        result['second_sensor'] = (second_note, second_ch)

        ch1_str = f" ch{first_ch}" if first_ch > 0 else ""
        ch2_str = f" ch{second_ch}" if second_ch > 0 else ""
        print(f"✓ Dual sensors: {first_note}{ch1_str} (first) → {second_note}{ch2_str} (second)")
    elif len(note_triggers) == 1:
        note, ch, _ = note_triggers[0]
        result['first_sensor'] = (note, ch)
        result['second_sensor'] = None
        ch_str = f" ch{ch}" if ch > 0 else ""
        print(f"✓ Single sensor: {note}{ch_str}")
    else:
        result['first_sensor'] = None
        result['second_sensor'] = None
        print(f"✓ Captured {len(note_triggers)} note(s)")

    return result


def map_all_keys(port: mido.ports.BaseInput) -> Dict:
    """Map all keys interactively."""
    mapping = {}
    total_keys = len(KEYS_TO_MAP)

    print(f"\n{'='*60}")
    print(f"MAPPING {total_keys} KEYS (C2 to C7)")
    print(f"{'='*60}")
    print("\nInstructions:")
    print("  1. Press the requested piano key")
    print("  2. Press Enter to confirm")
    print("  3. Or type 'r' + Enter to retry")
    print("  4. Press Ctrl+C to quit and save partial results\n")

    for idx, (key_name, expected_note) in enumerate(KEYS_TO_MAP):
        print(f"\n[{idx+1}/{total_keys}] {key_name} (MIDI note {expected_note})")

        while True:
            capture_result = capture_key_press(port)

            # Check if user wants to retry
            if capture_result.get('retry'):
                continue

            # Check if user wants to skip (empty capture)
            if capture_result.get('count', 0) == 0:
                choice = input("  No notes captured. Skip (s) or Quit (q)? ").strip().lower()
                if choice == 's':
                    capture_result = {'notes': [], 'count': 0, 'first_sensor': None, 'second_sensor': None}
                    break
                elif choice == 'q':
                    print("\nQuitting...")
                    return mapping
                else:
                    continue
            else:
                break

        # Extract sensor info (now tuples of (note, channel))
        first_sensor = capture_result.get('first_sensor')
        second_sensor = capture_result.get('second_sensor')

        # Map sensors to matrix positions
        positions = []
        if first_sensor:
            note, ch = first_sensor
            pos = note_to_matrix_position(note, ch)
            if pos:
                positions.append(pos)
            else:
                ch_str = f" ch{ch}" if ch > 0 else ""
                print(f"  WARNING: Note {note}{ch_str} not found in debug mapping!")

        if second_sensor:
            note, ch = second_sensor
            pos = note_to_matrix_position(note, ch)
            if pos:
                positions.append(pos)
            else:
                ch_str = f" ch{ch}" if ch > 0 else ""
                print(f"  WARNING: Note {note}{ch_str} not found in debug mapping!")

        mapping[key_name] = {
            "expected_midi_note": expected_note,
            "matrix_positions": positions,
            "first_sensor": first_sensor,
            "second_sensor": second_sensor
        }

        if positions:
            print(f"  Mapped to matrix positions: {positions}")

    return mapping


def save_results(mapping: Dict):
    """Save mapping results to multiple output files."""
    import os

    # Create test_results directory if it doesn't exist
    os.makedirs("test_results", exist_ok=True)

    # 1. Save JSON (machine-readable)
    json_file = "test_results/key_mapping.json"
    with open(json_file, 'w') as f:
        json.dump(mapping, f, indent=2)
    print(f"\n✓ Saved JSON mapping to: {json_file}")

    # 2. Save Markdown (human-readable)
    md_file = "test_results/key_mapping.md"
    with open(md_file, 'w') as f:
        f.write("# MIDI Key Mapping Results\n\n")
        f.write("Generated by map_keys.py with velocity sensor ordering\n\n")
        f.write("Format: Key (MIDI note): Positions [Sensor order: first → second]\n\n")

        for key_name, data in mapping.items():
            positions = data['matrix_positions']
            first_sensor = data.get('first_sensor')
            second_sensor = data.get('second_sensor')

            f.write(f"**{key_name}** (MIDI {data['expected_midi_note']}): ")
            if positions:
                pos_str = ", ".join([f"[{r},{c}]" for r, c in positions])
                f.write(f"Positions {pos_str}")

                # Add sensor order if available
                if first_sensor and second_sensor:
                    note1, ch1 = first_sensor
                    note2, ch2 = second_sensor
                    ch1_str = f" ch{ch1}" if ch1 > 0 else ""
                    ch2_str = f" ch{ch2}" if ch2 > 0 else ""
                    f.write(f" [Sensors: {note1}{ch1_str} (1st) → {note2}{ch2_str} (2nd)]")
                elif first_sensor:
                    note1, ch1 = first_sensor
                    ch1_str = f" ch{ch1}" if ch1 > 0 else ""
                    f.write(f" [Single sensor: {note1}{ch1_str}]")

                f.write("\n")
            else:
                f.write(f"(no mapping)\n")
    print(f"✓ Saved Markdown to: {md_file}")

    # 3. Generate C code for note_map array
    c_file = "generated_note_map.c"
    generate_c_code(mapping, c_file)
    print(f"✓ Saved C code to: {c_file}")


def generate_c_code(mapping: Dict, output_file: str):
    """Generate C code for two note_map arrays (first and second sensors)."""

    # Initialize two 12x12 matrices with NOTE_NONE
    first_sensor_map = [["NOTE_NONE" for _ in range(12)] for _ in range(12)]
    second_sensor_map = [["NOTE_NONE" for _ in range(12)] for _ in range(12)]

    # Fill in the matrices based on sensor order
    for key_name, data in mapping.items():
        expected_note = data['expected_midi_note']
        first_sensor = data.get('first_sensor')
        second_sensor = data.get('second_sensor')

        # Convert expected note to C note name
        note_name = midi_note_to_c_name(expected_note)

        # Map first sensor position (now a tuple of (note, channel))
        if first_sensor is not None:
            note, ch = first_sensor
            pos = note_to_matrix_position(note, ch)
            if pos:
                row, col = pos
                first_sensor_map[row][col] = note_name

        # Map second sensor position (now a tuple of (note, channel))
        if second_sensor is not None:
            note, ch = second_sensor
            pos = note_to_matrix_position(note, ch)
            if pos:
                row, col = pos
                second_sensor_map[row][col] = note_name

    # Generate C code
    with open(output_file, 'w') as f:
        f.write("/*\n")
        f.write(" * Auto-generated note_map arrays for velocity sensing\n")
        f.write(" * Generated by tools/map_keys.py\n")
        f.write(" * \n")
        f.write(" * TWO ARRAYS:\n")
        f.write(" * - first_sensor_map: Matrix positions for first sensor (top sensor)\n")
        f.write(" * - second_sensor_map: Matrix positions for second sensor (bottom sensor)\n")
        f.write(" * \n")
        f.write(" * INSTRUCTIONS:\n")
        f.write(" * 1. Review this mapping\n")
        f.write(" * 2. Copy both arrays into note_map.h\n")
        f.write(" * 3. Comment out #define DEBUG_MAPPING in note_map.h\n")
        f.write(" * 4. Rebuild and flash firmware\n")
        f.write(" */\n\n")

        # First sensor array
        f.write("// First sensor (triggers first when key is pressed)\n")
        f.write("static const uint8_t first_sensor_map[NUM_DRIVE_PINS][NUM_READ_PINS] = {\n")
        f.write("    //           Col:     0         1         2         3         4         5         6         7         8         9         10        11\n")
        f.write("    //           GPIO:   12        13        14        15        16        17        18        19        20        21        22        26\n")

        for row in range(12):
            row_values = ", ".join([f"{first_sensor_map[row][col]:>9}" for col in range(12)])
            f.write(f"    /* Row {row:2d}*/  {{ {row_values} }}")
            if row < 11:
                f.write(",")
            f.write("\n")

        f.write("};\n\n")

        # Second sensor array
        f.write("// Second sensor (triggers after first sensor)\n")
        f.write("static const uint8_t second_sensor_map[NUM_DRIVE_PINS][NUM_READ_PINS] = {\n")
        f.write("    //           Col:     0         1         2         3         4         5         6         7         8         9         10        11\n")
        f.write("    //           GPIO:   12        13        14        15        16        17        18        19        20        21        22        26\n")

        for row in range(12):
            row_values = ", ".join([f"{second_sensor_map[row][col]:>9}" for col in range(12)])
            f.write(f"    /* Row {row:2d}*/  {{ {row_values} }}")
            if row < 11:
                f.write(",")
            f.write("\n")

        f.write("};\n")


def midi_note_to_c_name(note: int) -> str:
    """Convert MIDI note number to C constant name (e.g., 60 -> 'C4')."""
    note_names = ['C', 'Cs', 'D', 'Ds', 'E', 'F', 'Fs', 'G', 'Gs', 'A', 'As', 'B']
    octave = (note // 12) - 1
    note_name = note_names[note % 12]

    if octave == -1:
        return f"{note_name}_1"
    else:
        return f"{note_name}{octave}"


def main():
    print("="*60)
    print("MIDI Key Mapping Tool")
    print("="*60)

    # Select MIDI port
    port = select_midi_port()

    try:
        # Map all keys
        mapping = map_all_keys(port)

        # Save results
        if mapping:
            save_results(mapping)
            print("\n" + "="*60)
            print("MAPPING COMPLETE!")
            print("="*60)
            print("\nNext steps:")
            print("1. Review generated_note_map.c")
            print("2. Copy the array into note_map.h (functional mapping section)")
            print("3. Comment out #define DEBUG_MAPPING in note_map.h")
            print("4. Rebuild and flash firmware")
        else:
            print("\nNo mapping data collected.")

    except KeyboardInterrupt:
        print("\n\nInterrupted by user. Saving partial results...")
        if mapping:
            save_results(mapping)

    finally:
        port.close()
        print("\nMIDI port closed.")


if __name__ == "__main__":
    main()
