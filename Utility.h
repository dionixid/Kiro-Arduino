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

    WiFi.disconnect();
    if (ssid.value.isNotEmpty()) {
        WiFi.begin(ssid.value.c_str(), password.value.c_str());
    }
}

/*----- Security -----*/

uint64_t getMacAddressInt() {
    uint8_t address[6];
    esp_efuse_mac_get_default(address);
    uint64_t mac = 18;
    for (uint8_t i = 0; i < 6; i++) {
        mac |= (uint64_t)(address[i] ^ DEVICE_MASK[i]) << (i * 8);
    }
    return mac;
}

void checkCounterfeit() {
    while (getMacAddressInt() != DEVICE_MAC) {
        delay(1000);
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

QiroGroup& getQiroGroup(const uint8_t& zeroBasedDOW) {
    DayOfWeek dow = zeroOnSundayToDayOfWeek(zeroBasedDOW);
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

void updatePrayerGroup(UniTime::DateTime dateTime) {
    double latitude  = g_Location.getSetting(Config::LATITUDE).value.toDouble();
    double longitude = g_Location.getSetting(Config::LONGITUDE).value.toDouble();
    double elevation = g_Location.getSetting(Config::ELEVATION).value.toDouble();

    UniTime::PrayerTime prayerTime = dateTime.toPrayerTime(latitude, longitude, elevation);

    g_PrayerGroup.fajr.time    = prayerTime.fajr.secondsOfTheDay();
    g_PrayerGroup.dhuhr.time   = prayerTime.dhuhr.secondsOfTheDay();
    g_PrayerGroup.asr.time     = prayerTime.asr.secondsOfTheDay();
    g_PrayerGroup.maghrib.time = prayerTime.maghrib.secondsOfTheDay();
    g_PrayerGroup.isha.time    = prayerTime.isha.secondsOfTheDay();

    g_PrayerGroup.setOffset(g_PrayerOffset);
    publish(RTTP_TOPIC_PRAYER_GROUP, g_PrayerGroup);
}

void updatePrayerQiroOngoing(UniTime::DateTime dateTime) {
    QiroGroup& qiroGroup = getQiroGroup(dateTime.dayOfWeek);
    Prayer& activePrayer = g_PrayerGroup.getActivePrayer(dateTime.secondsOfTheDay());

    if (g_PrayerOngoing != activePrayer) {
        g_PrayerOngoing = activePrayer;
        g_QiroOngoing   = qiroGroup.getQiro(activePrayer.name);
        publish(RTTP_TOPIC_PRAYER_ONGOING, g_PrayerOngoing);
        publish(RTTP_TOPIC_QIRO_ONGOING, g_QiroOngoing);
        post(Display::showPrayerOngoing);
        post(Display::showSurahOngoing);
        g_IsQiroCancelled = false;
        Log::info(
            TAG_PRAYER, "Ongoing: %s %s (%s)", g_PrayerOngoing.getNameString().c_str(),
            UniTime::Time::fromSecondsOfTheDay(g_PrayerOngoing.getActualTime()).format("HH:mm").c_str(),
            UniTime::Time::fromSecondsOfTheDay(g_PrayerOngoing.getActualTime() - g_QiroOngoing.durationMinutes * 60)
                .format("HH:mm")
                .c_str()
        );
    }
}

void checkPrayerTime() {
    uint32_t secondOfDay    = Time.secondsOfTheDay();
    UniTime::DateTime today = secondOfDay >= g_PrayerGroup.isha.getActualTime() ? Time.tomorrow() : Time.now();

    if (g_LastPrayerUpdateDate != today.toDate()) {
        updatePrayerGroup(today);
        g_LastPrayerUpdateDate = today.toDate();
        Log::info(TAG_PRAYER, "Prayer group has been updated");
        Log::info(
            TAG_PRAYER, "Fajr: %s, Dhuhr: %s, Asr: %s, Maghrib: %s, Isha: %s",
            UniTime::Time::fromSecondsOfTheDay(g_PrayerGroup.fajr.time).format("HH:mm").c_str(),
            UniTime::Time::fromSecondsOfTheDay(g_PrayerGroup.dhuhr.time).format("HH:mm").c_str(),
            UniTime::Time::fromSecondsOfTheDay(g_PrayerGroup.asr.time).format("HH:mm").c_str(),
            UniTime::Time::fromSecondsOfTheDay(g_PrayerGroup.maghrib.time).format("HH:mm").c_str(),
            UniTime::Time::fromSecondsOfTheDay(g_PrayerGroup.isha.time).format("HH:mm").c_str()
        );
    }

    updatePrayerQiroOngoing(today);

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

    g_AudioTimeoutTimer.setDuration(5000);
    g_AudioTimeoutTimer.setHandler([]() {
        Log::error(TAG_AUDIO, "Timed out waiting for audio to start");
        playNextSurah();
    });

    if (!g_Relay.get()) {
        g_Relay.set(true);
        g_AudioPlayTimer.setDuration(5000);
        g_AudioPlayTimer.setHandler([surah]() {
            g_AudioTimeoutTimer.start();
            g_DFPlayer.volume(surah.volume);
            g_DFPlayer.play(surah.id);
        });
        g_AudioPlayTimer.start();
    } else {
        g_AudioTimeoutTimer.start();
        g_DFPlayer.volume(surah.volume);
        g_DFPlayer.play(surah.id);
    }

    publish(RTTP_TOPIC_SURAH_ONGOING, g_SurahOngoing);

    Display::isQiroActive = false;
    post(Display::showPrayerOngoing);
    post(Display::showSurahOngoing);
    Log::info(TAG_AUDIO, "Playing %d - %s", surah.id, Display::getSurahName(surah.id).c_str());
}

void playPreviewAudio(const SurahAudio& audio) {
    if (audio.id == g_SurahPreview.id && audio.isPlaying == g_SurahPreview.isPlaying
        && audio.isPaused == g_SurahPreview.isPaused) {
        g_DFPlayer.volume(audio.volume);
        g_SurahPreview.volume = audio.volume;
        return;
    }

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
                g_Relay.set(true);
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
            g_Relay.set(false);
            g_SurahPreview.isPlaying = false;
            g_SurahPreview.isPaused  = false;
            publish(RTTP_TOPIC_SURAH_PREVIEW, g_SurahPreview);
        }
        checkPrayerTime();
    }
}

void forceStopAudio() {
    g_AudioPlayTimer.cancel();
    g_AudioTimeoutTimer.cancel();
    Display::isQiroActive    = false;
    g_SurahOngoing.isPlaying = false;
    g_SurahOngoing.isPaused  = false;
    g_DFPlayer.stop();
    g_Relay.set(false);
    publish(RTTP_TOPIC_SURAH_ONGOING, g_SurahOngoing);
    post(Display::showPrayerOngoing);
    post(Display::showSurahOngoing);
    Log::info(TAG_AUDIO, "Stopped");
}

#endif