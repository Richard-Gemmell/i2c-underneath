// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_TRACE_TEST_H
#define I2C_UNDERNEATH_BUS_TRACE_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include "fakes/common/hal/fake_clock.h"
#include "fakes/fake_serial.h"
#include <bus_trace/bus_trace.h>

namespace bus_trace {

class BusTraceTest : public TestSuite {
    static const size_t MAX_EVENTS = 20;

public:
    static void max_events_required_without_pin_events() {
        size_t write_1_byte = BusTrace::max_events_required(1, false);
        TEST_ASSERT_EQUAL_UINT32(60, write_1_byte); // SDA 22 - SCL 38
        size_t write_2_bytes = BusTrace::max_events_required(2, false);
        TEST_ASSERT_EQUAL_UINT32(88, write_2_bytes); // SDA 32 - SCL 56
    }

    static void max_events_required_with_pin_events() {
        size_t write_1_byte = BusTrace::max_events_required(1, true);
        TEST_ASSERT_EQUAL_UINT32(116, write_1_byte); // SDA 42 - SCL 74
        size_t write_2_bytes = BusTrace::max_events_required(2, true);
        TEST_ASSERT_EQUAL_UINT32(172, write_2_bytes); // SDA 62 - SCL 110
    }

    static void new_trace_is_empty() {
        BusEvent events[MAX_EVENTS];
        BusTrace trace(events, MAX_EVENTS);
        TEST_ASSERT_EQUAL_UINT32(0, trace.event_count());
    }

    static void cannot_get_event_that_has_not_been_added() {
        BusEvent events[MAX_EVENTS];
        BusTrace trace(events, MAX_EVENTS);

        // WHEN we try to get the first event before it's been added
        // THEN we get nullptr
        TEST_ASSERT_NULL(trace.event(0))
        TEST_ASSERT_NULL(trace.event(1))

        // WHEN we add an event
        trace.add_event(BusEvent(123, BusEventFlags::SDA_LINE_STATE));

        // THEN we can retrieve the new event
        auto actual = trace.event(0);
        TEST_ASSERT_NOT_NULL(actual)
        // BUT not one that hasn't been recorded
        TEST_ASSERT_NULL(trace.event(1))
    }

    static void add_event_drops_excess_events() {
        // GIVEN a trace which is full
        size_t max_event_count = 1;
        BusEvent events[max_event_count];
        BusTrace trace(events, max_event_count);
        BusEvent event(123, BusEventFlags::SCL_LINE_CHANGED);
        trace.add_event(event);
        TEST_ASSERT_EQUAL_UINT32(1, trace.event_count());

        // WHEN we attempt to record another event
        trace.add_event(BusEvent(45, BusEventFlags::SDA_LINE_STATE));

        // THEN the event is ignored
        TEST_ASSERT_EQUAL_UINT32(1, trace.event_count());
        TEST_ASSERT_NULL(trace.event(1))
        TEST_ASSERT_TRUE(event == *trace.event(0));
    }

    static void new_trace_compares_to_an_empty_trace() {
        // GIVEN 2 traces
        BusEvent events1[MAX_EVENTS];
        BusTrace trace1(events1, MAX_EVENTS);
        BusEvent trace2[0];
        TEST_ASSERT_NOT_NULL(trace2)

        // WHEN we compare them
        auto equivalent = trace1.compare_to(trace2, 0);

        // THEN they are equivalent
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, equivalent);
    }

    static void add_start_condition(BusTrace& trace) {
        trace.add_event(BusEvent(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE));   // Both HIGH
        trace.add_event(BusEvent(100, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED)); // SDA falls
        trace.add_event(BusEvent(100, BusEventFlags::SCL_LINE_CHANGED)); // SCL falls
    }

    static void traces_are_comparable_if_lines_match() {
        // GIVEN 2 traces with
        //   - the same level edges and states
        //   - different timings
        BusEvent events1[MAX_EVENTS];
        BusTrace trace1(events1, MAX_EVENTS);
        add_start_condition(trace1);
        BusEvent trace2[MAX_EVENTS];
        trace2[0].flags = BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE;
        trace2[0].delta_t_nanos = 0;
        trace2[1].flags = BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED;
        trace2[1].delta_t_nanos = 50;
        trace2[2].flags = BusEventFlags::SCL_LINE_CHANGED;
        trace2[2].delta_t_nanos = 50;

        // WHEN we compare the traces
        auto equivalent = trace1.compare_to(trace2, 3);

        // THEN they are equivalent if the levels edges occur
        // in the same order. The deltas are ignored.
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, equivalent);
    }

    static void traces_are_not_comparable_if_one_is_longer_than_the_other() {
        // GIVEN 2 traces with
        //   - the same level edges and states
        //   - different timings
        BusEvent events1[MAX_EVENTS];
        BusTrace trace1(events1, MAX_EVENTS);
        add_start_condition(trace1);
        BusEvent trace2[1];
        trace2[0].flags = BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE;
        trace2[0].delta_t_nanos = 0;

        // WHEN we compare the traces
        auto equivalent = trace1.compare_to(trace2, 1);

        // THEN they are equivalent if the levels edges occur
        // in the same order. The deltas are ignored.
        TEST_ASSERT_EQUAL_UINT32(1, equivalent);
    }

    static void compare_to_returns_index_of_first_difference() {
        // GIVEN 2 traces with different level edges
        BusEvent events1[MAX_EVENTS * 2];
        BusTrace trace1(events1, MAX_EVENTS * 2);
        add_start_condition(trace1);
        BusEvent trace2[MAX_EVENTS];
        trace2[0].flags = BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE;
        trace2[0].delta_t_nanos = 0;
        trace2[1].flags = BusEventFlags::SDA_LINE_CHANGED;
        trace2[1].delta_t_nanos = 50;
        trace2[2].flags = BusEventFlags::SCL_LINE_CHANGED;
        trace2[2].delta_t_nanos = 50;

        // WHEN we compare the traces
        auto first_difference = trace1.compare_to(trace2, MAX_EVENTS);

        // THEN compare_to returns the index of the first difference
        TEST_ASSERT_EQUAL_UINT32(1, first_difference);
    }

    static void print_trace() {
        // GIVEN a trace containing all event types
        BusEvent events[MAX_EVENTS];
        BusTrace trace(events, MAX_EVENTS);
        trace.add_event(BusEvent(10, BusEventFlags::BOTH_LOW_AND_UNCHANGED));
        trace.add_event(BusEvent(11, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE));
        trace.add_event(BusEvent(12, BusEventFlags::SDA_LINE_CHANGED));
        trace.add_event(BusEvent(13, BusEventFlags::SDA_LINE_STATE));
        trace.add_event(BusEvent(14, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE));
        trace.add_event(BusEvent(15, BusEventFlags::SCL_LINE_CHANGED));
        trace.add_event(BusEvent(21, BusEventFlags::SCL_LINE_STATE));

        // WHEN we print the trace
        FakeSerial serial;
        size_t bytes_printed = trace.printTo(serial);
//        Serial.print(trace);

        // THEN the output is correct
        String expected = "SDA _/\\'___\r\n";
        expected       += "SCL ____/\\'\r\n";
//        Serial.print(expected);
        TEST_ASSERT_EQUAL(0, serial.strcmp(expected));
        TEST_ASSERT_EQUAL_UINT32(expected.length(), bytes_printed);
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(max_events_required_without_pin_events);
        RUN_TEST(max_events_required_with_pin_events);
        RUN_TEST(new_trace_is_empty);
        RUN_TEST(cannot_get_event_that_has_not_been_added);
        RUN_TEST(add_event_drops_excess_events);
        RUN_TEST(new_trace_compares_to_an_empty_trace);
        RUN_TEST(traces_are_comparable_if_lines_match);
        RUN_TEST(traces_are_not_comparable_if_one_is_longer_than_the_other);
        RUN_TEST(compare_to_returns_index_of_first_difference);
        RUN_TEST(print_trace);
    }

    BusTraceTest() : TestSuite(__FILE__) {};
};

// Define statics

}
#endif //I2C_UNDERNEATH_BUS_TRACE_TEST_H