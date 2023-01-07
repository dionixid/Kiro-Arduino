#include "Crypto.h"

#include <lwip/def.h>
#include <random>

/**
 * @brief Generate a handshake key from a given key.
 * 
 * @param key is the key.
 * @return base64 encoded key.
 */
String Crypto::generateHandshakeKey(const String& key) {
    char base64[SHA1_BASE64_SIZE];
    SHA1(key)
        .add("258EAFA5-E914-47DA-95CA-C5AB0DC85B11")
        .finalize()
        .getBase64(base64);
    return String(base64);
}

/**
 * @brief Generate a random char sequence.
 * 
 * @param len is the length of the sequence.
 * @return a random char sequence.
 */
String Crypto::randomChars(const size_t& len) {
    static const char ALPHA_NUMERIC[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<int> dist(0, 61);
    
    char result[len + 1];
    result[len] = '\0';

    for (size_t i = 0; i < len; i++) {
        result[i] = ALPHA_NUMERIC[dist(rng)];
    }

    return result;
}

/**
 * @brief Get the bit sequence of a given data.
 * 
 * @param data is the data.
 * @param len is the length of the data.
 * @return A string representation of the bit sequence.
 */
String Crypto::getBitSequence(const uint16_t& data, const size_t& len) {
    String result;
    for (size_t i = 0; i < len; i++) {
        result += (data >> (len - i - 1)) & 1;
    }
    return result;
}

/**
 * @brief Encode a close reason code to a string.
 * The close reason code is encoded in big endian.
 * 
 * @param code is the close reason code. 
 * @return a string representation of the close reason code in big endian.
 */
String Crypto::encodeCloseReasonCode(const uint16_t& code) {
    uint16_t swappedCode = htons(code);
    String result;
    result.concat((char*)&swappedCode, 2);
    return result;
}

/**
 * @brief Check if a given key word should be added to the default header.
 * 
 * @param keyword is the key word. 
 * @param customHeaders is the custom headers.
 * @return true if the key word should be added to the default header.
 */
bool Crypto::shouldAddDefaultHeader(const String& keyword, const std::vector<std::pair<String, String>>& customHeaders) {
    for (const auto& header : customHeaders) {
        if (keyword.equals(header.first)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Generate a handshake request.
 * 
 * @param host is the host.
 * @param uri is the uri.
 * @param customHeaders is the custom headers.
 * @return a handshake request result.
 */
Crypto::HandshakeRequestResult Crypto::generateHandshake(const String& host, const String& uri, const std::vector<std::pair<String, String>>& customHeaders) {
    String key = Base64::encode(randomChars(16));
    String handshake = "GET " + uri + " HTTP/1.1\r\n";
    handshake += "Host: " + host + "\r\n";
    handshake += "Sec-WebSocket-Key: " + key + "\r\n";

    for (const auto& header : customHeaders) {
        handshake += header.first + ": " + header.second + "\r\n";
    }

    if (shouldAddDefaultHeader("Upgrade", customHeaders)) {
        handshake += "Upgrade: websocket\r\n";
    }

    if (shouldAddDefaultHeader("Connection", customHeaders)) {
        handshake += "Connection: Upgrade\r\n";
    }

    if (shouldAddDefaultHeader("Sec-WebSocket-Version", customHeaders)) {
        handshake += "Sec-WebSocket-Version: 13\r\n";
    }

    if (shouldAddDefaultHeader("User-Agent", customHeaders)) {
        handshake += "User-Agent: ESP32\r\n";
    }

    if (shouldAddDefaultHeader("Origin", customHeaders)) {
        handshake += "Origin: https://codedillo.com\r\n";
    }

    handshake += "\r\n";
    Crypto::HandshakeRequestResult result;
    result.requestStr = handshake;
    result.expectedAcceptKey = generateHandshakeKey(key);
    return result;
}

/**
 * @brief Parse a handshake response.
 * 
 * @param responseHeaders is the response headers.
 * @return a handshake response result.
 */
Crypto::HandshakeResponseResult Crypto::parseHandshakeResponse(std::vector<String> responseHeaders) {
    bool didUpgradeToWebsockets = false;
    bool isConnectionUpgraded = false;
    String serverAccept = "";

    for (String header : responseHeaders) {
        int colonIndex = header.indexOf(':');
        String key = header.substring(0, colonIndex);
        String value = header.substring(colonIndex + 1);
        key.trim();
        value.trim();
        key.toLowerCase();

        if (key.equals("upgrade")) {
            value.toLowerCase();
            didUpgradeToWebsockets = value.equals("websocket");
        } else if (key.equals("connection")) {
            value.toLowerCase();
            isConnectionUpgraded = value.equals("upgrade");
        } else if (key.equals("sec-websocket-accept")) {
            serverAccept = value;
        }
    }

    Crypto::HandshakeResponseResult result;
    result.isSuccess = serverAccept != "" && didUpgradeToWebsockets && isConnectionUpgraded;
    result.serverAccept = serverAccept;
    return result;
}

/**
 * @brief Parse a handshake request.
 * 
 * @param requestHeaders is the request headers.
 * @return a handshake server result.
 */
Crypto::HandshakeServerResult Crypto::parseHandshakeRequest(std::vector<String> requestHeaders) {
    bool isUpgrade = false;
    bool isConnection = false;
    bool isSecWebSocketKey = false;
    bool isSecWebSocketVersion = false;
    String handshakeKey;
    String path;

    for (String header : requestHeaders) {
        int colonIndex = header.indexOf(':');
        String key = header.substring(0, colonIndex);
        String value = header.substring(colonIndex + 1);
        key.trim();
        value.trim();
        key.toLowerCase();

        if (header.startsWith("GET")) {
            int pathStart = header.indexOf(' ') + 1;
            int pathEnd = header.indexOf(' ', pathStart);
            path = header.substring(pathStart, pathEnd);
        } else if (key.equals("connection")) {
            value.toLowerCase();
            isConnection = value.equals("upgrade");
        } else if (key.equals("upgrade")) {
            value.toLowerCase();
            isUpgrade = value.equals("websocket");
        } else if (key.equals("sec-websocket-version")) {
            isSecWebSocketVersion = value.equals("13");
        } else if (key.equals("sec-websocket-key")) {
            isSecWebSocketKey = !value.isEmpty();
            handshakeKey = value;
        }
    }

    Crypto::HandshakeServerResult result;
    result.isValid = isUpgrade && isConnection && isSecWebSocketKey && isSecWebSocketVersion;
    result.key = generateHandshakeKey(handshakeKey);
    result.path = path;
    return result;
}

/**
 * @brief Generate a random id.
 * 
 * @param len is the length of the id.
 * @return a random id.
 */
String Crypto::generateRandomId(const size_t& len) {
    return Base64::encode(randomChars(len));
}
