#ifndef CRYPTO_H
#define CRYPTO_H

#include "Arduino.h"
#include "Base64.h"
#include "SHA1.h"
#include "vector"

namespace Crypto {
    struct HandshakeRequestResult {
        String requestStr;
        String expectedAcceptKey;
    };

    struct HandshakeResponseResult {
        bool isSuccess;
        String serverAccept;
    };

    struct HandshakeServerResult {
        bool isValid;
        String key;
        String path;
    };

    String generateHandshakeKey(const String& key);
    String randomChars(const size_t& len);
    String getBitSequence(const uint16_t& data, const size_t& len);
    String encodeCloseReasonCode(const uint16_t& code);
    String generateRandomId(const size_t& len = 16);

    bool shouldAddDefaultHeader(const String& keyword, const std::vector<std::pair<String, String>>& customHeaders);
    HandshakeRequestResult generateHandshake(const String& host, const String& uri, const std::vector<std::pair<String, String>>& customHeaders);
    HandshakeResponseResult parseHandshakeResponse(std::vector<String> responseHeaders);
    HandshakeServerResult parseHandshakeRequest(std::vector<String> requestHeaders);
};

#endif
