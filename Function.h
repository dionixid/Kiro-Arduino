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

void initializeNetwork() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.setHostname(String("kiro-" + DEVICE_SUID).c_str());
    WiFi.softAPsetHostname(String("kiro-" + DEVICE_SUID).c_str());
    WiFi.onEvent(onWiFiEvent);

    MAC_ADDRESS[3] = 0x01;
    esp_wifi_set_mac(WIFI_IF_AP, MAC_ADDRESS);
    MAC_ADDRESS[3] = 0x02;
    esp_wifi_set_mac(WIFI_IF_STA, MAC_ADDRESS);

    restartAP();
    reconnectSTA();


    Log::info(TAG_WIFI, F("AP MAC Address: %s"), WiFi.softAPmacAddress().c_str());
    Log::info(TAG_WIFI, F("STA MAC Address: %s"), WiFi.macAddress().c_str());
}

void reconnectionTask(void*) {
    while (true) {
        if (!WiFi.isConnected()) {
            reconnectSTA();
        }
        delay(10000);
    }
}

#endif