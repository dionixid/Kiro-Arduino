#ifndef RTTP_SERVER_H
#define RTTP_SERVER_H

#include <functional>
#include <map>

#include "../Any/Any.h"
#include "../Timer/Timer.h"
#include "../WebSocket/WSServer.h"
#include "model/Auth.h"
#include "model/Channel.h"
#include "model/Message.h"
#include "model/Subscriber.h"

namespace RTTP {

/**
 * @brief RTTP stands for Real Time Transport Protocol.
 * RTTP is a high level protocol that is run on top of WebSockets.
 * Unlike MQTT, RTTP does not retain messages on the server.
 * This behaviour enables RTTP servers to be used in an embedded environment.
 *
 */
class Server {
   public:
    class Channel {
       public:
        using AuthHandler    = std::function<bool(const Auth& auth)>;
        using AuthedHandler  = std::function<void(const Auth& auth)>;
        using MessageHandler = std::function<void(const Message& message)>;
        using ClientHandler  = std::function<void(const String& ip, const uint16_t& port, const uint8_t& count)>;

        Channel();
        Channel(const String& name);
        Channel& onAuth(const AuthHandler& handler);
        Channel& onAuthenticated(const AuthedHandler& handler);
        Channel& onJoin(const ClientHandler& handler);
        Channel& onLeave(const ClientHandler& handler);
        Channel& addTopic(const String& topic, const MessageHandler& handler = NULL);
        Channel& removeTopic(const String& topic);

        bool hasTopic(const String& topic) const;

        friend class Server;

       private:
        String m_Name;
        AuthHandler m_AuthHandler          = NULL;
        AuthedHandler m_AuthenticatedHandler = NULL;
        ClientHandler m_JoinHandler        = NULL;
        ClientHandler m_LeaveHandler       = NULL;
        std::map<String, MessageHandler> m_Handlers;

        /**
         * @brief This handler is called when a new topic is added or removed from the channel.
         * This is used by the server to update the client's topic list.
         *
         */
        std::function<void()> m_OnTopicsUpdateHandler = NULL;
    };

    Server(const uint16_t& port);
    ~Server();

    void begin();
    void end();

    void send(
        const String& recipientId, const String& channel, const String& topic, const Message::Action& action,
        const Any& payload
    );
    void publish(const String& channel, const String& topic, const Message::Action& action, const Any& payload);

    Channel& createChannel(const String& channel);
    Channel* getChannel(const String& channel);
    void removeChannel(const String& channel);

    std::map<String, Channel>& getChannels();
    std::vector<Subscriber> getSubscribers(const String& channel);

    void onJoin(const Channel::ClientHandler& handler);
    void onLeave(const Channel::ClientHandler& handler);

    uint8_t getClientCount(const String& channel);

   private:
    WSServer m_Server;
    TimeHandle_t m_HeartBeatIntervalId;
    TimeHandle_t m_ChannelUpdateIntervalId;
    std::map<String, Channel> m_Channels;
    bool m_IsChannelUpdateRequired = false;

    Channel::ClientHandler m_JoinHandler  = NULL;
    Channel::ClientHandler m_LeaveHandler = NULL;

    void send(
        const String& senderId, const String& recipientId, const String& channel, const String& topic,
        const Message::Action& action, const Any& payload
    );
    void publish(
        const String& senderId, const String& channel, const String& topic, const Message::Action& action,
        const Any& payload
    );

    void sendChannels();
    void sendChannels(std::shared_ptr<WSClient> client);
    void sendSubscribers(const String& channel);

    bool isValidChannelName(const String& channel);
};

};  // namespace RTTP

#endif