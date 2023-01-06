#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <map>

#include "../Any/Any.h"

class UnitTest {
   public:
    struct Result {
        uint8_t passed;
        uint8_t failed;

        Result();
        Result& operator+=(const Result& other);
    };

    UnitTest(const String& name);

    void attach(Print& printer);
    Result run();

    UnitTest& assertEqual(const String& name, const Any& expected, const Any& actual);
    UnitTest& assertNotEqual(const String& name, const Any& expected, const Any& actual);
    UnitTest& assertTrue(const String& name, const bool& actual);
    UnitTest& assertFalse(const String& name, const bool& actual);

   private:
    struct TestResult {
        bool passed;
        String expected;
        String actual;
    };

    Print* m_Printer;
    String m_TestName;
    size_t m_MaxNameLength;
    std::map<String, TestResult> m_Units;

    String fillSpaces(const String& str, const size_t& length);
};

#endif