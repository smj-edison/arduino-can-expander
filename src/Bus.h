#pragma once

#include <MCP23017.h>

struct Bus {
    MCP23017 ios[4];
    int ios_length;
    uint8_t last[8];
    uint8_t current[8];
    uint64_t last_time_toggled[64];
};

struct BitChanged {
    uint8_t bit_index;
    bool new_state;
};

void bus_init_as_input (Bus &bus);

void bus_read (Bus &bus);

void bus_debounce (Bus &bus, uint64_t debounce_time, uint64_t now);

int bus_get_changes (Bus &bus, BitChanged *bits_out);

void bus_make_current_last (Bus &bus);
