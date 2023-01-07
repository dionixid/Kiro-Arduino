#ifndef DEFINITION_H
#define DEFINITION_H

#define VERSION "1.0.0"

/**
 * This file contains all the definitions used in the project.
 * The definitions are divided into sections, e.g. pins, constants,
 * instances, variables, etc.
 */

#include <AsyncUDP.h>
#include <SPIFFS.h>
#include <esp_wifi.h>

#include "src/Log/Log.h"
#include "src/RTTP/RTTP.h"
#include "src/Time/Time.h"
#include "src/Timer/Timer.h"
#include "src/TinyDB/TinyDB.h"
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
#include "src/model/SurahProperties.h"

/*----- Relay Pins ------*/
const uint8_t PIN_RELAY = 4;

/*----- LED Pins ------*/
const uint8_t PIN_LED = 13;

/*----- Button Pins ------*/
const uint8_t PIN_BUTTON = 2;

/*----- DB Key ------*/
const String KEY_DEVICE            = F("device");
const String KEY_SCHEDULES         = F("schedules");
const String KEY_SETTING_LOCATION  = F("s_location");
const String KEY_SETTING_WIFI      = F("s_wifi");
const String KEY_SETTING_SECURITY  = F("s_security");

/*----- Device Credential ------*/
const String DEVICE_NAME = F("Kiro");
const String DEVICE_PASS = F("12345678");
const String DEVICE_UUID = F("7ab16826-e32c-48b9-add6-5b72b1615a47");
const String DEVICE_SUID = F("5A47");

/*----- UDP ------*/
const uint16_t UDP_PORT = 46525;

/*-----MAC Address-----*/
uint8_t MAC_ADDRESS[6] = {0x52, 0x4F, 0x58, 0x00, 0x5A, 0x47};

/*----- Log Tag -----*/
const String TAG_VERSION  = F("Version");
const String TAG_WIFI     = F("WiFi");
const String TAG_DATABASE = F("Database");
const String TAG_RTTP     = F("RTTP");
const String TAG_UDP      = F("UDP");

/*----- Instances ------*/
TinyDB g_DB;
AsyncUDP g_UDP;
AsyncUDPMessage g_UDPMessage;
// Ex: RTTP::Server server(80);

/*----- Variables ------*/
Device g_Device;
SettingGroup g_DateTime;
SettingGroup g_Location;
SettingGroup g_WiFi;
SettingGroup g_Security;
SettingGroup g_About;

#endif
