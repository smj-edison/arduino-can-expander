#include <Arduino.h>
#include <AnalogIn.h>

void analog_in_read(AnalogIn &analog_in) {
    int reading_raw = analogRead(analog_in.pin);

    if(abs(reading_raw - analog_in.last_value) > analog_in.difference_threshold) {
        int reading = (int) (((long) reading_raw) * 256 / MAX_ANALOG_READING); // between 0 - 127

        analog_in.current_value = reading;
        analog_in.did_change = true;

        analog_in.last_value = reading_raw;
    } else {
        analog_in.did_change = false;
    }
}
