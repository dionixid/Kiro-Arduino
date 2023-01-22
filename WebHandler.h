#ifndef WEB_HANDLER_H
#define WEB_HANDLER_H

/**
 * This file contains web related functions.
 * For example, an implementation of the web server.
 *
 * DO NOT put any other functions in this file.
 */

#include "Config.h"
#include "Definition.h"
#include "Utility.h"

void onJoin(const String& ip, const uint16_t& port, const uint8_t& count) {
    Log::info(TAG_RTTP, "Client joined: %s:%d (%d)", ip.c_str(), port, count);
}

bool onAuth(const RTTP::Auth& auth) {
    bool accepted = auth.secret == g_Security.getSetting(Config::SECURITY_PASSWORD).value.toString();
    Log::info(TAG_RTTP, "%s has been %s", auth.name.c_str(), accepted ? "authenticated" : "rejected");
    return accepted;
}

void onAuthenticated(const RTTP::Auth& auth) {
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_PRAYER_GROUP, RTTP::Message::Set, g_PrayerGroup);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_PRAYER_ONGOING, RTTP::Message::Set, g_PrayerOngoing);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_QIRO_ONGOING, RTTP::Message::Set, g_QiroOngoing);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_QIRO_GROUP, RTTP::Message::Set, g_QiroMonday);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_QIRO_GROUP, RTTP::Message::Set, g_QiroTuesday);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_QIRO_GROUP, RTTP::Message::Set, g_QiroWednesday);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_QIRO_GROUP, RTTP::Message::Set, g_QiroThursday);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_QIRO_GROUP, RTTP::Message::Set, g_QiroFriday);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_QIRO_GROUP, RTTP::Message::Set, g_QiroSaturday);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_QIRO_GROUP, RTTP::Message::Set, g_QiroSunday);

    Array setting;
    setting.push(g_DateTime);
    setting.push(g_Location);
    setting.push(g_WiFi);
    setting.push(g_Security);
    setting.push(g_About);

    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_SETTING_ALL, RTTP::Message::Set, setting);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_SURAH_ONGOING, RTTP::Message::Set, g_SurahOngoing);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_SURAH_PREVIEW, RTTP::Message::Set, g_SurahPreview);
    g_Server.send(auth.id, RTTP_CHANNEL, RTTP_TOPIC_DEVICE, RTTP::Message::Set, g_Device);
    post(Display::showConnectedDevice);
}

void onLeave(const String& ip, const uint16_t& port, const uint8_t& count) {
    Log::info(TAG_RTTP, "Client left: %s:%d (%d)", ip.c_str(), port, count);
    post(Display::showConnectedDevice);
}

void onTopicPrayerOffset(const RTTP::Message& message) {
    if (message.recipientId != RTTP::SERVER_ID || message.action != RTTP::Message::Set) {
        return;
    }

    PrayerTimeOffset offset = message.payload;
    if (!offset) {
        return;
    }

    g_PrayerOffset = offset;
    updatePrayerGroup(Time.now());
    checkPrayerTime();
    g_DB.put(KEY_PRAYER_OFFSET, g_PrayerOffset);
}

void onTopicQiroGroup(const RTTP::Message& message) {
    if (message.recipientId != RTTP::SERVER_ID || message.action != RTTP::Message::Set) {
        return;
    }

    QiroGroup group = message.payload;
    if (!group) {
        return;
    }

    if (group.dayOfWeek == DayOfWeek::Monday) {
        g_QiroMonday = group;
        g_DB.put(KEY_SCHEDULE_MONDAY, group);
    }

    if (group.dayOfWeek == DayOfWeek::Tuesday) {
        g_QiroTuesday = group;
        g_DB.put(KEY_SCHEDULE_TUESDAY, group);
    }

    if (group.dayOfWeek == DayOfWeek::Wednesday) {
        g_QiroWednesday = group;
        g_DB.put(KEY_SCHEDULE_WEDNESDAY, group);
    }

    if (group.dayOfWeek == DayOfWeek::Thursday) {
        g_QiroThursday = group;
        g_DB.put(KEY_SCHEDULE_THURSDAY, group);
    }

    if (group.dayOfWeek == DayOfWeek::Friday) {
        g_QiroFriday = group;
        g_DB.put(KEY_SCHEDULE_FRIDAY, group);
    }

    if (group.dayOfWeek == DayOfWeek::Saturday) {
        g_QiroSaturday = group;
        g_DB.put(KEY_SCHEDULE_SATURDAY, group);
    }

    if (group.dayOfWeek == DayOfWeek::Sunday) {
        g_QiroSunday = group;
        g_DB.put(KEY_SCHEDULE_SUNDAY, group);
    }

    g_Server.publish(RTTP_CHANNEL, message.topic, RTTP::Message::Set, group);

    if (zeroOnSundayToDayOfWeek(Time.now().dayOfWeek) == group.dayOfWeek) {
        g_QiroOngoing = group.getQiro(g_PrayerOngoing.name);
        g_Server.publish(RTTP_CHANNEL, RTTP_TOPIC_QIRO_ONGOING, RTTP::Message::Set, g_QiroOngoing);
        checkPrayerTime();
    }
}

void onTopicSettingGroup(const RTTP::Message& message) {
    if (message.recipientId != RTTP::SERVER_ID || message.action != RTTP::Message::Set) {
        return;
    }

    SettingGroup group = message.payload;
    if (!group) {
        return;
    }

    if (group.name == Config::G_DATE_TIME) {
        Time.adjust(
            parseDateTime(group.getSetting(Config::DATE).value.toString(), group.getSetting(Config::TIME).value.toInt())
        );
        g_DateTime.getSetting(Config::TIME).value = Time.secondsOfTheDay();
        g_DateTime.getSetting(Config::DATE).value = Time.now().format("dd-MM-yyyy");
        g_Server.publish(RTTP_CHANNEL, RTTP_TOPIC_SETTING_GROUP, RTTP::Message::Set, g_DateTime);
        updatePrayerGroup(Time.secondsOfTheDay() >= g_PrayerGroup.isha.getActualTime() ? Time.tomorrow() : Time.now());
        checkPrayerTime();
        return;
    }

    if (group.name == Config::G_SECURITY) {
        g_Security.getSetting(Config::SECURITY_PASSWORD).value = group.getSetting(Config::SECURITY_PASSWORD).value;
        g_Server.publish(RTTP_CHANNEL, RTTP_TOPIC_SETTING_GROUP, RTTP::Message::Set, g_Security);
        g_DB.put(KEY_SETTING_SECURITY, g_Security);
        Timer::setTimeout(1000, []() { restartAP(); });
        return;
    }

    if (group.name == Config::G_WIFI) {
        g_WiFi.getSetting(Config::WIFI_SSID).value     = group.getSetting(Config::WIFI_SSID).value;
        g_WiFi.getSetting(Config::WIFI_PASSWORD).value = group.getSetting(Config::WIFI_PASSWORD).value;
        g_Server.publish(RTTP_CHANNEL, RTTP_TOPIC_SETTING_GROUP, RTTP::Message::Set, g_WiFi);
        g_DB.put(KEY_SETTING_WIFI, g_WiFi);
        Timer::setTimeout(1000, []() { reconnectSTA(); });
        return;
    }

    if (group.name == Config::G_LOCATION) {
        g_Location.getSetting(Config::LATITUDE).value  = group.getSetting(Config::LATITUDE).value;
        g_Location.getSetting(Config::LONGITUDE).value = group.getSetting(Config::LONGITUDE).value;
        g_Location.getSetting(Config::ELEVATION).value = group.getSetting(Config::ELEVATION).value;
        g_Server.publish(RTTP_CHANNEL, RTTP_TOPIC_SETTING_GROUP, RTTP::Message::Set, g_Location);
        g_DB.put(KEY_SETTING_LOCATION, g_Location);
        updatePrayerGroup(Time.secondsOfTheDay() >= g_PrayerGroup.isha.getActualTime() ? Time.tomorrow() : Time.now());
        checkPrayerTime();
    }
}

void onTopicSurahPreview(const RTTP::Message& message) {
    if (message.recipientId != RTTP::SERVER_ID || message.action != RTTP::Message::Set) {
        return;
    }

    SurahAudio preview = message.payload;
    if (!preview) {
        return;
    }

    playPreviewAudio(preview);
}

void onTopicSurahCollection(const RTTP::Message& message) {
    if (message.recipientId != RTTP::SERVER_ID || message.action != RTTP::Message::Get) {
        return;
    }
    g_Server.send(message.senderId, RTTP_CHANNEL, message.topic, RTTP::Message::Set, g_SurahCollection);
}

void onTopicSurahForceStop(const RTTP::Message& message) {
    if (message.recipientId != RTTP::SERVER_ID || message.action != RTTP::Message::Set) {
        return;
    }

    if (message.payload.isBool() && message.payload) {
        g_IsQiroCancelled = true;
        forceStopAudio();
    }
}

void onTopicSurahList(const RTTP::Message& message) {
    if (message.recipientId != RTTP::SERVER_ID || message.action != RTTP::Message::Get) {
        return;
    }

    uint16_t progress = 0;
    uint16_t total    = g_SurahCollection.totalSize;

    while (progress < total) {
        uint16_t size = min(total - progress, 10);
        Array surahList;

        for (uint16_t i = progress; i < progress + size; i++) {
            surahList.push(Raw(COLLECTIONS[i]));
        }

        g_Server.send(message.senderId, RTTP_CHANNEL, message.topic, RTTP::Message::Set, surahList);
        progress += size;
        delay(100);
    }
}

#endif