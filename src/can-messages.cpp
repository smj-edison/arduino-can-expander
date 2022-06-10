#include <can-messages.h>

#define NOTE_ON_MSG 0x00
#define NOTE_OFF_MSG 0x01

can_frame start_of_can_message () {
    can_frame frame;

    frame.can_id = DEVICE_ID;

    return frame;
}

can_frame note_on (uint8_t note) {
    can_frame frame = start_of_can_message();

    frame.can_dlc = 2;
    frame.data[0] = NOTE_ON_MSG;
    frame.data[1] = note;

    return frame;
}

can_frame note_off (uint8_t note) {
    can_frame frame = start_of_can_message();

    frame.can_dlc = 2;
    frame.data[0] = NOTE_OFF_MSG;
    frame.data[1] = note;

    return frame;
}
