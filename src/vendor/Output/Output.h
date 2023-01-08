#ifndef OUTPUT_H
#define OUTPUT_H

#if defined(ESP32) || defined(ESP8266)
#include <functional>
#endif

#include <Arduino.h>

#include "../Animator/Animator.h"
#include "../Timer/Timer.h"

class Output : public Animator {
   public:

#if defined(ESP32) || defined(ESP8266)
    using Callback = std::function<void()>;
    using StateCallback = std::function<void(bool)>;
    using DutyCycleCallback = std::function<void(uint16_t)>;
    using DutyPercentCallback = std::function<void(uint8_t)>;
#else
    using Callback = void (*)();
    using StateCallback = void (*)(bool);
    using DutyCycleCallback = void (*)(uint16_t);
    using DutyPercentCallback = void (*)(uint8_t);
#endif

#ifdef ESP32
    Output(const uint8_t& pin = -1, const bool& isPwm = false, const uint8_t& channel = 0);
#else
    Output(const uint8_t& pin = -1, const bool& isPwm = false);
#endif
    Output(const Output& output);
    ~Output();

    void begin(const bool& activeLow = false, const uint16_t& freq = 490, const uint8_t& resolution = 8);
    void blink(const int& count = 0);
    void stop();
    void onBlinkFinished(Callback callback);
    void pollEvent();

    void operator=(const Output& other);

    void set(const uint16_t& dutyCycle);
    void setPercent(const uint8_t& percent);
    void setFrequency(const uint16_t& freq);
    void setResolution(const uint8_t& resolution);
    void setDutyCycleOn(const uint16_t& dutyCycle);
    void setDutyCyclePercentOn(const uint8_t& dutyCycle);
    void setDutyCycleOff(const uint16_t& dutyCycle);
    void setDutyCyclePercentOff(const uint8_t& dutyCycle);
    void setBoundaries(const uint16_t& lower, const uint16_t& upper);
    void setBoundariesPercent(const uint8_t& lower, const uint8_t& upper);
    void onStateChanged(StateCallback callback);
    void onDutyCycleChanged(DutyCycleCallback callback);
    void onDutyPercentChanged(DutyPercentCallback callback);

    uint16_t getMaxDutyCycle();
    uint16_t getDutyCycleOn();
    uint16_t getDutyCycleOff();
    uint16_t getCurrentDutyCycle();
    uint8_t getDutyCyclePercentOn();
    uint8_t getDutyCyclePercentOff();
    uint8_t getCurrentDutyCyclePercent();

    bool get();
    bool isBlinking();

    void animatePercent(const uint8_t& percent, const uint16_t& duration = 1000, const uint16_t& delay = 0);

    template <typename... T>
    void setPattern(T&&... args) {
        uint32_t intervals[] = {static_cast<uint32_t>(args)...};
        patterns = ArrayList<uint32_t>(intervals, sizeof...(args));
        if (patterns.size() % 2 != 0) {
            patterns.add(0);
        }
        if (blinker.blinking) {
            blinker.counter = 0;
            blinker.index = 0;
            counter = millis();
            write(dutyOn);
        }
    }

   private:
    struct BlinkData {
        uint8_t index;
        int16_t count;
        uint32_t counter;
        boolean blinking;

        BlinkData()
            : counter(0), count(0), index(0), blinking(false) {}
        BlinkData(const BlinkData& blinkData)
            : counter(blinkData.counter), count(blinkData.count), index(blinkData.index), blinking(blinkData.blinking) {}
    };

    bool isPwm;
    uint8_t pin;
    uint16_t dutyOn;
    uint16_t dutyOff;
    uint16_t maxDuty;
    uint16_t currentDuty;
    uint16_t lowerBound;
    uint16_t upperBound;
    uint32_t id;
    uint32_t counter;
    Callback callback;
    BlinkData blinker;
    ArrayList<uint32_t> patterns;
    StateCallback stateCallback;
    DutyCycleCallback dutyCycleCallback;
    DutyPercentCallback dutyPercentCallback;

    boolean activeLow;
    void write(uint16_t state);

#ifdef ESP32
    uint8_t channel;
    uint8_t resolution;
    uint16_t freq;
#endif

    void onAnimationStart() override;
    void onAnimationUpdate(float currentValue) override;
    
    static ArrayList<Output*> outputs;
    static boolean registered;
#ifdef ESP32
    static void pollTask(void*);
#else
    static void run();
#endif
};

#endif
