#include "Function.h"

#if DEBUG
#pragma message("Debug mode is enabled")
#endif

#if SIMULATION
#include "Simulation.h"
#pragma message("Simulation mode is enabled")
#endif

void setup() {
    checkCounterfeit();
    Serial1.begin(9600, SERIAL_8N1, PIN_DF_RX, PIN_DF_TX);
#if DEBUG
    Serial.begin(115200);
    delay(1000);
    Log::attach(Serial, Log::Debug);
#endif
    g_DFPlayer.begin(Serial1);
    g_OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    g_Button.begin();
    g_DFBusy.begin();
    g_Relay.begin(false);

    g_Button.onPress(onButtonPressed);
    g_DFBusy.onPress(onStartPlayingAudio);
    g_DFBusy.onRelease(onFinishedPlayingAudio);

    initializeDatabase();
    initializeNetwork();

    g_Server.createChannel(RTTP_CHANNEL)
        .onAuth(onAuth)
        .onAuthenticated(onAuthenticated)
        .onJoin(onJoin)
        .onLeave(onLeave)
        .addTopic(RTTP_TOPIC_DEVICE)
        .addTopic(RTTP_TOPIC_PRAYER_GROUP)
        .addTopic(RTTP_TOPIC_PRAYER_ONGOING)
        .addTopic(RTTP_TOPIC_QIRO_ONGOING)
        .addTopic(RTTP_TOPIC_SETTING_ALL)
        .addTopic(RTTP_TOPIC_SURAH_ONGOING)
        .addTopic(RTTP_TOPIC_PRAYER_OFFSET, onTopicPrayerOffset)
        .addTopic(RTTP_TOPIC_QIRO_GROUP, onTopicQiroGroup)
        .addTopic(RTTP_TOPIC_SETTING_GROUP, onTopicSettingGroup)
        .addTopic(RTTP_TOPIC_SURAH_COLLECTION, onTopicSurahCollection)
        .addTopic(RTTP_TOPIC_SURAH_FORCE_STOP, onTopicSurahForceStop)
        .addTopic(RTTP_TOPIC_SURAH_LIST, onTopicSurahList)
        .addTopic(RTTP_TOPIC_SURAH_PREVIEW, onTopicSurahPreview);

    g_Server.begin();

    g_DFPlayer.setTimeOut(500);
    g_DFPlayer.EQ(DFPLAYER_EQ_NORMAL);
    g_DFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    g_DFPlayer.volume(20);

    Time.setTimezone(7);
    Time.enableNTP();
    Time.enableRTC();
#if !SIMULATION
    Time.onMinuteChange(onMinuteChange);
#endif

    Display::showBootMessage();
    Log::info(TAG_SYSTEM, Any(getMacAddressInt()).toString());

    Timer::setInterval(10000, Display::switchDisplay);
    Timer::setInterval(50, Display::scrollDisplay);
    Timer::registerEvent(runMainQueue);
    xTaskCreate(reconnectionTask, "reconnectionTask", 4096, NULL, 5, NULL);

#if SIMULATION
    Simulation::initialize();
#endif
}

void loop() {
    Timer::run();

#if SIMULATION
    if (Serial.available()) {
        String command = Serial.readString();
        command.trim();
        command.toLowerCase();

        if (command == "a") {
            Simulation::time.start();
        } else if (command == "s") {
            Simulation::time.stop();
            forceStopAudio();
        } else if (command == "p") {
            Simulation::time.pause();
        } else if (command == "r") {
            Simulation::time.resume();
        }
    }
#endif
}