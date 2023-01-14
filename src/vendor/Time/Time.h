#ifndef TIME_H
#define TIME_H

#include <Arduino.h>

#include "../Timer/Timer.h"

#ifdef ESP32
#include <WiFi.h>
#include <WiFiUdp.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#endif

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#endif

namespace UniTime {

extern String solarDayNames[7];
extern String pasaranDayNames[5];
extern String gregorianMonthNames[12];
extern String hijriMonthNames[12];
extern uint16_t gregorianDayCount[12];
extern uint16_t hijriDayCount[12];

struct HijriDate;
struct PrayerTime;
struct SolarTime;

struct Timezone {
    int8_t hour;
    int8_t minute;

    Timezone();
    Timezone(const double& offset);
    Timezone(const int8_t& hour, const int8_t& minute);

    bool operator==(const Timezone& time) const;
    bool operator!=(const Timezone& time) const;

    double toDouble() const;
    String toString(const bool& rfcPattern = false) const;

    static Timezone fromSeconds(const int32_t& seconds);
    static Timezone fromMinutes(const int16_t& minutes);
};

struct Date {
    uint16_t year;
    uint8_t month;
    uint8_t date;

    Date();
    Date(const uint16_t& year, const uint8_t& month, const uint8_t& date);

    bool operator==(const Date& time) const;
    bool operator!=(const Date& time) const;

    bool isLeapYear() const;

    virtual double daysOfTheYear() const;
    virtual String format(const String& fmt) const;
};

struct Time {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    Time();
    Time(const double& time);
    Time(const uint8_t& hour, const uint8_t& minute, const uint8_t& second);

    double secondsOfTheDay() const;
    double minutesOfTheDay() const;
    double hoursOfTheDay() const;

    bool operator==(const Time& time) const;
    bool operator!=(const Time& time) const;

    virtual String format(const String& fmt) const;

    static Time fromSecondsOfTheDay(const uint32_t& seconds);
    static Time fromMinutesOfTheDay(const uint32_t& minutes);
};

struct DateTime : public Date, public Time {
    Timezone timezone;
    uint8_t dayOfWeek;
    uint8_t pasaran;

    DateTime();
    DateTime(const uint32_t& timestamp, const Timezone& timezone = Timezone());
    DateTime(const uint16_t& year, const uint8_t& month, const uint8_t& date, const uint8_t& hour = 0, const uint8_t& minute = 0, const uint8_t& second = 0, const Timezone& timezone = Timezone());

    double daysOfTheYear() const override;
    uint32_t timestamp() const;

    bool operator==(const DateTime& time) const;
    bool operator!=(const DateTime& time) const;

    String format(const String& fmt) const override;
    SolarTime toSolarTime(const double& latitude, const double& longitude) const;
    PrayerTime toPrayerTime(const double& latitude, const double& longitude, const double& elevation) const;
    Date toDate() const;
    Time toTime() const;
    HijriDate toHijri() const;
};

struct HijriDate {
    uint16_t year;
    uint8_t month;
    uint8_t date;

    HijriDate();
    HijriDate(const uint32_t& timestamp);
    HijriDate(const DateTime& gregorianDate);
    HijriDate(const uint16_t& year, const uint8_t& month, const uint8_t& date);

    bool operator==(const HijriDate& time) const;
    bool operator!=(const HijriDate& time) const;

    String format(const String& fmt) const;
};

struct SolarTime {
    double latitude;
    double longitude;
    double equationOfTime;
    double declination;
    double rightAscension;
    double hourAngle;
    double zenithAngle;
    double altitudeAngle;
    double azimuthAngle;
    double offset;
    double hourAngleRiseSet;

    Time time;
    Time noon;
    Time sunrise;
    Time sunset;

    SolarTime();
    SolarTime(const DateTime& gregorianDate, const double& latitude, const double& longitude);

    double offsetMinute() const;
    double offsetSecond() const;
    double hourAngleFromAltitude(const double& altitude) const;
    
    bool operator==(const SolarTime& time) const;
    bool operator!=(const SolarTime& time) const;
};

struct PrayerTime {
    Time fajr;
    Time sunrise;
    Time dhuhr;
    Time asr;
    Time maghrib;
    Time isha;

    PrayerTime();
    PrayerTime(const SolarTime& solarTime, const double& elevation);
    PrayerTime(const uint8_t& fajr, const uint8_t& sunrise, const uint8_t& duha, const uint8_t& dhuhr, const uint8_t& asr, const uint8_t& maghrib, const uint8_t& isha);
    
    bool operator==(const PrayerTime& time) const;
    bool operator!=(const PrayerTime& time) const;
};

class DS3231 {
   public:
    enum SqwPinMode {
        DS3231_OFF = 0x01,
        DS3231_SquareWave1Hz = 0x00,
        DS3231_SquareWave1kHz = 0x08,
        DS3231_SquareWave4kHz = 0x10,
        DS3231_SquareWave8kHz = 0x18
    };

    DS3231();
    void begin();
    void adjust(const uint32_t& unix);
    bool lostPower();
    uint32_t timestamp();
    DateTime now();
    float temperature();
    SqwPinMode getSqwPinMode();
    void setSqwPinMode(const SqwPinMode& mode);

   private:
    static const int DS3231_ADDRESS;
    static const int DS3231_CONTROL;
    static const int DS3231_STATUSREG;
    static const int DS3231_TEMPERATUREREG;
};

class UniTime {
   public:
#if defined(ESP8266) || defined(ESP32)
    using VoidCallback = std::function<void()>;
#else
    using VoidCallback = void (*)();
#endif

    void enableNTP();
    void enableRTC();
    bool adjust(const uint32_t& unix);
    bool adjust(const DateTime& dateTime, bool isUtc = false);
    double julianDate() const;
    uint32_t timestamp() const;
    uint64_t timestampMillis() const;
    uint32_t secondsOfTheDay() const;
    uint16_t minutesOfTheDay() const;
    DateTime now() const;
    DateTime tomorrow() const;
    DateTime rtc();
    Timezone timezone() const;
    SolarTime solarTime(const double& latitude, const double& longitude) const;
    PrayerTime prayerTime(const double& latitude, const double& longitude, const double& elevation) const;
    void setTimezone(const int8_t& timezone);
    bool isSet() const;
    bool isRtcEnabled() const;
    bool isNtpEnabled() const;
    bool isValidTimestamp(const uint32_t& timestamp) const;
    void onUpdate(VoidCallback callback);
    void onMinuteChange(VoidCallback callback);
    void onHourChange(VoidCallback callback);
    void onDayChange(VoidCallback callback);

    template <typename... T>
    void setDayNames(T&&... dayNames) {
        String names[] = {dayNames...};
        for (int i = 0; i < sizeof...(dayNames); i++) {
            if (i >= 7) break;
            ::UniTime::solarDayNames[i] = names[i];
        }
    }

    template <typename... T>
    void setMonthNames(T&&... monthNames) {
        String names[] = {monthNames...};
        for (int i = 0; i < sizeof...(monthNames); i++) {
            if (i >= 12) break;
            ::UniTime::gregorianMonthNames[i] = names[i];
        }
    }

   private:
#if defined(ESP32) || defined(ESP8266)
    static WiFiUDP _udp;
#endif
    static ::UniTime::DS3231 _rtc;
    static int32_t _timezone;
    static bool _isRtcEnabled;
    static bool _isNtpEnabled;
    static bool _isTimeUpdateRegistered;
    static bool _isChangeDetectorRegistered;
    static uint32_t _lastUpdate;
    static uint32_t _lastTimestamp;
    static uint32_t _lastMinute;
    static uint32_t _lastHour;
    static uint32_t _lastDay;
    static VoidCallback _onUpdate;
    static VoidCallback _onMinuteChanged;
    static VoidCallback _onHourChanged;
    static VoidCallback _onDayChanged;
    static void _timeUpdater();
    static void _changeDetector();
};

};  // namespace UniTime

extern UniTime::UniTime Time;

#endif
