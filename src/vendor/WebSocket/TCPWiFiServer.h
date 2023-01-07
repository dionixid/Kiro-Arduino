#ifndef TCP_WIFI_SERVER_H
#define TCP_WIFI_SERVER_H

#include "TCPServer.h"
#include "TCPWiFiClient.h"

class TCPWiFiServer : public TCPServer {
   public:
    TCPWiFiServer(uint16_t port, uint8_t maxClients = 4)
#ifdef ESP32
        : m_Server(WiFiServer(port, maxClients)) {
        // m_Server.setNoDelay(true);
    }
#else
        : m_Server(WiFiServer(port)) {
        // m_Server.setNoDelay(true);
    }
#endif

    ~TCPWiFiServer() {
        end();
    }

    void begin() override {
        m_Server.begin();
    }

    std::shared_ptr<TCPClient> accept() override {
        return std::make_shared<TCPWiFiClient>(m_Server.available());
    }

    void end() override {
        m_Server.stop();
    }

   private:
    WiFiServer m_Server;
};

#endif