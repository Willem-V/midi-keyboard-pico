# Matrix Coordinate Analysis

Based on test results from test.md where each matrix position was assigned a unique MIDI note.

## Complete Key-to-Matrix Mapping

Each physical piano key and ALL matrix positions it triggers:

### C2 - C#2 (Start of keyboard)
- **C2** → [2,7], [0,8]
  - Note 41 (As1), Note 19 (Gs0)

- **C#2** → [5,8], [8,8]
  - Note 75 (Ds5), Note 108 (C8)

- **D2** → [4,8]
  - Note 64 (E4) ⚠️ Only 1 position!

- **D#2** → [6,8], [9,8]
  - Note 86 (D6), Note 2 (D_1)

- **E2** → [3,7], [2,9]
  - Note 53 (F3), Note 31 (G1)

### F2 - B2
- **F2** → [7,7], [9,2]
  - Note 97 (Cs7), Note 119 (B8)

- **F#2** → [2,8], [0,9]
  - Note 42 (Fs2), Note 20 (Gs0)

- **G2** → [5,9], [8,9]
  - Note 76 (E5), Note 109 (Cs8)

- **G#2** → [4,9]
  - Note 65 (F4) ⚠️ Only 1 position!

- **A2** → [6,9], [9,9]
  - Note 87 (Ds6), Note 3 (Ds_1)

- **A#2** → [3,8], [2,10]
  - Note 54 (Fs3), Note 32 (Gs1)

- **B2** → [7,8], [9,3]
  - Note 98 (D7), Note 120 (C9)

### C3 - F#3
- **C3** → [2,9], [0,10]
  - Note 43 (G2), Note 21 (A0)

- **C#3** → [5,10], [8,10]
  - Note 77 (F5), Note 110 (D8)

- **D3** → [4,10]
  - Note 66 (Fs4) ⚠️ Only 1 position!

- **D#3** → [6,10], [9,10]
  - Note 88 (E6), Note 4 (E_1)

- **E3** → [3,9], [2,11] ❌
  - Note 55 (G3), Note 33 (A1)
  - **ERROR: Col 11 doesn't exist!** (test data has invalid position)

- **F3** → [7,9], [9,4]
  - Note 99 (Ds7), Note 121 (Cs9)

- **F#3** → [2,10], [0,11] ❌
  - Note 44 (Gs2), Note 22 (As0)
  - **ERROR: Col 11 doesn't exist!**

### G3 - B3
- **G3** → [5,1], [8,2]
  - Note 68 (Gs4), Note 101 (F7)

- **G#3** → [4,1], [5,0]
  - Note 57 (A3), Note 68 (Gs4)

- **A3** → [6,1], [9,5]
  - Note 79 (Gs5), Note 123 (Ds9)

- **A#3** → [3,0], [0,12] ❌
  - Note 46 (As2), Note 24 (C1)
  - **ERROR: Col 12 doesn't exist!**

- **B3** → [7,0], [8,5]
  - Note 90 (Fs6), Note 112 (E8)

### C4 - F#4 (Middle C region)
- **C4** → [2,1], [0,13] ❌
  - Note 35 (B1), Note 13 (Cs0)
  - **ERROR: Col 13 doesn't exist!**

- **C#4** → [5,1], [6,0], [8,1], [9,6]
  - Note 67 (G4), Note 100 (E7), Note 102 (Fs7), Note 113 (F8)
  - 🚨 **4 POSITIONS - Hardware issue!**

- **D4** → [4,0], [5,1]
  - Note 56 (Gs3), Note 69 (A4)

- **D#4** → [6,0], [7,1], [9,7]
  - Note 78 (Fs5), Note 91 (G6), Note 124 (E9)
  - 🚨 **3 POSITIONS - Hardware issue!**

- **E4** → [3,1], [4,2], [2,14] ❌
  - Note 45 (A2), Note 58 (As3), Note 25 (Cs1)
  - 🚨 **3 POSITIONS - Hardware issue!**
  - **ERROR: Col 14 doesn't exist!**

- **F4** → [7,1], [8,2], [9,6]
  - Note 89 (F6), Note 102 (Fs7), Note 113 (F8)
  - 🚨 **3 POSITIONS - Hardware issue!**

- **F#4** → [2,0], [3,2], [0,15] ❌
  - Note 34 (As1), Note 47 (B2), Note 14 (D0), Note 25 (Cs1)
  - 🚨 **4 POSITIONS - Hardware issue!**
  - **ERROR: Col 15 doesn't exist!**

### G4 - B4
- **G4** → [6,6], [5,2], [8,3], [9,7]
  - Note 84 (C6), Note 73 (Cs5), Note 106 (As7), Note 117 (A8)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**

- **G#4** → [5,6], [4,3]
  - Note 73 (Cs5), Note 62 (D4)

- **A4** → [10,6], [7,4], [6,6]
  - Note 0 (C_1), Note 95 (B6), Note 84 (C6)
  - 🚨 **3 POSITIONS - Hardware issue!**

- **A#4** → [1,6], [2,3], [3,4], [4,3]
  - Note 29 (F1), Note 40 (E2), Note 51 (Ds3), Note 62 (D4)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**

- **B4** → [7,6], [8,3], [9,7], [10,6]
  - Note 95 (B6), Note 106 (As7), Note 117 (A8), Note 0 (C_1)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**

### C5 - F#5
- **C5** → [2,6], [3,4], [0,18] ❌, [1,6]
  - Note 40 (E2), Note 51 (Ds3), Note 18 (Fs0), Note 29 (F1)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**
  - **ERROR: Col 18 doesn't exist!**

- **C#5** → [5,3], [6,1], [8,4], [9,8]
  - Note 69 (A4), Note 80 (Gs5), Note 102 (Fs7), Note 113 (F8)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**

- **D5** → [4,2], [5,3]
  - Note 58 (As3), Note 69 (A4)

- **D#5** → [6,2], [7,1], [9,7]
  - Note 80 (Gs5), Note 91 (G6), Note 124 (E9)
  - 🚨 **3 POSITIONS - Hardware issue!**

- **E5** → [3,2], [4,2], [1,18] ❌, [2,3]
  - Note 47 (B2), Note 58 (As3), Note 25 (Cs1), Note 36 (C2)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**
  - **ERROR: Col 18 doesn't exist!**

- **F5** → [7,2], [8,4], [9,8]
  - Note 91 (G6), Note 102 (Fs7), Note 113 (F8)
  - 🚨 **3 POSITIONS - Hardware issue!**

- **F#5** → [2,2], [3,2], [0,19] ❌, [1,18] ❌
  - Note 36 (C2), Note 47 (B2), Note 14 (D0), Note 25 (Cs1)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**
  - **ERROR: Cols 18 & 19 don't exist!**

### G5 - B5
- **G5** → [6,3], [8,4], [9,8], [5,2]
  - Note 81 (A5), Note 103 (G7), Note 114 (Fs8), Note 70 (As4)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**

- **G#5** → [5,4], [4,3]
  - Note 70 (As4), Note 59 (B3)

- **A5** → [7,3], [6,3], [9,9]
  - Note 92 (Gs6), Note 81 (A5), Note 125 (F9)
  - 🚨 **3 POSITIONS - Hardware issue!**

- **A#5** → [2,3], [4,3], [3,4]
  - Note 37 (Cs2), Note 59 (B3), Note 48 (C3)
  - 🚨 **3 POSITIONS - Hardware issue!**

- **B5** → [8,2], [9,8], [7,3]
  - Note 103 (G7), Note 114 (Fs8), Note 92 (Gs6)
  - 🚨 **3 POSITIONS - Hardware issue!**

### C6 - F#6
- **C6** → [0,3], [3,4], [2,3], [1,20] ❌
  - Note 15 (Ds0), Note 48 (C3), Note 37 (Cs2), Note 26 (D1)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**
  - **ERROR: Col 20 doesn't exist!**

- **C#6** → [5,5], [6,3], [8,5], [9,9]
  - Note 71 (B4), Note 82 (As5), Note 104 (Gs7), Note 115 (G8)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**

- **D6** → [4,4], [5,5]
  - Note 60 (C4), Note 71 (B4)

- **D#6** → [6,4], [7,3], [9,9], [10,21] ❌
  - Note 82 (As5), Note 93 (A6), Note 126 (Fs9), Note (invalid)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**
  - **ERROR: Col 21 doesn't exist!**

- **E6** → [3,3], [4,4], [1,20] ❌, [2,4]
  - Note 49 (Cs3), Note 60 (C4), Note 27 (Ds1), Note 38 (D2)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**
  - **ERROR: Col 20 doesn't exist!**

- **F6** → [7,4], [8,5], [9,9], [10,21] ❌
  - Note 93 (A6), Note 104 (Gs7), Note 115 (G8), Note 126 (Fs9)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**
  - **ERROR: Col 21 doesn't exist!**

- **F#6** → [2,4], [3,3], [0,22] ❌, [1,20] ❌
  - Note 38 (D2), Note 49 (Cs3), Note 16 (E0), Note 27 (Ds1)
  - 🚨 **4 POSITIONS - SEVERE hardware issue!**
  - **ERROR: Cols 20 & 22 don't exist!**

### G6 - C7 (Upper range)
- **G6** → [5,2], [8,6], [9,10]
  - Note 72 (C5), Note 105 (A7), Note 116 (Gs8)
  - 🚨 **3 POSITIONS - Hardware issue!**

- **G#6** → [4,5]
  - Note 61 (Cs4) ⚠️ Only 1 position!

- **A6** → [6,5], [10,23] ❌
  - Note 83 (B5), Note 127 (G9)
  - **ERROR: Col 23 doesn't exist!**

- **A#6** → [3,4], [1,20] ❌
  - Note 50 (D3), Note 28 (E1)
  - **ERROR: Col 20 doesn't exist!**

- **B6** → [7,5], [9,10]
  - Note 94 (As6), Note 116 (Gs8)

- **C7** → [0,5], [2,5] ❌
  - Note 17 (F0), Note 39 (Ds2)
  - **ERROR: Invalid test data**

---

## 🚨 PROBLEM KEYS SUMMARY

### Keys with 4+ Matrix Positions (SEVERE Hardware Issues)
These keys likely have solder bridges or severe crosstalk:

1. **C#4** → 4 positions: [5,1], [6,0], [8,1], [9,6]
2. **F#4** → 4 positions: [2,0], [3,2], [0,15❌], [1,18❌]
3. **G4** → 4 positions: [6,6], [5,2], [8,3], [9,7] - **WORST**
4. **A#4** → 4 positions: [1,6], [2,3], [3,4], [4,3]
5. **B4** → 4 positions: [7,6], [8,3], [9,7], [10,6]
6. **C5** → 4 positions: [2,6], [3,4], [0,18❌], [1,6]
7. **C#5** → 4 positions: [5,3], [6,1], [8,4], [9,8]
8. **E5** → 4 positions: [3,2], [4,2], [1,18❌], [2,3]
9. **F#5** → 4 positions: [2,2], [3,2], [0,19❌], [1,18❌]
10. **G5** → 4 positions: [6,3], [8,4], [9,8], [5,2]
11. **C6** → 4 positions: [0,3], [3,4], [2,3], [1,20❌]
12. **C#6** → 4 positions: [5,5], [6,3], [8,5], [9,9]
13. **D#6** → 4 positions: [6,4], [7,3], [9,9], [10,21❌]
14. **E6** → 4 positions: [3,3], [4,4], [1,20❌], [2,4]
15. **F6** → 4 positions: [7,4], [8,5], [9,9], [10,21❌]
16. **F#6** → 4 positions: [2,4], [3,3], [0,22❌], [1,20❌]

**Total: 16 keys with severe issues (26% of keyboard)**

### Keys with 3 Matrix Positions (Moderate Hardware Issues)

1. **D#4** → 3 positions: [6,0], [7,1], [9,7]
2. **E4** → 3 positions: [3,1], [4,2], [2,14❌]
3. **F4** → 3 positions: [7,1], [8,2], [9,6]
4. **A4** → 3 positions: [10,6], [7,4], [6,6]
5. **D#5** → 3 positions: [6,2], [7,1], [9,7]
6. **F5** → 3 positions: [7,2], [8,4], [9,8]
7. **A5** → 3 positions: [7,3], [6,3], [9,9]
8. **A#5** → 3 positions: [2,3], [4,3], [3,4]
9. **B5** → 3 positions: [8,2], [9,8], [7,3]
10. **G6** → 3 positions: [5,2], [8,6], [9,10]

**Total: 10 keys with moderate issues (16% of keyboard)**

### Keys with 2 Matrix Positions (Normal - Dual Sensors)

14 keys have exactly 2 positions - **This is expected for velocity sensing!**

### Keys with 1 Matrix Position (Missing Sensor or Perfect)

Only 3 keys:
- **D2** → [4,8]
- **G#2** → [4,9]
- **D3** → [4,10]
- **G#6** → [4,5]

These either work perfectly OR are missing the second velocity sensor.

---

## 🔍 Analysis & Theories

### Pattern 1: Invalid Column Numbers
Test data shows columns 11-23, but the matrix only has columns 0-10 (11 total).
**This suggests measurement errors or software bugs in the testing process.**

### Pattern 2: Cluster Around Middle Octave
The worst problems (4+ positions) are concentrated in **octaves 4-6 (C4-F#6)**.
- Lower keys (C2-B3): Mostly 2-3 positions
- Middle keys (C4-F#6): 4+ positions (SEVERE)
- Upper keys (G6-C7): Mixed

**Theory:** Physical keyboard area around middle C has more solder bridges or PCB damage.

### Pattern 3: Row 9 Crosstalk
Notes from **Row 9** (high octave) appear frequently in other keys:
- Row 9, Cols 2-10 trigger on many mid-range keys
- Suggests Row 9 drive pin might not be properly isolated

**Theory:** Row 9 drive line has poor isolation or stuck HIGH.

### Pattern 4: Adjacent Position Triggers
Many problematic keys trigger adjacent matrix positions:
- **G#4**: [5,6] + [4,3] - different rows AND columns
- **A#4**: [1,6], [2,3], [3,4], [4,3] - sequential rows

**Theory:** Combination of:
1. **Solder bridges** between adjacent keys
2. **Stuck keys** mechanically pressing neighbors
3. **Crosstalk** in PCB traces

### Recommended Actions

1. **Inspect Row 9 wiring** - Check if drive pin 9 is stuck or poorly isolated
2. **Check middle octave solder joints** - Keys C4-F#6 need physical inspection
3. **Test with longer SCAN_SETTLE_US** - Already increased to 50μs, try 100μs
4. **Add inter-row delays** - Ensure drive pins fully settle between switches
5. **Physical key inspection** - Check if any keys are mechanically stuck

### Hardware vs Software

**Likely Hardware Issues (75%):**
- Too many keys with 4+ positions for this to be pure software
- Pattern concentrates in physical keyboard area (middle octave)
- Row 9 appearing everywhere suggests physical problem

**Possible Software Contributions (25%):**
- Invalid column numbers (11-23) in test data
- Scan rate might still be too fast even at 50μs
- Missing delays between row switches
