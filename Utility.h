#ifndef UTILITY_H
#define UTILITY_H

/**
 * This file contains utility functions.
 * Utility functions are functions that are used in multiple files.
 * Every function in this file MUST not depend on any other file,
 * except for the Definition.h file.
 */

#include "Definition.h"
#include "Display.h"

/*----- Task -----*/

void post(const std::function<void()>& runnable) {
    xSemaphoreTake(g_MainThreadQueueMutex, portMAX_DELAY);
    g_MainThreadQueue.push_back(runnable);
    xSemaphoreGive(g_MainThreadQueueMutex);
}

/*----- WiFi -----*/

void restartAP() {
    Setting& password = g_Security.getSetting(Config::SECURITY_PASSWORD);
    WiFi.softAPdisconnect();
    WiFi.softAP(g_Device.name.c_str(), password.value.c_str());
}

void reconnectSTA() {
    Setting& ssid     = g_WiFi.getSetting(Config::WIFI_SSID);
    Setting& password = g_WiFi.getSetting(Config::WIFI_PASSWORD);

    if (ssid.value.isNotEmpty()) {
        WiFi.disconnect();
        WiFi.begin(ssid.value.c_str(), password.value.c_str());
    }
}

/*----- RTTP -----*/

void publish(const String& topic, const Any& payload) {
    g_Server.publish(RTTP_CHANNEL, topic, RTTP::Message::Set, payload);
}

/*----- Time -----*/

DayOfWeek zeroOnSundayToDayOfWeek(const uint8_t& dow) {
    if (dow == 0) {
        return DayOfWeek::Sunday;
    }
    return static_cast<DayOfWeek>(dow);
}

UniTime::DateTime parseDateTime(const String& date, const uint32_t secondOfDay) {
    uint8_t dayOfMonth = date.substring(0, 2).toInt();
    uint8_t month      = date.substring(3, 5).toInt();
    uint16_t year      = date.substring(6, 10).toInt();
    uint8_t hour       = secondOfDay / 3600;
    uint8_t minute     = (secondOfDay % 3600) / 60;
    uint8_t second     = secondOfDay % 60;
    return UniTime::DateTime(year, month, dayOfMonth, hour, minute, second);
}

/*----- Prayer Time -----*/

void playNextSurah(bool fromStart = false);
void forceStopAudio();

QiroGroup& getCurrentQiroGroup() {
    DayOfWeek dow = zeroOnSundayToDayOfWeek(Time.now().dayOfWeek);
    switch (dow) {
        case DayOfWeek::Monday:
            return g_QiroMonday;
        case DayOfWeek::Tuesday:
            return g_QiroTuesday;
        case DayOfWeek::Wednesday:
            return g_QiroWednesday;
        case DayOfWeek::Thursday:
            return g_QiroThursday;
        case DayOfWeek::Friday:
            return g_QiroFriday;
        case DayOfWeek::Saturday:
            return g_QiroSaturday;
        default:
            return g_QiroSunday;
    }
}

void updatePrayerGroup() {
    double latitude  = g_Location.getSetting(Config::LATITUDE).value.toDouble();
    double longitude = g_Location.getSetting(Config::LONGITUDE).value.toDouble();
    double elevation = g_Location.getSetting(Config::ELEVATION).value.toDouble();

    UniTime::PrayerTime prayerTime = Time.prayerTime(latitude, longitude, elevation);

    g_PrayerGroup.fajr.time    = prayerTime.fajr.secondsOfTheDay();
    g_PrayerGroup.dhuhr.time   = prayerTime.dhuhr.secondsOfTheDay();
    g_PrayerGroup.asr.time     = prayerTime.asr.secondsOfTheDay();
    g_PrayerGroup.maghrib.time = prayerTime.maghrib.secondsOfTheDay();
    g_PrayerGroup.isha.time    = prayerTime.isha.secondsOfTheDay();

    g_PrayerGroup.setOffset(g_PrayerOffset);
    publish(RTTP_TOPIC_PRAYER_GROUP, g_PrayerGroup);
}

void updatePrayerQiroOngoing() {
    QiroGroup& qiroGroup = getCurrentQiroGroup();
    Prayer& activePrayer = g_PrayerGroup.getActivePrayer(Time.secondsOfTheDay());

    if (g_PrayerOngoing != activePrayer) {
        g_PrayerOngoing = activePrayer;
        g_QiroOngoing   = qiroGroup.getQiro(activePrayer.name);
        publish(RTTP_TOPIC_PRAYER_ONGOING, g_PrayerOngoing);
        publish(RTTP_TOPIC_QIRO_ONGOING, g_QiroOngoing);
        post(Display::showPrayerOngoing);
        post(Display::showSurahOngoing);
        g_IsQiroCancelled = false;
    }
}

void checkPrayerTime() {
    UniTime::Date today  = Time.now().toDate();
    uint32_t secondOfDay = Time.secondsOfTheDay();

    if (g_LastPrayerUpdateDate != today) {
        updatePrayerGroup();
        g_LastPrayerUpdateDate = today;
    }

    updatePrayerQiroOngoing();

    if (g_QiroOngoing.isActive(secondOfDay, g_PrayerOngoing)) {
        if (!g_SurahPreview.isPlaying && !g_SurahOngoing.isPlaying && !g_IsQiroCancelled) {
            playNextSurah(true);
        }
    } else {
        if (g_SurahOngoing.isPlaying) {
            forceStopAudio();
        }
    }
}

/*----- Audio -----*/

CountDownTimer g_AudioTimer;

void playNextSurah(bool fromStart) {
    static uint16_t index = 0;
    index                 = fromStart ? 0 : index + 1;

    if (index >= g_QiroOngoing.surahList.size()) {
        index = 0;
    }

    Surah surah              = g_QiroOngoing.surahList[index];
    g_SurahOngoing.id        = surah.id;
    g_SurahOngoing.volume    = surah.volume;
    g_SurahOngoing.isPlaying = true;
    g_SurahOngoing.isPaused  = false;

    g_AudioTimer.setDuration(5000);
    g_AudioTimer.setHandler([]() { playNextSurah(); });
    g_AudioTimer.start();

    g_DFPlayer.volume(surah.volume);
    g_DFPlayer.play(surah.id);

    publish(RTTP_TOPIC_SURAH_ONGOING, g_SurahOngoing);

    Display::isQiroActive = false;
    post(Display::showPrayerOngoing);
    post(Display::showSurahOngoing);
}

void playPreviewAudio(const SurahAudio& audio) {
    if (audio.isPlaying) {
        if (audio.isPaused) {
            if (g_SurahPreview.isPlaying && g_SurahPreview.id == audio.id) {
                g_DFPlayer.pause();
                g_SurahPreview.isPaused = true;
                publish(RTTP_TOPIC_SURAH_PREVIEW, g_SurahPreview);
            }
        } else {
            if (g_SurahPreview.isPlaying && g_SurahPreview.id == audio.id) {
                g_DFPlayer.start();
                g_SurahPreview.isPaused = false;
                publish(RTTP_TOPIC_SURAH_PREVIEW, g_SurahPreview);
            } else {
                g_DFPlayer.stop();
                g_DFPlayer.volume(audio.volume);
                g_DFPlayer.play(audio.id);
                g_SurahPreview.id        = audio.id;
                g_SurahPreview.volume    = audio.volume;
                g_SurahPreview.isPlaying = true;
                g_SurahPreview.isPaused  = false;
                publish(RTTP_TOPIC_SURAH_PREVIEW, g_SurahPreview);
            }
        }

        if (g_SurahOngoing.isPlaying) {
            g_SurahOngoing.isPlaying = false;
            g_SurahOngoing.isPaused  = false;
            publish(RTTP_TOPIC_SURAH_ONGOING, g_SurahOngoing);
        }
    } else {
        if (g_SurahPreview.isPlaying) {
            g_DFPlayer.stop();
            g_SurahPreview.isPlaying = false;
            g_SurahPreview.isPaused  = false;
            publish(RTTP_TOPIC_SURAH_PREVIEW, g_SurahPreview);
        }
        checkPrayerTime();
    }
}

void forceStopAudio() {
    g_SurahOngoing.isPlaying = false;
    g_SurahOngoing.isPaused  = false;
    g_DFPlayer.stop();
    publish(RTTP_TOPIC_SURAH_ONGOING, g_SurahOngoing);
    Log::info(TAG_AUDIO, "Stopped");
}

#endif