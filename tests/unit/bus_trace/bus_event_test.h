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

    void test() final {
        RUN_TEST(test_create_bus_event);
    }

    BusEventTest() : TestSuite(__FILE__) {};
};
}
#endif //I2C_UNDERNEATH_BUS_EVENT_TEST_H
