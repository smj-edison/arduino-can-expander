#include <Arduino.h>
#include <DiscreteIn.h>

void discrete_in_read(DiscreteIn &discrete_in) {
    int reading = digitalRead(discrete_in.pin);

    if(reading != discrete_in.last_value) {
        if (discrete_in.invert) {
            discrete_in.current_value = !reading;
        } else {
            discrete_in.current_value = reading;
        }

        discrete_in.did_change = true;

        

        discrete_in.last_value = reading;
    } else {
        discrete_in.did_change = false;
    }
}
