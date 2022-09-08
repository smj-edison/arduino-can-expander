const int MAX_ANALOG_READING = 1024;

struct AnalogIn {
    int pin;
    uint8_t channel;
    bool did_change;
    int current_value;
    int difference_threshold;
    int last_value;
};

void analog_in_read(AnalogIn &analog_in);
