#include <Arduino.h>
#include <Wire.h>
#include <MCP23017.h>
#include <Bus.h>
#include <SPI.h>
#include <mcp2515.h>
#include <EEPROM.h>
#include <can-messages.h>

#define EEPROM_CHECK_VALUE 0x21

Bus bus {
    {MCP23017(0x20), MCP23017(0x21), MCP23017(0x22), MCP23017(0x23)},
    4,
    {0}, {0}, {0}
};

uint8_t bytes_read[8] = {0};

BitChanged bits_changed[64];

MCP2515 mcp2515(10);

// device address will be read from EEPROM, but by default
// it's the configured value (for initialization purposes)
// the reason it's in EEPROM is it allows for it to be readdressed
// programmatically if necessary
uint8_t device_address = DEVICE_ID;

void print_byte_be (uint8_t to_print) {
    for (int i = 0; i < 8; i++) {
        Serial.write(bitRead(to_print, i) ? '1' : '0');
    }
}

void write_to_EEPROM() {
    // EEPROM values:
    // 0: check value (see if it's been initialized)
    // 1: device address
    EEPROM.write(0, EEPROM_CHECK_VALUE);
    EEPROM.write(1, device_address);
}

void setup() {
#ifndef OVERRIDE_EEPROM
    // check if EEPROM has been initialized
    if (EEPROM.read(0) == EEPROM_CHECK_VALUE) {
        // it has!

    } else {
        // else initialize it
        write_to_EEPROM();
    }
#else
    // if we're overriding EEPROM, write the new values to it
    write_to_EEPROM();
#endif

    Serial.begin(9600);
    Serial.println("Serial debugging initialized");

    for (int i = 0; i < sizeof(bits_changed) / sizeof(BitChanged); i++) {
        bits_changed[i] = BitChanged { 0, 0 };
    }

    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp2515.setNormalMode();

    Wire.begin();
    Wire.setClock(I2C_CLOCK_SPEED);

    bus_init_as_input(bus);
}

void loop() {
    bus_read(bus);

#ifdef DEBUGGING_VERBOSE
    for (int i = 0; i < sizeof(bus.current); i++) {
        print_byte_be(bus.current[i]);
    }
    Serial.write('\n');
#endif

    uint64_t now = micros();
    bus_debounce(bus, DEBOUNCE_TIME_MICROS, now);

    int num_changed = bus_get_changes(bus, bits_changed);

    for (int i = 0; i < num_changed; i++) {
#ifdef DEBUGGING
        Serial.print("Changed: ");
        Serial.print(bits_changed[i].bit_index);
        Serial.print("  ");
        Serial.println(bits_changed[i].new_state);
#endif

        can_frame frame;
        if (bits_changed[i].new_state == true) {
            frame = note_on(device_address, bits_changed[i].bit_index);
        } else {
            frame = note_off(device_address, bits_changed[i].bit_index);
        }

        mcp2515.sendMessage(&frame);
    }

    bus_make_current_last(bus);
}