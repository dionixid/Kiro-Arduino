#ifndef PRAYER_H
#define PRAYER_H

#include "../vendor/Any/Any.h"

struct Prayer : public Object {
    enum Name : u_int8_t {
        Fajr,
        Dhuhr,
        Asr,
        Maghrib,
        Isha
    };

    Name name;
    uint32_t time;
    int16_t offset;

    Prayer(const bool& IsValid = true)
        : m_IsValid(IsValid) {}

    Prayer(Name name, uint32_t time, int16_t offset)
        : name(name),
          time(time),
          offset(offset),
          m_IsValid(true) {}

    uint32_t getActualTime() const {
        return time + (offset * 60) - (time % 60);
    }

    String getNameString() {
        switch (name) {
            case Fajr:
                return "Subuh";
            case Dhuhr:
                return "Dzuhur";
            case Asr:
                return "Ashar";
            case Maghrib:
                return "Maghrib";
            case Isha:
                return "Isya";
            default:
                return "Unknown";
        }
    }

    String getFormattedTime() {
        uint8_t hour   = getActualTime() / 3600;
        uint8_t minute = (getActualTime() % 3600) / 60;

        String result = hour < 10 ? "0" : "";
        result += hour;
        result += ":";
        result += minute < 10 ? "0" : "";
        result += minute;
        return result;
    }

    void constructor(const std::vector<Any>& tokens) override {
        if (tokens.size() != size()) {
            m_IsValid = false;
            return;
        }

        if (!tokens[0].isNumber() || !tokens[1].isNumber() || !tokens[2].isNumber()) {
            m_IsValid = false;
            return;
        }

        name   = static_cast<Name>(tokens[0].toInt());
        time   = tokens[1].toInt();
        offset = tokens[2].toInt();

        if (name < Fajr || name > Isha) {
            m_IsValid = false;
            return;
        }
    }

    String toString() const override {
        return stringifyMembers(static_cast<uint8_t>(name), time, offset);
    }

    String serialize() const override {
        return serializeMembers(static_cast<uint8_t>(name), time, offset);
    }

    bool equals(const Object& other) const override {
        const Prayer& otherPrayer = static_cast<const Prayer&>(other);
        return name == otherPrayer.name && time == otherPrayer.time && offset == otherPrayer.offset;
    }

    size_t size() const override {
        return 3;
    }

    bool isValid() const override {
        return m_IsValid;
    }

    Object* clone() const override {
        return new Prayer(*this);
    }

   private:
    bool m_IsValid;
};

#endif