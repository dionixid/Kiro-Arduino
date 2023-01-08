#include "Output.h"

ArrayList<Output*> Output::outputs;
boolean Output::registered = false;

#ifdef ESP32
Output::Output(const uint8_t& pin, const bool& isPwm, const uint8_t& channel)
    : channel(channel),
      resolution(8),
      freq(5000),
#else
Output::Output(const uint8_t& pin, const bool& isPwm)
    :
#endif
      Animator(),
      pin(pin),
      isPwm(isPwm),
      dutyOn(255),
      dutyOff(0),
      maxDuty(255),
      lowerBound(0),
      upperBound(255),
      id(millis()),
      counter(0),
      callback(NULL),
      blinker(BlinkData()),
      patterns(ArrayList<uint32_t>()),
      activeLow(false),
      stateCallback(NULL),
      dutyCycleCallback(NULL),
      dutyPercentCallback(NULL) {
    patterns.add(500);
    patterns.add(500);

#if defined(ESP32) || defined(ESP8266)
    while (outputs.contains([this](Output* o) -> bool { return o->id == id; })) id++;
    outputs.add(this);
#else
    static uint32_t uid;
    uid = millis();
    while (outputs.contains([](Output* o) -> bool { return o->id == uid; })) uid++;
    id = uid;
    outputs.add(this);
#endif
}

Output::Output(const Output& other)
    : Animator(other),
      pin(other.pin),
#ifdef ESP32
      channel(other.channel),
      resolution(other.resolution),
      freq(other.freq),
#endif
      isPwm(other.isPwm),
      dutyOn(other.dutyOn),
      dutyOff(other.dutyOff),
      maxDuty(other.maxDuty),
      lowerBound(other.lowerBound),
      upperBound(other.upperBound),
      id(millis()),
      counter(other.counter),
      callback(other.callback),
      blinker(other.blinker),
      patterns(other.patterns),
      activeLow(other.activeLow),
      stateCallback(other.stateCallback),
      dutyCycleCallback(other.dutyCycleCallback),
      dutyPercentCallback(other.dutyPercentCallback) {
#if defined(ESP32) || defined(ESP8266)
    while (outputs.contains([this](Output* o) -> bool { return o->id == id; })) id++;
    outputs.add(this);
#else
    static uint32_t uid;
    uid = millis();
    while (outputs.contains([](Output* o) -> bool { return o->id == uid; })) uid++;
    id = uid;
    outputs.add(this);
#endif
}

Output::~Output() {
#if defined(ESP32) || defined(ESP8266)
    outputs.removeIf([this](Output* o) -> bool { return o->id == id; });
#else
    static uint32_t uid;
    uid = id;
    outputs.removeIf([](Output* o) -> bool { return o->id == uid; });
#endif
}

void Output::operator=(const Output& other) {
    pin = other.pin;
#ifdef ESP32
    channel = other.channel;
    resolution = other.resolution;
    freq = other.freq;
#endif
    isPwm = other.isPwm;
    dutyOn = other.dutyOn;
    dutyOff = other.dutyOff;
    maxDuty = other.maxDuty;
    lowerBound = other.lowerBound;
    upperBound = other.upperBound;
    counter = other.counter;
    callback = other.callback;
    blinker = other.blinker;
    patterns = other.patterns;
    activeLow = other.activeLow;
    stateCallback = other.stateCallback;
    dutyCycleCallback = other.dutyCycleCallback;
    dutyPercentCallback = other.dutyPercentCallback;
}

void Output::write(uint16_t state) {
    if (isPwm) {
        if (state == currentDuty) {
            return;
        }
        currentDuty = state;
        if (state > upperBound) currentDuty = upperBound;
        if (state < lowerBound) currentDuty = lowerBound;
        if ((int)upperBound - (int)lowerBound < 0) currentDuty = 0;
#ifdef ESP32
        ledcWrite(channel, activeLow ? maxDuty - currentDuty : currentDuty);
#else
        analogWrite(pin, activeLow ? maxDuty - currentDuty : currentDuty);
#endif
        if (dutyCycleCallback) {
            dutyCycleCallback(currentDuty);
        }

        if (dutyPercentCallback) {
            dutyPercentCallback(map(currentDuty, lowerBound, upperBound, 0, 100));
        }
    } else {
        if (get() != state) {
            digitalWrite(pin, activeLow ? !state : state);
            if (stateCallback) {
                stateCallback(state);
            }
        }
    }
}

void Output::set(const uint16_t& state) {
    if (isBlinking()) return;
    write(state);
}

void Output::setPercent(const uint8_t& percent) {
    if (isBlinking()) return;
    write(map(percent, 0, 100, lowerBound, upperBound));
}

void Output::setFrequency(const uint16_t& freq) {
    if (isPwm) {
#ifdef ESP32
        this->freq = freq;
        ledcSetup(channel, freq, resolution);
#elif defined(ESP8266)
        analogWriteFreq(freq);
#endif
    }
}

void Output::setResolution(const uint8_t& resolution) {
    if (isPwm) {
        maxDuty = round(pow(2.0, resolution) - 1);
        upperBound = maxDuty;
#ifdef ESP32
        this->resolution = resolution;
        ledcSetup(channel, freq, resolution);
#elif defined(ESP8266)
        analogWriteRange(maxDuty);
#endif
    }
}

void Output::setDutyCycleOn(const uint16_t& dutyCycle) {
    dutyOn = dutyCycle;
    if (isBlinking() && blinker.index % 2 == 0 && patterns[blinker.index] > 0) {
        write(dutyOn);
    }
}

void Output::setDutyCyclePercentOn(const uint8_t& percent) {
    setDutyCycleOn(map(percent, 0, 100, lowerBound, upperBound));
}

void Output::setDutyCycleOff(const uint16_t& dutyCycle) {
    dutyOff = dutyCycle;
    if (isBlinking() && blinker.index % 2 != 0 && patterns[blinker.index] > 0) {
        write(dutyOff);
    }
}

void Output::setDutyCyclePercentOff(const uint8_t& percent) {
    setDutyCycleOff(map(percent, 0, 100, lowerBound, upperBound));
}

void Output::setBoundaries(const uint16_t& lower, const uint16_t& upper) {
    lowerBound = lower;
    upperBound = upper;
}

void Output::setBoundariesPercent(const uint8_t& lower, const uint8_t& upper) {
    setBoundaries(map(lower, 0, 100, 0, maxDuty), map(upper, 0, 100, 0, maxDuty));
}

void Output::onStateChanged(StateCallback callback) {
    stateCallback = callback;
}

void Output::onDutyCycleChanged(DutyCycleCallback callback) {
    dutyCycleCallback = callback;
}

void Output::onDutyPercentChanged(DutyPercentCallback callback) {
    dutyPercentCallback = callback;
}

void Output::begin(const bool& activeLow, const uint16_t& freq, const uint8_t& resolution) {
    this->activeLow = activeLow;
#ifdef ESP32
    this->freq = freq;
    this->resolution = resolution;
#endif
    pinMode(pin, OUTPUT);
    if (isPwm) {
        maxDuty = round(pow(2.0, resolution));
        upperBound = maxDuty;
        dutyOn = maxDuty;
        dutyOff = 0;
#ifdef ESP32
        ledcAttachPin(pin, channel);
        ledcSetup(channel, freq, resolution);
#elif defined(ESP8266)
        analogWriteFreq(freq);
        analogWriteRange(maxDuty);
#endif
    }
    write(dutyOff);
#ifdef ESP32
    if (!registered) {
        registered = true;
        xTaskCreate(pollTask, "Output", 8192, NULL, 5, NULL);
    }
#else
    if (!registered) {
        registered = true;
        Timer.registerEvent(run);
    }
#endif
}

void Output::blink(const int& count) {
    if (patterns.size() > 0) {
        blinker.count = count;
        write(patterns[0] > 0 ? dutyOn : dutyOff);
        blinker.blinking = true;
        blinker.counter = 0;
        blinker.index = 0;
        counter = millis();
    }
}

void Output::stop() {
    if (blinker.blinking) {
        blinker.blinking = false;
        write(dutyOff);
        if (callback) callback();
    }
}

void Output::onBlinkFinished(Callback callback) {
    this->callback = callback;
}

void Output::pollEvent() {
    if (blinker.blinking && (blinker.counter < blinker.count || blinker.count <= 0)) {
        if (blinker.index % 2 == 0) {
            if (millis() - counter >= patterns[blinker.index]) {
                blinker.index++;
                counter = millis();
                if (patterns[blinker.index] > 0) write(dutyOff);
            }
        } else {
            if (millis() - counter >= patterns[blinker.index]) {
                blinker.index++;
                counter = millis();
                if (blinker.index >= patterns.size()) {
                    blinker.index = 0;
                    if (blinker.count > 0) blinker.counter++;
                }
                if (blinker.count > 0 && blinker.counter >= blinker.count) {
                    stop();
                } else {
                    if (patterns[blinker.index] > 0) write(dutyOn);
                }
            }
        }
    }
}

bool Output::get() {
    return activeLow ? !digitalRead(pin) : digitalRead(pin);
}

uint16_t Output::getMaxDutyCycle() {
    return maxDuty;
}

uint16_t Output::getDutyCycleOn() {
    return dutyOn;
}

uint16_t Output::getDutyCycleOff() {
    return dutyOff;
}

uint16_t Output::getCurrentDutyCycle() {
    return currentDuty;
}

uint8_t Output::getDutyCyclePercentOn() {
    return map(dutyOn, lowerBound, upperBound, 0, 100);
}

uint8_t Output::getDutyCyclePercentOff() {
    return map(dutyOff, lowerBound, upperBound, 0, 100);
}

uint8_t Output::getCurrentDutyCyclePercent() {
    return map(currentDuty, lowerBound, upperBound, 0, 100);
}

bool Output::isBlinking() {
    return blinker.blinking;
}

void Output::animatePercent(const uint8_t& percent, const uint16_t& duration, const uint16_t& delay) {
    setDuration(duration);
    setDelay(delay);
    animate(map(percent, 0, 100, lowerBound, upperBound));
}

void Output::onAnimationStart() {
    setCurrentValue((float)currentDuty);
}

void Output::onAnimationUpdate(float value) {
    set((uint16_t)value);
}

#ifdef ESP32
void Output::pollTask(void* pvParameters) {
    while (true) {
        outputs.forEach([](Output* o, size_t index) -> bool {
            o->pollEvent();
            return true;
        });
        delay(10);
    }
}
#else
void Output::run() {
    outputs.forEach([](Output* o, size_t index) -> bool {
        o->pollEvent();
        return true;
    });
}
#endif
