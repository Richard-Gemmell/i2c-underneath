// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_TRACE_TEST_H
#define I2C_UNDERNEATH_BUS_TRACE_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include <bus_trace/bus_trace.h>

namespace bus_trace {

class BusTraceTest : public TestSuite {
//    static int value;

public:
//    void setUp() final {
//        value = 5;
//    }
//
//    void tearDown() final {
//        value = 0;
//    }

    static void test_max_events_required_without_pin_events() {
        size_t write_1_byte = BusTrace::max_events_required(1, false);
        TEST_ASSERT_EQUAL(60, write_1_byte); // SDA 22 - SCL 38
        size_t write_2_bytes = BusTrace::max_events_required(2, false);
        TEST_ASSERT_EQUAL(88, write_2_bytes); // SDA 32 - SCL 56
    }

    static void test_max_events_required_with_pin_events() {
        size_t write_1_byte = BusTrace::max_events_required(1, true);
        TEST_ASSERT_EQUAL(116, write_1_byte); // SDA 42 - SCL 74
        size_t write_2_bytes = BusTrace::max_events_required(2, true);
        TEST_ASSERT_EQUAL(172, write_2_bytes); // SDA 62 - SCL 110
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(test_max_events_required_without_pin_events);
        RUN_TEST(test_max_events_required_with_pin_events);
    }

    BusTraceTest() : TestSuite(__FILE__) {};
};

// Define statics
//int BusTraceTest::value;

}
#endif //I2C_UNDERNEATH_BUS_TRACE_TEST_H