#ifndef SURAH_PROPERTIES_H
#define SURAH_PROPERTIES_H

#include "../vendor/Any/Any.h"

struct SurahProperties : public Object {
    uint16_t id;
    String name;
    uint8_t volume;
    uint32_t durationSeconds;

    SurahProperties(const bool& isValid = true)
        : m_IsValid(isValid) {}

    SurahProperties(const uint16_t& id, const String& name, const uint8_t& volume, const uint32_t& durationSeconds)
        : id(id),
          name(name),
          volume(volume),
          durationSeconds(durationSeconds),
          m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isNumber() || !tokens[1].isString() || !tokens[2].isNumber() || !tokens[3].isNumber()) {
            m_IsValid = false;
            return;
        }

        id              = tokens[0].toInt();
        name            = tokens[1].toString();
        volume          = tokens[2].toInt();
        durationSeconds = tokens[3].toInt();
    }

    String toString() const {
        return stringifyMembers(id, name, volume, durationSeconds);
    }

    String serialize() const {
        return serializeMembers(id, name, volume, durationSeconds);
    }

    bool equals(const Object& other) const {
        const SurahProperties& otherSurahProperties = static_cast<const SurahProperties&>(other);
        return id == otherSurahProperties.id && name == otherSurahProperties.name
               && volume == otherSurahProperties.volume && durationSeconds == otherSurahProperties.durationSeconds;
    }

    size_t size() const {
        return 4;
    }

    bool isValid() const {
        return m_IsValid;
    }

    Object* clone() const {
        return new SurahProperties(*this);
    }

   private:
    bool m_IsValid;
};

#endif