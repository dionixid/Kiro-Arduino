#ifndef DEVICE_H
#define DEVICE_H

#include "../Any/Any.h"

struct Device : public Object {
    String id;
    String name;

    Device(const bool& isValid = true)
        : m_IsValid(isValid) {}

    Device(String id, String name)
        : id(id), name(name), m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isString() || !tokens[1].isString()) {
            m_IsValid = false;
            return;
        }

        id = tokens[0].toString();
        name = tokens[1].toString();
    }

    String toString() const override {
        return stringifyMembers(id, name);
    }

    String serialize() const override {
        return serializeMembers(id, name);
    }

    bool equals(const Object& other) const override {
        const Device& otherDevice = static_cast<const Device&>(other);
        return id == otherDevice.id && name == otherDevice.name;
    }

    size_t size() const override {
        return 2;
    }

    bool IsValid() const override {
        return m_IsValid;
    }

    Object* clone() const override {
        return new Device(*this);
    }

   private:
    bool m_IsValid;
};

#endif