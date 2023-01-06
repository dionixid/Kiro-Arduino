#include "WSServer.h"

/**
 * @brief Create a WebSocket Server instance.
 *
 * @param port is the port to listen on.
 * @param maxClients is the maximum number of clients to accept.
 */
WSServer::WSServer(uint16_t port, uint8_t maxClients)
    : m_Server(std::make_shared<TCPWiFiServer>(port, maxClients)) {}

/**
 * @brief Create a WebSocket Server from a custom TCPServer instance.
 *
 * @param m_Server is the TCPServer instance to use.
 */
WSServer::WSServer(std::shared_ptr<TCPServer> server)
    : m_Server(server) {}

WSServer::~WSServer() {
#ifdef ESP32
    if (m_TaskHandler) {
        vTaskDelete(m_TaskHandler);
    }
#else
    Timer::unregisterEvent(m_EventId);
#endif
    end();
}

#ifdef ESP32
/**
 * @brief Start the WebSocket Server.
 *
 * @param stackSize is the stack size to use for the polling task.
 */
void WSServer::begin(const uint16_t& stackSize) {
    if (!m_Server) {
        return;
    }

    m_Server->begin();

    if (!m_TaskHandler) {
        xTaskCreate(_pollingTask, "serverTask", stackSize, this, 5, &m_TaskHandler);
    }
}
#else
/**
 * @brief Start the WebSocket Server.
 *
 */
void WSServer::begin() {
    if (!m_Server) {
        return;
    }
    m_Server->begin();

    Timer::unregisterEvent(m_EventId);
    m_EventId = Timer::registerEvent([this]() { 
        run();
    });
}
#endif

/**
 * @brief Stop the WebSocket Server.
 *
 */
void WSServer::end() {
    if (!m_Server) {
        return;
    }
    m_Server->end();
}

/**
 * @brief Get all clients connected to the server.
 *
 * @return a list of WSClient instances.
 */
std::vector<std::shared_ptr<WSClient>>& WSServer::getClients() {
    return m_Clients;
}

/**
 * @brief Set a handler to be called when a client connects to the server.
 *
 * @param path is the path to listen on.
 * @param handler is the handler to call.
 */
void WSServer::onConnection(const String& path, ConnectionHandler handler) {
    m_ConnectionHandlers[path] = handler;
}

/**
 * @brief Remove a connection handler.
 *
 * @param path is the path to remove the handler for.
 */
void WSServer::removeConnectionHandler(const String& path) {
    m_ConnectionHandlers.erase(path);
}

/**
 * @brief Close a client connection.
 *
 * @param id is the id of the client to close.
 */
void WSServer::close(String id) {
    for (int i = 0; i < m_Clients.size(); i++) {
        if (m_Clients[i]->id.equals(id)) {
            m_Clients[i]->close(WSClient::CloseReason::NormalClosure);
            m_Clients.erase(m_Clients.begin() + i);
            i--;
        }
    }
}

void WSServer::close(const std::shared_ptr<WSClient> client) {
    for (int i = 0; i < m_Clients.size(); i++) {
        if (m_Clients[i] == client) {
            m_Clients[i]->close(WSClient::CloseReason::NormalClosure);
            m_Clients.erase(m_Clients.begin() + i);
            i--;
            break;
        }
    }
}

/**
 * @brief Remove inactive clients.
 *
 */
void WSServer::_cleanup() {
    for (int i = 0; i < m_Clients.size(); i++) {
        if (!m_Clients[i]->isConnected()) {
            m_Clients.erase(m_Clients.begin() + i);
            i--;
        }
    }
}

/**
 * @brief Check if the server has any clients connected.
 *
 * @return true if there are clients connected. false otherwise.
 */
bool WSServer::hasClients() {
    return !m_Clients.empty();
}

/**
 * @brief Check if the server has a client with the given id.
 *
 * @param id is the id to check for.
 * @return true if the client exists. false otherwise.
 */
bool WSServer::hasClient(String id) {
    for (int i = 0; i < m_Clients.size(); i++) {
        if (m_Clients[i]->id == id) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Accept a new client connection.
 * If there is no client to accept, this function does nothing.
 *
 */
void WSServer::_accept() {
    if (!m_Server) {
        return;
    }

    std::shared_ptr<TCPClient> client = m_Server->accept();
    if (!client || !client->connected()) {
        return;
    }

    for (int i = 0; i < m_Clients.size(); i++) {
        if (m_Clients[i]->remoteIP() == client->remoteIP() && m_Clients[i]->remotePort() == client->remotePort()) {
            return;
        }
    }

    std::vector<String> requestHeaders;
    String line;

    while (client->available()) {
        line = client->readLine();
        line.trim();
        requestHeaders.push_back(line);
        yield();

        if (!line.length()) {
            break;
        }
    }

    Crypto::HandshakeServerResult result = Crypto::parseHandshakeRequest(requestHeaders);

    if (result.path.indexOf("?") != -1) {
        result.path = result.path.substring(0, result.path.indexOf("?"));
    }

    if (result.path != "/" && result.path.endsWith("/")) {
        result.path = result.path.substring(0, result.path.length() - 1);
    }

    if (!result.isValid || m_ConnectionHandlers.count(result.path) == 0) {
        client->end();
        return;
    }

    String response = "HTTP/1.1 101 Switching Protocols\r\n";
    response += "Connection: Upgrade\r\n";
    response += "Upgrade: websocket\r\n";
    response += "Sec-WebSocket-Version: 13\r\n";
    response += "Sec-WebSocket-Version: 13\r\n";
    response += "Sec-WebSocket-Accept: " + result.key + "\r\n\r\n";

    client->write(response);

    std::unique_ptr<WSClient> wsClient(new WSClient(std::forward<std::shared_ptr<TCPClient>>(client)));
    wsClient->id = Crypto::generateRandomId();
    wsClient->setUseMask(false);
    wsClient->m_CloseHandlerInternal = [this](WSClient* client) {
        for (int i = 0; i < m_Clients.size(); i++) {
            if (m_Clients[i].get() == client) {
                m_Clients.erase(m_Clients.begin() + i);
                i--;
                break;
            }
        }
    };

    m_Clients.push_back(std::move(wsClient));
    std::shared_ptr<WSClient> clientPtr = m_Clients.back();

    for (auto& callback : m_ConnectionHandlers) {
        if (callback.first == result.path) {
            callback.second(clientPtr);
            break;
        }
    }
}

/**
 * @brief Poll the server for new data.
 *
 */
void WSServer::run() {
    std::vector<std::shared_ptr<WSClient>> clients = m_Clients;
    for (int i = 0; i < clients.size(); i++) {
        clients[i]->poll();
    }

    if (millis() - m_LastAccept > 100) {
        m_LastAccept = millis();
        _accept();
    }

    if (millis() - m_LastCleanup > 1000) {
        m_LastCleanup = millis();
        _cleanup();
    }
}

#ifdef ESP32
void WSServer::_pollingTask(void* ptr) {
    WSServer* server = (WSServer*)ptr;
    while (true) {
        server->run();
        delay(2);
    }
}
#endif