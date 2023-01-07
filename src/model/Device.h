#ifndef DEVICE_H
#define DEVICE_H

#include "../vendor/Any/Any.h"

struct Device : public Object {
    String id;
    String name;
    String version;

    Device(const bool& isValid = true)
        : m_IsValid(isValid) {}

    Device(const String& id, const String &name, const String& version)
        : id(id), name(name), version(version), m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isString() || !tokens[1].isString() || !tokens[2].isString()) {
            m_IsValid = false;
            return;
        }

        id = tokens[0].toString();
        name = tokens[1].toString();
        version = tokens[2].toString();
    }

    String toString() const override {
        return stringifyMembers(id, name, version);
    }

    String serialize() const override {
        return serializeMembers(id, name, version);
    }

    bool equals(const Object& other) const override {
        const Device& otherDevice = static_cast<const Device&>(other);
        return id == otherDevice.id && name == otherDevice.name && version == otherDevice.version;
    }

    size_t size() const override {
        return 3;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    Object* clone() const override {
        return new Device(*this);
    }

   private:
    bool m_IsValid;
};

#endif