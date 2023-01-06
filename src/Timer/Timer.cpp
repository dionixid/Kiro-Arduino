#include "Timer.h"

/*-----------------------------------------------------------
 * COUNT DOWN TIMER CLASS IMPLEMENTATION
 *----------------------------------------------------------*/

ArrayList<CountDownTimer*> CountDownTimer::timers;

/**
 * @brief Create an instance of CountDownTimer.
 *
 */
CountDownTimer::CountDownTimer()
    : m_Duration(0),
      m_IsContinuous(false),
      m_Handler(nullptr) {
    timers.add(this);
}

/**
 * @brief Create an instance of CountDownTimer.
 *
 * @param duration is the duration of the timer in milliseconds.
 * @param handler is the handler to be called when the timer is finished.
 */
CountDownTimer::CountDownTimer(const uint32_t& duration, const CountDownHandler& handler)
    : m_Duration(duration),
      m_IsContinuous(false),
      m_Handler(handler) {
    timers.add(this);
}

/**
 * @brief Create an instance of CountDownTimer.
 *
 * @param duration is the duration of the timer in milliseconds.
 * @param isContinuous is a boolean value that indicates if the timer is continuous or not.
 * @param handler is the handler to be called when the timer is finished.
 */
CountDownTimer::CountDownTimer(const uint32_t& duration, const bool& isContinuous, const CountDownHandler& handler)
    : m_Duration(duration),
      m_IsContinuous(isContinuous),
      m_Handler(handler) {
    timers.add(this);
}

CountDownTimer::~CountDownTimer() {
    cancel();
    timers.remove(this);
}

/**
 * @brief Start the timer.
 *
 */
void CountDownTimer::start() {
    m_LastMillis = millis();
    m_IsRunning  = true;
}

/**
 * @brief Cancel the timer.
 *
 */
void CountDownTimer::cancel() {
    m_IsRunning = false;
}

/**
 * @brief Reset the timer.
 *
 */
void CountDownTimer::reset() {
    m_LastMillis = millis();
}

/**
 * @brief Set the continuous state of the timer.
 *
 * @param isContinuous is a boolean value that indicates if the timer is continuous or not.
 */
void CountDownTimer::setContinuous(const bool& isContinuous) {
    m_IsContinuous = isContinuous;
}

/**
 * @brief Set the duration of the timer.
 *
 * @param duration is the duration of the timer in milliseconds.
 */
void CountDownTimer::setDuration(const uint32_t& duration) {
    this->m_Duration = duration;
}

/**
 * @brief Set the handler of the timer.
 *
 * @param handler is the handler to be called when the timer is finished.
 */
void CountDownTimer::setHandler(const CountDownHandler& handler) {
    m_Handler = handler;
}

/**
 * @brief Check if the timer is continuous.
 *
 * @return true if the timer is continuous. false otherwise.
 */
bool CountDownTimer::isContinuous() {
    return m_IsContinuous;
}

/**
 * @brief Get the duration of the timer.
 *
 * @return the duration of the timer in milliseconds.
 */
uint32_t CountDownTimer::getDuration() {
    return m_Duration;
}

/**
 * @brief Check if the timer is running.
 *
 * @return true if the timer is running. false otherwise.
 */
bool CountDownTimer::isRunning() {
    return m_IsRunning;
}

/*-----------------------------------------------------------
 * TIME HANDLE TYPE IMPLEMENTATION
 *----------------------------------------------------------*/

TimeHandle_t::TimeHandle_t()
    : id(0) {}

TimeHandle_t::TimeHandle_t(uint32_t id)
    : id(id) {}

TimeHandle_t& TimeHandle_t::operator=(const uint32_t& newId) {
    id = newId;
    return *this;
}

TimeHandle_t::operator uint32_t() const {
    return id;
}

bool TimeHandle_t::operator==(const TimeHandle_t& other) const {
    return id == other.id;
}

bool TimeHandle_t::operator!=(const TimeHandle_t& other) const {
    return id != other.id;
}

bool TimeHandle_t::operator==(const uint32_t& other) const {
    return id == other;
}

bool TimeHandle_t::operator!=(const uint32_t& other) const {
    return id != other;
}

TimeHandle_t& TimeHandle_t::operator++() {
    id++;
    return *this;
}

TimeHandle_t TimeHandle_t::operator++(int) {
    TimeHandle_t temp = *this;
    id++;
    return temp;
}

TimeHandle_t& TimeHandle_t::operator--() {
    id--;
    return *this;
}

TimeHandle_t TimeHandle_t::operator--(int) {
    TimeHandle_t temp = *this;
    id--;
    return temp;
}

/*-----------------------------------------------------------
 * TIMER CLASS IMPLEMENTATION
 *----------------------------------------------------------*/

ArrayList<Timer::RegularEvent> Timer::m_RegularEvents;
ArrayList<Timer::TimeEvent> Timer::m_IntervalEvents;
ArrayList<Timer::TimeEvent> Timer::m_TimeoutEvents;

/**
 * @brief Register a regular event.
 * Regular events are called continuously without any delay.
 *
 * @param handler is the handler to be called.
 * @return the id of the event.
 */
TimeHandle_t Timer::registerEvent(const TimeHandler& handler) {
    static TimeHandle_t id;
    id = millis();
    while (m_RegularEvents.contains([](RegularEvent data) -> bool { return data.id == id; })) id++;
    m_RegularEvents.add(RegularEvent(id, handler));
    return id;
}

/**
 * @brief Register a periodic event.
 * Periodic events are called periodically with a fixed interval.
 *
 * @param handler is the handler to be called.
 * @param interval is the interval of the event in milliseconds.
 * @return the id of the event.
 */
TimeHandle_t Timer::setInterval(const TimeHandler& handler, const uint32_t& interval) {
    static TimeHandle_t id;
    id = millis();
    while (m_IntervalEvents.contains([](TimeEvent data) -> bool { return data.id == id; })) id++;
    m_IntervalEvents.add(TimeEvent(id, interval, millis(), handler));
    return id;
}

/**
 * @brief Register a periodic event.
 * Periodic events are called periodically with a fixed interval.
 *
 * @param interval is the interval of the event in milliseconds.
 * @param handler is the handler to be called.
 * @return the id of the event.
 */
TimeHandle_t Timer::setInterval(const uint32_t& interval, const TimeHandler& handler) {
    static TimeHandle_t id;
    id = millis();
    while (m_IntervalEvents.contains([](TimeEvent data) -> bool { return data.id == id; })) id++;
    m_IntervalEvents.add(TimeEvent(id, interval, millis(), handler));
    return id;
}

/**
 * @brief Register a timeout event.
 * Timeout events are called once after a fixed timeout.
 *
 * @param handler is the handler to be called.
 * @param timeout is the timeout of the event in milliseconds.
 * @return the id of the event.
 */
TimeHandle_t Timer::setTimeout(const TimeHandler& handler, const uint32_t& timeout) {
    static TimeHandle_t id;
    id = millis();
    while (m_TimeoutEvents.contains([](TimeEvent data) -> bool { return data.id == id; })) id++;
    m_TimeoutEvents.add(TimeEvent(id, timeout, millis(), handler));
    return id;
}

/**
 * @brief Register a timeout event.
 * Timeout events are called once after a fixed timeout.
 *
 * @param timeout is the timeout of the event in milliseconds.
 * @param handler is the handler to be called.
 * @return the id of the event.
 */
TimeHandle_t Timer::setTimeout(const uint32_t& timeout, const TimeHandler& handler) {
    static TimeHandle_t id = millis();
    while (m_TimeoutEvents.contains([](TimeEvent data) -> bool { return data.id == id; })) id++;
    m_TimeoutEvents.add(TimeEvent(id, timeout, millis(), handler));
    return id;
}

/**
 * @brief Unregister a regular event.
 *
 * @param eventId is the id of the event to be unregistered.
 */
void Timer::unregisterEvent(const TimeHandle_t& eventId) {
#if defined(ESP32) || defined(ESP8266)
    m_RegularEvents.removeIf([eventId](RegularEvent data) -> bool { return data.id == eventId; });
#else
    static TimeHandle_t id;
    id = eventId;
    m_RegularEvents.removeIf([](RegularEvent data) -> bool { return data.id == id; });
#endif
}

/**
 * @brief Clear a periodic event.
 *
 * @param eventId is the id of the event to be cleared.
 */
void Timer::clearInterval(const TimeHandle_t& eventId) {
#if defined(ESP32) || defined(ESP8266)
    m_IntervalEvents.removeIf([eventId](TimeEvent data) -> bool { return data.id == eventId; });
#else
    static TimeHandle_t id;
    id = eventId;
    m_IntervalEvents.removeIf([](TimeEvent data) -> bool { return data.id == id; });
#endif
}

/**
 * @brief Clear a timeout event.
 *
 * @param eventId is the id of the event to be cleared.
 */
void Timer::clearTimeout(const TimeHandle_t& eventId) {
#if defined(ESP32) || defined(ESP8266)
    m_TimeoutEvents.removeIf([eventId](TimeEvent data) -> bool { return data.id == eventId; });
#else
    static TimeHandle_t id;
    id = eventId;
    m_TimeoutEvents.removeIf([](TimeEvent data) -> bool { return data.id == id; });
#endif
}

/**
 * @brief Run all registered events.
 * This method should be called in the main loop.
 *
 */
void Timer::run() {
    for (size_t i = 0; i < m_RegularEvents.size(); i++) {
        if (m_RegularEvents[i].handler == NULL) {
            continue;
        }

        if (!m_RegularEvents[i].passed) {
            m_RegularEvents[i].handler();
        }
    }

    for (size_t i = 0; i < m_IntervalEvents.size(); i++) {
        auto& event = m_IntervalEvents[i];
        if (event.handler == NULL) {
            continue;
        }

        if (!event.passed && (millis() - event.counter >= event.interval)) {
            event.counter = millis();
            event.handler();
        }
    }

    for (size_t i = 0; i < m_TimeoutEvents.size(); i++) {
        auto& event = m_TimeoutEvents[i];
        if (event.handler == NULL) {
            continue;
        }

        if (!event.passed && (millis() - event.counter >= event.interval)) {
            event.passed = true;
            event.handler();
        }
    }

    for (size_t i = 0; i < CountDownTimer::timers.size(); i++) {
        auto& timer = CountDownTimer::timers[i];
        if (timer->m_Duration == 0 || timer->m_Handler == NULL) {
            continue;
        }

        if (timer->m_IsRunning && (millis() - timer->m_LastMillis >= timer->m_Duration)) {
            if (timer->m_IsContinuous) {
                timer->m_LastMillis = millis();
            } else {
                timer->m_IsRunning = false;
            }
            timer->m_Handler();
        }
    }

    static uint32_t lastMillis = millis();
    if (millis() - lastMillis >= 1000) {
        lastMillis = millis();
        m_TimeoutEvents.removeIf([](TimeEvent data) -> bool {
            if (data.passed) {
                return true;
            } else if (millis() - data.counter >= data.interval + 1000) {
                data.passed = true;
                return true;
            }
            return false;
        });
    }
}
