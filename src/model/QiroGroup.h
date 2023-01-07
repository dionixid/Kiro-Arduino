#ifndef QIRO_GROUP_H
#define QIRO_GROUP_H

#include "../vendor/Any/Any.h"
#include "DayOfWeek.h"
#include "Qiro.h"

struct QiroGroup : public Object {
    DayOfWeek dayOfWeek;
    Qiro fajr;
    Qiro dhuhr;
    Qiro asr;
    Qiro maghrib;
    Qiro isha;

    QiroGroup(const bool& IsValid = true)
        : m_IsValid(IsValid) {}

    QiroGroup(
        const DayOfWeek& dayOfWeek, const Qiro& fajr, const Qiro& dhuhr, const Qiro& asr, const Qiro& maghrib,
        const Qiro& isha
    )
        : dayOfWeek(dayOfWeek),
          fajr(fajr),
          dhuhr(dhuhr),
          asr(asr),
          maghrib(maghrib),
          isha(isha) {}

    Qiro& getQiro(const Prayer::Name& name) {
        switch (name) {
            case Prayer::Name::Fajr:
                return fajr;
            case Prayer::Name::Dhuhr:
                return dhuhr;
            case Prayer::Name::Asr:
                return asr;
            case Prayer::Name::Maghrib:
                return maghrib;
            case Prayer::Name::Isha:
                return isha;
            default:
                return fajr;
        }
    }

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isNumber() || !tokens[1].isObject() || !tokens[2].isObject() || !tokens[3].isObject()
            || !tokens[4].isObject() || !tokens[5].isObject()) {
            m_IsValid = false;
            return;
        }

        dayOfWeek = static_cast<DayOfWeek>(tokens[0].toInt());

        if (dayOfWeek < DayOfWeek::Monday || dayOfWeek > DayOfWeek::Sunday) {
            m_IsValid = false;
            return;
        }

        fajr    = tokens[1];
        dhuhr   = tokens[2];
        asr     = tokens[3];
        maghrib = tokens[4];
        isha    = tokens[5];

        if (!fajr || !dhuhr || !asr || !maghrib || !isha) {
            m_IsValid = false;
        }
    }

    String toString() const override {
        return stringifyMembers(static_cast<uint8_t>(dayOfWeek), fajr, dhuhr, asr, maghrib, isha);
    }

    String serialize() const override {
        return serializeMembers(static_cast<uint8_t>(dayOfWeek), fajr, dhuhr, asr, maghrib, isha);
    }

    bool equals(const Object& other) const override {
        const QiroGroup& otherGroup = static_cast<const QiroGroup&>(other);
        return dayOfWeek == otherGroup.dayOfWeek && fajr == otherGroup.fajr && dhuhr == otherGroup.dhuhr
               && asr == otherGroup.asr && maghrib == otherGroup.maghrib && isha == otherGroup.isha;
    }

    size_t size() const override {
        return 6;
    }

    bool isValid() const {
        return m_IsValid;
    }

    Object* clone() const override {
        return new QiroGroup(*this);
    }

   private:
    bool m_IsValid;
};

#endif
