// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_TEENSY_CLOCK_TEST_H
#define I2C_UNDERNEATH_TEENSY_CLOCK_TEST_H
#include <unity.h>
#include <Arduino.h>
#include <common/hal/teensy/teensy_clock.h>
#include "utils/test_suite.h"

namespace common {
namespace hal {

class TeensyClockTest : public TestSuite {
public:
    static void get_system_tick() {
        TeensyClock clock = TeensyClock();

        uint32_t beforeActual = ARM_DWT_CYCCNT;
        uint32_t actual = clock.GetSystemTick();
        uint32_t afterActual = ARM_DWT_CYCCNT;

        TEST_ASSERT_GREATER_OR_EQUAL(beforeActual, actual);
        TEST_ASSERT_LESS_OR_EQUAL(afterActual, actual);
    }

    static void no_overhead_to_get_ticks() {
        TeensyClock clock = TeensyClock();

        uint32_t beforeExpected = ARM_DWT_CYCCNT;
        uint32_t expectedTotal = 0; // Hack to stop compiler optimising away the critical call which is does sometimes but not always.
        for (int i = 0; i < 100; ++i) {
            expectedTotal += ARM_DWT_CYCCNT;
        }
        uint32_t expectedDelta = ARM_DWT_CYCCNT - beforeExpected;

        uint32_t beforeActual = ARM_DWT_CYCCNT;
        uint32_t actualTotal = 0; // Hack to stop compiler optimising away the critical call which is does sometimes but not always.
        for (int i = 0; i < 100; ++i) {
            actualTotal += clock.GetSystemTick();
        }
        uint32_t actualDelta = ARM_DWT_CYCCNT - beforeActual;

        Serial.print("Junk. Please Ignore. ");Serial.print(expectedTotal);Serial.println(actualTotal);
//        Serial.print("Raw: ");Serial.print(expectedDelta);
//        Serial.print(" Clock: ");Serial.println(actualDelta);
        TEST_ASSERT_UINT32_WITHIN(10, expectedDelta, actualDelta);
    }

    static void get_system_millis() {
        TeensyClock clock = TeensyClock();

        uint32_t beforeActual = millis();
        uint32_t actual = clock.GetSystemMills();
        uint32_t afterActual = millis();

        TEST_ASSERT_GREATER_OR_EQUAL(beforeActual, actual);
        TEST_ASSERT_LESS_OR_EQUAL(afterActual, actual);
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(get_system_tick);
        RUN_TEST(no_overhead_to_get_ticks);
        RUN_TEST(get_system_millis);
    }

    TeensyClockTest() : TestSuite(__FILE__) {};
};

}
}

#endif //I2C_UNDERNEATH_TEENSY_CLOCK_TEST_H
