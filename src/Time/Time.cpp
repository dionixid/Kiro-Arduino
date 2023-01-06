#include "Time.h"

#ifdef ESP32
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#endif

#include <Wire.h>

using UTime = UniTime::UniTime;
using UniTime::Date;
using UniTime::DateTime;
using UniTime::DS3231;
using UniTime::gregorianMonthNames;
using UniTime::HijriDate;
using UniTime::hijriMonthNames;
using UniTime::pasaranDayNames;
using UniTime::PrayerTime;
using UniTime::solarDayNames;
using UniTime::SolarTime;
using UniTime::Timezone;

String UniTime::solarDayNames[7] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
String UniTime::pasaranDayNames[5] = {"Legi", "Pahing", "Pon", "Wage", "Kliwon"};
String UniTime::gregorianMonthNames[12] = {"Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"};
String UniTime::hijriMonthNames[12] = {"Muharram", "Safar", "Rabiul Awal", "Rabiul Akhir", "Jumadil Awal", "Jumadil Akhir", "Rajab", "Sya'ban", "Ramadhan", "Syawal", "Dzulkaidah", "Dzulhijjah"};
uint16_t UniTime::gregorianDayCount[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
uint16_t UniTime::hijriDayCount[12] = {0, 30, 59, 89, 118, 148, 177, 207, 236, 266, 295, 325};

#if defined(ESP32) || defined(ESP8266)
WiFiUDP UTime::_udp;
#endif
DS3231 UTime::_rtc;
boolean UTime::_isRtcEnabled = false;
boolean UTime::_isNtpEnabled = false;
boolean UTime::_isTimeUpdateRegistered = false;
boolean UTime::_isChangeDetectorRegistered = false;
int32_t UTime::_timezone = 0;
uint32_t UTime::_lastUpdate = 0;
uint32_t UTime::_lastTimestamp = 0;
uint32_t UTime::_lastMinute = 0;
uint32_t UTime::_lastHour = 0;
uint32_t UTime::_lastDay = 0;
UTime::VoidCallback UTime::_onUpdate = NULL;
UTime::VoidCallback UTime::_onMinuteChanged = NULL;
UTime::VoidCallback UTime::_onHourChanged = NULL;
UTime::VoidCallback UTime::_onDayChanged = NULL;

const int DS3231::DS3231_ADDRESS = 0x68;
const int DS3231::DS3231_CONTROL = 0x0E;
const int DS3231::DS3231_STATUSREG = 0x0F;
const int DS3231::DS3231_TEMPERATUREREG = 0x11;

static uint32_t calculateUnixTimestamp(const uint16_t &year, const uint8_t &month, const uint8_t &date, const uint8_t &hour, const uint8_t &minute, const uint8_t &second);
static String format(const String &fmt, const bool &isHijri, const uint16_t &year = 1970, const uint8_t &month = 1, const uint8_t &date = 1, const uint8_t &hour = 0, const uint8_t &minute = 0, const uint8_t &second = 0, const uint8_t &dayOfWeek = 0, const uint8_t &pasaran = 0, const Timezone &timezone = Timezone());
static bool isLeapYear(const uint16_t &year);
static String padZero(const uint8_t &value);

/*------ Timezone ------*/
Timezone::Timezone()
    : hour(0), minute(0) {}

Timezone::Timezone(const double &offset)
    : hour((int)offset), minute((int)((offset - hour) * 60)) {}

Timezone::Timezone(const int8_t &hour, const int8_t &minute)
    : hour(hour), minute(minute) {}

double Timezone::toDouble() const {
    return hour + (minute / 60.0);
}

String Timezone::toString(const bool &rfcPattern) const {
    if (rfcPattern) {
        return (hour < 0 ? "-" : "+") + padZero(abs(hour)) + ":" + padZero(abs(minute));
    } else {
        return (hour < 0 ? "GMT-" : "GMT+") + padZero(abs(hour)) + ":" + padZero(abs(minute));
    }
}

Timezone Timezone::fromSeconds(const int32_t &seconds) {
    return Timezone(seconds / 3600, (seconds % 3600) / 60);
}

Timezone Timezone::fromMinutes(const int16_t &minutes) {
    return Timezone(minutes / 60, minutes % 60);
}

/*------ Date ------*/
Date::Date()
    : year(1970), month(1), date(1) {}

Date::Date(const uint16_t &year, const uint8_t &month, const uint8_t &date)
    : year(year), month(month), date(date) {}

bool Date::isLeapYear() const {
    return ::isLeapYear(year);
}

double Date::daysOfTheYear() const {
    return gregorianDayCount[month - 1] + date + (isLeapYear() && month > 2 ? 1 : 0);
}

String Date::format(const String &fmt) const {
    return ::format(fmt, false, year, month, date, 0, 0, 0, 0, 0, 0);
}

/*------ Time ------*/
UniTime::Time::Time()
    : hour(0), minute(0), second(0) {}

UniTime::Time::Time(const double &time) {
    hour = (int)time;
    minute = (int)((time - hour) * 60);
    second = (int)(((time - hour) * 60 - minute) * 60);
}

UniTime::Time::Time(const uint8_t &hour, const uint8_t &minute, const uint8_t &second)
    : hour(hour), minute(minute), second(second) {}

String UniTime::Time::format(const String &fmt) const {
    return ::format(fmt, false, 1970, 1, 1, hour, minute, second, 0, 0, 0);
}

double UniTime::Time::secondsOfTheDay() const {
    return hour * 3600 + minute * 60 + second;
}

double UniTime::Time::minutesOfTheDay() const {
    return hour * 60.0 + minute + second / 60.0;
}

double UniTime::Time::hoursOfTheDay() const {
    return hour + (minute / 60.0) + (second / 3600.0);
}

UniTime::Time UniTime::Time::fromSecondsOfTheDay(const uint32_t &seconds) {
    return Time(seconds / 3600, (seconds % 3600) / 60, seconds % 60);
}

UniTime::Time UniTime::Time::fromMinutesOfTheDay(const uint32_t &minutes) {
    return Time(minutes / 60, minutes % 60, 0);
}

/*------ DateTime ------*/
DateTime::DateTime()
    : Date(), Time(), timezone(Timezone()), dayOfWeek(0), pasaran(0) {}

DateTime::DateTime(const uint32_t &timestamp, const Timezone &timezone) {
    int32_t z = timestamp / 86400 + 719468;
    int32_t era = (z >= 0 ? z : z - 146096) / 146097;
    uint32_t doe = static_cast<uint32_t>(z - era * 146097);
    uint32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    year = static_cast<uint16_t>(yoe) + era * 400;
    uint32_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    uint32_t mp = (5 * doy + 2) / 153;
    date = doy - (153 * mp + 2) / 5 + 1;
    month = mp + (mp < 10 ? 3 : -9);
    year += (month <= 2);
    int32_t sod = timestamp % 86400;
    hour = sod / 3600;
    minute = (sod % 3600) / 60;
    second = sod % 60;
    dayOfWeek = (((timestamp / 86400) + 4) % 7);
    pasaran = uint32_t(timestamp / 86400.0 + 2440587.5) % 5;
    this->timezone = timezone;
}

DateTime::DateTime(const uint16_t &year, const uint8_t &month, const uint8_t &date, const uint8_t &hour, const uint8_t &minute, const uint8_t &second, const Timezone &timezone)
    : DateTime(calculateUnixTimestamp(year, month, date, hour, minute, second), timezone) {}

double DateTime::daysOfTheYear() const {
    return Date::daysOfTheYear() - 1 + Time::hoursOfTheDay() / 24.0;
}

uint32_t DateTime::timestamp() const {
    return calculateUnixTimestamp(year, month, date, hour, minute, second);
}

String DateTime::format(const String &fmt) const {
    return ::format(fmt, false, year, month, date, hour, minute, second, dayOfWeek, pasaran, timezone.toDouble());
}

SolarTime DateTime::toSolarTime(const double &latitude, const double &longitude) const {
    return SolarTime(*this, latitude, longitude);
}

PrayerTime DateTime::toPrayerTime(const double &latitude, const double &longitude, const double &elevation) const {
    return PrayerTime(toSolarTime(latitude, longitude), elevation);
}

Date DateTime::toDate() const {
    return Date(year, month, date);
}

UniTime::Time DateTime::toTime() const {
    return Time(hour, minute, second);
}

HijriDate DateTime::toHijri() const {
    return HijriDate(*this);
}

/*------ SolarTime ------*/
SolarTime::SolarTime()
    : latitude(0), longitude(0), equationOfTime(0), declination(0), rightAscension(0), hourAngle(0), zenithAngle(0), altitudeAngle(0), azimuthAngle(0), offset(0), hourAngleRiseSet(0) {}

SolarTime::SolarTime(const DateTime &gregorianDate, const double &latitude, const double &longitude) {
    this->latitude = latitude;
    this->longitude = longitude;
    double localSolarTime = gregorianDate.hoursOfTheDay();
    double longitudinalVariation = 4 * (longitude - 15.0 * gregorianDate.timezone.toDouble());
    double fractionalYear = (2.0 * PI / (gregorianDate.isLeapYear() ? 366.0 : 365.0)) * (gregorianDate.daysOfTheYear() + (localSolarTime - gregorianDate.timezone.toDouble()) / 24.0);

    equationOfTime = 229.18 * (0.000075 + 0.001868 * cos(fractionalYear) - 0.032077 * sin(fractionalYear) - 0.014615 * cos(2.0 * fractionalYear) - 0.04089 * sin(2.0 * fractionalYear));
    declination = degrees(0.006918 - 0.399912 * cos(fractionalYear) + 0.070257 * sin(fractionalYear) - 0.006758 * cos(2.0 * fractionalYear) + 0.000907 * sin(2.0 * fractionalYear) - 0.002697 * cos(3.0 * fractionalYear) + 0.00148 * sin(3.0 * fractionalYear));

    time = Time(localSolarTime + (equationOfTime + longitudinalVariation) / 60.0);
    hourAngle = 15.0 * (time.hoursOfTheDay() - 12.0);

    zenithAngle = degrees(acos(sin(radians(latitude)) * sin(radians(declination)) + cos(radians(latitude)) * cos(radians(declination)) * cos(radians(hourAngle))));
    altitudeAngle = 90.0 - zenithAngle;
    azimuthAngle = degrees(acos(-((sin(radians(latitude)) * cos(radians(zenithAngle))) - sin(radians(declination))) / (cos(radians(latitude)) * sin(radians(zenithAngle)))));
    rightAscension = degrees(atan2(sin(radians(hourAngle)), (cos(radians(hourAngle)) * sin(radians(latitude)) - tan(radians(declination)) * cos(radians(latitude)))));

    if (hourAngle > 0) {
        azimuthAngle = 360.0 - azimuthAngle;
    }

    offset = gregorianDate.hoursOfTheDay() - time.hoursOfTheDay();
    noon = Time(12.0 + offset);

    hourAngleRiseSet = degrees(acos((cos(90.8333 * PI / 180.0) / (cos(radians(latitude)) * cos(radians(declination)))) - tan(radians(latitude)) * tan(radians(declination))));
    sunrise = Time(12.0 - hourAngleRiseSet / 15.0 + offset);
    sunset = Time(12.0 + hourAngleRiseSet / 15.0 + offset);
}

double SolarTime::offsetMinute() const {
    return offset * 60.0;
}

double SolarTime::offsetSecond() const {
    return offset * 3600.0;
}

double SolarTime::hourAngleFromAltitude(const double &altitude) const {
    return degrees(acos((sin(radians(altitude)) - sin(radians(latitude)) * sin(radians(declination))) / (cos(radians(latitude)) * cos(radians(declination)))));
}

/*------ HijriDate ------*/
HijriDate::HijriDate()
    : year(1), month(1), date(1) {}

HijriDate::HijriDate(const uint32_t &timestamp)
    : HijriDate(DateTime(timestamp)) {}

HijriDate::HijriDate(const DateTime &gregorianDate) {
    uint32_t dayOfGregorianYear = gregorianDayCount[gregorianDate.month - 1] + gregorianDate.date;
    if (gregorianDate.month > 2 && gregorianDate.isLeapYear()) {
        dayOfGregorianYear++;
    }

    uint32_t dayOfGregorianEra = ((gregorianDate.year - 1) * 365.25) + dayOfGregorianYear - 13;
    uint32_t dayOfHijriEra = dayOfGregorianEra - 227015;

    year = (dayOfHijriEra / 354.367) + 1;
    uint32_t dayOfHijriYear = dayOfHijriEra - ((year - 1) * 354.367);

    if (::Time.now().hour >= 18) {
        dayOfHijriYear++;
    }

    for (int i = 0; i < 12; i++) {
        if (hijriDayCount[i] > dayOfHijriYear) {
            month = i;
            date = dayOfHijriYear - hijriDayCount[i - 1];
            break;
        }
    }
}

HijriDate::HijriDate(const uint16_t &year, const uint8_t &month, const uint8_t &date)
    : year(year), month(month), date(date) {}

String HijriDate::format(const String &fmt) const {
    return ::format(fmt, true, year, month, date);
}

/*------ PrayerTime ------*/
PrayerTime::PrayerTime()
    : fajr(0), sunrise(0), dhuhr(0), asr(0), maghrib(0), isha(0) {}

PrayerTime::PrayerTime(const SolarTime &solarTime, const double &elevation) {
    dhuhr = solarTime.noon;

    double altitudeAsr = degrees(atan(1 / (1 + tan(radians(abs(solarTime.declination - solarTime.latitude))))));
    double hourAngleAsr = solarTime.hourAngleFromAltitude(altitudeAsr);
    asr = Time(dhuhr.hoursOfTheDay() + hourAngleAsr / 15.0);

    double altitudeMaghrib = -0.8333 - 0.0347 * sqrt(elevation);
    double hourAngleMaghrib = solarTime.hourAngleFromAltitude(altitudeMaghrib);
    maghrib = Time(dhuhr.hoursOfTheDay() + hourAngleMaghrib / 15.0);

    double hourAngleIsha = solarTime.hourAngleFromAltitude(-18.0);
    isha = Time(dhuhr.hoursOfTheDay() + hourAngleIsha / 15.0);

    double hourAngleFajr = solarTime.hourAngleFromAltitude(-20.0);
    fajr = Time(dhuhr.hoursOfTheDay() - hourAngleFajr / 15.0);

    sunrise = solarTime.sunrise;
}

PrayerTime::PrayerTime(const uint8_t &fajr, const uint8_t &sunrise, const uint8_t &duha, const uint8_t &dhuhr, const uint8_t &asr, const uint8_t &maghrib, const uint8_t &isha)
    : fajr(fajr), sunrise(sunrise), dhuhr(dhuhr), asr(asr), maghrib(maghrib), isha(isha) {}

/*------ UniTime ------*/
void UTime::enableRTC() {
    _isRtcEnabled = true;
    _rtc.begin();
    _lastTimestamp = _rtc.timestamp();
    _lastUpdate = millis();
    if (!_isTimeUpdateRegistered) {
        _isTimeUpdateRegistered = true;
        Timer.setInterval(1000, _timeUpdater);
    }
}

void UTime::enableNTP() {
    _isNtpEnabled = true;
#if defined(ESP32) || defined(ESP8266)
    _udp.begin(2390);
#endif
    if (!_isTimeUpdateRegistered) {
        _isTimeUpdateRegistered = true;
        Timer.setInterval(1000, _timeUpdater);
    }
}

bool UTime::adjust(const uint32_t &unix) {
    if (!isValidTimestamp(unix)) {
        return false;
    }
    if (_isRtcEnabled) {
        _rtc.adjust(unix);
    }
    _lastUpdate = millis();
    _lastTimestamp = unix;
    if (_onUpdate) {
        _onUpdate();
    }
    return true;
}

bool UTime::adjust(const DateTime &dt, bool isUtc) {
    if (isUtc) {
        return adjust(calculateUnixTimestamp(dt.year, dt.month, dt.date, dt.hour, dt.minute, dt.second));
    } else {
        return adjust(calculateUnixTimestamp(dt.year, dt.month, dt.date, dt.hour, dt.minute, dt.second) - _timezone);
    }
}

uint32_t UTime::timestamp() const {
    return _lastTimestamp + ((millis() - _lastUpdate) / 1000);
}

uint64_t UTime::timestampMillis() const {
    return uint64_t(_lastTimestamp) * 1000ULL + (millis() - _lastUpdate);
}

double UTime::julianDate() const {
    return timestamp() / 86400.0 + 2440587.5;
}

uint32_t UTime::secondsOfTheDay() const {
    return (timestamp() + _timezone) % 86400;
}

uint16_t UTime::minutesOfTheDay() const {
    return secondsOfTheDay() / 60;
}

void UTime::setTimezone(const int8_t &timezone) {
    this->_timezone = timezone * 3600;
}

bool UTime::isSet() const {
    return timestamp() > 1600000000;
}

bool UTime::isRtcEnabled() const {
    return _isRtcEnabled;
}

bool UTime::isNtpEnabled() const {
    return _isNtpEnabled;
}

bool UTime::isValidTimestamp(const uint32_t &timestamp) const {
    return timestamp > 1600000000;
}

DateTime UTime::now() const {
    return DateTime(_lastTimestamp + ((millis() - _lastUpdate) / 1000) + _timezone, timezone());
}

DateTime UTime::rtc() {
    return _rtc.now();
}

Timezone UTime::timezone() const {
    return Timezone::fromSeconds(_timezone);
}

SolarTime UTime::solarTime(const double &latitude, const double &longitude) const {
    return SolarTime(now(), latitude, longitude);
}

PrayerTime UTime::prayerTime(const double &latitude, const double &longitude, const double &elevation) const {
    return PrayerTime(solarTime(latitude, longitude), elevation);
}

void UTime::onUpdate(VoidCallback callback) {
    _onUpdate = callback;
}

void UTime::onMinuteChange(VoidCallback callback) {
    _onMinuteChanged = callback;
    if (!_isChangeDetectorRegistered) {
        _isChangeDetectorRegistered = true;
        Timer.setInterval(1000, _changeDetector);
    }
}

void UTime::onHourChange(VoidCallback callback) {
    _onHourChanged = callback;
    if (!_isChangeDetectorRegistered) {
        _isChangeDetectorRegistered = true;
        Timer.setInterval(1000, _changeDetector);
    }
}

void UTime::onDayChange(VoidCallback callback) {
    _onDayChanged = callback;
    if (!_isChangeDetectorRegistered) {
        _isChangeDetectorRegistered = true;
        Timer.setInterval(1000, _changeDetector);
    }
}

/*------ DS3231 Utility ------*/
static uint8_t read_i2c_register(uint8_t addr, uint8_t reg) {
    Wire.beginTransmission(addr);
    Wire.write((byte)reg);
    Wire.endTransmission();
    Wire.requestFrom(addr, (byte)1);
    return Wire.read();
}

static void write_i2c_register(uint8_t addr, uint8_t reg, uint8_t val) {
    Wire.beginTransmission(addr);
    Wire.write((byte)reg);
    Wire.write((byte)val);
    Wire.endTransmission();
}

static uint8_t bcd2bin(uint8_t val) {
    return val - 6 * (val >> 4);
}

static uint8_t bin2bcd(uint8_t val) {
    return val + 6 * (val / 10);
}

static uint8_t conv2d(const char *p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

/*------ DS3231 ------*/

DS3231::DS3231() {}

void DS3231::begin() {
    Wire.begin();
}

bool DS3231::lostPower() {
    return (read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG) >> 7);
}

void DS3231::adjust(const uint32_t &unix) {
    DateTime dt(unix);
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write((byte)0);
    Wire.write(bin2bcd(dt.second));
    Wire.write(bin2bcd(dt.minute));
    Wire.write(bin2bcd(dt.hour));
    Wire.write(bin2bcd(0));
    Wire.write(bin2bcd(dt.date));
    Wire.write(bin2bcd(dt.month));
    Wire.write(bin2bcd(dt.year - 2000));
    Wire.endTransmission();

    uint8_t statreg = read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG);
    statreg &= ~0x80;
    write_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, statreg);
}

uint32_t DS3231::timestamp() {
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write((byte)0);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 7);
    uint8_t ss = bcd2bin(Wire.read() & 0x7F);
    uint8_t mm = bcd2bin(Wire.read());
    uint8_t hh = bcd2bin(Wire.read());
    Wire.read();
    uint8_t d = bcd2bin(Wire.read());
    uint8_t m = bcd2bin(Wire.read());
    uint16_t y = bcd2bin(Wire.read()) + 2000;

    return calculateUnixTimestamp(y, m, d, hh, mm, ss);
}

DateTime DS3231::now() {
    return DateTime(timestamp());
}

DS3231::SqwPinMode DS3231::getSqwPinMode() {
    int mode;

    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_CONTROL);
    Wire.endTransmission();

    Wire.requestFrom((uint8_t)DS3231_ADDRESS, (uint8_t)1);
    mode = Wire.read();

    mode &= 0x93;
    return static_cast<DS3231::SqwPinMode>(mode);
}

void DS3231::setSqwPinMode(const DS3231::SqwPinMode &mode) {
    uint8_t ctrl;
    ctrl = read_i2c_register(DS3231_ADDRESS, DS3231_CONTROL);

    ctrl &= ~0x04;
    ctrl &= ~0x18;

    if (mode == DS3231_OFF) {
        ctrl |= 0x04;
    } else {
        ctrl |= mode;
    }
    write_i2c_register(DS3231_ADDRESS, DS3231_CONTROL, ctrl);
}

float DS3231::temperature() {
    uint8_t msb, lsb;
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_TEMPERATUREREG);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 2);
    msb = Wire.read();
    lsb = Wire.read();

    return (float)msb + (lsb >> 6) * 0.25f;
}

/*------ FreeRTOS Task ------*/

#if defined(ESP32) || defined(ESP8266)
void UTime::_timeUpdater() {
    static bool _isWaiting = false;
    static uint16_t ntpCounter = 0;
    static uint16_t rtcCounter = 0;

    if (_isNtpEnabled && WiFi.isConnected() && !_isWaiting && ((!::Time.isSet() && ntpCounter > 5) || ntpCounter > 60)) {
        ntpCounter = 0;
        if (_udp.beginPacket("pool.ntp.org", 123)) {
            _isWaiting = true;
            byte buffer[48];
            memset(buffer, 0, 48);

            buffer[0] = 0xE3;
            buffer[1] = 0x00;
            buffer[2] = 0x06;
            buffer[3] = 0xEC;
            buffer[12] = 0x31;
            buffer[13] = 0x4E;
            buffer[14] = 0x31;
            buffer[15] = 0x34;

            _udp.write(buffer, 48);
            _udp.endPacket();
        }
    }

    if (_isRtcEnabled && (!::Time.isSet() || rtcCounter > 60)) {
        rtcCounter = 0;
        _lastTimestamp = _rtc.timestamp();
        _lastUpdate = millis();
    }

    size_t packetLen = _udp.parsePacket();
    if (packetLen && packetLen >= 48) {
        _isWaiting = false;
        byte buffer[packetLen];
        _udp.read(buffer, packetLen);
        _lastTimestamp = (word(buffer[40], buffer[41]) << 16 | word(buffer[42], buffer[43])) - 2208988800;
        _lastUpdate = millis();
        if (_isRtcEnabled) {
            _rtc.adjust(_lastTimestamp);
        }
        if (_onUpdate) {
            _onUpdate();
        }
    }

    ntpCounter++;
    rtcCounter++;
}

#else
void UTime::_timeUpdater() {
    if (_isRtcEnabled) {
        _lastTimestamp = _rtc.timestamp();
        _lastUpdate = millis();
    }
}
#endif

void UTime::_changeDetector() {
    DateTime now = ::Time.now();
    if (_onMinuteChanged && _lastMinute != now.minute) {
        _lastMinute = now.minute;
        _onMinuteChanged();
    }
    if (_onHourChanged && _lastHour != now.hour) {
        _lastHour = now.hour;
        _onHourChanged();
    }
    if (_onDayChanged && _lastDay != now.date) {
        _lastDay = now.date;
        _onDayChanged();
    }
}

/*------ Utility ------*/

static uint16_t daysInMonth(const uint16_t &year, const uint8_t &month) {
    if (month == 2) {
        return isLeapYear(year) ? 29 : 28;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    } else {
        return 31;
    }
}

static uint16_t daysInYear(const uint16_t &year) {
    return isLeapYear(year) ? 366 : 365;
}

uint16_t calculateDaysUntil(const uint16_t &yearFrom, const uint16_t &year, const uint8_t &month, const uint8_t &day) {
    uint16_t days = 0;
    for (uint16_t i = yearFrom; i < year; i++) {
        days += daysInYear(i);
    }
    for (uint8_t i = 1; i < month; i++) {
        days += daysInMonth(year, i);
    }
    return days + day - 1;
}

uint32_t calculateUnixTimestamp(const uint16_t &year, const uint8_t &month, const uint8_t &date, const uint8_t &hour, const uint8_t &minute, const uint8_t &second) {
    if (year < 1970 || month < 1 || month > 12 || date < 1 || date > 31 || hour > 23 || minute > 59 || second > 59) {
        return 0;
    }

    if (year >= 2020) {
        return 1577836800UL + (calculateDaysUntil(2020, year, month, date) * 86400UL) + (hour * 3600UL) + (minute * 60UL) + second;
    }

    if (year >= 2000) {
        return 946684800UL + (calculateDaysUntil(2000, year, month, date) * 86400UL) + (hour * 3600UL) + (minute * 60UL) + second;
    }

    return (calculateDaysUntil(1970, year, month, date) * 86400UL) + (hour * 3600UL) + (minute * 60UL) + second;
}

static int findClosingQuotationMark(const String &str, int start) {
    if (start < 0 || start >= str.length() - 1) return -1;
    for (int i = start; i < str.length(); i++) {
        if (i == str.length() - 1) {
            if (str[i] == '\'') {
                return i;
            }
        } else {
            if (str[i] == '\'') {
                if (str[i + 1] != '\'') {
                    return i;
                } else {
                    i++;
                }
            }
        }
    }
    return -1;
}

static int findLastAdjacentDuplicateCharacter(const String &str, int start, char character) {
    if (start < 0 || start > str.length() - 1) return -1;
    for (int i = start; i < str.length(); i++) {
        if (str[i] != character) {
            return i - 1;
        }
    }
    return str.length() - 1;
}

String format(const String &fmt, const bool &isHijri, const uint16_t &year, const uint8_t &month, const uint8_t &date, const uint8_t &hour, const uint8_t &minute, const uint8_t &second, const uint8_t &dayOfWeek, const uint8_t &pasaran, const Timezone &timezone) {
    String result;
    for (int i = 0; i < fmt.length(); i++) {
        if (fmt[i] == '\'') {
            if (i != fmt.length() - 1) {
                if (fmt[i + 1] != '\'') {
                    int idx = findClosingQuotationMark(fmt, i + 1);
                    if (idx != -1) {
                        String substr = fmt.substring(i + 1, idx);
                        substr.replace("''", "'");
                        result += substr;
                    } else {
                        return "Invalid date format";
                    }
                    i = idx;
                } else {
                    return "Invalid date format";
                }
            }
        } else {
            switch (fmt[i]) {
                case 'y': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'y');
                    switch (idx - i + 1) {
                        case 2:
                            result += padZero(year % 100);
                            break;
                        default:
                            result += String(year);
                            break;
                    }
                    i = idx;
                    break;
                }
                case 'M': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'M');
                    switch (idx - i + 1) {
                        case 1:
                            result += String(month);
                            break;
                        case 2:
                            result += padZero(month);
                            break;
                        case 3:
                            result += (isHijri ? hijriMonthNames[month - 1].substring(0, 3) : gregorianMonthNames[month - 1].substring(0, 3));
                            break;
                        default:
                            result += (isHijri ? hijriMonthNames[month - 1] : gregorianMonthNames[month - 1]);
                            break;
                    }
                    i = idx;
                    break;
                }
                case 'd': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'd');
                    switch (idx - i + 1) {
                        case 1:
                            result += String(date);
                            break;
                        default:
                            result += padZero(date);
                            break;
                    }
                    i = idx;
                    break;
                }
                case 'H': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'H');
                    switch (idx - i + 1) {
                        case 1:
                            result += String(hour);
                            break;
                        default:
                            result += padZero(hour);
                            break;
                    }
                    i = idx;
                    break;
                }
                case 'K': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'K');
                    switch (idx - i + 1) {
                        case 1:
                            result += String(hour % 12);
                            break;
                        default:
                            result += padZero(hour % 12);
                            break;
                    }
                    i = idx;
                    break;
                }
                case 'h': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'h');
                    switch (idx - i + 1) {
                        case 1:
                            result += String(hour % 12 == 0 ? 12 : hour % 12);
                            break;
                        default:
                            result += String(hour % 12 == 0 ? "12" : padZero(hour % 12));
                            break;
                    }
                    i = idx;
                    break;
                }
                case 'k': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'k');
                    switch (idx - i + 1) {
                        case 1:
                            result += String(hour == 0 ? 24 : hour);
                            break;
                        default:
                            result += String(hour == 0 ? "24" : padZero(hour));
                    }
                    i = idx;
                    break;
                }
                case 'm': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'm');
                    switch (idx - i + 1) {
                        case 1:
                            result += String(minute);
                            break;
                        default:
                            result += padZero(minute);
                            break;
                    }
                    i = idx;
                    break;
                }
                case 's': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 's');
                    switch (idx - i + 1) {
                        case 1:
                            result += String(second);
                            break;
                        default:
                            result += padZero(second);
                            break;
                    }
                    i = idx;
                    break;
                }
                case 'a': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'a');
                    result += String(hour < 12 ? "AM" : "PM");
                    i = idx;
                    break;
                }
                case 'E': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'E');
                    switch (idx - i + 1) {
                        case 3:
                            result += solarDayNames[dayOfWeek].substring(0, 3);
                            break;
                        default:
                            result += solarDayNames[dayOfWeek];
                            break;
                    }
                    i = idx;
                    break;
                }
                case 'P': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'P');
                    switch (idx - i + 1) {
                        case 3:
                            result += pasaranDayNames[pasaran].substring(0, 3);
                            break;
                        default:
                            result += pasaranDayNames[pasaran];
                            break;
                    }
                    i = idx;
                    break;
                }
                case 'z': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'z');
                    result += timezone.toString(false);
                    i = idx;
                    break;
                }
                case 'Z': {
                    int idx = findLastAdjacentDuplicateCharacter(fmt, i, 'Z');
                    result += timezone.toString(true);
                    i = idx;
                    break;
                }
                default:
                    if ((fmt[i] >= '0' && fmt[i] <= '9') || (fmt[i] >= 'A' && fmt[i] <= 'Z') || (fmt[i] >= 'a' && fmt[i] <= 'z')) {
                        return "Invalid date format";
                    } else {
                        result += fmt[i];
                    }
                    break;
            }
        }
    }

    return result;
}

bool isLeapYear(const uint16_t &year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

String padZero(const uint8_t &value) {
    return value < 10 ? "0" + String(value) : String(value);
}

UTime Time;
