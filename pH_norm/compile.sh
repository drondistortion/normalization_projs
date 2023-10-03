#!/bin/sh
arduino-cli compile -b esp32:esp32:piranha_esp-32 -e -v --build-property "build.partitions=default" --build-property "build.code_debug=5"
