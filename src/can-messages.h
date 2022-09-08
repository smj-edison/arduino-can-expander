#pragma once

#include <can.h>
#include <config.h>

can_frame bit_on (uint8_t device_address, uint8_t note);

can_frame bit_off (uint8_t device_address, uint8_t note);

can_frame continous_change (uint8_t device_address, uint8_t channel, uint8_t value);
