#ifndef PRAYER_GROUP_H
#define PRAYER_GROUP_H

#include "../Any/Any.h"
#include "Prayer.h"

struct PrayerGroup : public Object {
    Prayer fajr;
    Prayer dhuhr;
    Prayer asr;
    Prayer maghrib;
    Prayer isha;

    PrayerGroup(const bool& IsValid = true)
        : m_IsValid(IsValid) {}

    PrayerGroup(const Prayer& fajr, const Prayer& dhuhr, const Prayer& asr, const Prayer& maghrib, const Prayer& isha)
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

        if (!tokens[0].isObject() || !tokens[1].isObject() || !tokens[2].isObject() || !tokens[3].isObject()
            || !tokens[4].isObject()) {
            m_IsValid = false;
            return;
        }

        fajr    = tokens[0];
        dhuhr   = tokens[1];
        asr     = tokens[2];
        maghrib = tokens[3];
        isha    = tokens[4];

        if (!fajr || !dhuhr || !asr || !maghrib || !isha) {
            m_IsValid = false;
        }
    }

    String toString() const override {
        return stringifyMembers(fajr, dhuhr, asr, maghrib, isha);
    }

    String serialize() const override {
        return serializeMembers(fajr, dhuhr, asr, maghrib, isha);
    }

    bool equals(const Object& other) const override {
        const PrayerGroup& otherGroup = static_cast<const PrayerGroup&>(other);
        return fajr == otherGroup.fajr && dhuhr == otherGroup.dhuhr && asr == otherGroup.asr
               && maghrib == otherGroup.maghrib && isha == otherGroup.isha;
    }

    size_t size() const override {
        return 5;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    Object* clone() const override {
        return new PrayerGroup(*this);
    }

   private:
    bool m_IsValid;
};

#endif
