#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <functional>
#include <map>
#include <memory>

#include "../Timer/Timer.h"
#include "TCPWiFiServer.h"
#include "WSClient.h"

class WSServer {
   public:
    using ConnectionHandler = std::function<void(std::shared_ptr<WSClient>)>;

    WSServer(uint16_t port = 80, uint8_t maxClients = 4);
    WSServer(std::shared_ptr<TCPServer> server);
    ~WSServer();

    WSServer(const WSServer&)            = delete;
    WSServer(WSServer&&)                 = delete;
    WSServer& operator=(const WSServer&) = delete;
    WSServer& operator=(WSServer&&)      = delete;

#ifdef ESP32
    void begin(const uint16_t& stackSize = 2 * 8192);
#else
    void begin();
#endif
    void end();
    void run();
    void close(String id);
    void close(std::shared_ptr<WSClient> client);
    bool hasClients();
    bool hasClient(String id);
    std::vector<std::shared_ptr<WSClient>>& getClients();
    void onConnection(const String& path, ConnectionHandler handler);
    void removeConnectionHandler(const String& path);

   private:
    std::shared_ptr<TCPServer> m_Server;
    std::vector<std::shared_ptr<WSClient>> m_Clients;
    std::map<String, ConnectionHandler> m_ConnectionHandlers;
    uint32_t m_LastAccept  = 0;
    uint32_t m_LastCleanup = 0;

    void _accept();
    void _cleanup();
#ifdef ESP32
    TaskHandle_t m_TaskHandler = NULL;
    static void _pollingTask(void* ptr);
#else
    uint32_t m_EventId;
#endif
};

#endif