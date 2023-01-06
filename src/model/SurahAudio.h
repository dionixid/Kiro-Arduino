#ifndef SURAH_AUDIO_H
#define SURAH_AUDIO_H

#include "../Any/Any.h"

struct SurahAudio : public Object {
    uint16_t id;
    uint8_t volume;
    bool isPaused;
    bool isPlaying;

    SurahAudio(const bool& isValid = true)
        : m_IsValid(isValid) {}

    SurahAudio(const uint16_t& id, const uint8_t& volume, const bool& isPaused, const bool& isPlaying)
        : id(id),
          volume(volume),
          isPaused(isPaused),
          isPlaying(isPlaying),
          m_IsValid(true) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isNumber() || !tokens[1].isNumber() || !tokens[2].isBool() || !tokens[3].isBool()) {
            m_IsValid = false;
            return;
        }

        id        = tokens[0].toInt();
        volume    = tokens[1].toInt();
        isPaused  = tokens[2].toBool();
        isPlaying = tokens[3].toBool();
    }

    String toString() const override {
        return stringifyMembers(id, volume, isPaused, isPlaying);
    }

    String serialize() const override {
        return serializeMembers(id, volume, isPaused, isPlaying);
    }

    bool equals(const Object& other) const override {
        const SurahAudio& otherSurahAudio = static_cast<const SurahAudio&>(other);
        return id == otherSurahAudio.id && volume == otherSurahAudio.volume && isPaused == otherSurahAudio.isPaused
               && isPlaying == otherSurahAudio.isPlaying;
    }

    size_t size() const override {
        return 4;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    Object* clone() const override {
        return new SurahAudio(*this);
    }

   private:
    bool m_IsValid;
};

#endif