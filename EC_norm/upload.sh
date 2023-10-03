#!/bin/sh
PORT=/dev/ttyUSB0
if [[ $# -gt 0 ]]
then PORT=$1
fi
arduino-cli upload -b esp32:esp32:piranha_esp-32 -v -p $PORT
