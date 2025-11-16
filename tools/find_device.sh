#!/bin/bash
# Find MIDI Keyboard Controller USB devices

echo "=== MIDI Keyboard Controller Finder ==="
echo

# Check for USB device
echo "1. USB Device:"
if lsusb &>/dev/null; then
    USB_DEV=$(lsusb | grep -i "2e8a")
    if [ -n "$USB_DEV" ]; then
        echo "   ✓ Found: $USB_DEV"
    else
        echo "   ✗ Not found (unplug/replug and try again)"
    fi
else
    echo "   (lsusb not available)"
fi
echo

# Check for serial port
echo "2. Serial Debug Port:"
if ls /dev/ttyACM* &>/dev/null; then
    for port in /dev/ttyACM*; do
        echo "   ✓ Found: $port"
        echo "     Connect: screen $port 115200"
    done
elif ls /dev/cu.usbmodem* &>/dev/null; then
    for port in /dev/cu.usbmodem*; do
        echo "   ✓ Found: $port"
        echo "     Connect: screen $port 115200"
    done
else
    echo "   ✗ No serial port found"
fi
echo

# Check for MIDI device
echo "3. MIDI Interface:"
if command -v aconnect &>/dev/null; then
    MIDI_DEV=$(aconnect -l | grep -i "MIDI Keyboard")
    if [ -n "$MIDI_DEV" ]; then
        echo "   ✓ Found:"
        echo "$MIDI_DEV" | sed 's/^/     /'
    else
        echo "   ✗ Not found in ALSA"
    fi
elif command -v aseqdump &>/dev/null; then
    MIDI_DEV=$(aseqdump -l | grep -i "MIDI Keyboard")
    if [ -n "$MIDI_DEV" ]; then
        echo "   ✓ Found: $MIDI_DEV"
    else
        echo "   ✗ Not found"
    fi
else
    echo "   (ALSA tools not available)"
fi
echo

# Show dmesg output
echo "4. Recent USB kernel messages:"
if [ -r /var/log/kern.log ]; then
    tail -50 /var/log/kern.log | grep -i "usb.*2e8a\|ttyACM\|MIDI" | tail -5 | sed 's/^/   /'
elif command -v dmesg &>/dev/null; then
    dmesg | grep -i "usb.*2e8a\|ttyACM\|MIDI" | tail -5 | sed 's/^/   /'
else
    echo "   (Need sudo for dmesg)"
fi
echo

echo "=== Quick Actions ==="
echo "Monitor serial:  ./monitor_serial.sh"
echo "Monitor MIDI:    aseqdump -p 'MIDI Keyboard'"
echo "Watch USB:       sudo dmesg -w"
