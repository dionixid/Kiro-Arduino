#include "Function.h"

void setup() {
    Serial.begin(115200);
    Log::attach(Serial, Log::Debug);
    delay(1000);
    Serial1.begin(9600, SERIAL_8N1, PIN_DF_TX, PIN_DF_RX);
    g_DFPlayer.begin(Serial1);
    g_OLED.begin();
    g_Button.begin();

    initializeDatabase();
    initializeNetwork();

    xTaskCreate(reconnectionTask, "reconnectionTask", 4096, NULL, 5, NULL);
}

void loop() {
    Timer::run();
}