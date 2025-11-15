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

// MIDI Note Definitions (Note Name = MIDI Number)
// Octave -1
#define C_1   0
#define Cs_1  1
#define D_1   2
#define Ds_1  3
#define E_1   4
#define F_1   5
#define Fs_1  6
#define G_1   7
#define Gs_1  8
#define A_1   9
#define As_1  10
#define B_1   11

// Octave 0
#define C0   12
#define Cs0  13
#define D0   14
#define Ds0  15
#define E0   16
#define F0   17
#define Fs0  18
#define G0   19
#define Gs0  20
#define A0   21
#define As0  22
#define B0   23

// Octave 1
#define C1   24
#define Cs1  25
#define D1   26
#define Ds1  27
#define E1   28
#define F1   29
#define Fs1  30
#define G1   31
#define Gs1  32
#define A1   33
#define As1  34
#define B1   35

// Octave 2
#define C2   36
#define Cs2  37
#define D2   38
#define Ds2  39
#define E2   40
#define F2   41
#define Fs2  42
#define G2   43
#define Gs2  44
#define A2   45
#define As2  46
#define B2   47

// Octave 3
#define C3   48
#define Cs3  49
#define D3   50
#define Ds3  51
#define E3   52
#define F3   53
#define Fs3  54
#define G3   55
#define Gs3  56
#define A3   57
#define As3  58
#define B3   59

// Octave 4 (Middle C = C4 = 60)
#define C4   60
#define Cs4  61
#define D4   62
#define Ds4  63
#define E4   64
#define F4   65
#define Fs4  66
#define G4   67
#define Gs4  68
#define A4   69  // A440 concert pitch
#define As4  70
#define B4   71

// Octave 5
#define C5   72
#define Cs5  73
#define D5   74
#define Ds5  75
#define E5   76
#define F5   77
#define Fs5  78
#define G5   79
#define Gs5  80
#define A5   81
#define As5  82
#define B5   83

// Octave 6
#define C6   84
#define Cs6  85
#define D6   86
#define Ds6  87
#define E6   88
#define F6   89
#define Fs6  90
#define G6   91
#define Gs6  92
#define A6   93
#define As6  94
#define B6   95

// Octave 7
#define C7   96
#define Cs7  97
#define D7   98
#define Ds7  99
#define E7   100
#define F7   101
#define Fs7  102
#define G7   103
#define Gs7  104
#define A7   105
#define As7  106
#define B7   107

// Octave 8
#define C8   108
#define Cs8  109
#define D8   110
#define Ds8  111
#define E8   112
#define F8   113
#define Fs8  114
#define G8   115
#define Gs8  116
#define A8   117
#define As8  118
#define B8   119

// Octave 9
#define C9   120
#define Cs9  121
#define D9   122
#define Ds9  123
#define E9   124
#define F9   125
#define Fs9  126
#define G9   127  // Highest MIDI note

/*
 * Note mapping array: note_map[drive_pin][read_pin] = MIDI_note
 *
 * EXPERIMENTAL MAPPING:
 * Each matrix position gets a unique note for testing/discovery.
 * Press each physical key and note which MIDI note it produces.
 * This helps you map the physical keyboard layout to matrix positions.
 *
 * Matrix: 11 rows × 11 columns = 121 positions
 * Notes assigned: 21 (A0) through 127 (G9) sequentially
 *
 * To find your physical key mapping:
 * 1. Flash this firmware
 * 2. Connect to MIDI monitor software
 * 3. Press each key and record which note it produces
 * 4. Update this array based on your findings
 */

// FUNCTIONAL MAPPING - Based on experimental test results
// Each matrix position maps to the piano key it physically activates
// Unmapped positions use NOTE_NONE

static const uint8_t note_map[NUM_DRIVE_PINS][NUM_READ_PINS] = {
    //           Col:     0         1         2         3         4         5         6         7         8         9         10
    /* Row 0 */  { NOTE_NONE, NOTE_NONE, NOTE_NONE,      C6,  NOTE_NONE,      C7,  NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE },
    /* Row 1 */  { NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE,     As4,  NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE },
    /* Row 2 */  {     Fs4,      C4,     F5,     As5,     Fs6,  NOTE_NONE,      C5,      C2,     Fs2,      C3,     Fs3  },
    /* Row 3 */  {     As3,      E4,      E5,      E6,     As6,  NOTE_NONE, NOTE_NONE,      E2,     As2,      E3,  NOTE_NONE },
    /* Row 4 */  {      D4,     Gs3,      D5,  NOTE_NONE,      D6,     Gs6,  NOTE_NONE, NOTE_NONE,       D2,     Gs2,      D3  },
    /* Row 5 */  {      G3,     Cs4,      G6,     Cs5,     Gs5,     Cs6,     Gs4,  NOTE_NONE,     Cs2,      G2,     Cs3  },
    /* Row 6 */  {     Ds4,      A3,     Ds5,      G5,     Ds6,      A6,      G4,  NOTE_NONE,     Ds2,      A2,     Ds3  },
    /* Row 7 */  {      B3,      F4,      F5,      A5,      F6,      B6,      B4,      F2,      B2,      F3,  NOTE_NONE },
    /* Row 8 */  { NOTE_NONE, NOTE_NONE,      B5,  NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE },
    /* Row 9 */  { NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE },
    /* Row 10*/  { NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE,      A4,  NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE }
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
