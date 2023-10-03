#!/bin/sh
PORT=/dev/ttyUSB0
if [[ $# -gt 0 ]]
then PORT=$1
fi
echo $PORT
arduino-cli monitor -p $PORT -c 115200,8,1
