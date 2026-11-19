#!/bin/bash

# Configuration
SKETCH="esp8266.ino"
ESP8266_FQBN="esp8266:esp8266:nodemcuv2"
ESP32_FQBN="esp32:esp32:esp32"
DEFAULT_PORT="/dev/ttyUSB0"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

usage() {
    echo "Usage: $0 [esp8266|esp32] [port]"
    echo "Default port is $DEFAULT_PORT"
    exit 1
}

if [ "$1" == "" ]; then
    usage
fi

BOARD_TYPE=$1
PORT=${2:-$DEFAULT_PORT}

if [ "$BOARD_TYPE" == "esp8266" ]; then
    FQBN=$ESP8266_FQBN
elif [ "$BOARD_TYPE" == "esp32" ]; then
    FQBN=$ESP32_FQBN
else
    echo -e "${RED}Error: Invalid board type. Use 'esp8266' or 'esp32'.${NC}"
    usage
fi

echo -e "${GREEN}Selected Board: $BOARD_TYPE ($FQBN)${NC}"
echo -e "${GREEN}Selected Port:  $PORT${NC}"

# Compile
echo "Compiling $SKETCH..."
arduino-cli compile --fqbn $FQBN $SKETCH
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi

# Upload
echo "Uploading to $PORT..."
arduino-cli upload -p $PORT --fqbn $FQBN $SKETCH
if [ $? -ne 0 ]; then
    echo -e "${RED}Upload failed! Check connection and permissions.${NC}"
    exit 1
fi

echo -e "${GREEN}Success!${NC}"
