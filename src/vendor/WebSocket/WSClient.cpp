#include "WSClient.h"

/**
 * @brief Create a WebSocket client from a WiFiClient.
 *
 */
WSClient::WSClient()
    : m_Client(std::make_shared<TCPWiFiClient>()),
      m_State(Closed) {
    _reshuffleMask();
#ifdef ESP32
    m_TaskHandler = NULL;
#endif
}

/**
 * @brief Create a WebSocket client from a custom TCP client.
 * Take a look the implementation of TCPWiFiClient.h for an example.
 *
 * @param client
 */
WSClient::WSClient(const std::shared_ptr<TCPClient>& client)
    : m_Client(client),
      m_RemoteIP(client->remoteIP()),
      m_RemotePort(client->remotePort()),
      m_State(client && client->connected() ? Connected : Closed) {
    _reshuffleMask();
#ifdef ESP32
    m_TaskHandler = NULL;
#endif
}

WSClient::~WSClient() {
#ifdef ESP32
    if (m_TaskHandler) {
        vTaskDelete(m_TaskHandler);
    }
#else
    Timer::unregisterEvent(m_EventId);
#endif
    _close();
}

/**
 * @brief Add a custom header to the WebSocket handshake.
 *
 * @param key is the header name.
 * @param value is the header value.
 */
void WSClient::addHeader(const String& key, const String& value) {
    m_CustomHeaders.push_back({key, value});
}

#ifdef ESP32
/**
 * @brief Start the WebSocket connection.
 *
 * @param url is the WebSocket server URL.
 * @param stackSize is the size of the stack for the polling task.
 * @return true if the connection is established. false otherwise.
 */
bool WSClient::begin(String url, const bool& autoReconnect, const uint16_t& stackSize) {
    if (!m_Client) {
        if (m_ErrorHandler) {
            m_ErrorHandler(*this, "Client is not initialized");
        }
        return false;
    }

    m_StackSize = stackSize;

    if (url.startsWith("ws://")) {
        url.replace("ws://", "");
    } else if (url.startsWith("wss://")) {
        url.replace("wss://", "");
    } else {
        if (m_ErrorHandler) {
            m_ErrorHandler(*this, "Invalid URL");
        }
        return false;
    }

    url.trim();
    int pathIdx = url.indexOf("/");

    if (pathIdx > 0) {
        m_Path = url.substring(pathIdx);
        url    = url.substring(0, pathIdx);
    } else {
        m_Path = "/";
    }

    int portIdx = url.indexOf(":");

    if (portIdx > 0) {
        m_Port = url.substring(portIdx + 1).toInt();
        m_Host = url.substring(0, portIdx);
    } else {
        m_Port = 80;
        m_Host = url;
    }

    _close();

    m_State         = Connecting;
    m_AutoReconnect = autoReconnect;

    if (m_Client->begin(m_Host, m_Port, m_Path, m_CustomHeaders)) {
        if (m_OpenHandler) {
            m_OpenHandler(*this);
        }
        m_State = Connected;

        if (!m_TaskHandler) {
            xTaskCreate(_pollingTask, "pollingTask", stackSize, this, 5, &m_TaskHandler);
        }

        m_RemoteIP   = m_Client->remoteIP();
        m_RemotePort = m_Client->remotePort();
        return true;
    }

    if (m_ErrorHandler) {
        m_ErrorHandler(*this, "Cannot connect to " + url);
    }
    m_State = Closed;

    if (!m_TaskHandler) {
        xTaskCreate(_pollingTask, "pollingTask", stackSize, this, 5, &m_TaskHandler);
    }

    m_State = Closed;
    return false;
}
#else
/**
 * @brief Start the WebSocket connection.
 *
 * @param url is the WebSocket server URL.
 * @return true if the connection is established. false otherwise.
 */
bool WSClient::begin(String url, const bool& autoReconnect) {
    if (!m_Client) {
        if (m_ErrorHandler) {
            m_ErrorHandler(*this, "Client is not initialized");
        }
        return false;
    }

    if (url.startsWith("ws://")) {
        url.replace("ws://", "");
    } else if (url.startsWith("wss://")) {
        url.replace("wss://", "");
    } else {
        if (m_ErrorHandler) {
            m_ErrorHandler(*this, "Invalid URL");
        }
        return false;
    }

    url.trim();
    int pathIdx = url.indexOf("/");

    if (pathIdx > 0) {
        m_Path = url.substring(pathIdx);
        url = url.substring(0, pathIdx);
    } else {
        m_Path = "/";
    }

    int portIdx = url.indexOf(":");

    if (portIdx > 0) {
        m_Port = url.substring(portIdx + 1).toInt();
        m_Host = url.substring(0, portIdx);
    } else {
        m_Port = 80;
        m_Host = url;
    }

    _close();

    m_State = Connecting;
    m_AutoReconnect = autoReconnect;

    if (m_Client->begin(m_Host, m_Port, m_Path, m_CustomHeaders)) {
        if (m_OpenHandler) {
            m_OpenHandler(*this);
        }
        m_State = Connected;

        Timer::unregisterEvent(m_EventId);
        m_EventId = Timer::registerEvent([this]() { run(); });

        m_RemoteIP = m_Client->remoteIP();
        m_RemotePort = m_Client->remotePort();
        return true;
    }

    if (m_ErrorHandler) {
        m_ErrorHandler(*this, "Cannot connect to " + url);
    }

    Timer::unregisterEvent(m_EventId);
    m_EventId = Timer::registerEvent([this]() { run(); });

    m_State = Closed;
    return false;
}
#endif

/**
 * @brief Send a message to the WebSocket server.
 *
 * @param opcode is the opcode of the message.
 * @param fin is the FIN bit of the message.
 * @param payload is the payload of the message.
 * @param length is the length of the payload.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::send(const Frame::Opcode& opcode, const uint8_t& fin, const uint8_t* data, const size_t& length) {
    if (!m_Client || length > 65535) {
        return false;
    }

    Frame::Header header(fin, 0, m_UseMask ? 1 : 0, opcode, length);

    uint16_t bin             = htons(header.toBinary());
    uint16_t extendedPayload = htons(header.extendedPayload);
    uint32_t payloadLength   = length > 125 ? length + (m_UseMask ? 8 : 4) : length + (m_UseMask ? 6 : 2);

    uint8_t payload[payloadLength];
    memcpy(payload, (uint8_t*)&bin, 2);
    uint16_t start = 2;

    if (length > 125) {
        memcpy(payload + start, (uint8_t*)&extendedPayload, 2);
        start += 2;
    }

    if (m_UseMask) {
        memcpy(payload + start, m_MaskingKey, 4);
        start += 4;
    }

    memcpy(payload + start, data, length);

    if (m_UseMask) {
        for (size_t i = start; i < payloadLength; i++) {
            payload[i] ^= m_MaskingKey[i % 4];
        }
        _reshuffleMask();
    }

    return m_Client->write(payload, payloadLength) > 0;
}

/**
 * @brief Send a message to the WebSocket server.
 *
 * @param opcode is the opcode of the message.
 * @param fin is the FIN bit of the message.
 * @param data is the message to send.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::send(const Frame::Opcode& opcode, const uint8_t& fin, const String& data) {
    return send(opcode, fin, (uint8_t*)data.c_str(), data.length());
}

/**
 * @brief Send a text message to the WebSocket server.
 *
 * @param data is the message to send.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::sendText(const String& data) {
    return send(Frame::Text, 1, data);
}

/**
 * @brief Send a binary message to the WebSocket server.
 *
 * @param data is the message to send.
 * @param length is the length of the message.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::sendBinary(const uint8_t* data, const size_t& length) {
    return send(Frame::Binary, 1, data, length);
}

/**
 * @brief Start a fragmented text message.
 *
 * @param data is the first fragment of the message.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::beginFragmentText(const String& data) {
    return send(Frame::Text, 0, data);
}

/**
 * @brief Start a fragmented binary message.
 *
 * @param data is the first fragment of the message.
 * @param length is the length of the message.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::beginFragmentBinary(const uint8_t* data, const size_t& length) {
    return send(Frame::Binary, 0, data, length);
}

/**
 * @brief Send a fragment of a fragmented message.
 *
 * @param data is the fragment of the message.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::sendFragmentText(const String& data) {
    return send(Frame::Continuation, 0, data);
}

/**
 * @brief Send a fragment of a fragmented message.
 *
 * @param data is the fragment of the message.
 * @param length is the length of the message.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::sendFragmentBinary(const uint8_t* data, const size_t& length) {
    return send(Frame::Continuation, 0, data, length);
}

/**
 * @brief Send the last fragment of a fragmented message.
 *
 * @param data is the last fragment of the message.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::endFragmentText(const String& data) {
    return send(Frame::Continuation, 1, data);
}

/**
 * @brief Send the last fragment of a fragmented message.
 *
 * @param data is the last fragment of the message.
 * @param length is the length of the message.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::endFragmentBinary(const uint8_t* data, const size_t& length) {
    return send(Frame::Continuation, 1, data, length);
}

/**
 * @brief Send a ping message to the WebSocket server.
 *
 * @param data is the payload of the ping message.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::ping(const String& data) {
    if (!m_Client) {
        return false;
    }
    return send(Frame::Ping, 1, data);
}

/**
 * @brief Send a pong message to the WebSocket server.
 *
 * @param data is the payload of the pong message.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::pong(const String& data) {
    if (!m_Client) {
        return false;
    }
    return send(Frame::Pong, 1, data);
}

/**
 * @brief Close the WebSocket connection.
 *
 * @param code is the close reason code.
 * @param reason is the close reason.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::close(const CloseReason& code, const String& reason) {
    m_AutoReconnect = false;
    return _close(code, reason);
}

/**
 * @brief Close the WebSocket connection.
 * This is the internal implementation of the close method.
 *
 * @param code is the close reason code.
 * @param reason is the close reason.
 * @return true if the message is sent. false otherwise.
 */
bool WSClient::_close(const CloseReason& code, const String& reason, const bool& sendCloseFrame) {
    if (!m_Client || m_State != Connected) {
        return false;
    }

    m_State = Closed;
    String data =
        Crypto::encodeCloseReasonCode((uint16_t)code) + (reason.length() > 0 ? reason : getCloseReasonName(code));

    int res = 1;
    if (sendCloseFrame) {
        int res = send(Frame::Close, 1, data);
    }
    m_Client->end();

    if (m_CloseHandlerInternal) {
        m_CloseHandlerInternal(this);
    }

    if (m_CloseHandler) {
        m_CloseHandler(
            *this, code,
            String((uint16_t)code) + " -> " + getCloseReasonName(code) + (reason.length() > 0 ? ": " + reason : "")
        );
    }

    return res;
}

/**
 * @brief Set the callback to call when the WebSocket connection is opened.
 *
 * @param callback is the callback to call.
 */
void WSClient::onOpen(const OpenHandler& callback) {
    m_OpenHandler = callback;
}

/**
 * @brief Set the callback to call when the WebSocket connection is closed.
 *
 * @param callback is the callback to call.
 */
void WSClient::onClose(const CloseHandler& callback) {
    m_CloseHandler = callback;
}

/**
 * @brief Set the callback to call when a text message is received.
 *
 * @param callback is the callback to call.
 */
void WSClient::onTextMessage(const TextHandler& callback) {
    m_TextHandler = callback;
}

/**
 * @brief Set the callback to call when a binary message is received.
 *
 * @param callback is the callback to call.
 */
void WSClient::onBinaryMessage(const BinaryHandler& callback) {
    m_BinaryHandler = callback;
}

/**
 * @brief Set the callback to call when a ping message is received.
 *
 * @param callback is the callback to call.
 */
void WSClient::onPing(const TextHandler& callback) {
    m_PingHandler = callback;
}

/**
 * @brief Set the callback to call when a pong message is received.
 *
 * @param callback is the callback to call.
 */
void WSClient::onPong(const TextHandler& callback) {
    m_PongHandler = callback;
}

/**
 * @brief Set the callback to call when an error occurs.
 *
 * @param callback is the callback to call.
 */
void WSClient::onError(const TextHandler& callback) {
    m_ErrorHandler = callback;
}

/**
 * @brief Check if the WebSocket connection is opened.
 *
 * @return true if the WebSocket connection is opened. false otherwise.
 */
bool WSClient::isConnected() {
    if (!m_Client) {
        return false;
    }
    return m_Client->connected();
}

/**
 * @brief Reconnect to the WebSocket server.
 *
 * @return true if the WebSocket connection is opened. false otherwise.
 */
bool WSClient::reconnect() {
    if (!m_Client || m_State == Connecting) {
        return false;
    }
    m_State = Connecting;

    if (m_Client->begin(m_Host, m_Port, m_Path, m_CustomHeaders)) {
        if (m_OpenHandler) {
            m_OpenHandler(*this);
        }

        m_RemoteIP   = m_Client->remoteIP();
        m_RemotePort = m_Client->remotePort();
        m_State      = Connected;
        return true;
    }

    m_State = Closed;
    if (m_ErrorHandler) {
        m_ErrorHandler(*this, "Reconnection failed");
    }

    return false;
}

/**
 * @brief Set if the WebSocket client should use a mask for the payload.
 * The mask is only required when sending data from the client to the server.
 *
 * @param useMask
 */
void WSClient::setUseMask(const bool& useMask) {
    m_UseMask = useMask;
}

/**
 * @brief Get the remote IP address of the WebSocket server.
 *
 * @return the remote IP address of the WebSocket server.
 */
IPAddress WSClient::remoteIP() {
    return m_RemoteIP;
}

/**
 * @brief Get the remote port of the WebSocket server.
 *
 * @return the remote port of the WebSocket server.
 */
uint16_t WSClient::remotePort() {
    return m_RemotePort;
}

/**
 * @brief Poll the WebSocket connection.
 *
 */
void WSClient::poll() {
    if (!m_Client || !m_Client->available()) {
        return;
    }

    uint16_t bin;
    m_Client->read((uint8_t*)&bin, 2);

    Frame::Header header(ntohs(bin));
    uint16_t payloadLen = header.payload;

    if (!header.isValid() || (m_UseMask && header.mask) || (!m_UseMask && !header.mask)) {
        _close(CloseReason::ProtocolError);
        return;
    }

    if (header.payload == 127) {
        _close(CloseReason::MessageTooBig);
        return;
    }

    if (header.payload == 126) {
        m_Client->read((uint8_t*)&bin, 2);
        payloadLen = htons(bin);
    }

    uint8_t maskingKey[4];
    uint8_t payload[payloadLen + 1];
    payload[payloadLen] = 0;

    if (header.mask) {
        m_Client->read(maskingKey, 4);
    }

    if (payloadLen > 0) {
        m_Client->read(payload, payloadLen);
    }

    if (header.mask) {
        for (int i = 0; i < payloadLen; i++) {
            payload[i] ^= maskingKey[i % 4];
        }
    }

    if (header.opcode == Frame::Close) {
        uint16_t code = static_cast<uint16_t>(CloseReason::NormalClosure);
        String reason;

        if (payloadLen >= 2) {
            code = (payload[0] << 8) | payload[1];
        }

        if (payloadLen > 2) {
            reason = String((char*)payload + 2);
        }

        _close(static_cast<CloseReason>(code), reason, false);
        return;
    }

    if (header.opcode == Frame::Ping) {
        String payloadData((char*)payload);

        pong(payloadData);
        if (m_PingHandler) {
            m_PingHandler(*this, payloadData);
        }

        return;
    }

    if (header.opcode == Frame::Pong) {
        String payloadData((char*)payload);

        if (m_PongHandler) {
            m_PongHandler(*this, payloadData);
        }
        return;
    }

    if (header.opcode == Frame::Text && header.fin == 1) {
        if (m_FragmentType != FragmentType::None) {
            _close(CloseReason::ProtocolError);
            return;
        }

        if (m_TextHandler) {
            m_TextHandler(*this, String((char*)payload));
        }
        return;
    }

    if (header.opcode == Frame::Binary && header.fin == 1) {
        if (m_FragmentType != FragmentType::None) {
            _close(CloseReason::ProtocolError);
            return;
        }
        if (m_BinaryHandler) {
            m_BinaryHandler(*this, payload, payloadLen);
        }
        return;
    }

    if (header.opcode == Frame::Text && header.fin == 0) {
        if (m_FragmentType != FragmentType::None) {
            _close(CloseReason::ProtocolError);
            return;
        }
        m_FragmentType = FragmentType::Text;
        m_TextBuffer   = String((char*)payload);
        return;
    }

    if (header.opcode == Frame::Binary && header.fin == 0) {
        if (m_FragmentType != FragmentType::None) {
            _close(CloseReason::ProtocolError);
            return;
        }
        m_FragmentType = FragmentType::Binary;
        m_BinaryBuffer = std::vector<uint8_t>(payload, payload + payloadLen);
        return;
    }

    if (header.opcode == Frame::Continuation && header.fin == 0) {
        if (m_FragmentType == FragmentType::None) {
            _close(CloseReason::ProtocolError);
            return;
        }

        if (m_FragmentType == FragmentType::Text) {
            m_TextBuffer += String((char*)payload);
        } else {
            m_BinaryBuffer.insert(m_BinaryBuffer.end(), payload, payload + payloadLen);
        }

        return;
    }

    if (header.opcode == Frame::Continuation && header.fin == 1) {
        if (m_FragmentType == FragmentType::None) {
            _close(CloseReason::ProtocolError);
            return;
        }

        if (m_FragmentType == FragmentType::Text) {
            m_TextBuffer += String((char*)payload);
            if (m_TextHandler) {
                m_TextHandler(*this, m_TextBuffer);
            }
            m_TextBuffer.clear();
        } else {
            m_BinaryBuffer.insert(m_BinaryBuffer.end(), payload, payload + payloadLen);
            if (m_BinaryHandler) {
                m_BinaryHandler(*this, m_BinaryBuffer.data(), m_BinaryBuffer.size());
            }
            m_BinaryBuffer.clear();
        }

        m_FragmentType = FragmentType::None;
    }
}

/**
 * @brief Get the close reason name.
 *
 * @param code is the close reason code.
 * @return the close reason name.
 */
String WSClient::getCloseReasonName(const CloseReason& code) {
    switch (code) {
        case CloseReason::NormalClosure:
            return "Normal Closure";
        case CloseReason::GoingAway:
            return "Going Away";
        case CloseReason::ProtocolError:
            return "Protocol Error";
        case CloseReason::UnsupportedData:
            return "Unsupported Data";
        case CloseReason::NoStatusRcvd:
            return "No Status Received";
        case CloseReason::AbnormalClosure:
            return "Abnormal Closure";
        case CloseReason::InvalidPayloadData:
            return "Invalid Payload Data";
        case CloseReason::PolicyViolation:
            return "Policy Violation";
        case CloseReason::MessageTooBig:
            return "Message Too Big";
        case CloseReason::InternalServerError:
            return "Internal Server Error";
        default:
            return "No Reason";
    }
}

/**
 * @brief Run the WebSocket client.
 * For ESP8266, this method must be called in the loop.
 * For ESP32, this method is called by the pollTask(),
 * which is running on different FreeRTOS task.
 *
 */
void WSClient::run() {
    if (isConnected()) {
        if (m_State == Connected) {
            poll();
        }
    } else if (m_AutoReconnect && millis() - m_LastReconnectAttempt > 5000) {
        m_LastReconnectAttempt = millis();
        if (m_State == Connecting) {
            return;
        }
        if (m_State == Connected) {
            _close(CloseReason::InternalServerError);
        }
        reconnect();
    }
}

/**
 * @brief Generate a random masking key.
 * The masking key is only used one time.
 * This method is called automatically when sending data.
 *
 */
void WSClient::_reshuffleMask() {
    for (int i = 0; i < 4; i++) {
        m_MaskingKey[i] = random(256);
    }
}

#ifdef ESP32
void WSClient::_pollingTask(void* ptr) {
    WSClient* client = (WSClient*)ptr;
    while (true) {
        client->run();
        delay(2);
    }
}
#endif
