#ifndef RTP_AUTH_H
#define RTP_AUTH_H

#include "../../Any/Any.h"

namespace RTTP {

/**
 * @brief Auth is a data structure that holds the authentication information of a client.
 * 
 */
struct Auth : public Object {

    /**
     * @brief The id of the client.
     * 
     */
    String id;

    /**
     * @brief The name of the client.
     * 
     */
    String name;

    /**
     * @brief The secret key of the client.
     * 
     */
    String secret;

    Auth(const bool& isValid = false)
        : m_IsValid(isValid) {}

    Auth(const String& id, const String& name, const String& secret)
        : id(id),
          name(name),
          secret(secret),
          m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isString() || !tokens[1].isString() || !tokens[2].isString()) {
            m_IsValid = false;
            return;
        }

        id        = tokens[0].toString();
        name      = tokens[1].toString();
        secret    = tokens[2].toString();
        m_IsValid = true;
    }

    String toString() const override {
        return stringifyMembers(id, name, secret);
    }

    String serialize() const override {
        return serializeMembers(id, name, secret);
    }

    bool equals(const Object& other) const override {
        const Auth& otherAuth = static_cast<const Auth&>(other);
        return id == otherAuth.id && name == otherAuth.name && secret == otherAuth.secret;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    size_t size() const override {
        return 3;
    }

    Object* clone() const override {
        return new Auth(*this);
    }

   private:
    bool m_IsValid = false;
};

};  // namespace RTTP

#endif