#ifndef QIRO_H
#define QIRO_H

#include "../Any/Any.h"
#include "Prayer.h"

struct Qiro : public Object {
    Prayer::Name name;
    uint32_t volume;
    uint32_t durationMinutes;
    std::vector<uint16_t> surahIds;

    Qiro(const bool& IsValid = true)
        : m_IsValid(IsValid) {}

    Qiro(Prayer::Name name, uint32_t volume, uint32_t durationMinutes, std::vector<uint16_t> surahIds)
        : name(name),
          volume(volume),
          durationMinutes(durationMinutes),
          surahIds(surahIds),
          m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isNumber() || !tokens[1].isNumber() || !tokens[2].isNumber() || !tokens[3].isArray()) {
            m_IsValid = false;
            return;
        }

        name            = static_cast<Prayer::Name>(tokens[0].toInt());
        volume          = tokens[1].toInt();
        durationMinutes = tokens[2].toInt();

        if (name < Prayer::Fajr || name > Prayer::Isha) {
            m_IsValid = false;
            return;
        }

        Array list = tokens[3];
        surahIds.clear();

        for (auto& value : list) {
            if (!value.isNumber()) {
                m_IsValid = false;
                return;
            } else {
                surahIds.push_back(value.toInt());
            }
        }
    }

    String toString() const override {
        return stringifyMembers(static_cast<uint8_t>(name), volume, durationMinutes, surahIds);
    }

    String serialize() const override {
        return serializeMembers(static_cast<uint8_t>(name), volume, durationMinutes, surahIds);
    }

    bool equals(const Object& other) const override {
        const Qiro& otherQiro = static_cast<const Qiro&>(other);
        return name == otherQiro.name && volume == otherQiro.volume && durationMinutes == otherQiro.durationMinutes
               && surahIds == otherQiro.surahIds;
    }

    size_t size() const override {
        return 4;
    }

    bool IsValid() const override {
        return m_IsValid;
    }

    Object* clone() const override {
        return new Qiro(*this);
    }

   private:
    bool m_IsValid;
};

#endif