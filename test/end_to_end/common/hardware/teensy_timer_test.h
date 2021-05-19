// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_HARDWARE_TEENSY_TIMER_TEST_H
#define I2C_UNDERNEATH_COMMON_HARDWARE_TEENSY_TIMER_TEST_H

#include <unity.h>
#include <Arduino.h>
#include <common/hardware/teensy_timer.h>
#include "../../../utils/test_suite.h"

namespace common {
namespace hardware {

class TeensyTimerTest : public TestSuite {
public:
    static common::hardware::TeensyTimer* the_timer;

    static void timer_isr() {
        the_timer->raise_timer_event();
    }

    static void begin_triggers_callbacks() {
        // GIVEN a timer
        common::hardware::TeensyTimer timer = TeensyTimer();
        the_timer = &timer;
        uint32_t before = ARM_DWT_CYCCNT;
        uint32_t after = before;
        auto callback = [&after]() {
            after = ARM_DWT_CYCCNT;
        };
        timer.set_timer_isr(timer_isr);

        // WHEN I start the timer
        timer.begin_micros(callback, 50);

        // THEN the callback is executed after the correct delay
        delayMicroseconds(55);
        timer.end();    // Make sure end is called if the test fails.
        TEST_ASSERT_GREATER_OR_EQUAL_INT32(50'000 * 0.6, after - before);
    }

    static void end_stops_timer() {
        // GIVEN a timer
        common::hardware::TeensyTimer timer = TeensyTimer();
        the_timer = &timer;
        uint32_t callback_count = 0;
        auto callback = [&callback_count]() {
            callback_count++;
        };
        timer.set_timer_isr(timer_isr);
        timer.begin_micros(callback, 20);
        delayMicroseconds(50);

        // WHEN I call end
        timer.end();
        delayMicroseconds(20);

        // THEN the callback was called exactly twice
        timer.end();    // Make sure end is called if the test fails.
        TEST_ASSERT_EQUAL(2, callback_count);
    }

    static void destructor_ends_timer() {
        // GIVEN a timer
        auto timer = new TeensyTimer();
        the_timer = timer;
        uint32_t callback_count = 0;
        auto callback = [&callback_count]() {
            callback_count++;
        };
        timer->set_timer_isr(timer_isr);
        timer->begin_micros(callback, 20);
        delayMicroseconds(50);

        // WHEN I call the destructor
        delete(timer);
        delayMicroseconds(20);

        // THEN the callback was called exactly twice
        timer->end();    // Make sure end is called if the test fails.
        TEST_ASSERT_EQUAL(2, callback_count);
    }

    static void restart_timer() {
        // GIVEN a timer
        common::hardware::TeensyTimer timer = TeensyTimer();
        the_timer = &timer;
        uint32_t callback_count = 0;
        auto callback = [&callback_count]() {
            callback_count++;
        };
        timer.set_timer_isr(timer_isr);
        timer.begin_micros(callback, 20);
        delayMicroseconds(15);

        // WHEN I call end
        timer.restart();
        delayMicroseconds(35);

        // THEN the callback was called exactly twice
        timer.end();    // Make sure end is called if the test fails.
        TEST_ASSERT_EQUAL(1, callback_count);
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(begin_triggers_callbacks);
        RUN_TEST(end_stops_timer);
        RUN_TEST(destructor_ends_timer);
        RUN_TEST(restart_timer);
    }

    TeensyTimerTest() : TestSuite(__FILE__) {};
};

common::hardware::TeensyTimer* TeensyTimerTest::the_timer;

}
}
#endif //I2C_UNDERNEATH_COMMON_HARDWARE_TEENSY_TIMER_TEST_H
