#ifndef TIMER_H
#define TIMER_H

#if defined(ESP32) || defined(ESP8266)
#include <functional>
#endif

#include <Arduino.h>

#include "../ArrayList/ArrayList.h"

class Timer;

class CountDownTimer {
   public:
#if defined(ESP32) || defined(ESP8266)
    using CountDownHandler = std::function<void()>;
#else
    using CountDownHandler = void (*)();
#endif

    CountDownTimer();
    CountDownTimer(const uint32_t& duration, const CountDownHandler& handler);
    CountDownTimer(const uint32_t& duration, const bool& isContinuous, const CountDownHandler& handler);
    ~CountDownTimer();

    void start();
    void cancel();
    void reset();

    void setContinuous(const bool& isContinuous);
    void setDuration(const uint32_t& duration);
    void setHandler(const CountDownHandler& handler);

    bool isContinuous();
    uint32_t getDuration();
    bool isRunning();

    CountDownTimer(const CountDownTimer& other)            = delete;
    CountDownTimer& operator=(const CountDownTimer& other) = delete;

    friend class Timer;

   private:
    uint32_t m_Duration;
    uint32_t m_LastMillis;
    CountDownHandler m_Handler;

    bool m_IsContinuous = false;
    bool m_IsRunning    = false;

    static ArrayList<CountDownTimer*> timers;
};

struct TimeHandle_t {
    uint32_t id = 0;

    TimeHandle_t();
    TimeHandle_t(uint32_t id);

    TimeHandle_t& operator=(const uint32_t& newId);
    operator uint32_t() const;

    bool operator==(const TimeHandle_t& other) const;
    bool operator!=(const TimeHandle_t& other) const;

    bool operator==(const uint32_t& other) const;
    bool operator!=(const uint32_t& other) const;
    
    TimeHandle_t& operator++();
    TimeHandle_t& operator--();

    TimeHandle_t operator++(int);
    TimeHandle_t operator--(int);
};

class Timer {
   public:
#if defined(ESP32) || defined(ESP8266)
    using TimeHandler = std::function<void()>;
#else
    using TimeHandler      = void (*)();
#endif

    Timer()                              = delete;
    Timer(const Timer& other)            = delete;
    Timer& operator=(const Timer& other) = delete;

    static TimeHandle_t registerEvent(const TimeHandler& handler);
    static TimeHandle_t setInterval(const TimeHandler& handler, const uint32_t& interval);
    static TimeHandle_t setInterval(const uint32_t& interval, const TimeHandler& handler);
    static TimeHandle_t setTimeout(const TimeHandler& handler, const uint32_t& timeout);
    static TimeHandle_t setTimeout(const uint32_t& timeout, const TimeHandler& handler);

    static void unregisterEvent(const TimeHandle_t& eventId);
    static void clearInterval(const TimeHandle_t& eventId);
    static void clearTimeout(const TimeHandle_t& eventId);

    static void run();

   private:
    struct RegularEvent {
        TimeHandle_t id;
        boolean passed;
        TimeHandler handler;

        RegularEvent()
            : id(0),
              passed(false),
              handler(NULL) {}
        RegularEvent(const TimeHandle_t& id, const TimeHandler& handler)
            : id(id),
              passed(false),
              handler(handler) {}
    };

    struct TimeEvent {
        TimeHandle_t id;
        uint32_t interval;
        uint32_t counter;
        boolean passed;
        TimeHandler handler;

        TimeEvent()
            : id(0),
              interval(0),
              counter(0),
              passed(false),
              handler(NULL) {}
        TimeEvent(const TimeHandle_t& id, const uint32_t& interval, const uint32_t& counter, const TimeHandler& handler)
            : id(id),
              interval(interval),
              counter(counter),
              passed(false),
              handler(handler) {}
    };

    static ArrayList<RegularEvent> m_RegularEvents;
    static ArrayList<TimeEvent> m_IntervalEvents;
    static ArrayList<TimeEvent> m_TimeoutEvents;
};

#endif
