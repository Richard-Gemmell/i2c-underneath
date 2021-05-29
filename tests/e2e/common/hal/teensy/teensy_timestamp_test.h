// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_HAL_TEENSY_TEENSY_TIMESTAMP_TEST_H
#define I2C_UNDERNEATH_COMMON_HAL_TEENSY_TEENSY_TIMESTAMP_TEST_H

#include <unity.h>
#include <Arduino.h>
#include <common/hal/teensy/teensy_timestamp.h>
#include "../../../../utils/test_suite.h"

namespace common {
namespace hal {

class TeensyTimestampTest : public TestSuite {
public:
    static void ticks_to_nanos() {
        TEST_ASSERT_EQUAL_MESSAGE(600'000'000, F_CPU_ACTUAL, "This test assumes an Teensy 4 running at the standard frequency.");

        TEST_ASSERT_EQUAL_UINT32(0, TeensyTimestamp::ticks_to_nanos(0));
        // All results are a little low because we're rounding a double value downwards
        TEST_ASSERT_EQUAL_UINT32(100-1, TeensyTimestamp::ticks_to_nanos(60));
        TEST_ASSERT_EQUAL_UINT32(4'000'000'000-96, TeensyTimestamp::ticks_to_nanos(2'400'000'000));
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX-103, TeensyTimestamp::ticks_to_nanos(2'576'980'377));

        // Large values are capped to UINT32_MAX
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, TeensyTimestamp::ticks_to_nanos(UINT32_MAX));
    }

    static void nanos_between_ticks() {
        TEST_ASSERT_EQUAL_MESSAGE(600'000'000, F_CPU_ACTUAL, "This test assumes an Teensy 4 running at the standard frequency.");

        // No difference
        TEST_ASSERT_EQUAL_UINT32(0, TeensyTimestamp::nanos_between(0, 0));
        TEST_ASSERT_EQUAL_UINT32(0, TeensyTimestamp::nanos_between(UINT32_MAX, UINT32_MAX));
        TEST_ASSERT_EQUAL_UINT32(0, TeensyTimestamp::nanos_between(123456789, 123456789));

        // Full range
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, TeensyTimestamp::nanos_between(0, UINT32_MAX));

        // Differences
        TEST_ASSERT_EQUAL_UINT32(TeensyTimestamp::ticks_to_nanos(1000), TeensyTimestamp::nanos_between(0, 1'000));
        TEST_ASSERT_EQUAL_UINT32(TeensyTimestamp::ticks_to_nanos(60), TeensyTimestamp::nanos_between(1'000, 1'060));

        // Wrap around
        TEST_ASSERT_EQUAL_UINT32(1, TeensyTimestamp::nanos_between(UINT32_MAX, 0));
        TEST_ASSERT_EQUAL_UINT32(TeensyTimestamp::ticks_to_nanos(6), TeensyTimestamp::nanos_between(UINT32_MAX, 5));
        uint32_t wrap_around = (UINT32_MAX - 4'000'000'000) + 1'000'000'000 + 1;
        TEST_ASSERT_EQUAL_UINT32(TeensyTimestamp::ticks_to_nanos(wrap_around), TeensyTimestamp::nanos_between(4'000'000'000, 1'000'000'000));
    }

    static void not_timed_out_nanos_immediately() {
        // GIVEN a new timestamp
        TeensyTimestamp timestamp;

        // WHEN I check a timeout immediately
        bool timed_out = timestamp.timed_out_nanos(1000);

        // THEN the time has not passed
        TEST_ASSERT_FALSE(timed_out);
    }

    // This test covers the case where we're only concerned
    // with whole milliseconds. The tick comparison isn't required.
    static void timed_out_nanos_millisecond_checks() {
        // GIVEN a new timestamp
        uint32_t nanos_per_milli = 1'000'000;
        TeensyTimestamp timestamp;

        // WHEN some milliseconds have passed
        delayMicroseconds(50'000);

        // THEN a slightly shorter interval has passed
        TEST_ASSERT_TRUE(timestamp.timed_out_nanos(49 * nanos_per_milli));
        // AND the same interval has passed
        TEST_ASSERT_TRUE(timestamp.timed_out_nanos(50 * nanos_per_milli));
        // AND a slightly longer interval has not passed
        TEST_ASSERT_FALSE(timestamp.timed_out_nanos(51 * nanos_per_milli));
    }

    // This test covers the case where the number of milliseconds
    // and the tick count are needed to confirm the result.
    static void timed_out_nanos() {
        // GIVEN a new timestamp
        TeensyTimestamp timestamp;

        // WHEN some milliseconds have passed
        delayMicroseconds(5'000);

        // THEN a slightly shorter interval has passed
        TEST_ASSERT_TRUE(timestamp.timed_out_nanos(4'999'000));
        // AND the same interval has passed
        TEST_ASSERT_TRUE(timestamp.timed_out_nanos(5'000'000));
        // AND a slightly longer interval has not passed
        TEST_ASSERT_FALSE(timestamp.timed_out_nanos(5'001'000));
    }

    static void timed_out_nanos_after_nanoseconds() {
        // GIVEN a new timestamp
        TeensyTimestamp timestamp;

        // WHEN some nanoseconds have passed
        delayNanoseconds(500);

        // THEN a slightly shorter interval has passed
        TEST_ASSERT_TRUE(timestamp.timed_out_nanos(500));

        // AND a slightly longer interval has not passed
        TEST_ASSERT_FALSE(timestamp.timed_out_nanos(650));
    }

    static void mark_resets_time() {
        // GIVEN a timestamp created in the past
        TeensyTimestamp timestamp;
        delayMicroseconds(5'000);
        uint32_t timeout = 4'990'000;
        TEST_ASSERT_TRUE(timestamp.timed_out_nanos(timeout));

        // WHEN we reset the time
        timestamp.reset();

        // THEN the timeout has no longer passed
        TEST_ASSERT_FALSE(timestamp.timed_out_nanos(timeout));
        // AND nor has a very short timeout
        TEST_ASSERT_FALSE(timestamp.timed_out_nanos(150));
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(ticks_to_nanos);
        RUN_TEST(nanos_between_ticks);
        RUN_TEST(not_timed_out_nanos_immediately);
        RUN_TEST(timed_out_nanos_millisecond_checks);
        RUN_TEST(timed_out_nanos);
        RUN_TEST(timed_out_nanos_after_nanoseconds);
        RUN_TEST(mark_resets_time);
    }

    TeensyTimestampTest() : TestSuite(__FILE__) {};
};

}
}
#endif //I2C_UNDERNEATH_COMMON_HAL_TEENSY_TEENSY_TIMESTAMP_TEST_H
