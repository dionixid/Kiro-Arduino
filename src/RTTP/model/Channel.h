#ifndef RTTP_CHANNEL_H
#define RTTP_CHANNEL_H

#include "../../Any/Any.h"

namespace RTTP {

/**
 * @brief Channel is a data structure that holds the information of a channel.
 * This data structure is different from the Server::Channel data structure.
 * The Server::Channel data structure is used to store the information of a channel in the server.
 * Whereas, this data structure is used to send channel information to the client.
 *
 */
struct Channel : public Object {

    /**
     * @brief The name of the channel.
     * 
     */
    String name;

    /**
     * @brief The topics of the channel.
     * 
     */
    Array topics;

    Channel(const bool& isValid = false)
        : m_IsValid(isValid) {}

    Channel(const String& name, const Array& topics)
        : name(name),
          topics(topics),
          m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isString() || !tokens[1].isArray()) {
            m_IsValid = false;
            return;
        }

        name      = tokens[0].toString();
        topics    = tokens[1];
        m_IsValid = true;
    }

    String toString() const override {
        return stringifyMembers(name, topics);
    }

    String serialize() const override {
        return serializeMembers(name, topics);
    }

    bool equals(const Object& other) const override {
        const Channel& otherChannel = static_cast<const Channel&>(other);
        return name == otherChannel.name && topics == otherChannel.topics;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    size_t size() const override {
        return 2;
    }

    Object* clone() const override {
        return new Channel(*this);
    }

    bool hasTopic(const String& topic) const {
        for (size_t i = 0; i < topics.size(); i++) {
            if (topics[i].toString() == topic) {
                return true;
            }
        }

        return false;
    }

   private:
    bool m_IsValid;
};

};  // namespace RTTP

#endif