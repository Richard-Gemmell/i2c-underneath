// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_EVENT_FLAGS_TEST_H
#define I2C_UNDERNEATH_BUS_EVENT_FLAGS_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include <bus_trace/bus_event_flags.h>

namespace bus_trace {

class BusEventFlagsTest : public TestSuite {
public:
    static void test_values() {
        TEST_ASSERT_EQUAL(0x80, BusEventFlags::SDA_PIN_CHANGED);
        TEST_ASSERT_EQUAL(0x40, BusEventFlags::SCL_PIN_CHANGED);
        TEST_ASSERT_EQUAL(0x20, BusEventFlags::SDA_PIN_STATE);
        TEST_ASSERT_EQUAL(0x10, BusEventFlags::SCL_PIN_STATE);
        TEST_ASSERT_EQUAL(0x08, BusEventFlags::SDA_LINE_CHANGED);
        TEST_ASSERT_EQUAL(0x04, BusEventFlags::SCL_LINE_CHANGED);
        TEST_ASSERT_EQUAL(0x02, BusEventFlags::SDA_LINE_STATE);
        TEST_ASSERT_EQUAL(0x01, BusEventFlags::SCL_LINE_STATE);
        TEST_ASSERT_EQUAL(0x00, BusEventFlags::BOTH_LOW_AND_UNCHANGED);
    }

    static void test_operator_or() {
        auto flags = BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE;
        TEST_ASSERT_EQUAL(0x0A, flags);
    }

    static void test_operator_and() {
        auto blank = BusEventFlags::SDA_LINE_CHANGED & BusEventFlags::SDA_LINE_STATE;
        TEST_ASSERT_EQUAL(0x0, blank);

        auto flags = (BusEventFlags)0xFF & BusEventFlags::SDA_LINE_STATE;
        TEST_ASSERT_EQUAL(BusEventFlags::SDA_LINE_STATE, flags);
    }

    static void test_operator_xor() {
        auto flags = BusEventFlags::SDA_LINE_CHANGED ^ BusEventFlags::SDA_LINE_STATE;
        TEST_ASSERT_EQUAL(BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE, flags);

        flags = BusEventFlags::SDA_LINE_CHANGED ^ BusEventFlags::SDA_LINE_CHANGED;
        TEST_ASSERT_EQUAL(0x00, flags);

        flags = BusEventFlags::SDA_LINE_CHANGED ^ (BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE);
        TEST_ASSERT_EQUAL(BusEventFlags::SDA_LINE_STATE, flags);
    }

    static void test_operator_not() {
        auto flags = BusEventFlags::SDA_LINE_CHANGED;

        flags = ~flags;
        TEST_ASSERT_EQUAL(0b11110111, flags);

        flags = ~flags;
        TEST_ASSERT_EQUAL(BusEventFlags::SDA_LINE_CHANGED, flags);
    }

    void test() final {
        RUN_TEST(test_values);
        RUN_TEST(test_operator_or);
        RUN_TEST(test_operator_and);
        RUN_TEST(test_operator_xor);
        RUN_TEST(test_operator_not);
    }

    BusEventFlagsTest() : TestSuite(__FILE__) {};
};
}
#endif //I2C_UNDERNEATH_BUS_EVENT_FLAGS_TEST_H
