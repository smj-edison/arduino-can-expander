#include <can-messages.h>
#include <can-protocol.h>

// my CAN network implementation is pretty simple
// LSB bits 1-5 are the "to" address
// LSB bits 6-10 are the "from" address
// LSB bit 11 is a flag, if true it is a broadcast to everyone

can_frame start_of_can_message (uint8_t from, uint8_t to) {
    can_frame frame;

    frame.can_id = (from & 0x1F) << 5 | (to & 0x1F);

    return frame;
}

can_frame note_on (uint8_t device_address, uint8_t note) {
    can_frame frame = start_of_can_message(device_address, MIDI_INTERFACE_ID);

    frame.can_dlc = 2;
    frame.data[0] = BIT_ON_MSG;
    frame.data[1] = note;

    return frame;
}

can_frame note_off (uint8_t device_address, uint8_t note) {
    can_frame frame = start_of_can_message(device_address, MIDI_INTERFACE_ID);

    frame.can_dlc = 2;
    frame.data[0] = BIT_OFF_MSG;
    frame.data[1] = note;

    return frame;
}
