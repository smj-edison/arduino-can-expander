#pragma once

#include <can.h>
#include <config.h>

can_frame note_on (uint8_t device_address, uint8_t note);

can_frame note_off (uint8_t device_address, uint8_t note);
