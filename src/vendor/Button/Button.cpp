#include "Button.h"

ArrayList<Button*> Button::buttons = ArrayList<Button*>();
boolean Button::registered = false;
uint32_t Button::uid = 0;

Button::Button()
    : id(0),
      counter(0),
      longPressDuration(700),
      pullup(true),
      _isPressed(false),
      _isLongPressed(false),
      pressCallback(NULL),
      isLongPressContinuous(false),
      longCallback(NULL),
      releaseCallback(NULL) {}

Button::Button(uint8_t pin) : Button() {
    this->pin = pin;
}

Button::~Button() {
    uid = id;
    buttons.removeIf([](Button* b) -> bool { return b->id == uid; });
}

void Button::begin(bool pullup) {
    this->pullup = pullup;
    pinMode(pin, pullup ? INPUT_PULLUP : INPUT);
    uid = millis();
    while (buttons.contains([](Button* b) -> bool { return b->id == uid; })) uid++;
    id = uid;
    if (!buttons.contains([](Button* b) -> bool { return b->id == uid; })) {
        buttons.add(this);
    }
#ifdef ESP32
    if (!registered) {
        registered = true;
        xTaskCreate(pollTask, "Button", 8192, NULL, 5, NULL);
    }
#else
    if (!registered) {
        registered = true;
        Timer.setInterval(run, 100);
    }
#endif
}

bool Button::read() {
    return pullup ? !digitalRead(pin) : digitalRead(pin);
}

bool Button::isPressed() {
    return _isPressed;
}

bool Button::isLongPressed() {
    return _isLongPressed;
}

void Button::onPress(Callback callback) {
    this->pressCallback = callback;
}

void Button::onLongPress(Callback callback, const uint32_t& duration, const bool& isContinuous) {
    isLongPressContinuous = isContinuous;
    longPressDuration = duration;
    this->longCallback = callback;
}

void Button::onRelease(Callback callback) {
    this->releaseCallback = callback;
}

void Button::pollEvent() {
    if (read()) {
        if (!_isPressed) {
            _isPressed = true;
            counter = millis();
            if (pressCallback) pressCallback();
        } else {
            if (millis() - counter < longPressDuration) return;
            if (isLongPressContinuous) {
                _isLongPressed = true;
                if (longCallback) longCallback();
            } else {
                if (!_isLongPressed) {
                    _isLongPressed = true;
                    if (longCallback) longCallback();
                }
            }
        }
    } else {
        if (_isPressed) {
            _isPressed = false;
            _isLongPressed = false;
            if (releaseCallback) releaseCallback();
        }
    }
}

#ifdef ESP32
void Button::pollTask(void*) {
    while (true) {
        buttons.forEach([](Button* b, size_t) -> bool {
            if (b) b->pollEvent();
            return true;
        });
        delay(100);
    }
}
#else
void Button::run() {
    buttons.forEach([](Button* b, size_t) -> bool {
        if (b) b->pollEvent();
        return true;
    });
}

#endif