#include <Bus.h>

void bus_init_as_input (Bus &bus) {
    for(int i = 0; i < bus.ios_length; i++) {
        bus.ios[i].init();
        bus.ios[i].portMode(MCP23017Port::A, 0b11111111);
        bus.ios[i].portMode(MCP23017Port::B, 0b11111111);
    }
}

void bus_read (Bus &bus) {
    int byte_index = 0;

    for (int i = 0; i < bus.ios_length; i++) {
        bus.current[byte_index++] = ~bus.ios[i].readPort(MCP23017Port::A);
        bus.current[byte_index++] = ~bus.ios[i].readPort(MCP23017Port::B);
    }
}

void bus_debounce (Bus &bus, uint64_t debounce_time, uint64_t now) {
    for (int i = 0; i < bus.ios_length * 2; i++) {
        // first, figure out which -- if any -- bits changed
        uint8_t changed = bus.current[i] ^ bus.last[i];
        uint8_t to_revert = 0; // a mask for which bits to revert

        if (changed > 0) {
            for (int j = 0; j < 8; j++) {
                if ((changed >> j) & 1) { // if this bit changed from last time
                    if (now - bus.last_time_toggled[i * 8 + j] < debounce_time) { // was it not long enough?
                        to_revert |= 1 << j;
                    } else { // set new debounce time for bit
                        bus.last_time_toggled[i * 8 + j] = now;
                    }
                }
            }

            if (to_revert > 0) {
                // revert any bits that changed too fast
                bus.current[i] = (bus.current[i] & ~to_revert) | (bus.last[i] & to_revert);
            }
        }
    }
}

int bus_get_changes (Bus &bus, BitChanged *bits_out) {
    int bits_changed = 0;

    for (uint8_t i = 0; i < bus.ios_length * 2; i++) {
        // see if any changed in this byte
        uint8_t changed = bus.current[i] ^ bus.last[i];

        if (changed > 0) {
            for (uint8_t j = 0; j < 8; j++) {
                if ((changed >> j) & 1) { // if this bit changed from last time
                    uint8_t bit_index = i * 8 + j;

                    *bits_out = BitChanged {
                        bit_index, ((bus.current[i] >> j) & 1) > 0
                    };

                    bits_out++;
                    bits_changed++;
                }
            }
        }
    }

    return bits_changed;
}

void bus_make_current_last (Bus &bus) {
    for (int i = 0; i < bus.ios_length * 2; i++) {
        bus.last[i] = bus.current[i];
    }
}
