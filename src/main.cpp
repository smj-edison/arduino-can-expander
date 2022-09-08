#include <Arduino.h>
#include <Wire.h>
#include <MCP23017.h>
#include <Bus.h>
#include <SPI.h>
#include <mcp2515.h>
#include <EEPROM.h>
#include <can-messages.h>
#include <can-protocol.h>
#include <AnalogIn.h>
#include <DiscreteIn.h>

// Here's the main bus definition
#ifdef USE_BUS_1
Bus bus1 {
    {MCP23017(0x20), MCP23017(0x21), MCP23017(0x22), MCP23017(0x23)},
    4,
    {0}, {0}, {0}
};
#endif

#ifdef USE_BUS_2
Bus bus2 {
    {MCP23017(0x24), MCP23017(0x25), MCP23017(0x26), MCP23017(0x27)},
    4,
    {0}, {0}, {0}
};
#endif

#ifdef ANALOG_IN_1_PIN
AnalogIn analog_in_1 {
    ANALOG_IN_1_PIN, ANALOG_IN_1_CHANNEL, false, 0, ANALOG_IN_1_DIFFERENCE_THRESHOLD,
    0
};
#endif

#ifdef ANALOG_IN_2_PIN
AnalogIn analog_in_2 {
    ANALOG_IN_2_PIN, ANALOG_IN_2_CHANNEL, false, 0, ANALOG_IN_2_DIFFERENCE_THRESHOLD,
    0
};
#endif

#ifdef DISCRETE_IN_1_PIN
DiscreteIn discrete_in_1 {
    DISCRETE_IN_1_PIN, DISCRETE_IN_1_BIT_INDEX, false, false, false
};
#endif

// This is used to capture the results from `bus_get_changes`
BitChanged bits_changed[64];

// CAN interface
MCP2515 mcp2515(10);
struct can_frame frame_read;
volatile bool interrupt = false;

void irqHandler() {
    interrupt = true;
}

void sendMessage(can_frame frame) {
    MCP2515::ERROR status = mcp2515.sendMessage(&frame);

#ifdef DEBUGGING
    Serial.print("Received send status code: ");
    Serial.println(status);
#endif
}

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

#define EEPROM_CHECK_VALUE 0x21
void write_to_EEPROM() {
    // EEPROM values:
    // 0x00: check value (see if it's been initialized)
    // 0x01: device address
    EEPROM.write(0, EEPROM_CHECK_VALUE);
    EEPROM.write(1, device_address);
}

void config_MCP2515_filters(bool enable_config_mode = true) {
    if (enable_config_mode) {
        mcp2515.setConfigMode();
    }

    // my CAN network implementation is pretty simple:
    // LSB bits 1-5 are the "to" address
    // LSB bits 6-10 are the "from" address
    // LSB bit 11 is a flag, if true it is a broadcast to everyone

    // we only care about bits 1-5, as that's the "to" address
    // if it's not to us, we should disregard it
    mcp2515.setFilterMask(MCP2515::MASK0, false, 0b00000011111);
    mcp2515.setFilter(MCP2515::RXF0, false, device_address);

    // however, if bit 11 is set, we should look at the message
    mcp2515.setFilterMask(MCP2515::MASK1, false, 0b10000000000);
    mcp2515.setFilter(MCP2515::RXF2, false,      0b10000000000);
    mcp2515.setFilter(MCP2515::RXF3, false,      0b10000000000);
    mcp2515.setFilter(MCP2515::RXF4, false,      0b10000000000);
    mcp2515.setFilter(MCP2515::RXF5, false,      0b10000000000);

    if (enable_config_mode) {
        mcp2515.setNormalMode();
    }
}

void setup() {
#ifndef OVERRIDE_EEPROM
    // check if EEPROM has been initialized
    if (EEPROM.read(0) == EEPROM_CHECK_VALUE) {
        // it has!
        device_address = EEPROM.read(1);
    } else {
        // else initialize it
        write_to_EEPROM();
    }
#else
    // if we're overriding EEPROM, write the new values to it
    write_to_EEPROM();
#endif

#ifdef DEBUGGING
    Serial.begin(9600);
    Serial.println("Serial debugging initialized");
    Serial.print("Current address: ");
    Serial.println(device_address, HEX);
#endif

    // init bits_changed
    for (uint16_t i = 0; i < sizeof(bits_changed) / sizeof(BitChanged); i++) {
        bits_changed[i] = BitChanged { 0, 0 };
    }

    // set up CAN interface
    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
    config_MCP2515_filters();
    attachInterrupt(0, irqHandler, FALLING);

    // set up I2C
    Wire.begin();
    Wire.setClock(I2C_CLOCK_SPEED);

    // init the MCP23017 chips as inputs
#ifdef USE_BUS_1
    bus_init_as_input(bus1);
#endif
#ifdef USE_BUS_2
    bus_init_as_input(bus2);
#endif
}

void loop() {
    // CHECK FOR INCOMING CAN //
    if (interrupt) {
        interrupt = false;

        while (mcp2515.readMessage(&frame_read) == MCP2515::ERROR_OK) {
            switch (frame_read.data[0]) {
                case ADDRESS_REASSIGN:
                    device_address = frame_read.data[1] & 0x1F;
                    write_to_EEPROM();
                    config_MCP2515_filters();
                break;
            }
        }
    }

    // READ BUSES //
#ifdef USE_BUS_1
    bus_read(bus1);
#endif
#ifdef USE_BUS_2
    bus_read(bus2);
#endif

#ifdef DEBUGGING_VERBOSE
    for (int i = 0; i < sizeof(bus.current); i++) {
        print_byte_be(bus.current[i]);
    }
    Serial.write('\n');
#endif

    // DEBOUNCE BUSES //
    uint64_t now = micros();
#ifdef USE_BUS_1
    bus_debounce(bus1, DEBOUNCE_TIME_MICROS, now);
#endif
#ifdef USE_BUS_2
    bus_debounce(bus2, DEBOUNCE_TIME_MICROS, now);
#endif

#ifdef USE_BUS_1
    // SEND CHANGES //
    int num_changed = bus_get_changes(bus1, bits_changed);

    for (int i = 0; i < num_changed; i++) {
#ifdef DEBUGGING
        Serial.print("Changed (bus 1): ");
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

        sendMessage(frame);
    }

    bus_make_current_last(bus1);
#endif

#ifdef USE_BUS_2
    num_changed = bus_get_changes(bus2, bits_changed);

    for (int i = 0; i < num_changed; i++) {
#ifdef DEBUGGING
        Serial.print("Changed (bus 2): ");
        Serial.print(bits_changed[i].bit_index);
        Serial.print("  ");
        Serial.println(bits_changed[i].new_state);
#endif

        can_frame frame;
        if (bits_changed[i].new_state == true) {
            frame = note_on(device_address, bits_changed[i].bit_index + 64);
        } else {
            frame = note_off(device_address, bits_changed[i].bit_index + 64);
        }

        sendMessage(frame);
    }

    bus_make_current_last(bus2);
#endif

    // ANALOG INPUTS //
#ifdef ANALOG_IN_1_PIN
    analog_in_read(analog_in_1);

    if (analog_in_1.did_change) {
        can_frame frame = continous_change(device_address, analog_in_1.channel, analog_in_1.current_value);
        sendMessage(frame);
    }
#endif
#ifdef ANALOG_IN_2_PIN
    analog_in_read(analog_in_2);

    if (analog_in_2.did_change) {
        can_frame frame = continous_change(device_address, analog_in_2.channel, analog_in_2.current_value);
        sendMessage(frame);
    }
#endif
#ifdef DISCRETE_IN_1_PIN
    discrete_in_read(discrete_in_1);

    if (discrete_in_1.did_change) {
        can_frame frame;
        if (discrete_in_1.current_value) {
            frame = note_on(device_address, discrete_in_1.bit_index);
        } else {
            frame = note_off(device_address, discrete_in_1.bit_index);
        }

        sendMessage(frame);
    }
#endif
}