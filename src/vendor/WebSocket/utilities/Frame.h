#ifndef FRAME_H
#define FRAME_H

#include "Arduino.h"
#include "Crypto.h"

class Frame {
   public:
    enum Opcode {
        Continuation = 0x0,
        Text         = 0x1,
        Binary       = 0x2,
        Close        = 0x8,
        Ping         = 0x9,
        Pong         = 0xA
    };

    struct Header {
        uint8_t fin     : 1;
        uint8_t rsv     : 3;
        uint8_t mask    : 1;
        uint8_t opcode  : 4;
        uint8_t payload : 7;
        uint16_t extendedPayload;

        Header(const uint16_t& data = 0);
        Header(const uint8_t& fin, const uint8_t& rsv, const uint8_t& mask, const uint8_t& opcode, const uint16_t& len);

        uint16_t toBinary();
        String getBinarySequence(String delimiter = "");
        bool isValid();
    };
};

#endif
