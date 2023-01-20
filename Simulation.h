#ifndef SIMULATION_H
#define SIMULATION_H

/**
 * This file contains simulation related functions.
 *
 * DO NOT put any other functions in this file.
 */

#include "Function.h"
#include "src/vendor/Simulator/TimeSimulator.h"

namespace Simulation {

TimeSimulator time;

void checkPrayerTime(UniTime::DateTime& dateTime) {
    uint32_t secondOfDay    = dateTime.secondsOfTheDay();
    UniTime::DateTime today = secondOfDay >= g_PrayerGroup.isha.getActualTime() ? dateTime.plusDays(1) : dateTime;

    if (g_LastPrayerUpdateDate != today.toDate()) {
        updatePrayerGroup(today);
        g_LastPrayerUpdateDate = today.toDate();
        Log::info(TAG_SIMULATOR, "Prayer group updated for %s", today.format("EEEE, dd MMMM yyyy").c_str());
        Log::info(
            TAG_SIMULATOR, "Fajr: %s, Dhuhr: %s, Asr: %s, Maghrib: %s, Isha: %s",
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
            time.jumpTo(
                UniTime::Time::fromSecondsOfTheDay(
                    g_PrayerOngoing.getActualTime() - (g_QiroOngoing.durationMinutes + 5) * 60
                ),
                false
            );
            Log::debug(TAG_SIMULATOR, "Jump to %s", time.now().format("EEEE, dd MMMM yyyy").c_str());
        } else {
            if (g_QiroOngoing.surahList.empty()) {
                time.jumpTo(
                    UniTime::Time::fromSecondsOfTheDay(
                        g_PrayerOngoing.getActualTime() - (g_QiroOngoing.durationMinutes + 5) * 60
                    ),
                    g_QiroOngoing.name != Prayer::Isha
                );
                Log::debug(TAG_SIMULATOR, "%s", "Surah list is empty");
                Log::debug(TAG_SIMULATOR, "Jump to %s", time.now().format("EEEE, dd MMMM yyyy").c_str());
            }
        }
    }
}

void onMinuteChange(UniTime::DateTime& dateTime) {
    Log::info(TAG_SIMULATOR, dateTime.format("EEEE PPPP, dd MMMM yyyy HH:mm:ss"));
    checkPrayerTime(dateTime);
}

void initialize() {
    time.adjust(UniTime::DateTime(2023, 1, 20, 4, 0, 0, UniTime::Timezone(7)));
    time.onMinuteChanged(onMinuteChange);
    time.setDivider(TimeSimulator::DIVIDER_20);
}

};  // namespace Simulation

#endif