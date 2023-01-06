#ifndef PRAYER_TIME_OFFSET_H
#define PRAYER_TIME_OFFSET_H

#include "../Any/Any.h"

struct PrayerTimeOffset : public Object {
    int16_t fajr;
    int16_t dhuhr;
    int16_t asr;
    int16_t maghrib;
    int16_t isha;

    PrayerTimeOffset(const bool& IsValid = true)
        : m_IsValid(IsValid) {}

    PrayerTimeOffset(const int16_t& fajr, const int16_t& dhuhr, const int16_t& asr, const int16_t& maghrib, const int16_t& isha)
        : fajr(fajr),
          dhuhr(dhuhr),
          asr(asr),
          maghrib(maghrib),
          isha(isha) {}

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isNumber() || !tokens[1].isNumber() || !tokens[2].isNumber() || !tokens[3].isNumber()
            || !tokens[4].isNumber()) {
            m_IsValid = false;
            return;
        }

        fajr    = tokens[0];
        dhuhr   = tokens[1];
        asr     = tokens[2];
        maghrib = tokens[3];
        isha    = tokens[4];
    }

    String toString() const override {
        return stringifyMembers(fajr, dhuhr, asr, maghrib, isha);
    }

    String serialize() const override {
        return serializeMembers(fajr, dhuhr, asr, maghrib, isha);
    }

    bool equals(const Object& other) const override {
        const PrayerTimeOffset& otherGroup = static_cast<const PrayerTimeOffset&>(other);
        return fajr == otherGroup.fajr && dhuhr == otherGroup.dhuhr && asr == otherGroup.asr
               && maghrib == otherGroup.maghrib && isha == otherGroup.isha;
    }

    size_t size() const override {
        return 5;
    }

    bool isValid() const {
        return m_IsValid;
    }

    Object* clone() const override {
        return new PrayerTimeOffset(*this);
    }

   private:
    bool m_IsValid;
};

#endif