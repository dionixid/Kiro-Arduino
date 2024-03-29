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

/*----- Button -----*/

void onButtonPressed() {
    if (g_SurahOngoing.isPlaying) {
        g_IsQiroCancelled = true;
        forceStopAudio();
    } else if (g_SurahPreview.isPlaying) {
        g_SurahPreview.isPlaying = false;
        g_SurahPreview.isPaused  = false;
        g_DFPlayer.stop();
        g_Relay.set(false);
        publish(RTTP_TOPIC_SURAH_PREVIEW, g_SurahPreview);
    } else {
        g_IsQiroCancelled = false;
        checkPrayerTime();
    }
    Log::debug(TAG_BUTTON, "Pressed");
}

/*----- Audio -----*/

void onStartPlayingAudio() {
    g_AudioTimeoutTimer.cancel();
}

void onFinishedPlayingAudio() {
    if (g_SurahPreview.isPlaying && !g_SurahPreview.isPaused) {
        g_SurahPreview.isPlaying = false;
        g_SurahPreview.isPaused  = false;
        publish(RTTP_TOPIC_SURAH_PREVIEW, g_SurahPreview);
    }

    if (g_SurahOngoing.isPlaying) {
        playNextSurah();
    }

#if !SIMULATION
    checkPrayerTime();
#endif

    if (g_SurahPreview.isPaused) {
        Log::info(TAG_AUDIO, "Paused");
    } else {
        Log::info(TAG_AUDIO, "Finished");
    }
}

/*----- Time -----*/

void onMinuteChange() {
    Log::info(TAG_TIME, Time.now().format("EEEE PPPP, dd MMMM yyyy HH:mm:ss"));
    g_DateTime.getSetting(Config::TIME).value = Time.secondsOfTheDay();
    g_DateTime.getSetting(Config::DATE).value = Time.now().format("dd-MM-yyyy");
    publish(RTTP_TOPIC_SETTING_GROUP, g_DateTime);
    checkPrayerTime();
}

/*----- Database -----*/

void loadDatabase() {
    if (g_DB.contains(KEY_SETTING_LOCATION)) {
        g_Location = g_DB.get(KEY_SETTING_LOCATION);
    }

    if (g_DB.contains(KEY_SETTING_WIFI)) {
        g_WiFi = g_DB.get(KEY_SETTING_WIFI);
    }

    if (g_DB.contains(KEY_SETTING_SECURITY)) {
        g_Security = g_DB.get(KEY_SETTING_SECURITY);
    }

    if (g_DB.contains(KEY_PRAYER_OFFSET)) {
        g_PrayerOffset = g_DB.get(KEY_PRAYER_OFFSET);
    }

    if (g_DB.contains(KEY_SCHEDULE_MONDAY)) {
        g_QiroMonday = g_DB.get(KEY_SCHEDULE_MONDAY);
    }

    if (g_DB.contains(KEY_SCHEDULE_TUESDAY)) {
        g_QiroTuesday = g_DB.get(KEY_SCHEDULE_TUESDAY);
    }

    if (g_DB.contains(KEY_SCHEDULE_WEDNESDAY)) {
        g_QiroWednesday = g_DB.get(KEY_SCHEDULE_WEDNESDAY);
    }

    if (g_DB.contains(KEY_SCHEDULE_THURSDAY)) {
        g_QiroThursday = g_DB.get(KEY_SCHEDULE_THURSDAY);
    }

    if (g_DB.contains(KEY_SCHEDULE_FRIDAY)) {
        g_QiroFriday = g_DB.get(KEY_SCHEDULE_FRIDAY);
    }

    if (g_DB.contains(KEY_SCHEDULE_SATURDAY)) {
        g_QiroSaturday = g_DB.get(KEY_SCHEDULE_SATURDAY);
    }

    if (g_DB.contains(KEY_SCHEDULE_SUNDAY)) {
        g_QiroSunday = g_DB.get(KEY_SCHEDULE_SUNDAY);
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

/*----- Discovery Service -----*/

void onPacket(AsyncUDPPacket packet) {
    String payload      = packet.readString();
    IPAddress remoteIP  = packet.remoteIP();
    uint16_t remotePort = packet.remotePort();
    String localIP      = remoteIP.toString().startsWith("192.168.4.") ? "192.168.4.1" : WiFi.localIP().toString();

    if (payload.equals("_kiro._tcp")) {
        g_UDPMessage.flush();
        g_UDPMessage.print("_kiro._tcp.name:Kiro.id:" + Config::getSimpleID() + ".ip:" + localIP + ".local.");
        g_UDP.sendTo(g_UDPMessage, remoteIP, remotePort);
    }

    Log::info(
        TAG_UDP, F("Device has been scanned. Payload: %s, ip: %s, port: %s"), payload.c_str(),
        remoteIP.toString().c_str(), String(remotePort).c_str()
    );
}

/*----- Network -----*/

void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event) {
        case SYSTEM_EVENT_STA_CONNECTED: {
            g_WiFi.getSetting(Config::WIFI_STATUS).value = "connected";
            publish(RTTP_TOPIC_SETTING_GROUP, g_WiFi);
            break;
        }
        case SYSTEM_EVENT_STA_DISCONNECTED: {
            g_WiFi.getSetting(Config::WIFI_STATUS).value = "disconnected";
            publish(RTTP_TOPIC_SETTING_GROUP, g_WiFi);
            if (!WiFi.SSID().isEmpty()) {
                Log::info(TAG_WIFI, F("Disconnected from %s"), WiFi.SSID().c_str());
            }
            break;
        }
        case SYSTEM_EVENT_STA_GOT_IP: {
            Log::info(TAG_WIFI, F("Connected with IP: %s"), WiFi.localIP().toString().c_str());
            break;
        }
        case SYSTEM_EVENT_AP_STACONNECTED: {
            Log::info(TAG_WIFI, F("Client connected"));
            break;
        }
        case SYSTEM_EVENT_AP_STAIPASSIGNED: {
            Log::info(TAG_WIFI, F("IP assigned: %s"), IPAddress(info.wifi_ap_staipassigned.ip.addr).toString().c_str());
            break;
        }
        case SYSTEM_EVENT_AP_STADISCONNECTED: {
            Log::info(TAG_WIFI, F("Client disconnected"));
            break;
        }
    }
}

void initializeNetwork() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.setHostname(String("kiro-" + Config::getSimpleID()).c_str());
    WiFi.softAPsetHostname(String("kiro-" + Config::getSimpleID()).c_str());
    WiFi.onEvent(onWiFiEvent);

    MAC_ADDRESS[4] = getMacAddressInt() >> 32;
    MAC_ADDRESS[5] = getMacAddressInt() >> 40;

    MAC_ADDRESS[3] = 0x01;
    esp_wifi_set_mac(WIFI_IF_AP, MAC_ADDRESS);
    MAC_ADDRESS[3] = 0x02;
    esp_wifi_set_mac(WIFI_IF_STA, MAC_ADDRESS);

    restartAP();
    reconnectSTA();

    g_UDP.listen(UDP_PORT);
    g_UDP.onPacket(onPacket);

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

/*----- Task -----*/

void runMainQueue() {
    if (g_MainThreadQueue.empty()) {
        return;
    }

    xSemaphoreTake(g_MainThreadQueueMutex, portMAX_DELAY);
    std::vector<std::function<void()>> queue = g_MainThreadQueue;
    g_MainThreadQueue.clear();
    xSemaphoreGive(g_MainThreadQueueMutex);

    for (auto& runnable : queue) {
        if (runnable) {
            runnable();
        }
    }
}

#endif