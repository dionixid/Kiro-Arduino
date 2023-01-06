#ifndef TCP_WIFI_CLIENT_H
#define TCP_WIFI_CLIENT_H

#include "TCPClient.h"
#ifdef ESP32
#include "WiFi.h"
#elif defined(ESP8266)
#include "ESP8266WiFi.h"
#else
#error "Unsupported platform. By default, only ESP32 and ESP8266 are supported."
#endif

class TCPWiFiClient : public TCPClient {
   public:
    TCPWiFiClient()
        : m_Client(WiFiClient()) {
        // m_Client.setNoDelay(true);
    }
    TCPWiFiClient(WiFiClient m_Client)
        : m_Client(m_Client) {
        // m_Client.setNoDelay(true);
    }

    ~TCPWiFiClient() {
        disconnect();
    }

    bool connect(const String &host, const uint16_t &port) override {
        if (!WiFi.isConnected()) {
            return false;
        }
        return m_Client.connect(host.c_str(), port);
    }

    size_t write(uint8_t *data, size_t len) override {
        if (connected()) return m_Client.write(data, len);
        return 0;
    }

    int read(uint8_t *buffer, size_t len) override {
        if (available()) return m_Client.read(buffer, len);
        return -1;
    }

    int available() override {
        return m_Client.available();
    }

    int connected() override {
        return m_Client.connected();
    }

    IPAddress remoteIP() override {
        return m_Client.remoteIP();
    }

    uint16_t remotePort() override {
        return m_Client.remotePort();
    }

    void disconnect() override {
        m_Client.stop();
    }

   private:
    WiFiClient m_Client;
};

#endif