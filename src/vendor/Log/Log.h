#ifndef LOG_H
#define LOG_H

#include <Arduino.h>

#include "../ArrayList/ArrayList.h"

class Log {
   public:
    enum Level : uint8_t {
        Error,
        Warning,
        Info,
        Debug
    };

    /**
     * @brief Attach a printer with the given severity level.
     *
     * @param printer is the Print object.
     * @param level is the severity level.
     */
    static void attach(Print& printer, const Level& level);

    /**
     * @brief Detach the given printer.
     *
     * @param printer is the Print object.
     */
    static void detach(Print& printer);

#if defined(ESP8266) || defined(ESP32)
    /**
     * @brief Print a formatted string to all attached printers with the given severity level.
     *
     * @tparam Arg is the variadic argument type.
     * @param level is the severity level.
     * @param tag is the tag.
     * @param format is the format string.
     * @param args is the variadic arguments.
     */
    template <typename... Arg>
    static void println(const Level& level, const String& tag, const String& format, Arg... args) {
        for (auto& p : s_Printers) {
            if (static_cast<int>(p.level) >= static_cast<int>(level)) {
                p.printer->printf(
                    String("%s[%s]: " + format + "\n").c_str(), s_Severities[static_cast<int>(level)].c_str(),
                    tag.c_str(), args...
                );
            }
        }
    }

    /**
     * @brief Print a formatted string to all attached printers with the Error severity level.
     *
     * @tparam Arg is the variadic argument type.
     * @param tag is the tag.
     * @param format is the format string.
     * @param args is the variadic arguments.
     */
    template <typename... Arg>
    static void error(const String& tag, const String& format, Arg... args) {
        println(Level::Error, tag, format, args...);
    }

    /**
     * @brief Print a formatted string to all attached printers with the Info severity level.
     *
     * @tparam Arg is the variadic argument type.
     * @param tag is the tag.
     * @param format is the format string.
     * @param args is the variadic arguments.
     */
    template <typename... Arg>
    static void info(const String& tag, const String& format, Arg... args) {
        println(Level::Info, tag, format, args...);
    }

    /**
     * @brief Print a formatted string to all attached printers with the Warning severity level.
     *
     * @tparam Arg is the variadic argument type.
     * @param tag is the tag.
     * @param format is the format string.
     * @param args is the variadic arguments.
     */
    template <typename... Arg>
    static void warn(const String& tag, const String& format, Arg... args) {
        println(Level::Warning, tag, format, args...);
    }

    /**
     * @brief Print a formatted string to all attached printers with the Debug severity level.
     *
     * @tparam Arg is the variadic argument type.
     * @param tag is the tag.
     * @param format is the format string.
     * @param args is the variadic arguments.
     */
    template <typename... Arg>
    static void debug(const String& tag, const String& format, Arg... args) {
        println(Level::Debug, tag, format, args...);
    }
#else
    /**
     * @brief Print a string message to all attached printers with the given severity level.
     *
     * @param level is the severity level.
     * @param tag is the tag.
     * @param value is the string message.
     */
    static void println(const Level& level, const String& tag, const String& value) {
        for (auto& p : s_Printers) {
            if (static_cast<int>(p.level) >= static_cast<int>(level)) {
                p.printer->println(String(s_Severities[static_cast<int>(level)].c_str()) + "[" + tag + "]: " + value);
            }
        }
    }

    /**
     * @brief Print a string message to all attached printers with the Error severity level.
     *
     * @param tag is the tag.
     * @param value is the string message.
     */
    static void error(const String& tag, const String& value) {
        println(Level::Error, tag, value);
    }

    /**
     * @brief Print a string message to all attached printers with the Info severity level.
     *
     * @param tag is the tag.
     * @param value is the string message.
     */
    static void info(const String& tag, const String& value) {
        println(Level::Info, tag, value);
    }

    /**
     * @brief Print a string message to all attached printers with the Warning severity level.
     *
     * @param tag is the tag.
     * @param value is the string message.
     */
    static void warn(const String& tag, const String& value) {
        println(Level::Warning, tag, value);
    }

    /**
     * @brief Print a string message to all attached printers with the Debug severity level.
     *
     * @param tag is the tag.
     * @param value is the string message.
     */
    static void debug(const String& tag, const String& value) {
        println(Level::Debug, tag, value);
    }
#endif

   private:
    struct Printer {
        Print* printer;
        Level level;

        Printer()
            : printer(nullptr),
              level(Level::Error) {}

        Printer(Print* printer, Level level)
            : printer(printer),
              level(level) {}
    };

    static ArrayList<Printer> s_Printers;
    static String s_Severities[];
};

#endif