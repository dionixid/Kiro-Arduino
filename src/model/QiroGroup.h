#ifndef QIRO_GROUP_H
#define QIRO_GROUP_H

#include "../Any/Any.h"
#include "Qiro.h"

struct QiroGroup : public Object {
    uint8_t dayOfWeek;
    Qiro fajr;
    Qiro dhuhr;
    Qiro asr;
    Qiro maghrib;
    Qiro isha;

    QiroGroup(const bool& IsValid = true)
        : m_IsValid(IsValid) {}

    QiroGroup(
        uint8_t dayOfWeek, const Qiro& fajr, const Qiro& dhuhr, const Qiro& asr, const Qiro& maghrib,
        const Qiro& isha
    )
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

        fajr    = Any::parse(tokens[0].toString());
        dhuhr   = Any::parse(tokens[1].toString());
        asr     = Any::parse(tokens[2].toString());
        maghrib = Any::parse(tokens[3].toString());
        isha    = Any::parse(tokens[4].toString());

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
        const QiroGroup& otherGroup = static_cast<const QiroGroup&>(other);
        return fajr == otherGroup.fajr && dhuhr == otherGroup.dhuhr && asr == otherGroup.asr
               && maghrib == otherGroup.maghrib && isha == otherGroup.isha;
    }

    size_t size() const override {
        return 5;
    }
    bool IsValid() const override {
        return m_IsValid;
    }

   private:
    bool m_IsValid;
};

#endif
