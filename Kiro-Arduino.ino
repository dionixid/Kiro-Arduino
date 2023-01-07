#include "Function.h"

void setup() {
    Serial.begin(115200);
    Log::attach(Serial, Log::Debug);
    delay(1000);

    initializeDatabase();
    initializeNetwork();

    xTaskCreate(reconnectionTask, "reconnectionTask", 4096, NULL, 5, NULL);
}

void loop() {
    Timer::run();
}