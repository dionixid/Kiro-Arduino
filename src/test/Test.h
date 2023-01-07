#ifndef DEVICE_UNIT_TEST_H
#define DEVICE_UNIT_TEST_H

#include "../vendor/UnitTest/UnitTest.h"
#include "../model/Device.h"
#include "../model/Prayer.h"
#include "../model/PrayerGroup.h"
#include "../model/PrayerTimeOffset.h"
#include "../model/Qiro.h"
#include "../model/QiroGroup.h"
#include "../model/Setting.h"
#include "../model/SettingGroup.h"
#include "../model/Surah.h"
#include "../model/SurahAudio.h"
#include "../model/SurahCollection.h"
#include "../model/SurahProperties.h"

namespace Test {

UnitTest::Result runDevice(Print& printer) {
    UnitTest device("Device Unit Test");

    device.assertEqual(
        "Device_SerializationIsCorrect", "{\"id\",\"name\",\"version\"}", Device("id", "name", "version").serialize()
    );

    device.assertEqual(
        "Device_DeserializationIsCorrect", Device("id", "name", "version"),
        Any::parse("{\"id\",\"name\",\"version\"}").as<Device>()
    );

    device.attach(printer);
    return device.run();
}

UnitTest::Result runPrayer(Print& printer) {
    UnitTest prayer("Prayer Unit Test");

    prayer.assertEqual("Prayer_SerializationIsCorrect", "{2,36000,2}", Prayer(Prayer::Name::Asr, 36000, 2).serialize());

    prayer.assertEqual(
        "Prayer_DeserializationIsCorrect", Prayer(Prayer::Name::Asr, 36000, 2), Any::parse("{2,36000,2}").as<Prayer>()
    );

    prayer.attach(printer);
    return prayer.run();
}

UnitTest::Result runPrayerGroup(Print& printer) {
    UnitTest prayerGroup("PrayerGroup Unit Test");

    prayerGroup.assertEqual(
        "PrayerGroup_SerializationIsCorrect", "{{0,36000,2},{1,36000,2},{2,36000,2},{3,36000,2},{4,36000,2}}",
        PrayerGroup(
            Prayer(Prayer::Name::Fajr, 36000, 2), Prayer(Prayer::Name::Dhuhr, 36000, 2),
            Prayer(Prayer::Name::Asr, 36000, 2), Prayer(Prayer::Name::Maghrib, 36000, 2),
            Prayer(Prayer::Name::Isha, 36000, 2)
        )
            .serialize()
    );

    prayerGroup.assertEqual(
        "PrayerGroup_DeserializationIsCorrect",
        PrayerGroup(
            Prayer(Prayer::Name::Fajr, 36000, 2), Prayer(Prayer::Name::Dhuhr, 36000, 2),
            Prayer(Prayer::Name::Asr, 36000, 2), Prayer(Prayer::Name::Maghrib, 36000, 2),
            Prayer(Prayer::Name::Isha, 36000, 2)
        ),
        Any::parse("{{0,36000,2},{1,36000,2},{2,36000,2},{3,36000,2},{4,36000,2}}").as<PrayerGroup>()
    );

    prayerGroup.attach(printer);
    return prayerGroup.run();
}

UnitTest::Result runPrayerTimeOffset(Print& printer) {
    UnitTest prayerTimeOffset("PrayerTimeOffset Unit Test");

    prayerTimeOffset.assertEqual(
        "PrayerTimeOffset_SerializationIsCorrect", "{1,2,3,4,5}", PrayerTimeOffset(1, 2, 3, 4, 5).serialize()
    );

    prayerTimeOffset.assertEqual(
        "PrayerTimeOffset_DeserializationIsCorrect", PrayerTimeOffset(1, 2, 3, 4, 5),
        Any::parse("{1,2,3,4,5}").as<PrayerTimeOffset>()
    );

    prayerTimeOffset.attach(printer);
    return prayerTimeOffset.run();
}

UnitTest::Result runQiro(Print& printer) {
    UnitTest qiro("Qiro Unit Test");

    qiro.assertEqual(
        "Qiro_SerializationIsCorrect", "{3,10,[{0,20},{1,20},{2,20}]}",
        Qiro(Prayer::Name::Maghrib, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)}).serialize()
    );

    qiro.assertEqual(
        "Qiro_DeserializationIsCorrect", Qiro(Prayer::Name::Maghrib, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)}),
        Any::parse("{3,10,[{0,20},{1,20},{2,20}]}").as<Qiro>()
    );

    qiro.attach(printer);
    return qiro.run();
}

UnitTest::Result runQiroGroup(Print& printer) {
    UnitTest qiroGroup("QiroGroup Unit Test");

    qiroGroup.assertEqual(
        "QiroGroup_SerializationIsCorrect",
        "{3,{0,10,[{0,20},{1,20},{2,20}]},{1,10,[{0,20},{1,20},{2,20}]},{2,10,[{0,20},{1,20},{2,20}]},{3,10,[{0,20},{1,"
        "20},{2,20}]},{4,10,[{0,20},{1,20},{2,20}]}}",
        QiroGroup(
            DayOfWeek::Wednesday, Qiro(Prayer::Name::Fajr, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)}),
            Qiro(Prayer::Name::Dhuhr, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)}),
            Qiro(Prayer::Name::Asr, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)}),
            Qiro(Prayer::Name::Maghrib, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)}),
            Qiro(Prayer::Name::Isha, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)})
        )
            .serialize()
    );

    qiroGroup.assertEqual(
        "QiroGroup_DeserializationIsCorrect",
        QiroGroup(
            DayOfWeek::Wednesday, Qiro(Prayer::Name::Fajr, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)}),
            Qiro(Prayer::Name::Dhuhr, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)}),
            Qiro(Prayer::Name::Asr, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)}),
            Qiro(Prayer::Name::Maghrib, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)}),
            Qiro(Prayer::Name::Isha, 10, {Surah(0, 20), Surah(1, 20), Surah(2, 20)})
        ),
        Any::parse("{3,{0,10,[{0,20},{1,20},{2,20}]},{1,10,[{0,20},{1,20},{2,20}]},{2,10,[{0,20},{1,20},{2,20}]},{3,10,"
                   "[{0,20},{1,20},{2,20}]},{4,10,[{0,20},{1,20},{2,20}]}}")
            .as<QiroGroup>()
    );

    qiroGroup.attach(printer);
    return qiroGroup.run();
}

UnitTest::Result runSetting(Print& printer) {
    UnitTest setting("Setting Unit Test");

    setting.assertEqual(
        "Setting_SerializationIsCorrect", "{\"id\",6,\"Password\",\"12345678\",true}",
        Setting("id", Setting::Type::WiFi, "Password", "12345678", true).serialize()
    );

    setting.assertEqual(
        "Setting_DeserializationIsCorrect", Setting("id", Setting::Type::WiFi, "Password", "12345678", true),
        Any::parse("{\"id\",6,\"Password\",\"12345678\",true}").as<Setting>()
    );

    setting.attach(printer);
    return setting.run();
}

UnitTest::Result runSettingGroup(Print& printer) {
    UnitTest settingGroup("SettingGroup Unit Test");

    settingGroup.assertEqual(
        "SettingGroup_SerializationIsCorrect",
        "{\"Date and Time\",[{\"DT0\",5,\"Time\",36000,false},{\"DT1\",4,\"Date\",\"01-01-1972\",false}]}",
        SettingGroup(
            "Date and Time", {Setting("DT0", Setting::Type::Time, "Time", 36000, false),
                              Setting("DT1", Setting::Type::Date, "Date", "01-01-1972", false)}
        )
            .serialize()
    );

    settingGroup.assertEqual(
        "SettingGroup_DeserializationIsCorrect",
        SettingGroup(
            "Date and Time", {Setting("DT0", Setting::Type::Time, "Time", 36000, false),
                              Setting("DT1", Setting::Type::Date, "Date", "01-01-1972", false)}
        ),
        Any::parse("{\"Date and Time\",[{\"DT0\",5,\"Time\",36000,false},{\"DT1\",4,\"Date\",\"01-01-1972\",false}]}")
            .as<SettingGroup>()
    );

    settingGroup.attach(printer);
    return settingGroup.run();
}

UnitTest::Result runSurah(Print& printer) {
    UnitTest surah("Surah Unit Test");

    surah.assertEqual("Surah_SerializationIsCorrect", "{25,20}", Surah(25, 20).serialize());

    surah.assertEqual("Surah_DeserializationIsCorrect", Surah(25, 20), Any::parse("{25,20}").as<Surah>());

    surah.attach(printer);
    return surah.run();
}

UnitTest::Result runSurahAudio(Print& printer) {
    UnitTest surahAudio("SurahAudio Unit Test");

    surahAudio.assertEqual(
        "SurahAudio_SerializationIsCorrect", "{25,20,false,true}", SurahAudio(25, 20, false, true).serialize()
    );

    surahAudio.assertEqual(
        "SurahAudio_DeserializationIsCorrect", SurahAudio(25, 20, false, true),
        Any::parse("{25,20,false,true}").as<SurahAudio>()
    );

    surahAudio.attach(printer);
    return surahAudio.run();
}

UnitTest::Result runSurahProperties(Print& printer) {
    UnitTest surahProperties("SurahProperties Unit Test");

    surahProperties.assertEqual(
        "SurahProperties_SerializationIsCorrect", "{25,\"name\",20,600}",
        SurahProperties(25, "name", 20, 600).serialize()
    );

    surahProperties.assertEqual(
        "SurahProperties_DeserializationIsCorrect", SurahProperties(25, "name", 20, 600),
        Any::parse("{25,\"name\",20,600}").as<SurahProperties>()
    );

    surahProperties.attach(printer);
    return surahProperties.run();
}

UnitTest::Result runSurahCollection(Print& printer) {
    UnitTest surahCollection("SurahCollection Unit Test");

    surahCollection.assertEqual(
        "surahCollection_SerializationIsCorrect", "{\"name\",32,12}", SurahCollection("name", 32, 12).serialize()
    );

    surahCollection.assertEqual(
        "surahCollection_DeserializationIsCorrect", SurahCollection("name", 32, 12),
        Any::parse("{\"name\",32,12}").as<SurahCollection>()
    );

    surahCollection.attach(printer);
    return surahCollection.run();
}

UnitTest::Result runAll(Print& printer) {
    UnitTest::Result result;

    result += runDevice(printer);
    result += runPrayer(printer);
    result += runPrayerGroup(printer);
    result += runPrayerTimeOffset(printer);
    result += runQiro(printer);
    result += runQiroGroup(printer);
    result += runSetting(printer);
    result += runSettingGroup(printer);
    result += runSurah(printer);
    result += runSurahAudio(printer);
    result += runSurahProperties(printer);
    result += runSurahCollection(printer);

    printer.printf(
        "Finished %d tests with %d passed and %d failed.", result.passed + result.failed, result.passed, result.failed
    );

    return result;
}

};  // namespace Test

#endif