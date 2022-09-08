#pragma once

#define DEVICE_ID 0x05
// #define OVERRIDE_EEPROM

#define MIDI_INTERFACE_ID 0x00

#define I2C_CLOCK_SPEED 400000
#define DEBOUNCE_TIME_MICROS 30000

#define USE_BUS_1
// #define USE_BUS_2

#define ANALOG_IN_1_PIN A0
#define ANALOG_IN_1_CHANNEL 0
#define ANALOG_IN_1_DIFFERENCE_THRESHOLD 15

#define ANALOG_IN_2_PIN A1
#define ANALOG_IN_2_CHANNEL 1
#define ANALOG_IN_2_DIFFERENCE_THRESHOLD 15

#define DISCRETE_IN_1_PIN A2
#define DISCRETE_IN_1_BIT_INDEX 64
#define DISCRETE_IN_1_INVERT false
#define DISCRETE_IN_1_PULLUP

// #define DEBUGGING
// #define DEBUGGING_VERBOSE
