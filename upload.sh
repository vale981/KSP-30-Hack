#! /usr/bin/env bash
sudo arduino-cli compile -v -p /dev/ttyUSB0 -b arduino:avr:nano:cpu=atmega328old keyboard
sudo arduino-cli upload -v -p /dev/ttyUSB0 -b arduino:avr:nano:cpu=atmega328old keyboard
