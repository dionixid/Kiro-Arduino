#ifndef BUTTON_H
#define BUTTON_H

#if defined(ESP32) || defined(ESP8266)
#include <functional>
#endif

#include "Arduino.h"
#include "../Timer/Timer.h"

class Button {
   public:
#if defined(ESP32) || defined(ESP8266)
    using Callback = std::function<void()>;
#else
    using Callback = void (*)();
#endif
    Button();
    Button(uint8_t pin);
    ~Button();
    void begin(bool pullup = true);
    bool read();
    bool isPressed();
    bool isLongPressed();
    void onPress(Callback callback);
    void onLongPress(Callback callback, const uint32_t& duration = 700, const bool& isContinuous = false);
    void onRelease(Callback callback);
    void pollEvent();

   private:
    uint32_t id;
    uint32_t counter;
    uint32_t longPressDuration;
    uint8_t pin;
    bool pullup;
    bool isLongPressContinuous;
    bool _isPressed;
    bool _isLongPressed;

    Callback pressCallback;
    Callback longCallback;
    Callback releaseCallback;

    static ArrayList<Button*> buttons;
    static boolean registered;
    static uint32_t uid;
#ifdef ESP32
    static void pollTask(void*);
#else
    static void run();
#endif
};

#endif
