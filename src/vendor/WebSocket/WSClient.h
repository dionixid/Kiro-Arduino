#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <functional>
#include <memory>
#include <utility>

#include "../Timer/Timer.h"
#include "Arduino.h"
#include "TCPWiFiClient.h"
#include "utilities/Frame.h"

class WSServer;

class WSClient {
   public:
    enum class CloseReason {
        None                = -1,
        NormalClosure       = 1000,
        GoingAway           = 1001,
        ProtocolError       = 1002,
        UnsupportedData     = 1003,
        NoStatusRcvd        = 1005,
        AbnormalClosure     = 1006,
        InvalidPayloadData  = 1007,
        PolicyViolation     = 1008,
        MessageTooBig       = 1009,
        InternalServerError = 1011
    };

    using OpenHandler   = std::function<void(WSClient&)>;
    using TextHandler   = std::function<void(WSClient&, const String&)>;
    using BinaryHandler = std::function<void(WSClient&, const uint8_t*, const size_t&)>;
    using CloseHandler  = std::function<void(WSClient&, const CloseReason&, const String&)>;

    /**
     * @brief A unique id for the client.
     * The id can be used to identify a client. If the client is managed by a WSServer,
     * the id will be set to random 16 byte string. Otherwise, the default id is an empty string.
     * The id can be set manually, but it is the user's responsibility to ensure that the id is unique.
     *
     */
    String id;

    /**
     * @brief The index of the client.
     * This is merely just a number that the user can set to identify the client.
     * By default, the index is 0.
     */
    uint32_t index;

    /**
     * @brief The name of the client.
     * This is merely just a string that the user can set to identify the client.
     * By default, the name is an empty string.
     *
     */
    String name;

    /**
     * @brief The type of the client.
     * This is merely just a string that the user can set to identify the client.
     * By default, the type is an empty string.
     *
     */
    String type;

    /**
     * @brief The channel of the client.
     * This is merely just a string that the user can set to identify the client.
     * By default, the channel is an empty string.
     *
     */
    String channel;

    /**
     * @brief A flag to indicate if the client is alive.
     * This flag is not managed by neither the WSClient nor the WSServer.
     * This is merely just a flag that the user can set to indicate if the client is alive.
     * The user can use this flag to manage the heartbeat of the client.
     *
     */
    bool isAlive;

    WSClient();
    WSClient(const std::shared_ptr<TCPClient>& client);
    ~WSClient();

    WSClient(const WSClient& client)            = delete;
    WSClient(WSClient&& client)                 = delete;
    WSClient& operator=(const WSClient& client) = delete;
    WSClient& operator=(WSClient&& client)      = delete;

    void addHeader(const String& key, const String& value);
#ifdef ESP32
    bool begin(String url, const bool& autoReconnect = true, const uint16_t& stackSize = 2 * 8192);
#else
    bool begin(String url, const bool& autoReconnect = true);
#endif
    bool send(const Frame::Opcode& opcode, const uint8_t& fin, const uint8_t* data, const size_t& length);
    bool send(const Frame::Opcode& opcode, const uint8_t& fin, const String& data);

    bool sendText(const String& data);
    bool sendBinary(const uint8_t* data, const size_t& length);

    bool beginFragmentText(const String& data);
    bool beginFragmentBinary(const uint8_t* data, const size_t& length);
    bool sendFragmentText(const String& data);
    bool sendFragmentBinary(const uint8_t* data, const size_t& length);
    bool endFragmentText(const String&);
    bool endFragmentBinary(const uint8_t* data, const size_t& length);

    bool ping(const String& data = "");
    bool pong(const String& data = "");
    bool close(const CloseReason& code = CloseReason::GoingAway, const String& reason = "");

    void onOpen(const OpenHandler& callback);
    void onClose(const CloseHandler& callback);
    void onTextMessage(const TextHandler& callback);
    void onBinaryMessage(const BinaryHandler& callback);
    void onPing(const TextHandler& callback);
    void onPong(const TextHandler& callback);
    void onError(const TextHandler& callback);

    bool isConnected();
    bool reconnect();
    void setUseMask(const bool& useMask);

    IPAddress remoteIP();
    uint16_t remotePort();

    void poll();
    void run();

    static String getCloseReasonName(const CloseReason& code);

    friend class WSServer;

   private:
    enum State {
        Connecting,
        Connected,
        Closed
    };

    enum FragmentType {
        None,
        Text,
        Binary
    };

    std::shared_ptr<TCPClient> m_Client;
    String m_Host;
    String m_Path;
    uint16_t m_Port;
    bool m_AutoReconnect = true;

    bool m_UseMask = true;
    uint8_t m_MaskingKey[4];

    State m_State = Closed;
    IPAddress m_RemoteIP;
    uint16_t m_RemotePort;
    uint32_t m_LastReconnectAttempt = 0;

    String m_TextBuffer;
    std::vector<uint8_t> m_BinaryBuffer;
    FragmentType m_FragmentType = None;
    std::vector<std::pair<String, String>> m_CustomHeaders;

    OpenHandler m_OpenHandler     = NULL;
    CloseHandler m_CloseHandler   = NULL;
    TextHandler m_TextHandler     = NULL;
    TextHandler m_PingHandler     = NULL;
    TextHandler m_PongHandler     = NULL;
    TextHandler m_ErrorHandler    = NULL;
    BinaryHandler m_BinaryHandler = NULL;

    /**
     * @brief This callback is used by the WSServer to remove the client from the client list.
     *
     */
    std::function<void(WSClient*)> m_CloseHandlerInternal = NULL;

    bool _close(
        const CloseReason& code = CloseReason::GoingAway, const String& reason = "", const bool& sendCloseFrame = true
    );
    void _reshuffleMask();
#ifdef ESP32
    uint16_t m_StackSize;
    TaskHandle_t m_TaskHandler = NULL;
    static void _pollingTask(void* ptr);
#else
    uint32_t m_EventId;
#endif
};

#endif
