#ifndef CONFIG_H
#define CONFIG_H

/**
 * This file contains the configuration of the project.
 * For example, the Qiro Schedule and the Setting objects
 * are constructed in this file.
 *
 * DO NOT put any other functions in this file.
 */

#include "Definition.h"

namespace Config {

const String TIME              = F("DT0");
const String DATE              = F("DT1");
const String LATITUDE          = F("L0");
const String LONGITUDE         = F("L1");
const String ELEVATION         = F("L2");
const String WIFI_STATUS       = F("W0");
const String WIFI_SSID         = F("W1");
const String WIFI_PASSWORD     = F("W2");
const String SECURITY_PASSWORD = F("S0");

void initialize() {
    g_Device.id      = DEVICE_UUID;
    g_Device.name    = DEVICE_NAME + " " + DEVICE_SUID;
    g_Device.version = VERSION;

    g_DateTime.name = "Date and Time";
    g_DateTime.settings.push_back(Setting(TIME, Setting::Type::Time, "Time", 0));
    g_DateTime.settings.push_back(Setting(DATE, Setting::Type::Date, "Date", 0));

    g_Location.name = "Location";
    g_Location.settings.push_back(Setting(LATITUDE, Setting::Type::Latitude, "Latitude", 0.0));
    g_Location.settings.push_back(Setting(LONGITUDE, Setting::Type::Longitude, "Longitude", 0.0));
    g_Location.settings.push_back(Setting(ELEVATION, Setting::Type::Elevation, "Elevation", 0.0));

    g_WiFi.name = "WiFi";
    g_WiFi.settings.push_back(Setting(WIFI_STATUS, Setting::Type::Info, "Status", "disconnected"));
    g_WiFi.settings.push_back(Setting(WIFI_SSID, Setting::Type::String, "SSID", ""));
    g_WiFi.settings.push_back(Setting(WIFI_PASSWORD, Setting::Type::String, "Password", "", true));

    g_Security.name = "Security";
    g_Security.settings.push_back(Setting(SECURITY_PASSWORD, Setting::Type::String, "Password", DEVICE_PASS, true));

    g_About.name = "About";
    g_About.settings.push_back(Setting("A0", Setting::Type::Info, "Version", VERSION));
}

};  // namespace Config

#endif