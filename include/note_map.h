/*
 * Note Mapping for MIDI Keyboard Controller
 *
 * Maps physical key matrix positions (row, col) to MIDI note numbers.
 *
 * Matrix: 11 drive pins (rows) × 11 read pins (columns) = 121 positions
 * MIDI Notes: 0-127 (21-108 for 88-key piano range: A0 to C8)
 *
 * Special value: 0xFF = No key at this position (unmapped/unused)
 */

#ifndef NOTE_MAP_H
#define NOTE_MAP_H

#include <stdint.h>

// Total matrix dimensions
#define NUM_DRIVE_PINS  11
#define NUM_READ_PINS   11

// Special value for unmapped keys
#define NOTE_NONE       0xFF

/*
 * Note mapping array: note_map[drive_pin][read_pin] = MIDI_note
 *
 * Current mapping: Chromatic layout starting at C2 (MIDI note 36)
 * This gives a range of C2 to C12 (notes 36-156), though most software
 * only supports up to G9 (127).
 *
 * Customize this array to match your physical keyboard layout.
 * For example, you might want:
 * - Piano layout (white keys + black keys)
 * - Chromatic layout (every semitone)
 * - Custom button mapping
 */

static const uint8_t note_map[NUM_DRIVE_PINS][NUM_READ_PINS] = {
    // Drive 0 (Row 0): Notes 36-46 (C2 to Bb2)
    { 36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46},

    // Drive 1 (Row 1): Notes 47-57 (B2 to A3)
    { 47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57},

    // Drive 2 (Row 2): Notes 58-68 (Bb3 to Ab4)
    { 58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68},

    // Drive 3 (Row 3): Notes 69-79 (A4 to G5)
    { 69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79},

    // Drive 4 (Row 4): Notes 80-90 (Ab5 to Gb6)
    { 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90},

    // Drive 5 (Row 5): Notes 91-101 (G6 to F7)
    { 91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101},

    // Drive 6 (Row 6): Notes 102-112 (Gb7 to E8)
    {102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112},

    // Drive 7 (Row 7): Notes 113-123 (F8 to Eb9)
    {113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123},

    // Drive 8 (Row 8): Notes 124-127 + unmapped
    {124, 125, 126, 127, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE},

    // Drive 9 (Row 9): All unmapped (for future expansion)
    {NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE},

    // Drive 10 (Row 10): All unmapped (for future expansion)
    {NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE}
};

/*
 * Helper function to get MIDI note from matrix position
 */
static inline uint8_t get_note(uint8_t drive, uint8_t read) {
    if (drive >= NUM_DRIVE_PINS || read >= NUM_READ_PINS) {
        return NOTE_NONE;
    }
    return note_map[drive][read];
}

/*
 * MIDI Note Reference:
 *
 * Octave  C   C#  D   D#  E   F   F#  G   G#  A   A#  B
 * -1      0   1   2   3   4   5   6   7   8   9   10  11
 *  0     12  13  14  15  16  17  18  19  20  21  22  23
 *  1     24  25  26  27  28  29  30  31  32  33  34  35
 *  2     36  37  38  39  40  41  42  43  44  45  46  47  <- Start of mapping
 *  3     48  49  50  51  52  53  54  55  56  57  58  59
 *  4     60  61  62  63  64  65  66  67  68  69  70  71  <- Middle C = 60
 *  5     72  73  74  75  76  77  78  79  80  81  82  83
 *  6     84  85  86  87  88  89  90  91  92  93  94  95
 *  7     96  97  98  99 100 101 102 103 104 105 106 107
 *  8    108 109 110 111 112 113 114 115 116 117 118 119  <- C8 = 108 (top of 88-key piano)
 *  9    120 121 122 123 124 125 126 127                  <- 127 = G9 (MIDI max)
 *
 * Common ranges:
 * - 88-key piano: A0 (21) to C8 (108)
 * - 76-key keyboard: E1 (28) to G7 (103)
 * - 61-key keyboard: C2 (36) to C7 (96)
 * - 49-key keyboard: C2 (36) to C6 (84)
 *
 * Special notes:
 * - Middle C (C4): MIDI note 60
 * - A440 (concert pitch): MIDI note 69
 */

#endif // NOTE_MAP_H
