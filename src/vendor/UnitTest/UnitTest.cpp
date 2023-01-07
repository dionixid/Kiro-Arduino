#include "UnitTest.h"

#include <algorithm>

UnitTest::Result::Result()
    : passed(0),
      failed(0) {}

UnitTest::Result& UnitTest::Result::operator+=(const Result& other) {
    passed += other.passed;
    failed += other.failed;

    return *this;
}

UnitTest::UnitTest(const String& name)
    : m_TestName(name),
      m_MaxNameLength(0),
      m_Printer(NULL) {}

/**
 * @brief Attach a printer to the unit test.
 *
 * @param printer is the printer to attach.
 */
void UnitTest::attach(Print& printer) {
    m_Printer = &printer;
}

/**
 * @brief Run the unit test.
 *
 * @return the result of the unit test.
 */
UnitTest::Result UnitTest::run() {
    if (m_Printer == NULL) {
        return Result();
    }

    m_Printer->println("+---------------------------------------------------");
    m_Printer->println("| " + m_TestName);
    m_Printer->println("+---------------------------------------------------");
    m_Printer->println("| ");

    m_Printer->println("| --------------------------------------------------");

    for (auto& unit : m_Units) {
        m_Printer->println("| " + fillSpaces(unit.first, m_MaxNameLength) + (unit.second.passed ? " : PASS" : " : FAIL"));

        if (!unit.second.passed) {
            m_Printer->println("| Expected : " + unit.second.expected);
            m_Printer->println("| Actual   : " + unit.second.actual);
        }
        m_Printer->println("| --------------------------------------------------");
    }

    Result result;
    result.passed = std::count_if(
        m_Units.begin(), 
        m_Units.end(), 
        [](const std::pair<String, TestResult>& unit) {
            return unit.second.passed;
        }
    );

    result.failed = m_Units.size() - result.passed;

    m_Printer->println("| ");
    m_Printer->println("| --------------------------------------------------");
    m_Printer->printf("| Total  : %d\n", m_Units.size());
    m_Printer->printf("| Passed : %d\n", result.passed);
    m_Printer->printf("| Failed : %d\n", result.failed);
    m_Printer->println("+---------------------------------------------------");
    m_Printer->println();

    return result;
}

/**
 * @brief Assert that two values are equal.
 *
 * @param name is the name of the test.
 * @param expected is the expected value.
 * @param actual is the actual value.
 * @return this instance.
 */
UnitTest& UnitTest::assertEqual(const String& name, const Any& expected, const Any& actual) {
    TestResult result;
    result.passed   = expected == actual;
    result.expected = expected.toString();
    result.actual   = actual.toString();

    m_Units[name]   = result;
    m_MaxNameLength = max(m_MaxNameLength, name.length());

    return *this;
}

/**
 * @brief Assert that two values are not equal.
 *
 * @param name is the name of the test.
 * @param expected is the expected value.
 * @param actual is the actual value.
 * @return this instance.
 */
UnitTest& UnitTest::assertNotEqual(const String& name, const Any& expected, const Any& actual) {
    TestResult result;
    result.passed   = expected != actual;
    result.expected = expected.toString();
    result.actual   = actual.toString();

    m_Units[name]   = result;
    m_MaxNameLength = max(m_MaxNameLength, name.length());

    return *this;
}

/**
 * @brief Assert that a value is true.
 *
 * @param name is the name of the test.
 * @param actual is the actual value.
 * @return this instance.
 */
UnitTest& UnitTest::assertTrue(const String& name, const bool& actual) {
    TestResult result;
    result.passed   = actual;
    result.expected = "true";
    result.actual   = actual ? "true" : "false";

    m_Units[name]   = result;
    m_MaxNameLength = max(m_MaxNameLength, name.length());

    return *this;
}

/**
 * @brief Assert that a value is false.
 *
 * @param name is the name of the test.
 * @param actual is the actual value.
 * @return this instance.
 */
UnitTest& UnitTest::assertFalse(const String& name, const bool& actual) {
    TestResult result;
    result.passed   = !actual;
    result.expected = "false";
    result.actual   = actual ? "true" : "false";

    m_Units[name]   = result;
    m_MaxNameLength = max(m_MaxNameLength, name.length());

    return *this;
}

/**
 * @brief Fill a string with spaces to a given length.
 *
 * @param str is the string to fill.
 * @param length is the length to fill to.
 * @return the filled string.
 */
String UnitTest::fillSpaces(const String& str, const size_t& length) {
    String result = str;
    for (int i = 0; i < length - str.length(); i++) {
        result += " ";
    }

    return result;
}