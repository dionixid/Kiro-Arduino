#include "TimeSimulator.h"

TimeSimulator::TimeSimulator()
    : m_SecondChangedHandler(NULL),
      m_MinuteChangedHandler(NULL),
      m_HourChangedHandler(NULL),
      m_DayChangedHandler(NULL),
      m_TaskHandle(NULL),
      m_Divider(Divider::NO_DIVIDER),
      m_IsRunning(false),
      m_TimestampOrigin(0),
      m_Timestamp(0),
      m_LastMinute(0),
      m_LastHour(0),
      m_LastDay(0) {
    xTaskCreate(
        [](void* param) {
            TimeSimulator* self = (TimeSimulator*)param;
            while (true) {
                if (!self->m_IsRunning) {
                    delay(1000);
                    continue;
                }

                self->m_Timestamp++;
                UniTime::DateTime dateTime(self->m_Timestamp, self->m_Timezone);

                if (self->m_SecondChangedHandler) {
                    self->m_SecondChangedHandler(dateTime);
                }

                if (self->m_MinuteChangedHandler && dateTime.minute != self->m_LastMinute) {
                    self->m_LastMinute = dateTime.minute;
                    self->m_MinuteChangedHandler(dateTime);
                }

                if (self->m_HourChangedHandler && dateTime.hour != self->m_LastHour) {
                    self->m_LastHour = dateTime.hour;
                    self->m_HourChangedHandler(dateTime);
                }

                if (self->m_DayChangedHandler && dateTime.date != self->m_LastDay) {
                    self->m_LastDay = dateTime.date;
                    self->m_DayChangedHandler(dateTime);
                }

                delay(1000 / self->m_Divider);
            }
        },
        "TimeSimulator", 8192, this, 5, &m_TaskHandle
    );
}

TimeSimulator::~TimeSimulator() {
    if (m_TaskHandle) {
        vTaskDelete(m_TaskHandle);
    }
}

void TimeSimulator::start() {
    m_Timestamp = m_TimestampOrigin;
    m_IsRunning = true;
}

void TimeSimulator::resume() {
    m_IsRunning = true;
}

void TimeSimulator::pause() {
    m_IsRunning = false;
}

void TimeSimulator::stop() {
    m_IsRunning = false;
    m_Timestamp = m_TimestampOrigin;
}

void TimeSimulator::adjust(const UniTime::DateTime& dateTime) {
    m_Timezone        = dateTime.timezone;
    m_TimestampOrigin = dateTime.timestamp();
    m_Timestamp       = m_TimestampOrigin;
}

void TimeSimulator::setDivider(const Divider& divider) {
    m_Divider = divider;
}

UniTime::DateTime TimeSimulator::now() {
    return UniTime::DateTime(m_Timestamp, m_Timezone);
}

void TimeSimulator::jumpTo(const UniTime::Time& time, const bool& forceSameDay) {
    jumpTo(time.hour, time.minute, time.second, forceSameDay);
}

void TimeSimulator::jumpTo(const uint8_t& hour, const uint8_t& minute, const uint8_t& second, const bool& forceSameDay) {
    UniTime::DateTime dateTime(m_Timestamp, m_Timezone);
    bool isDayChanged = dateTime.hour > hour || (dateTime.hour == hour && dateTime.minute > minute)
                        || (dateTime.hour == hour && dateTime.minute == minute && dateTime.second > second);

    dateTime.hour   = hour;
    dateTime.minute = minute;
    dateTime.second = second;

    m_Timestamp = dateTime.timestamp() + (isDayChanged && !forceSameDay ? 86400 : 0);
}

void TimeSimulator::forwardSeconds(const uint32_t& seconds) {
    m_Timestamp += seconds;
}

void TimeSimulator::forwardMinutes(const uint32_t& minutes) {
    m_Timestamp += minutes * 60;
}

void TimeSimulator::forwardHours(const uint32_t& hours) {
    m_Timestamp += hours * 3600;
}

void TimeSimulator::forwardDays(const uint32_t& days) {
    m_Timestamp += days * 86400;
}

void TimeSimulator::backwardSeconds(const uint32_t& seconds) {
    m_Timestamp -= seconds;
}

void TimeSimulator::backwardMinutes(const uint32_t& minutes) {
    m_Timestamp -= minutes * 60;
}

void TimeSimulator::backwardHours(const uint32_t& hours) {
    m_Timestamp -= hours * 3600;
}

void TimeSimulator::backwardDays(const uint32_t& days) {
    m_Timestamp -= days * 86400;
}

void TimeSimulator::onSecondChanged(const DateTimeHandler& handler) {
    m_SecondChangedHandler = handler;
}

void TimeSimulator::onMinuteChanged(const DateTimeHandler& handler) {
    m_MinuteChangedHandler = handler;
}

void TimeSimulator::onHourChanged(const DateTimeHandler& handler) {
    m_HourChangedHandler = handler;
}

void TimeSimulator::onDayChanged(const DateTimeHandler& handler) {
    m_DayChangedHandler = handler;
}