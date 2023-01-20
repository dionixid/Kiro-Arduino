#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <Arduino.h>

#include <functional>

#include "../Time/Time.h"

class TimeSimulator {
   public:
    using DateTimeHandler = std::function<void(UniTime::DateTime&)>;

    enum Divider : uint8_t {
        NO_DIVIDER  = 1,
        DIVIDER_2   = 2,
        DIVIDER_4   = 4,
        DIVIDER_5   = 5,
        DIVIDER_8   = 8,
        DIVIDER_10  = 10,
        DIVIDER_20  = 20,
        DIVIDER_25  = 25,
        DIVIDER_40  = 40,
        DIVIDER_50  = 50,
        DIVIDER_100 = 100,
        DIVIDER_200 = 200,
        DIVIDER_250 = 250
    };

    TimeSimulator();
    ~TimeSimulator();

    void start();
    void resume();
    void pause();
    void stop();

    void adjust(const UniTime::DateTime& dateTime);
    void setDivider(const Divider& divider);

    UniTime::DateTime now();

    void jumpTo(const UniTime::Time& time, const bool & forceSameDay = true);
    void jumpTo(const uint8_t& hour, const uint8_t& minute, const uint8_t& second, const bool & forceSameDay = true);

    void forwardSeconds(const uint32_t& seconds);
    void forwardMinutes(const uint32_t& minutes);
    void forwardHours(const uint32_t& hours);
    void forwardDays(const uint32_t& days);

    void backwardSeconds(const uint32_t& seconds);
    void backwardMinutes(const uint32_t& minutes);
    void backwardHours(const uint32_t& hours);
    void backwardDays(const uint32_t& days);

    void onSecondChanged(const DateTimeHandler& handler);
    void onMinuteChanged(const DateTimeHandler& handler);
    void onHourChanged(const DateTimeHandler& handler);
    void onDayChanged(const DateTimeHandler& handler);

   private:
    DateTimeHandler m_SecondChangedHandler;
    DateTimeHandler m_MinuteChangedHandler;
    DateTimeHandler m_HourChangedHandler;
    DateTimeHandler m_DayChangedHandler;

    UniTime::Timezone m_Timezone;
    uint32_t m_TimestampOrigin;
    uint32_t m_Timestamp;
    Divider m_Divider;
    bool m_IsRunning;

    uint32_t m_LastMinute;
    uint32_t m_LastHour;
    uint32_t m_LastDay;

    TaskHandle_t m_TaskHandle;
};

#endif