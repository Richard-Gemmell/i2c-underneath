// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_TRACE_TEST_H
#define I2C_UNDERNEATH_BUS_TRACE_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include "fakes/common/hal/fake_clock.h"
#include <bus_trace/bus_trace.h>

namespace bus_trace {

#define NUM_EVENTS 3

class BusTraceTest : public TestSuite {
    static common::hal::FakeClock clock;

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
        BusEvent events[NUM_EVENTS];
        BusTrace trace(events, NUM_EVENTS, clock);
        TEST_ASSERT_EQUAL_UINT32(0, trace.event_count());
    }

    static void cannot_get_event_that_has_not_been_recorded() {
        BusEvent events[NUM_EVENTS];
        BusTrace trace(events, NUM_EVENTS, clock);

        // WHEN we try to get the first event before it's been added
        // THEN we get nullptr
        TEST_ASSERT_NULL(trace.event(0))
        TEST_ASSERT_NULL(trace.event(1))

        // WHEN we record an event
        trace.record_event(BusEventFlags::SDA_LINE_STATE);

        // THEN we can retrieve the new event
        auto actual = trace.event(0);
        TEST_ASSERT_NOT_NULL(actual)
        // BUT not one that hasn't been recorded
        TEST_ASSERT_NULL(trace.event(1))
    }

    static void first_call_to_record_event_creates_correct_event() {
        BusEvent events[NUM_EVENTS];
        BusTrace trace(events, NUM_EVENTS, clock);

        // WHEN we record an event
        auto flags = BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE;
        trace.record_event(flags);

        // THEN the trace contains a single event
        TEST_ASSERT_EQUAL_UINT32(1, trace.event_count());
        // AND the event is created correctly
        auto actualEvent = trace.event(0);
        TEST_ASSERT_NOT_NULL(actualEvent)
        TEST_ASSERT_EQUAL(flags, actualEvent->flags);
        TEST_ASSERT_EQUAL_UINT32(0, actualEvent->delta_t_in_ticks);
    }

    static void second_call_to_record_event_creates_event_with_correct_delta() {
        // GIVEN a trace containing a single event
        BusEvent events[NUM_EVENTS];
        BusTrace trace(events, NUM_EVENTS, clock);
        trace.record_event(BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE);
        clock.system_tick = clock.system_tick + 999;

        // WHEN we record the second event
        auto flags = BusEventFlags::SDA_LINE_STATE;
        trace.record_event(flags);

        // THEN the trace contains 2 events
        TEST_ASSERT_EQUAL_UINT32(2, trace.event_count());
        // AND the event has the correct delta
        auto actualEvent = trace.event(1);
        TEST_ASSERT_NOT_NULL(actualEvent)
        TEST_ASSERT_EQUAL(flags, actualEvent->flags);
        TEST_ASSERT_EQUAL_UINT32(999, actualEvent->delta_t_in_ticks);
    }

    static void later_events_have_correct_delta() {
        // GIVEN a trace containing 2 events
        BusEvent events[NUM_EVENTS];
        BusTrace trace(events, NUM_EVENTS, clock);
        trace.record_event(BusEventFlags::SDA_LINE_STATE);
        clock.system_tick = clock.system_tick + 300;
        trace.record_event(BusEventFlags::SDA_LINE_CHANGED);
        clock.system_tick = clock.system_tick + 200;

        // WHEN we record another event
        auto flags = BusEventFlags::SDA_LINE_STATE;
        trace.record_event(flags);

        // THEN the trace contains 3 events
        TEST_ASSERT_EQUAL_UINT32(3, trace.event_count());
        // AND the event has the correct delta
        auto actualEvent = trace.event(2);
        TEST_ASSERT_NOT_NULL(actualEvent)
        TEST_ASSERT_EQUAL(flags, actualEvent->flags);
        TEST_ASSERT_EQUAL_UINT32(200, actualEvent->delta_t_in_ticks);
    }

    static void delta_is_calculated_correctly_when_tick_count_wraps() {
        // GIVEN we've recorded an event just before the system tick count hits its maximum value
        BusEvent events[NUM_EVENTS];
        BusTrace trace(events, NUM_EVENTS, clock);
        clock.system_tick = UINT32_MAX - 100;   // A very large tick count
        trace.record_event(BusEventFlags::SCL_LINE_CHANGED);

        // WHEN we record another event after the clock wraps
        clock.system_tick = 150;    // A very small tick count
        trace.record_event(BusEventFlags::SDA_LINE_STATE);

        // THEN the delta is calculated correctly
        auto actualEvent = trace.event(1);
        TEST_ASSERT_NOT_NULL(actualEvent)
        TEST_ASSERT_EQUAL_UINT32(251, actualEvent->delta_t_in_ticks);
    }

    static void record_event_drops_excess_events() {
        // GIVEN a trace which is full
        size_t max_event_count = 1;
        BusEvent events[max_event_count];
        BusTrace trace(events, max_event_count, clock);
        trace.record_event(BusEventFlags::SCL_LINE_CHANGED);
        TEST_ASSERT_EQUAL_UINT32(1, trace.event_count());

        // WHEN we attempt to record another event
        trace.record_event(BusEventFlags::SDA_LINE_STATE);

        // THEN the event is ignored
        TEST_ASSERT_EQUAL_UINT32(1, trace.event_count());
        TEST_ASSERT_NULL(trace.event(1))
        TEST_ASSERT_EQUAL(BusEventFlags::SCL_LINE_CHANGED, trace.event(0)->flags);
    }

    static void new_trace_compares_to_an_empty_trace() {
        // GIVEN 2 traces
        BusEvent events1[NUM_EVENTS];
        BusTrace trace1(events1, NUM_EVENTS, clock);
        BusEvent trace2[0];
        TEST_ASSERT_NOT_NULL(trace2)

        // WHEN we compare them
        auto equivalent = trace1.compare_to(trace2, 0);

        // THEN they are equivalent
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, equivalent);
    }

    static void record_start_condition(BusTrace& trace) {
        trace.record_event(BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE);   // Both HIGH
        clock.system_tick += 100;
        trace.record_event(BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED); // SDA falls
        clock.system_tick += 100;
        trace.record_event(BusEventFlags::SCL_LINE_CHANGED);                                 // SCL falls
    }

    static void traces_are_comparable_if_lines_match() {
        // GIVEN 2 traces with
        //   - the same level edges and states
        //   - different timings
        BusEvent events1[NUM_EVENTS*2];
        BusTrace trace1(events1, NUM_EVENTS*2, clock);
        record_start_condition(trace1);
        BusEvent trace2[NUM_EVENTS];
        trace2[0].flags = BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE;
        trace2[0].delta_t_in_ticks = 0;
        trace2[1].flags = BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED;
        trace2[1].delta_t_in_ticks = 50;
        trace2[2].flags = BusEventFlags::SCL_LINE_CHANGED;
        trace2[2].delta_t_in_ticks = 50;

        // WHEN we compare the traces
        auto equivalent = trace1.compare_to(trace2, NUM_EVENTS);

        // THEN they are equivalent if the levels edges occur
        // in the same order. The deltas are ignored.
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, equivalent);
    }

    static void traces_are_not_comparable_if_one_is_longer_than_the_other() {
        // GIVEN 2 traces with
        //   - the same level edges and states
        //   - different timings
        BusEvent events1[NUM_EVENTS];
        BusTrace trace1(events1, NUM_EVENTS, clock);
        record_start_condition(trace1);
        BusEvent trace2[1];
        trace2[0].flags = BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE;
        trace2[0].delta_t_in_ticks = 0;

        // WHEN we compare the traces
        auto equivalent = trace1.compare_to(trace2, 1);

        // THEN they are equivalent if the levels edges occur
        // in the same order. The deltas are ignored.
        TEST_ASSERT_EQUAL_UINT32(1, equivalent);
    }

    static void compare_to_returns_index_of_first_difference() {
        // GIVEN 2 traces with different level edges
        BusEvent events1[NUM_EVENTS*2];
        BusTrace trace1(events1, NUM_EVENTS*2, clock);
        record_start_condition(trace1);
        BusEvent trace2[NUM_EVENTS];
        trace2[0].flags = BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE;
        trace2[0].delta_t_in_ticks = 0;
        trace2[1].flags = BusEventFlags::SDA_LINE_CHANGED;
        trace2[1].delta_t_in_ticks = 50;
        trace2[2].flags = BusEventFlags::SCL_LINE_CHANGED;
        trace2[2].delta_t_in_ticks = 50;

        // WHEN we compare the traces
        auto first_difference = trace1.compare_to(trace2, NUM_EVENTS);

        // THEN compare_to returns the index of the first difference
        TEST_ASSERT_EQUAL_UINT32(1, first_difference);
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(max_events_required_without_pin_events);
        RUN_TEST(max_events_required_with_pin_events);
        RUN_TEST(new_trace_is_empty);
        RUN_TEST(first_call_to_record_event_creates_correct_event);
        RUN_TEST(cannot_get_event_that_has_not_been_recorded);
        RUN_TEST(second_call_to_record_event_creates_event_with_correct_delta);
        RUN_TEST(later_events_have_correct_delta);
        RUN_TEST(delta_is_calculated_correctly_when_tick_count_wraps);
        RUN_TEST(record_event_drops_excess_events);
        RUN_TEST(new_trace_compares_to_an_empty_trace);
        RUN_TEST(traces_are_comparable_if_lines_match);
        RUN_TEST(traces_are_not_comparable_if_one_is_longer_than_the_other);
        RUN_TEST(compare_to_returns_index_of_first_difference);
    }

    BusTraceTest() : TestSuite(__FILE__) {};
};

// Define statics
common::hal::FakeClock bus_trace::BusTraceTest::clock = common::hal::FakeClock();

}
#endif //I2C_UNDERNEATH_BUS_TRACE_TEST_H