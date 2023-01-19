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

const String G_DATE_TIME = "Date and Time";
const String G_LOCATION  = "Location";
const String G_WIFI      = "WiFi";
const String G_SECURITY  = "Security";
const String G_ABOUT     = "About";

const String TIME              = F("DT0");
const String DATE              = F("DT1");
const String LATITUDE          = F("L0");
const String LONGITUDE         = F("L1");
const String ELEVATION         = F("L2");
const String WIFI_STATUS       = F("W0");
const String WIFI_SSID         = F("W1");
const String WIFI_PASSWORD     = F("W2");
const String SECURITY_PASSWORD = F("S0");

String generateUUID() {
    String uuid;
    uint8_t address[6];
    esp_efuse_mac_get_default(address);
    
    for (uint8_t i = 0; i < DEVICE_MASK.length(); i++) {
        uuid += address[i % 6] ^ DEVICE_MASK[i];
    }
    
    uuid = Base64::encode(uuid);
    uuid.replace('+', 'K');
    uuid.replace('/', 'k');
    uuid.replace('=', 'z');
    
    return uuid;
}

String getSimpleID() {
    String id = g_Device.id.substring(g_Device.id.length() - 4);
    id.toUpperCase();
    return id;
}

void initialize() {
    g_Device.id      = generateUUID();
    g_Device.name    = DEVICE_NAME + " " + getSimpleID();
    g_Device.version = VERSION;

    g_DateTime.name = G_DATE_TIME;
    g_DateTime.settings.push_back(Setting(TIME, Setting::Type::Time, "Time", 0));
    g_DateTime.settings.push_back(Setting(DATE, Setting::Type::Date, "Date", 0));

    g_Location.name = G_LOCATION;
    g_Location.settings.push_back(Setting(LATITUDE, Setting::Type::Latitude, "Latitude", 0.0));
    g_Location.settings.push_back(Setting(LONGITUDE, Setting::Type::Longitude, "Longitude", 0.0));
    g_Location.settings.push_back(Setting(ELEVATION, Setting::Type::Elevation, "Elevation", 0.0));

    g_WiFi.name = G_WIFI;
    g_WiFi.settings.push_back(Setting(WIFI_STATUS, Setting::Type::Info, "Status", "disconnected"));
    g_WiFi.settings.push_back(Setting(WIFI_SSID, Setting::Type::WiFi, "SSID", ""));
    g_WiFi.settings.push_back(Setting(WIFI_PASSWORD, Setting::Type::WiFi, "Password", "", true));

    g_Security.name = G_SECURITY;
    g_Security.settings.push_back(Setting(SECURITY_PASSWORD, Setting::Type::String, "Password", DEVICE_PASS, true));

    g_About.name = G_ABOUT;
    g_About.settings.push_back(Setting("A0", Setting::Type::Info, "Version", VERSION));

    g_QiroMonday.dayOfWeek    = DayOfWeek::Monday;
    g_QiroTuesday.dayOfWeek   = DayOfWeek::Tuesday;
    g_QiroWednesday.dayOfWeek = DayOfWeek::Wednesday;
    g_QiroThursday.dayOfWeek  = DayOfWeek::Thursday;
    g_QiroFriday.dayOfWeek    = DayOfWeek::Friday;
    g_QiroSaturday.dayOfWeek  = DayOfWeek::Saturday;
    g_QiroSunday.dayOfWeek    = DayOfWeek::Sunday;

    g_SurahCollection.name = COLLECTION_NAME;
    g_SurahCollection.totalSize = sizeof(COLLECTIONS) / sizeof(COLLECTIONS[0]);
}

};  // namespace Config

#endif