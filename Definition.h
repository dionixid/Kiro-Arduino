#ifndef DEFINITION_H
#define DEFINITION_H

#define VERSION "1.0.0"
#define DEBUG true
#define SIMULATION false

/**
 * This file contains all the definitions used in the project.
 * The definitions are divided into sections, e.g. pins, constants,
 * instances, variables, etc.
 */

#include <AsyncUDP.h>
#include <SPIFFS.h>
#include <esp_wifi.h>

#include "src/collection/alyssum.h"
#include "src/model/Device.h"
#include "src/model/Prayer.h"
#include "src/model/PrayerGroup.h"
#include "src/model/PrayerTimeOffset.h"
#include "src/model/Qiro.h"
#include "src/model/QiroGroup.h"
#include "src/model/Setting.h"
#include "src/model/SettingGroup.h"
#include "src/model/Surah.h"
#include "src/model/SurahAudio.h"
#include "src/model/SurahCollection.h"
#include "src/model/SurahProperties.h"
#include "src/vendor/Adafruit/SSD1306/Adafruit_SSD1306.h"
#include "src/vendor/Button/Button.h"
#include "src/vendor/DFPlayer/DFRobotDFPlayerMini.h"
#include "src/vendor/Log/Log.h"
#include "src/vendor/Output/Output.h"
#include "src/vendor/RTTP/RTTP.h"
#include "src/vendor/Time/Time.h"
#include "src/vendor/Timer/Timer.h"
#include "src/vendor/TinyDB/TinyDB.h"

/*----- Relay Pins ------*/
const uint8_t PIN_RELAY = 25;

/*----- Button Pins ------*/
const uint8_t PIN_BUTTON = 26;

/*----- DFPlayer Pins ------*/
const uint8_t PIN_DF_RX   = 16;
const uint8_t PIN_DF_TX   = 17;
const uint8_t PIN_DF_BUSY = 18;

/*----- OLED Pins ------*/
const uint8_t PIN_OLED_SDA = 21;
const uint8_t PIN_OLED_SCL = 22;

/*----- DB Key ------*/
const String KEY_DEVICE             = F("device");
const String KEY_PRAYER_OFFSET      = F("prayer_offset");
const String KEY_SCHEDULE_MONDAY    = F("sc_monday");
const String KEY_SCHEDULE_TUESDAY   = F("sc_tuesday");
const String KEY_SCHEDULE_WEDNESDAY = F("sc_wednesday");
const String KEY_SCHEDULE_THURSDAY  = F("sc_thursday");
const String KEY_SCHEDULE_FRIDAY    = F("sc_friday");
const String KEY_SCHEDULE_SATURDAY  = F("sc_saturday");
const String KEY_SCHEDULE_SUNDAY    = F("sc_sunday");
const String KEY_SETTING_LOCATION   = F("s_location");
const String KEY_SETTING_WIFI       = F("s_wifi");
const String KEY_SETTING_SECURITY   = F("s_security");

/*----- Device Credential ------*/
const String DEVICE_NAME  = F("Kiro");
const String DEVICE_PASS  = F("12345678");
const String DEVICE_MASK  = F("tXEoN92q");
const uint64_t DEVICE_MAC = 40777588842070LL;

/*----- MAC Address -----*/
uint8_t MAC_ADDRESS[6] = {0x52, 0x4F, 0x58, 0x00, 0x00, 0x00};

/*----- UDP ------*/
const uint16_t UDP_PORT = 46525;

/*----- RTTP -----*/
const String RTTP_CHANNEL                = F("kiro");
const String RTTP_TOPIC_DEVICE           = F("device");
const String RTTP_TOPIC_PRAYER_GROUP     = F("prayer-group");
const String RTTP_TOPIC_PRAYER_OFFSET    = F("prayer-offset");
const String RTTP_TOPIC_PRAYER_ONGOING   = F("prayer-ongoing");
const String RTTP_TOPIC_QIRO_GROUP       = F("qiro-group");
const String RTTP_TOPIC_QIRO_ONGOING     = F("qiro-ongoing");
const String RTTP_TOPIC_SETTING_GROUP    = F("setting-group");
const String RTTP_TOPIC_SETTING_ALL      = F("setting-all");
const String RTTP_TOPIC_SURAH_COLLECTION = F("surah-collection");
const String RTTP_TOPIC_SURAH_ONGOING    = F("surah-ongoing");
const String RTTP_TOPIC_SURAH_PREVIEW    = F("surah-preview");
const String RTTP_TOPIC_SURAH_FORCE_STOP = F("surah-force-stop");
const String RTTP_TOPIC_SURAH_LIST       = F("surah-list");

/*----- Log Tag -----*/
const String TAG_VERSION   = F("Version");
const String TAG_WIFI      = F("WiFi");
const String TAG_DATABASE  = F("Database");
const String TAG_RTTP      = F("RTTP");
const String TAG_UDP       = F("UDP");
const String TAG_TIME      = F("Time");
const String TAG_AUDIO     = F("Audio");
const String TAG_BUTTON    = F("Button");
const String TAG_SYSTEM    = F("System");
const String TAG_PRAYER    = F("Prayer");
const String TAG_SIMULATOR = F("Simulator");

/*----- Instances ------*/
TinyDB g_DB;
AsyncUDP g_UDP;
DFRobotDFPlayerMini g_DFPlayer;
Adafruit_SSD1306 g_OLED(128, 64);
AsyncUDPMessage g_UDPMessage;
RTTP::Server g_Server(80);
Button g_Button(PIN_BUTTON);
Button g_DFBusy(PIN_DF_BUSY);
Output g_Relay(PIN_RELAY);

/*----- Variables ------*/
Device g_Device;

SettingGroup g_DateTime;
SettingGroup g_Location;
SettingGroup g_WiFi;
SettingGroup g_Security;
SettingGroup g_About;

QiroGroup g_QiroMonday;
QiroGroup g_QiroTuesday;
QiroGroup g_QiroWednesday;
QiroGroup g_QiroThursday;
QiroGroup g_QiroFriday;
QiroGroup g_QiroSaturday;
QiroGroup g_QiroSunday;
Qiro g_QiroOngoing;

PrayerTimeOffset g_PrayerOffset;
PrayerGroup g_PrayerGroup;
Prayer g_PrayerOngoing;

SurahAudio g_SurahOngoing;
SurahAudio g_SurahPreview;
SurahCollection g_SurahCollection;

UniTime::Date g_LastPrayerUpdateDate;
std::vector<std::function<void()>> g_MainThreadQueue;
SemaphoreHandle_t g_MainThreadQueueMutex = xSemaphoreCreateMutex();

CountDownTimer g_AudioTimeoutTimer;
CountDownTimer g_AudioPlayTimer;

bool g_IsQiroCancelled = false;

#endif
