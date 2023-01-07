#ifndef RTTP_SUBSCRIBER_H
#define RTTP_SUBSCRIBER_H

#include "../../Any/Any.h"

namespace RTTP {

/**
 * @brief Subscriber is a data structure that holds the information of a subscriber.
 * 
 */
struct Subscriber : public Object {

    /**
     * @brief The id of the subscriber.
     * 
     */
    String id;

    /**
     * @brief The name of the subscriber.
     * 
     */
    String name;

    Subscriber(const bool& isValid = false)
        : m_IsValid(isValid) {}

    Subscriber(const String& id, const String& name)
        : id(id),
          name(name),
          m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isString() || !tokens[1].isString()) {
            m_IsValid = false;
            return;
        }

        id        = tokens[0].toString();
        name      = tokens[1].toString();
        m_IsValid = true;
    }

    String toString() const override {
        return stringifyMembers(id, name);
    }

    String serialize() const override {
        return serializeMembers(id, name);
    }

    bool equals(const Object& other) const override {
        const Subscriber& otherSubscriber = static_cast<const Subscriber&>(other);
        return id == otherSubscriber.id && name == otherSubscriber.name;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    size_t size() const override {
        return 2;
    }

    Object* clone() const override {
        return new Subscriber(*this);
    }

   private:
    bool m_IsValid;
};

};  // namespace RTTP

#endif