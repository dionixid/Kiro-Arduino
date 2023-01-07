#include "Frame.h"

#include <lwip/def.h>

/**
 * @brief Create a Frame::Header object.
 *
 * @param data is the first 2 bytes of the frame header.
 * @param extendedPayload is the extended payload length.
 */
Frame::Header::Header(const uint16_t& data) {
    fin     = (data >> 15) & 0x1;
    rsv     = (data >> 12) & 0x8;
    opcode  = (data >> 8) & 0xF;
    mask    = (data >> 7) & 0x1;
    payload = (data & 0x7F);
}

/**
 * @brief Create a Frame::Header object.
 *
 * @param fin is the FIN bit. It is 1 if this is the final fragment in a message. Otherwise, it is 0.
 * @param rsv is the RSV1, RSV2, and RSV3 bits.
 * @param mask is the MASK bit. It is 1 if the payload is masked. Otherwise, it is 0.
 * @param opcode is the opcode.
 * @param len is the payload length.
 */
Frame::Header::Header(const uint8_t& fin, const uint8_t& rsv, const uint8_t& mask, const uint8_t& opcode, const uint16_t& len)
    : fin(fin),
      rsv(rsv),
      mask(mask),
      opcode(opcode),
      payload(0) {
    if (len < 126) {
        payload         = len;
        extendedPayload = 0;
    } else {
        payload         = 126;
        extendedPayload = len;
    }
}

/**
 * @brief Get the binary representation of the frame header.
 *
 * @return The binary representation of the header.
 */
uint16_t Frame::Header::toBinary() {
    uint16_t ret = 0;
    ret |= fin << 15;
    ret |= rsv << 12;
    ret |= opcode << 8;
    ret |= mask << 7;
    ret |= payload;
    return ret;
}

/**
 * @brief Get the binary representation of the frame header.
 * This method is used for debugging purposes.
 *
 * @param delimiter is the delimiter between each bit.
 * @return is the binary representation of the frame header in string format.
 */
String Frame::Header::getBinarySequence(String delimiter) {
    String result = Crypto::getBitSequence(fin, 1) + delimiter + Crypto::getBitSequence(rsv, 3) + delimiter
                    + Crypto::getBitSequence(mask, 1) + delimiter + Crypto::getBitSequence(opcode, 4) + delimiter
                    + Crypto::getBitSequence(payload, 7);

    if (payload == 126) {
        result += delimiter + Crypto::getBitSequence(extendedPayload, 16);
    }
    return result;
}

/**
 * @brief Check if the frame header is valid.
 *
 * @return true if the  frame header is valid. false otherwise.
 */
bool Frame::Header::isValid() {
    return rsv == 0
           && (opcode == Continuation || opcode == Text || opcode == Binary || opcode == Close || opcode == Ping
               || opcode == Pong)
           && (fin == 1 || (fin == 0 && (opcode == Continuation || opcode == Text || opcode == Binary)));
}
