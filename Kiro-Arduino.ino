#include "Function.h"

void setup() {
    Serial.begin(115200);
    Serial1.begin(9600, SERIAL_8N1, PIN_DF_RX, PIN_DF_TX);
    delay(1000);
    Log::attach(Serial, Log::Debug);
    g_DFPlayer.begin(Serial1);
    g_OLED.begin();
    g_Button.begin();
    g_DFBusy.begin();
    g_Relay.begin();

    g_Button.onPress(onButtonPressed);
    g_DFBusy.onPress(onStartPlayingAudio);
    g_DFBusy.onRelease(onFinishedPlayingAudio);

    initializeDatabase();
    initializeNetwork();

    g_DFPlayer.setTimeOut(500);
    g_DFPlayer.EQ(DFPLAYER_EQ_NORMAL);
    g_DFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    g_DFPlayer.volume(20);
    xTaskCreate(reconnectionTask, "reconnectionTask", 4096, NULL, 5, NULL);
}

void loop() {
    Timer::run();
}