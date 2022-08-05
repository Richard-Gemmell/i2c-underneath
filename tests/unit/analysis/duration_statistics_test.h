// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_I2C_DURATION_STATISTICS_TEST_H
#define I2C_UNDERNEATH_I2C_DURATION_STATISTICS_TEST_H
#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include <analysis/duration_statistics.h>
#include <analysis/i2c_design_parameters.h>

namespace analysis {
class DurationStatisticsTest : public TestSuite {
public:
    static void include_increments_count() {
        // WHEN we create a new object
        DurationStatistics statistics;

        // THEN the count is zero
        TEST_ASSERT_EQUAL_UINT32(0, statistics.count());

        // WHEN we add durations
        // THEN the count increases
        statistics.include(1);
        TEST_ASSERT_EQUAL_UINT32(1, statistics.count());
        statistics.include(1);
        TEST_ASSERT_EQUAL_UINT32(2, statistics.count());
    }

    static void can_get_minimum_value() {
        // WHEN we create a new object
        DurationStatistics statistics;

        // THEN the min value is undefined
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, statistics.min());

        // WHEN we add the first duration
        statistics.include(500);
        // THEN the minimum is updated to this value
        TEST_ASSERT_EQUAL_UINT32(500, statistics.min());

        // WHEN we add a larger value
        statistics.include(10'000);
        // THEN the minimum doesn't change
        TEST_ASSERT_EQUAL_UINT32(500, statistics.min());

        // WHEN we add a smaller value
        statistics.include(499);
        // THEN the minimum becomes the smaller value
        TEST_ASSERT_EQUAL_UINT32(499, statistics.min());
    }

    static void can_get_maximum_value() {
        // WHEN we create a new object
        DurationStatistics statistics;

        // THEN the min value is undefined
        TEST_ASSERT_EQUAL_UINT32(0, statistics.max());

        // WHEN we add the first duration
        statistics.include(500);
        // THEN the maximum is updated to this value
        TEST_ASSERT_EQUAL_UINT32(500, statistics.max());

        // WHEN we add a smaller value
        statistics.include(300);
        // THEN the maximum doesn't change
        TEST_ASSERT_EQUAL_UINT32(500, statistics.max());

        // WHEN we add a larger value
        statistics.include(501);
        // THEN the maximum is updated
        TEST_ASSERT_EQUAL_UINT32(501, statistics.max());
    }

    static void calculates_average_correctly() {
        DurationStatistics statistics;

        // WHEN there are no measurements
        // THEN the average is 0
        TEST_ASSERT_EQUAL_UINT32(0, statistics.average());

        // WHEN there is one measurement
        // THEN the average is the same as the measurement
        statistics.include(12345);
        TEST_ASSERT_EQUAL_UINT32(12345, statistics.average());

        // WHEN there are many measurements
        // THEN the average is the average of all of them
        statistics.include(1004);
        statistics.include(555);
        TEST_ASSERT_EQUAL_UINT32(4635, statistics.average());
    }

    static void meets_specification() {
        // GIVEN some statistics
        DurationStatistics statistics;
        statistics.include(100);
        statistics.include(199);

        // THEN it meets the specification if the min >= spec.min
        // AND the max <= spec.max
        TEST_ASSERT_TRUE(statistics.meets_specification({.min=99, .max=200}))
        TEST_ASSERT_TRUE(statistics.meets_specification({.min=100, .max=199}))
        // THEN it does not meet the spec if the actual minimum is too low
        TEST_ASSERT_FALSE(statistics.meets_specification({.min=101, .max=199}))
        // OR if the actual max is too high
        TEST_ASSERT_FALSE(statistics.meets_specification({.min=100, .max=198}))
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(include_increments_count);
        RUN_TEST(can_get_minimum_value);
        RUN_TEST(can_get_maximum_value);
        RUN_TEST(calculates_average_correctly);
        RUN_TEST(meets_specification);
    }

    DurationStatisticsTest() : TestSuite(__FILE__) {};
};
} // analysis
#endif //I2C_UNDERNEATH_I2C_DURATION_STATISTICS_TEST_H
