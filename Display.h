#ifndef DISPLAY_H
#define DISPLAY_H

/**
 * This file contains display related functions.
 *
 * DO NOT put any other functions in this file.
 */

#include "Config.h"
#include "Definition.h"

namespace Display {

String surahNames;
String deviceNames;

int16_t xSurah  = 0;
int16_t xDevice = 0;

bool isSurahScrolling  = false;
bool isDeviceScrolling = false;
bool isShowingDateTime = false;
bool isQiroActive      = false;
bool isOnHold          = true;

int16_t getTextWidth(const String& text) {
    return text.length() * g_OLED.getTextSizeX() * 6;
}

void centerHorizontal(const String& text, int y) {
    g_OLED.setCursor((g_OLED.width() - getTextWidth(text)) / 2, y);
    g_OLED.print(text);
}

void spaceBetween(const String& textLeft, const String& textRight, int y) {
    g_OLED.setCursor(0, y);
    g_OLED.print(textLeft);

    g_OLED.setCursor(g_OLED.width() - getTextWidth(textRight), y);
    g_OLED.print(textRight);
}

void clearDisplay(const uint16_t& start, const uint16_t& height) {
    g_OLED.fillRect(0, start, g_OLED.width(), height, SSD1306_BLACK);
}

String getSurahName(uint16_t index) {
    index        = constrain(index - 1, 0, g_SurahCollection.totalSize - 1);
    String surah = String(COLLECTIONS[index]);
    size_t start = surah.indexOf("\"");
    size_t end   = surah.indexOf("\"", start + 1);
    return surah.substring(start + 1, end);
}

void showBootMessage() {
    g_OLED.setTextColor(SSD1306_WHITE);
    g_OLED.clearDisplay();

    g_OLED.setTextSize(2);
    centerHorizontal("Kiro", 16);

    g_OLED.setTextSize(1);
    centerHorizontal("v" + g_Device.version, 36);
    g_OLED.display();
}

void showPrayerOngoing() {
    if (isShowingDateTime || isOnHold) {
        return;
    }

    g_OLED.setTextColor(SSD1306_WHITE);
    clearDisplay(0, 40);

    g_OLED.setTextSize(1);
    centerHorizontal(isQiroActive ? "Berlangsung" : "Selanjutnya", 0);

    String qiroDuration = g_QiroOngoing.durationMinutes == 0 ? " (Tidak aktif)"
                                                             : " (" + String(g_QiroOngoing.durationMinutes) + " menit)";
    centerHorizontal(g_PrayerOngoing.getNameString() + qiroDuration, 12);

    g_OLED.setTextSize(2);
    centerHorizontal(g_PrayerOngoing.getFormattedTime(), 24);
    g_OLED.display();
}

void showSurahOngoing() {
    if (isShowingDateTime || isOnHold) {
        return;
    }

    if (isQiroActive) {
        surahNames = getSurahName(g_SurahOngoing.id);
    } else {
        std::vector<Surah> surahList = g_QiroOngoing.surahList;
        surahNames.clear();
        bool first = true;

        for (const auto& surah : surahList) {
            if (first) {
                first = false;
            } else {
                surahNames += ", ";
            }

            surahNames += getSurahName(surah.id);
        }
    }

    g_OLED.setTextColor(SSD1306_WHITE);
    clearDisplay(44, 8);
    surahNames.trim();
    g_OLED.setTextSize(1);

    if (getTextWidth(surahNames) > g_OLED.width()) {
        isSurahScrolling = true;
        xSurah           = g_OLED.width();
    } else {
        isSurahScrolling = false;
        centerHorizontal(surahNames, 44);
    }

    g_OLED.display();
}

void showConnectedDevice() {
    if (isShowingDateTime) {
        return;
    }

    deviceNames.clear();
    bool first = true;

    for (const auto& client : g_Server.getSubscribers(RTTP_CHANNEL)) {
        if (!client.name.isEmpty()) {
            if (first) {
                first = true;
            } else {
                deviceNames += ", ";
            }
            deviceNames += client.name;
        }
    }

    g_OLED.setTextColor(SSD1306_WHITE);
    clearDisplay(56, 8);
    deviceNames.trim();
    g_OLED.setTextSize(1);

    if (getTextWidth(deviceNames) > g_OLED.width()) {
        isDeviceScrolling = true;
        xDevice           = g_OLED.width();
    } else {
        isDeviceScrolling = false;
        centerHorizontal(deviceNames, 56);
    }

    g_OLED.display();
}

void showDateTime() {
    UniTime::DateTime now = Time.now();
    g_OLED.clearDisplay();
    g_OLED.setTextColor(SSD1306_WHITE);

    g_OLED.setTextSize(2);
    centerHorizontal(now.format("HH:mm"), 4);

    g_OLED.setTextSize(1);
    centerHorizontal(now.format("dd MMMM yyyy"), 24);

    int16_t latitude  = g_Location.getSetting(Config::LATITUDE).value.toInt();
    int16_t longitude = g_Location.getSetting(Config::LONGITUDE).value.toInt();
    int16_t elevation = g_Location.getSetting(Config::ELEVATION).value.toInt();

    String location = abs(latitude) + String((latitude < 0) ? " LS" : " LU") + " " + abs(longitude)
                      + String((longitude < 0) ? " BB" : " BT");

    g_OLED.setTextSize(1);
    centerHorizontal(location, 40);
    centerHorizontal(String(elevation) + " mdpl", 52);
    g_OLED.display();
}

void switchDisplay() {
    static uint8_t counter = -1;
    counter++;

    if (counter > 2) {
        counter = 0;
    }

    if (counter == 0) {
        isShowingDateTime = true;
    } else if (counter == 1) {
        isShowingDateTime = false;
    } else {
        return;
    }

    isOnHold = false;

    if (isShowingDateTime) {
        g_OLED.clearDisplay();
        showDateTime();
    } else {
        g_OLED.clearDisplay();
        showPrayerOngoing();
        showSurahOngoing();
        showConnectedDevice();
    }
}

void scrollDisplay() {
    if (isShowingDateTime) {
        return;
    }

    if (isSurahScrolling) {
        clearDisplay(44, 8);
        g_OLED.setTextSize(1);
        g_OLED.setTextWrap(false);

        if (xSurah <= -getTextWidth(surahNames)) {
            xSurah = g_OLED.width();
        } else {
            xSurah--;
        }

        g_OLED.setCursor(xSurah, 44);
        g_OLED.print(surahNames);
        g_OLED.display();
    }

    if (isDeviceScrolling) {
        clearDisplay(56, 8);
        g_OLED.setTextSize(1);
        g_OLED.setTextWrap(false);

        if (xDevice <= -getTextWidth(deviceNames)) {
            xDevice = g_OLED.width();
        } else {
            xDevice--;
        }

        g_OLED.setCursor(xDevice, 56);
        g_OLED.print(deviceNames);
        g_OLED.display();
    }
}

};  // namespace Display

#endif