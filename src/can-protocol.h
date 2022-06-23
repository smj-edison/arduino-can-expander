#pragma once

// my CAN network implementation is pretty simple:
// LSB bits 1-5 are the "to" address
// LSB bits 6-10 are the "from" address
// LSB bit 11 is a flag, if true it is a broadcast to everyone

// address reassign, 1 byte, the new address
#define ADDRESS_REASSIGN 0x00

#define NOTE_ON_MSG 0x20
#define NOTE_OFF_MSG 0x21
