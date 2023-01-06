#ifndef SURAH_H
#define SURAH_H

#include "../Any/Any.h"

struct Surah : public Object {
    uint16_t id;
    uint8_t volume;

    Surah(const bool& isValid = true)
        : m_IsValid(isValid) {}

    Surah(const uint16_t& id, const uint8_t& volume)
        : id(id), volume(volume), m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isNumber() || !tokens[1].isNumber()) {
            m_IsValid = false;
            return;
        }

        id = tokens[0].toInt();
        volume = tokens[1].toInt();
    }

    String toString() const override {
        return stringifyMembers(id, volume);
    }

    String serialize() const override {
        return serializeMembers(id, volume);
    }

    bool equals(const Object& other) const override {
        const Surah& otherSurah = static_cast<const Surah&>(other);
        return id == otherSurah.id && volume == otherSurah.volume;
    }

    size_t size() const override {
        return 2;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    Object* clone() const override {
        return new Surah(*this);
    }
    
   private:
    bool m_IsValid;
};

#endif