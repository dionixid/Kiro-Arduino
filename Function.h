#ifndef FUNCTION_H
#define FUNCTION_H

/**
 * This file contains the core functions of the project.
 * The core functions are the functions that dictate the functionality of the project.
 * For example, the scheduling task should be done in this file.
 */

#include "Config.h"
#include "Definition.h"
#include "Display.h"
#include "Utility.h"
#include "WebHandler.h"

/*----- Database -----*/

void loadDatabase() {
    if (g_DB.contains(KEY_SETTING_LOCATION)) {
        g_Location = g_DB.get(KEY_SETTING_LOCATION);
    } else {
        g_DB.put(KEY_SETTING_LOCATION, g_Location);
    }

    if (g_DB.contains(KEY_SETTING_WIFI)) {
        g_WiFi = g_DB.get(KEY_SETTING_WIFI);
    } else {
        g_DB.put(KEY_SETTING_WIFI, g_WiFi);
    }

    if (g_DB.contains(KEY_SETTING_SECURITY)) {
        g_Security = g_DB.get(KEY_SETTING_SECURITY);
    } else {
        g_DB.put(KEY_SETTING_SECURITY, g_Security);
    }
}

void initializeDatabase() {
    Log::info(TAG_VERSION, F("Kiro v%s"), VERSION);
    Log::info(TAG_DATABASE, F("Initializing..."));

    while (!SPIFFS.begin(true)) {
        delay(1000);
        Serial.print(".");
    }

    g_DB.begin(SPIFFS);
    Config::initialize();
    loadDatabase();
    Log::info(TAG_DATABASE, F("Database has been initialized"));
}

#endif