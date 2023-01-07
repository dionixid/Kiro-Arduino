#ifndef UTILITY_H
#define UTILITY_H

/**
 * This file contains utility functions.
 * Utility functions are functions that are used in multiple files.
 * Every function in this file MUST not depend on any other file,
 * except for the Definition.h file.
 */

#include "Definition.h"

/*----- WiFi -----*/

void restartAP() {
    Setting& password = g_WiFi.getSetting(Config::SECURITY_PASSWORD);
    WiFi.softAPdisconnect();
    WiFi.softAP(g_Device.name.c_str(), password.value.c_str());
}

void reconnectSTA() {
    Setting& ssid = g_WiFi.getSetting(Config::WIFI_SSID);
    Setting& password = g_WiFi.getSetting(Config::WIFI_PASSWORD);
    
    if (ssid.value.isNotEmpty()) {
        WiFi.disconnect();
        WiFi.begin(ssid.value.c_str(), password.value.c_str());
    }
}


#endif