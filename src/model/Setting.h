#ifndef SETTING_H
#define SETTING_H

#include "../Any/Any.h"

struct Setting : public Object {
    enum class Type : uint8_t {
        Info,
        String,
        Float,
        Integer,
        Date,
        Time,
        WiFi,
        Latitude,
        Longitude,
        Elevation
    };

    String id;
    Type type;
    String label;
    Any value;
    bool isConfidential;

    Setting(const bool& isValid = true)
        : m_IsValid(isValid) {}

    Setting(const String& id, const Type& type, const String& label, const Any& value, const bool& isConfidential = false)
        : id(id),
          type(type),
          label(label),
          value(value),
          isConfidential(isConfidential),
          m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isString() || !tokens[1].isNumber() || !tokens[2].isString() || !tokens[4].isBool()) {
            m_IsValid = false;
            return;
        }

        id             = tokens[0].toString();
        type           = static_cast<Type>(tokens[1].toInt());
        label          = tokens[2].toString();
        value          = tokens[3];
        isConfidential = tokens[4].toBool();

        if (type < Type::Info || type > Type::Elevation) {
            m_IsValid = false;
        }
    }

    String toString() const override {
        return stringifyMembers(id, static_cast<uint8_t>(type), label, value, isConfidential);
    }

    String serialize() const override {
        return serializeMembers(id, static_cast<uint8_t>(type), label, value, isConfidential);
    }

    bool equals(const Object& other) const override {
        const Setting& otherSetting = static_cast<const Setting&>(other);
        return id == otherSetting.id && type == otherSetting.type && label == otherSetting.label
               && value == otherSetting.value && isConfidential == otherSetting.isConfidential;
    }

    size_t size() const override {
        return 5;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    Object* clone() const override {
        return new Setting(*this);
    }

   private:
    bool m_IsValid;
};

#endif