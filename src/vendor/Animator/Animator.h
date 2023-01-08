#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <Arduino.h>

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#include <vector>
#endif

#ifndef ESP32
#include "../Timer/Timer.h"
#endif

class Animator {
   public:
#if defined(ESP8266) || defined(ESP32)
    using VoidCallback = std::function<void()>;
#else
    using VoidCallback = void (*)();
#endif

    Animator();
    Animator(const Animator& other);
    Animator(Animator&& other);
    ~Animator();

    Animator& setDuration(const uint32_t& duration);
    uint32_t getDuration();

    Animator& setDelay(const uint32_t& delay);
    uint32_t getDelay();

    Animator& setCurrentValue(const float& current);
    float getCurrentValue();

    void animate(const float& target);
    void cancel();

    Animator& withEnd(VoidCallback callback);

   protected:
    virtual void onAnimationStart() {}
    virtual void onAnimationCancel() {}
    virtual void onAnimationEnd() {}
    virtual void onAnimationUpdate(float currentValue) {}

   private:
    uint32_t _duration = 1000;
    uint32_t _delay = 0;
    uint32_t _startTime = 0;
    float _step = 0;
    float _current = 0;
    float _target = 0;
    bool _isRunning = false;
    bool _isIncreasing = false;
    void _run(const uint32_t& currentTime);

    VoidCallback _endCallback = [] {};

    static uint32_t _timeStep;
    static bool _isInitialized;
    static std::vector<Animator*> _animators;
#ifdef ESP32
    static void _pollTask(void*);
#else
    static void _pollTask();
#endif
};

#endif