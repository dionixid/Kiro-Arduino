#include "Server.h"

namespace RTTP {

/*-----------------------------------------------------------
 * CHANNEL CLASS IMPLEMENTATION
 *----------------------------------------------------------*/

/**
 * @brief Create a channel instance.
 *
 */
Server::Channel::Channel()
    : m_AuthHandler(NULL) {}

/**
 * @brief Create a channel instance.
 *
 * @param name is the name of the channel.
 */
Server::Channel::Channel(const String& name)
    : m_Name(name),
      m_AuthHandler(NULL) {}

/**
 * @brief Set the authentication handler.
 * The authentication handler is called when a client tries to connect to the channel.
 * The handler should return true if the client is allowed to connect.
 *
 * @param handler is the handler to set.
 * @return A reference to the channel instance.
 */
Server::Channel& Server::Channel::onAuth(const AuthHandler& handler) {
    m_AuthHandler = handler;
    return *this;
}

/**
 * @brief Set a handler to be called when a client is authenticated.
 * The handler is called after the authentication handler.
 * 
 * @param handler is the handler to call when a client is authenticated.
 * @return A reference to the channel instance.
 */
Server::Channel& Server::Channel::onAuthenticated(const AuthedHandler& handler) {
    m_AuthenticatedHandler = handler;
    return *this;
}

/**
 * @brief Add a topic to the channel.
 * The topic is a string that is used to identify a message.
 * The handler is called when a message with the topic is received.
 *
 * @param topic is the topic name.
 * @param handler is the handler to call when a message with the topic is received.
 * @return A reference to the channel instance.
 */
Server::Channel& Server::Channel::addTopic(const String& topic, const MessageHandler& handler) {
    m_Handlers[topic] = handler;
    if (m_OnTopicsUpdateHandler) {
        m_OnTopicsUpdateHandler();
    }
    return *this;
}

/**
 * @brief Set a handler to be called when a client joins the channel.
 *
 * @param handler is the handler to call when a client joins the channel.
 * @return A reference to the channel instance.
 */
Server::Channel& Server::Channel::onJoin(const ClientHandler& handler) {
    m_JoinHandler = handler;
    return *this;
}

/**
 * @brief Set a handler to be called when a client leaves the channel.
 *
 * @param handler is the handler to call when a client leaves the channel.
 * @return A reference to the channel instance.
 */
Server::Channel& Server::Channel::onLeave(const ClientHandler& handler) {
    m_LeaveHandler = handler;
    return *this;
}

/**
 * @brief Remove a topic from the channel.
 *
 * @param topic is the topic name.
 */
Server::Channel& Server::Channel::removeTopic(const String& topic) {
    m_Handlers.erase(topic);
    if (m_OnTopicsUpdateHandler) {
        m_OnTopicsUpdateHandler();
    }
    return *this;
}

/**
 * @brief Check if the channel has a topic.
 *
 * @param topic is the topic name.
 * @return true if the channel has the topic. false otherwise.
 */
bool Server::Channel::hasTopic(const String& topic) const {
    return m_Handlers.count(topic) > 0;
}

/*-----------------------------------------------------------
 * RTTP SERVER CLASS IMPLEMENTATION
 *----------------------------------------------------------*/

Server::Server(const uint16_t& port)
    : m_Server(port) {}

Server::~Server() {
    end();
}

/**
 * @brief Start the RTTP server.
 *
 */
void Server::begin() {
    Timer::clearInterval(m_HeartBeatIntervalId);
    Timer::clearInterval(m_ChannelUpdateIntervalId);

    m_HeartBeatIntervalId = Timer::setInterval(5000, [this]() {
        std::vector<std::shared_ptr<WSClient>> clients = m_Server.getClients();
        for (int i = 0; i < clients.size(); i++) {
            if (clients[i]->isAlive) {
                clients[i]->isAlive = false;
                clients[i]->ping();
            } else {
                clients[i]->close();
            }
        }
    });

    m_ChannelUpdateIntervalId = Timer::setInterval(1000, [this]() {
        if (m_IsChannelUpdateRequired) {
            m_IsChannelUpdateRequired = false;
            sendChannels();
        }
    });

    m_Server.onConnection("/rttp", [this](std::shared_ptr<WSClient> client) {
        client->isAlive = true;
        sendChannels(client);

        if (m_JoinHandler) {
            m_JoinHandler(client->remoteIP().toString(), client->remotePort(), getClientCount(client->channel));
        }

        client->onPong([this](WSClient& client, const String& message) { client.isAlive = true; });

        client->onClose([this](WSClient& client, const WSClient::CloseReason& code, const String& message) {
            if (m_LeaveHandler) {
                m_LeaveHandler(client.remoteIP().toString(), client.remotePort(), getClientCount(client.channel));
            }
        });
    });

    m_Server.begin();
}

/**
 * @brief Stop the RTTP server.
 *
 */
void Server::end() {
    m_Server.end();
}

/**
 * @brief Send a message to a specific client.
 *
 * @param senderId is the id of the sender.
 * @param recipientId is the id of the recipient.
 * @param topic is the topic of the message.
 * @param action is the action of the message.
 * @param payload is the payload of the message.
 */
void Server::send(
    const String& recipientId, const String& channel, const String& topic, const Message::Action& action,
    const Any& payload
) {
    send(RTTP::SERVER_ID, recipientId, channel, topic, action, payload);
}

/**
 * @brief Publish a message to all clients.
 *
 * @param senderId is the id of the sender.
 * @param topic is the topic of the message.
 * @param payload is the payload of the message.
 */
void Server::publish(const String& channel, const String& topic, const Message::Action& action, const Any& payload) {
    publish(RTTP::SERVER_ID, channel, topic, action, payload);
}

/**
 * @brief Create a channel.
 * The channel  name is directly used as a URI path.
 * So it must be a valid URI path.
 * Only the following characters are allowed: [a-z], [A-Z], [0-9], [-], [_].
 * Any other invalid characters are removed.
 *
 * NOTE:
 * The channel name is converted to lower case.
 * So the channel name "MyChannel" is the same as "mychannel".
 *
 * @param channel is the name of the channel.
 * @return A reference to the newly created channel.
 */
Server::Channel& Server::createChannel(const String& channel) {
    if (channel.isEmpty() || !isValidChannelName(channel)) {
        return m_Channels["__invalid__"] = Channel("__invalid__");
    }

    m_Channels[channel]                         = Channel(channel);
    m_Channels[channel].m_OnTopicsUpdateHandler = [this]() { m_IsChannelUpdateRequired = true; };
    m_IsChannelUpdateRequired                   = true;

    String path = "/rttp/" + channel;
    path.toLowerCase();

    m_Server.onConnection(path, [this, channel](std::shared_ptr<WSClient> client) {
        client->isAlive = true;
        client->channel = channel;
        sendChannels(client);

        if (m_Channels[channel].m_JoinHandler) {
            m_Channels[channel].m_JoinHandler(
                client->remoteIP().toString(), client->remotePort(), getClientCount(channel) + 1
            );
        }

        client->onBinaryMessage([this](WSClient& c, const uint8_t* data, const size_t& size) {
            String text;
            text.concat((char*)data, size);
            Auth auth = Any::parse(text);

            if (!auth || !m_Channels[c.channel].m_AuthHandler) {
                return;
            }

            if (!m_Channels[c.channel].m_AuthHandler(auth)) {
                c.sendBinary((uint8_t*)"auth-failed", 11);
                return;
            }

            c.id   = auth.id;
            c.name = auth.name;
            c.sendBinary((uint8_t*)"auth-ok", 7);
            sendSubscribers(c.channel);
            
            if (m_Channels[c.channel].m_AuthenticatedHandler) {
                m_Channels[c.channel].m_AuthenticatedHandler(auth);
            }
        });

        client->onTextMessage([this](WSClient& c, const String& textMessage) {
            Message message = Any::parse(textMessage);

            if (!message || message.senderId != c.id || message.action == Message::Unknown) {
                return;
            }

            if (m_Channels[c.channel].m_Handlers.count(message.topic) == 0 && message.topic != RTTP::ALL_TOPICS) {
                return;
            }

            if (message.recipientId == RTTP::ALL_RECIPIENTS) {
                publish(message.senderId, c.channel, message.topic, message.action, message.payload);
            } else if (message.recipientId != RTTP::SERVER_ID) {
                send(message.senderId, message.recipientId, c.channel, message.topic, message.action, message.payload);
            }

            if (message.topic == RTTP::ALL_TOPICS) {
                for (auto& handler : m_Channels[c.channel].m_Handlers) {
                    if (handler.second) {
                        handler.second(message);
                    }
                }
                return;
            }

            if (m_Channels[c.channel].m_Handlers[message.topic]) {
                m_Channels[c.channel].m_Handlers[message.topic](message);
            }
        });

        client->onPong([this](WSClient& c, const String& message) { c.isAlive = true; });

        client->onClose([this](WSClient& c, const WSClient::CloseReason& code, const String& message) {
            if (m_Channels.count(c.channel) == 0) {
                return;
            }

            sendSubscribers(c.channel);

            if (m_Channels[c.channel].m_LeaveHandler) {
                m_Channels[c.channel].m_LeaveHandler(
                    c.remoteIP().toString(), c.remotePort(), getClientCount(c.channel)
                );
            }
        });
    });

    return m_Channels[channel];
}

/**
 * @brief Get a channel.
 *
 * @param channel is the name of the channel.
 * @return A pointer to the channel or NULL if the channel does not exist.
 */
Server::Channel* Server::getChannel(const String& channel) {
    if (m_Channels.count(channel) == 0) {
        return NULL;
    }

    return &m_Channels[channel];
}

/**
 * @brief Remove a channel.
 *
 * @param channel is the name of the channel.
 */
void Server::removeChannel(const String& channel) {
    m_Channels.erase(channel);
    m_Server.removeConnectionHandler(channel);
}

/**
 * @brief Get all channels.
 *
 * @return A map of channels.
 */
std::map<String, Server::Channel>& Server::getChannels() {
    return m_Channels;
}

/**
 * @brief Get all subscribers of a channel.
 *
 * @param channel is the name of the channel.
 * @return A vector of subscribers.
 */
std::vector<Subscriber> Server::getSubscribers(const String& channel) {
    std::vector<std::shared_ptr<WSClient>> clients = m_Server.getClients();
    std::vector<Subscriber> subscribers;

    for (int i = 0; i < clients.size(); i++) {
        if (clients[i]->channel == channel) {
            subscribers.push_back(Subscriber(clients[i]->id, clients[i]->name));
        }
    }

    return subscribers;
}

/**
 * @brief Set a handler that is called when a client joins to global channel.
 * This handler is different from the channel's onJoin handler.
 * When a client joins to a specific channel, the channel's onJoin handler is called.
 *
 * @param handler is the handler to be called.
 */
void Server::onJoin(const Channel::ClientHandler& handler) {
    m_JoinHandler = handler;
}

/**
 * @brief Set a handler that is called when a client leaves global channel.
 * This handler is different from the channel's onLeave handler.
 * When a client leaves a specific channel, the channel's onLeave handler is called.
 *
 * @param handler is the handler to be called.
 */
void Server::onLeave(const Channel::ClientHandler& handler) {
    m_LeaveHandler = handler;
}

uint8_t Server::getClientCount(const String& channel) {
    std::vector<std::shared_ptr<WSClient>> clients = m_Server.getClients();
    uint8_t count                                  = 0;

    for (int i = 0; i < clients.size(); i++) {
        if (clients[i]->channel == channel) {
            count++;
        }
    }

    return count;
}

/**
 * @brief Send a message to a specific client.
 *
 * @param senderId is the id of the sender.
 * @param recipientId is the id of the recipient.
 * @param topic is the topic of the message.
 * @param action is the action of the message.
 * @param payload is the payload of the message.
 */
void Server::send(
    const String& senderId, const String& recipientId, const String& channel, const String& topic,
    const Message::Action& action, const Any& payload
) {
    if (m_Channels.count(channel) == 0) {
        return;
    }

    if (!m_Channels[channel].hasTopic(topic)) {
        return;
    }

    std::vector<std::shared_ptr<WSClient>> clients = m_Server.getClients();

    for (int i = 0; i < clients.size(); i++) {
        if (clients[i]->channel == channel && clients[i]->id == recipientId) {
            clients[i]->sendText(Message(senderId, recipientId, topic, action, payload).serialize());
            break;
        }
    }
}

/**
 * @brief Publish a message to all clients.
 *
 * @param senderId is the id of the sender.
 * @param senderId is the id of the sender.
 * @param topic is the topic of the message.
 * @param payload is the payload of the message.
 */
void Server::publish(
    const String& senderId, const String& channel, const String& topic, const Message::Action& action,
    const Any& payload
) {
    if (m_Channels.count(channel) == 0) {
        return;
    }

    if (!m_Channels[channel].hasTopic(topic)) {
        return;
    }

    std::vector<std::shared_ptr<WSClient>> clients = m_Server.getClients();

    for (int i = 0; i < clients.size(); i++) {
        clients[i]->sendText(Message(senderId, clients[i]->id, topic, action, payload).serialize());
    }
}

/**
 * @brief Send channel information to all clients.
 *
 */
void Server::sendChannels() {
    Array channels;

    for (auto& channel : m_Channels) {
        Array topicNames;
        for (auto& topic : channel.second.m_Handlers) {
            topicNames.push(topic.first);
        }
        channels.push(RTTP::Channel(channel.first, topicNames));
    }

    std::vector<std::shared_ptr<WSClient>> clients = m_Server.getClients();
    for (int i = 0; i < clients.size(); i++) {
        clients[i]->sendText(
            Message(RTTP::SERVER_ID, clients[i]->id, RTTP::CHANNELS_TOPIC, Message::Info, channels).serialize()
        );
    }
}

/**
 * @brief Send channel information to a client.
 *
 * @param client is the client to send the channel information.
 */
void Server::sendChannels(std::shared_ptr<WSClient> client) {
    Array channels;

    for (auto channel : m_Channels) {
        Array topicNames;
        for (auto topic : channel.second.m_Handlers) {
            topicNames.push(topic.first);
        }
        channels.push(RTTP::Channel(channel.first, topicNames));
    }

    client->sendText(Message(RTTP::SERVER_ID, client->id, RTTP::CHANNELS_TOPIC, Message::Info, channels).serialize());
}

/**
 * @brief Send subscriber information to all clients.
 *
 * @param channel is the name of the channel.
 */
void Server::sendSubscribers(const String& channel) {
    std::vector<std::shared_ptr<WSClient>> clients = m_Server.getClients();
    Array subscribers;

    for (int i = 0; i < clients.size(); i++) {
        if (clients[i]->channel == channel) {
            subscribers.push(Subscriber(clients[i]->id, clients[i]->name));
        }
    }

    for (int i = 0; i < clients.size(); i++) {
        clients[i]->sendText(
            Message(RTTP::SERVER_ID, clients[i]->id, RTTP::SUBSCRIBERS_TOPIC, Message::Info, subscribers).serialize()
        );
    }
}

/**
 * @brief Check if a channel name is valid.
 *
 * @param channel is the channel name to validate.
 * @return True if the channel name is valid, false otherwise.
 */
bool Server::isValidChannelName(const String& channel) {
    for (auto& c : channel) {
        if (c >= 'a' && c <= 'z') {
            continue;
        }
        if (c >= 'A' && c <= 'Z') {
            continue;
        }
        if (c >= '0' && c <= '9') {
            continue;
        }
        if (c == '-' || c == '_') {
            continue;
        }
        return false;
    }

    return true;
}

};  // namespace RTTP