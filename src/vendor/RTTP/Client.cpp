#include "Client.h"

namespace RTTP {

/**
 * @brief Create a RTTP Client instance.
 *
 */
Client::Client(const String& host, const uint16_t& port, const String& name, const String& id)
    : m_Id(id),
      m_Name(name),
      m_Host(host),
      m_Port(port) {}

Client::~Client() {
    // There is no need to call leave() here because the destructor of WSClient will do it for us.
}

/**
 * @brief Join to global channel.
 * The global channel is a channel that is used to get information about the server.
 *
 */
void Client::join() {
    join("", "");
}

/**
 * @brief Join to a channel.
 * The channel  name is directly used as a URI path.
 * So it must be a valid URI path.
 * The channel name is converted to lower case.
 * Only the following characters are allowed: [a-z], [0-9], [-], [_].
 * Any other invalid characters are removed.
 *
 * @param channel is the channel to join to.
 * @param secret is the secret to authenticate with.
 * @param keepJoin is a flag that indicates if the client should keep trying to join the channel if the authentication fails.
 */
void Client::join(const String& channel, const String& secret, const bool& keepJoin) {
    if (m_Host.isEmpty() || m_Port == 0 || channel.isEmpty() || !isValidChannelName(channel)) {
        return;
    }

    m_Channel.name = channel;

    m_Client.onOpen([this, secret](WSClient& client) {
        String serialized = Auth(m_Id, m_Name, secret).serialize();
        client.sendBinary((uint8_t*)serialized.c_str(), serialized.length());

        if (m_OnJoinHandler) {
            m_OnJoinHandler();
        }
    });

    m_Client.onBinaryMessage([this](WSClient& c, const uint8_t* data, size_t size) {
        String text;
        text.concat((char*)data, size);
        m_IsRegistered = text.equals("auth-ok");
        
        if (!m_IsRegistered && !m_KeepJoinOnAuthFailed) {
            leave();
        }

        if (m_OnAuthHandler) {
            m_OnAuthHandler(m_IsRegistered);
        }
    });

    m_Client.onTextMessage([this](WSClient& c, const String& textMessage) {
        Message message = Any::parse(textMessage);

        if (!message) {
            return;
        }

        if (message.topic == RTTP::CHANNELS_TOPIC) {
            if (message.senderId != RTTP::SERVER_ID) {
                return;
            }

            Array channels = Any::parse(message.payload);
            m_Channels.clear();

            for (Channel channel : channels) {
                if (channel) {
                    m_Channels.push_back(channel);
                }

                if (channel.name == m_Channel.name) {
                    m_Channel = channel;
                }
            }

            if (m_OnChannelsUpdatedHandler) {
                m_OnChannelsUpdatedHandler();
            }

            return;
        }

        if (message.topic == RTTP::SUBSCRIBERS_TOPIC) {
            if (message.senderId != RTTP::SERVER_ID) {
                return;
            }

            Array subscribers = Any::parse(message.payload);
            m_Subscribers.clear();

            for (Subscriber subscriber : subscribers) {
                if (subscriber) {
                    m_Subscribers.push_back(subscriber);
                }
            }

            if (m_OnSubscribersUpdatedHandler) {
                m_OnSubscribersUpdatedHandler();
            }

            return;
        }

        for (auto& handler : m_MessageHandlers) {
            if (handler.first == message.topic || handler.first == RTTP::ALL_TOPICS) {
                handler.second(message);
            }
        }
    });

    m_Client.onClose([this](WSClient& client, const WSClient::CloseReason& code, const String& reason) {
        m_IsRegistered = false;
        
        if (m_OnLeaveHandler) {
            m_OnLeaveHandler();
        }
    });
    
    String url = "ws://" + m_Host + ":" + String(m_Port) + "/rttp/" + channel;
    url.toLowerCase();
    m_Client.begin(url);
}

/**
 * @brief Leave the channel.
 *
 */
void Client::leave() {
    m_Client.close();
}

/**
 * @brief Send a message to a recipient.
 *
 * @param recipientId is the recipient id.
 * @param topic is the topic of the message.
 * @param payload is the payload of the message.
 */
void Client::send(
    const String& recipientId, const String& topic, const Message::Action& action, const Any& payload
) {
    if (!m_IsRegistered) {
        return;
    }

    if (!m_Channel.hasTopic(topic)) {
        return;
    }

    m_Client.sendText(Message(m_Id, recipientId, topic, action, payload).serialize());
}

/**
 * @brief Publish a message to all clients.
 *
 * @param topic is the topic of the message.
 * @param payload is the payload of the message.
 */
void Client::publish(const String& topic, const Message::Action& action, const Any& payload) {
    if (!m_IsRegistered) {
        return;
    }

    if (!m_Channel.hasTopic(topic)) {
        return;
    }

    m_Client.sendText(Message(m_Id, RTTP::ALL_RECIPIENTS, topic, action, payload).serialize());
}

/**
 * @brief Register a message handler for a topic.
 *
 * @param topic is the topic to register the handler for.
 * @param handler is the handler to register.
 */
void Client::on(const String& topic, const MessageHandler& handler) {
    m_MessageHandlers[topic] = handler;
}

/**
 * @brief Unregister a message handler for a topic.
 *
 * @param topic is the topic to unregister the handler for.
 */
void Client::off(const String& topic) {
    m_MessageHandlers.erase(topic);
}

/**
 * @brief Register a handler for when the client is authenticated.
 * 
 * @param handler is the handler to register.
 */
void Client::onAuth(const AuthHandler& handler) {
    m_OnAuthHandler = handler;
}

/**
 * @brief Register a handler for when the client joins a channel.
 * 
 * @param handler is the handler to register.
 */
void Client::onJoin(const EventHandler& handler) {
    m_OnJoinHandler = handler;
}

/**
 * @brief Register a handler for when the client leaves a channel.
 * 
 * @param handler is the handler to register.
 */
void Client::onLeave(const EventHandler& handler) {
    m_OnLeaveHandler = handler;
}

/**
 * @brief Register a handler for when the available channels are updated.
 *
 * @param handler is the handler to register.
 */
void Client::onChannelsUpdated(const EventHandler& handler) {
    m_OnChannelsUpdatedHandler = handler;
}

/**
 * @brief Register a handler for when the available subscribers are updated.
 *
 * @param handler is the handler to register.
 */
void Client::onSubscribersUpdated(const EventHandler& handler) {
    m_OnSubscribersUpdatedHandler = handler;
}

/**
 * @brief Set whether to keep trying to join even if authentication fails.
 * 
 * @param keepJoin is whether to keep the join request on authentication failure.
 */
void Client::setKeepJoinOnAuthFailed(const bool& keepJoin) {
    m_KeepJoinOnAuthFailed = keepJoin;
}

/**
 * @brief Get the list of available channels.
 *
 * @return Array is the list of available channels.
 */
std::vector<Channel> Client::getChannels() const {
    return m_Channels;
}

/**
 * @brief Get the list of available subscribers.
 *
 * @return Array is the list of available subscribers.
 */
std::vector<Subscriber> Client::getSubscribers() const {
    return m_Subscribers;
}

/**
 * @brief Check if a channel name is valid.
 * 
 * @param channel is the channel name to validate.
 * @return The validated channel name.
 */
bool Client::isValidChannelName(const String& channel) {
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

};