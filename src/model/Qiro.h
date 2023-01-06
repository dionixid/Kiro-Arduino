#ifndef QIRO_H
#define QIRO_H

#include "../Any/Any.h"
#include "Prayer.h"
#include "Surah.h"

struct Qiro : public Object {
    Prayer::Name name;
    uint16_t durationMinutes;
    std::vector<Surah> surahList;
    uint8_t volume;

    Qiro(const bool& IsValid = true)
        : m_IsValid(IsValid) {}

    Qiro(
        const Prayer::Name& name, const uint16_t& durationMinutes, const std::vector<Surah>& surahList,
        const uint8_t& volume
    )
        : name(name),
          durationMinutes(durationMinutes),
          surahList(surahList),
          volume(volume),
          m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isNumber() || !tokens[1].isNumber() || !tokens[2].isArray() || !tokens[3].isNumber()) {
            m_IsValid = false;
            return;
        }

        name            = static_cast<Prayer::Name>(tokens[0].toInt());
        durationMinutes = tokens[1].toInt();
        volume          = tokens[3].toInt();

        if (name < Prayer::Fajr || name > Prayer::Isha) {
            m_IsValid = false;
            return;
        }

        Array array = tokens[2];
        surahList.clear();

        for (auto& e : array) {
            if (!e.isObject()) {
                m_IsValid = false;
                return;
            }

            Surah surah = e;
            if (!surah) {
                m_IsValid = false;
                return;
            }

            surahList.push_back(surah);
        }
    }

    String toString() const override {
        return stringifyMembers(static_cast<uint8_t>(name), durationMinutes, Array::of(surahList), volume);
    }

    String serialize() const override {
        return serializeMembers(static_cast<uint8_t>(name), durationMinutes, Array::of(surahList), volume);
    }

    bool equals(const Object& other) const override {
        const Qiro& otherQiro = static_cast<const Qiro&>(other);
        return name == otherQiro.name && durationMinutes == otherQiro.durationMinutes
               && surahList == otherQiro.surahList && volume == otherQiro.volume;
    }

    size_t size() const override {
        return 4;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    Object* clone() const override {
        return new Qiro(*this);
    }

   private:
    bool m_IsValid;
};

#endif
