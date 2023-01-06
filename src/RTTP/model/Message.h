#ifndef RTTP_MESSAGE_H
#define RTTP_MESSAGE_H

#include "../../Any/Any.h"

namespace RTTP {

/**
 * @brief Message is the data structure used to send messages between clients and the server.
 *
 */
struct Message : public Object {
    enum Action : uint8_t {
        Get     = 0xF0,
        Set     = 0xF1,
        Update  = 0xF2,
        Delete  = 0xF3,
        Info    = 0xF4,
        Unknown = 0xFF
    };

    /**
     * @brief The id of the sender.
     * The sender id MUST be set by the client.
     * If the sender id is not set, the message will be ignored.
     * 
     */
    String senderId;

    /**
     * @brief The id of the recipient.
     * The server will forward the message to the recipient if the recipient is online.
     * Otherwise, the message will be ignored.
     */
    String recipientId;

    /**
     * @brief The topic of the message.
     * The topic is set by the server and is used to identify the topic of the message.
     * If the topic is not listed in the server, the message will be ignored.
     * 
     */
    String topic;

    /**
     * @brief The action of the message.
     * Can be one of the following:
     * - Get
     * - Set
     * - Update
     * - Delete
     * - Info
     */
    Action action;

    /**
     * @brief The payload of the message.
     * The payload can be any type of data including null and a custom 
     * data structure which is inherited from Object.
     * 
     */
    Any payload;

    Message(const bool& isValid = false)
        : m_IsValid(isValid) {}

    Message(
        const String& senderId, const String& recipientId, const String& topic, const Action& action, const Any& payload
    )
        : senderId(senderId),
          recipientId(recipientId),
          topic(topic),
          action(action),
          payload(payload),
          m_IsValid(true) {}

    void constructor(const  std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isString() || !tokens[1].isString() || !tokens[2].isString() || !tokens[3].isNumber()) {
            m_IsValid = false;
            return;
        }

        senderId    = tokens[0].toString();
        recipientId = tokens[1].toString();
        topic       = tokens[2].toString();
        action      = _intToAction(tokens[3].toInt());
        payload     = tokens[4];
        m_IsValid   = true;
    }

    String toString() const override {
        return stringifyMembers(senderId, recipientId, topic, (uint8_t)action, payload);
    }

    String serialize() const override {
        return serializeMembers(senderId, recipientId, topic, (uint8_t)action, payload);
    }

    bool equals(const Object& other) const override {
        const Message& otherPayload = static_cast<const Message&>(other);
        return senderId == otherPayload.senderId && recipientId == otherPayload.recipientId
               && topic == otherPayload.topic && action == otherPayload.action && payload == otherPayload.payload;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    size_t size() const override {
        return 5;
    }

    Object* clone() const override {
        return new Message(*this);
    }

   private:
    bool m_IsValid = false;

    Action _intToAction(const int& action) {
        switch (action) {
            case 0xF0:
                return Action::Get;
            case 0xF1:
                return Action::Set;
            case 0xF2:
                return Action::Update;
            case 0xF3:
                return Action::Delete;
            default:
                return Action::Unknown;
        }
    }
};

};  // namespace RTTP

namespace RTTP {
const String SERVER_ID         = "RTTP_SERVER";
const String CHANNELS_TOPIC    = "_channels";
const String SUBSCRIBERS_TOPIC = "_subscribers";
const String ALL_RECIPIENTS    = "*";
const String ALL_TOPICS        = "*";
};  // namespace RTTP

#endif