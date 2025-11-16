#!/bin/bash
# Monitor serial debug output from MIDI keyboard

# Find the serial port
PORT=""
if ls /dev/ttyACM* &>/dev/null; then
    PORT=$(ls /dev/ttyACM* | head -1)
elif ls /dev/cu.usbmodem* &>/dev/null; then
    PORT=$(ls /dev/cu.usbmodem* | head -1)
fi

if [ -z "$PORT" ]; then
    echo "Error: No serial port found!"
    echo
    echo "Make sure:"
    echo "  1. Pico is plugged in"
    echo "  2. Firmware is flashed (midi_keyboard.uf2)"
    echo "  3. USB cable supports data (not just power)"
    echo
    echo "Try: ./find_device.sh"
    exit 1
fi

echo "Connecting to $PORT..."
echo "Press Ctrl-A then K to exit"
echo

# Check if screen is available
if command -v screen &>/dev/null; then
    screen $PORT 115200
elif command -v minicom &>/dev/null; then
    minicom -D $PORT -b 115200
elif command -v python3 &>/dev/null && python3 -c "import serial" &>/dev/null; then
    python3 -m serial.tools.miniterm $PORT 115200
else
    echo "Error: No serial terminal found!"
    echo
    echo "Install one of:"
    echo "  - screen:   sudo apt install screen"
    echo "  - minicom:  sudo apt install minicom"
    echo "  - pyserial: pip install pyserial"
    exit 1
fi
