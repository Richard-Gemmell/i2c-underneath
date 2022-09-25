// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_EVENT_TEST_H
#define I2C_UNDERNEATH_BUS_EVENT_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include <bus_trace/bus_event.h>

namespace bus_trace {

class BusEventTest : public TestSuite {

public:
    static void test_create_bus_event() {
        BusEvent event(123, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE);
        TEST_ASSERT_EQUAL(BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE, event.flags);
        TEST_ASSERT_EQUAL(123, event.delta_t_in_ticks);
    }

    static void test_copy() {
        BusEvent event(123, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE);
        BusEvent actual = event;
        TEST_ASSERT_EQUAL(BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE, actual.flags);
        TEST_ASSERT_EQUAL(123, actual.delta_t_in_ticks);
    }

    static void comparison_operators() {
        BusEvent a(123, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE);
        BusEvent a1(123, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE);
        BusEvent b(123, BusEventFlags::BOTH_LOW_AND_UNCHANGED);
        BusEvent c(44, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE);

        // Object equals itself
        TEST_ASSERT_TRUE(a == a);
        TEST_ASSERT_FALSE(a != a);
        // Object equals identical object
        TEST_ASSERT_TRUE(a == a1);
        TEST_ASSERT_FALSE(a != a1);
        // Object does not equal similar but different objects
        TEST_ASSERT_FALSE(a == b);
        TEST_ASSERT_TRUE(a != b);
        TEST_ASSERT_FALSE(a == c);
        TEST_ASSERT_TRUE(a != c);
    }

    void test() final {
        RUN_TEST(test_create_bus_event);
        RUN_TEST(test_copy);
        RUN_TEST(comparison_operators);
    }

    BusEventTest() : TestSuite(__FILE__) {};
};
}
#endif //I2C_UNDERNEATH_BUS_EVENT_TEST_H
