#ifndef RTTP_CLIENT_H
#define RTTP_CLIENT_H

#include <functional>
#include <map>
#include <memory>

#include "../Any/Any.h"
#include "../WebSocket/WSClient.h"
#include "model/Auth.h"
#include "model/Channel.h"
#include "model/Message.h"
#include "model/Subscriber.h"

namespace RTTP {

class Server;

/**
 * @brief RTTP stands for Real Time Transport Protocol.
 * RTTP is a high level protocol that is run on top of WebSockets.
 * Unlike MQTT, RTTP does not retain messages on the server.
 * This behaviour enables RTTP servers to be used in an embedded environment.
 *
 */
class Client {
   public:
    using MessageHandler = std::function<void(const Message& message)>;
    using AuthHandler    = std::function<void(const bool& success)>;
    using EventHandler   = std::function<void()>;

    Client(const String& host, const uint16_t& port, const String& name, const String& id);
    ~Client();

    void join();
    void join(const String& channel, const String& secret, const bool& keepJoin = false);
    void leave();

    void send(const String& recipientId, const String& topic, const Message::Action& action, const Any& payload);
    void publish(const String& topic, const Message::Action& action, const Any& payload);

    void on(const String& topic, const MessageHandler& handler);
    void off(const String& topic);

    void onAuth(const AuthHandler& handler);
    void onJoin(const EventHandler& handler);
    void onLeave(const EventHandler& handler);

    void onChannelsUpdated(const EventHandler& handler);
    void onSubscribersUpdated(const EventHandler& handler);

    void setKeepJoinOnAuthFailed(const bool& keepJoin);

    std::vector<Channel> getChannels() const;
    std::vector<Subscriber> getSubscribers() const;

    friend class RTTPServer;

   private:
    WSClient m_Client;
    String m_Id;
    String m_Name;
    String m_Host;
    uint16_t m_Port;

    bool m_IsRegistered         = false;
    bool m_KeepJoinOnAuthFailed = false;

    Channel m_Channel;
    std::vector<Channel> m_Channels;
    std::vector<Subscriber> m_Subscribers;
    std::map<String, MessageHandler> m_MessageHandlers;

    AuthHandler m_OnAuthHandler   = NULL;
    EventHandler m_OnJoinHandler  = NULL;
    EventHandler m_OnLeaveHandler = NULL;

    EventHandler m_OnChannelsUpdatedHandler    = NULL;
    EventHandler m_OnSubscribersUpdatedHandler = NULL;

    bool isValidChannelName(const String& channel);
};

};  // namespace RTTP

#endif