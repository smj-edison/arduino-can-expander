#include <Arduino.h>

struct DiscreteIn {
    int pin;
    uint8_t bit_index;
    bool did_change;
    bool current_value;
    bool last_value;
};

void discrete_in_read(DiscreteIn &discrete_in);
