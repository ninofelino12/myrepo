#!/usr/bin/env bash
set -e

PORT="${1:-/dev/ttyUSB0}"
BAUD="115200"

echo "==> Opening Arduino Serial Monitor on $PORT at ${BAUD} baud..."
echo "==> Press Ctrl+C to exit."

if command -v arduino-cli &> /dev/null; then
    arduino-cli monitor -p "$PORT" -b "$BAUD"
elif python3 -c "import serial" &> /dev/null; then
    echo "==> Using Python pyserial monitor..."
    python3 -c "
import serial, sys
try:
    ser = serial.Serial('$PORT', $BAUD, timeout=1)
    while True:
        line = ser.readline()
        if line:
            sys.stdout.write(line.decode('utf-8', errors='ignore'))
except KeyboardInterrupt:
    print('\nExiting monitor.')
"
elif command -v picocom &> /dev/null; then
    picocom -b "$BAUD" "$PORT"
elif command -v screen &> /dev/null; then
    screen "$PORT" "$BAUD"
else
    echo "Error: No suitable serial monitor tool found (arduino-cli, python3-serial, picocom, or screen)." >&2
    exit 1
fi
