#! /usr/bin/env bash
sudo arduino-cli compile -v -p /dev/ttyACM0 -b arduino:avr:micro .
sudo arduino-cli upload -v -p /dev/ttyACM0 -b arduino:avr:micro .
