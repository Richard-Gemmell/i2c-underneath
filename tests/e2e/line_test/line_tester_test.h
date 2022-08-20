// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_LINE_TESTER_TEST_H
#define I2C_UNDERNEATH_LINE_TESTER_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include <common/hal/teensy/super_fast_io.h>
#include <line_test/line_tester.h>

namespace line_test {

class LineTesterTest : public TestSuite {
//    const static uint32_t PIN_SNIFF_SDA = 23;
    const static uint32_t PIN_SNIFF_SCL = 22;

public:
    static void line_test_report_converts_measured_ticks_to_nanos() {
        analysis::DurationStatistics measured_fall_time;
        measured_fall_time.include(10);
        measured_fall_time.include(12);
        measured_fall_time.include(11);
        analysis::DurationStatistics measured_rise_time;
        measured_rise_time.include(33);
        measured_rise_time.include(35);
        auto report = LineTestReport(measured_fall_time, measured_rise_time,
                                     analysis::DurationStatistics(), analysis::DurationStatistics());

        TEST_ASSERT_EQUAL_UINT32(3, report.get_1_to_0_time().count());
        TEST_ASSERT_EQUAL_UINT32(11, report.get_1_to_0_time().average());
        TEST_ASSERT_EQUAL_UINT32(2, report.get_0_to_1_time().count());
        TEST_ASSERT_EQUAL_UINT32(34, report.get_0_to_1_time().average());
    }

    static void measure_rise_and_fall_times() {
        // This test assumes an 8k external pullup
        // It will fail if you attach an oscilloscope.
        auto result = LineTester::TestLine(PIN_SNIFF_SCL);

        TEST_ASSERT_EQUAL(9, result.get_1_to_0_time().count());
        TEST_ASSERT_UINT32_WITHIN(2, 4, result.get_1_to_0_time().average());
        TEST_ASSERT_EQUAL(9, result.get_0_to_1_time().count());
        TEST_ASSERT_UINT32_WITHIN(30, 244, result.get_0_to_1_time().average());
    }

    static void estimate_rise_and_fall_times() {
        // This test assumes an 8k external pullup
        // It will fail if you attach an oscilloscope.
        auto result = LineTester::TestLine(PIN_SNIFF_SCL);

        TEST_ASSERT_EQUAL(9, result.get_1_to_0_time().count());
        TEST_ASSERT_UINT32_WITHIN(2, 5, result.get_estimated_fall_time().average());
        TEST_ASSERT_EQUAL(9, result.get_0_to_1_time().count());
        TEST_ASSERT_UINT32_WITHIN(30, 300, result.get_estimated_rise_time().average());
    }

    static void report_timeout() {
        // WHEN we test the line with a really short timeout
        auto result = LineTester::TestLine(PIN_SNIFF_SCL, 20);

        // THEN the fall time is recorded normally
        TEST_ASSERT_EQUAL(9, result.get_1_to_0_time().count());
        TEST_ASSERT_UINT32_WITHIN(2, 4, result.get_1_to_0_time().average());
        // AND the rise time is invalid
        TEST_ASSERT_EQUAL(9, result.get_0_to_1_time().count());
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, result.get_0_to_1_time().average());
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(line_test_report_converts_measured_ticks_to_nanos);
        RUN_TEST(measure_rise_and_fall_times);
        RUN_TEST(estimate_rise_and_fall_times);
        RUN_TEST(report_timeout);
    }

    LineTesterTest() : TestSuite(__FILE__) {};
};

} // line_test
#endif //I2C_UNDERNEATH_LINE_TESTER_TEST_H
