#ifndef PRAYER_GROUP_H
#define PRAYER_GROUP_H

#include "../vendor/Any/Any.h"
#include "Prayer.h"
#include "PrayerTimeOffset.h"

struct PrayerGroup : public Object {
    Prayer fajr;
    Prayer dhuhr;
    Prayer asr;
    Prayer maghrib;
    Prayer isha;

    PrayerGroup(const bool& IsValid = true)
        : fajr(Prayer(Prayer::Fajr, 0, 0)),
          dhuhr(Prayer(Prayer::Dhuhr, 0, 0)),
          asr(Prayer(Prayer::Asr, 0, 0)),
          maghrib(Prayer(Prayer::Maghrib, 0, 0)),
          isha(Prayer(Prayer::Isha, 0, 0)),
          m_IsValid(IsValid) {}

    PrayerGroup(const Prayer& fajr, const Prayer& dhuhr, const Prayer& asr, const Prayer& maghrib, const Prayer& isha)
        : fajr(fajr),
          dhuhr(dhuhr),
          asr(asr),
          maghrib(maghrib),
          isha(isha) {}

    Prayer& getActivePrayer(const uint32_t secondOfDay) {
        if (secondOfDay >= fajr.getActualTime() && secondOfDay < dhuhr.getActualTime()) {
            return fajr;
        } else if (secondOfDay >= dhuhr.getActualTime() && secondOfDay < asr.getActualTime()) {
            return dhuhr;
        } else if (secondOfDay >= asr.getActualTime() && secondOfDay < maghrib.getActualTime()) {
            return asr;
        } else if (secondOfDay >= maghrib.getActualTime() && secondOfDay < isha.getActualTime()) {
            return maghrib;
        } else {
            return isha;
        }
    }

    PrayerTimeOffset toPrayerTimeOffset() const {
        return PrayerTimeOffset(fajr.offset, dhuhr.offset, asr.offset, maghrib.offset, isha.offset);
    }

    void setOffset(const PrayerTimeOffset& offset) {
        fajr.offset    = offset.fajr;
        dhuhr.offset   = offset.dhuhr;
        asr.offset     = offset.asr;
        maghrib.offset = offset.maghrib;
        isha.offset    = offset.isha;
    }

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
