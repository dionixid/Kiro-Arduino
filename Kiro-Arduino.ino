#include "Function.h"

void setup() {
    checkCounterfeit();
    Serial.begin(115200);
    Serial1.begin(9600, SERIAL_8N1, PIN_DF_RX, PIN_DF_TX);
    delay(1000);
    Log::attach(Serial, Log::Debug);
    g_DFPlayer.begin(Serial1);
    g_OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    g_Button.begin();
    g_DFBusy.begin();
    g_Relay.begin(true);

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
    Time.onMinuteChange(onMinuteChange);

    Display::showBootMessage();
    Log::info(TAG_SYSTEM, Any(getMacAddressInt()).toString());

    Timer::setInterval(10000, Display::switchDisplay);
    Timer::setInterval(50, Display::scrollDisplay);
    Timer::registerEvent(runMainQueue);
    xTaskCreate(reconnectionTask, "reconnectionTask", 4096, NULL, 5, NULL);
}

void loop() {
    Timer::run();
}