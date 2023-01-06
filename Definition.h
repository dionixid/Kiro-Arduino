#ifndef DEFINITION_H
#define DEFINITION_H

/**
 * This file contains all the definitions used in the project.
 * The definitions are divided into sections, e.g. pins, constants,
 * instances, variables, etc.
 */

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
const String KEY_DEVICE = "device";
const String KEY_SCHEDULE = "schedule";
const String KEY_SETTINGS = "settings";

/*----- Instances ------*/
// Ex: RTTP::Server server(80);

/*----- Variables ------*/
Device device;

#endif
