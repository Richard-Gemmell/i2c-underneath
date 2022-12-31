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
    static const size_t MAX_EVENTS = 100;

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
        TEST_ASSERT_NULL(trace.event(0));
        TEST_ASSERT_NULL(trace.event(1));

        // WHEN we add an event
        trace.add_event(BusEvent(123, BusEventFlags::SDA_LINE_STATE));

        // THEN we can retrieve the new event
        auto actual = trace.event(0);
        TEST_ASSERT_NOT_NULL(actual);
        // BUT not one that hasn't been recorded
        TEST_ASSERT_NULL(trace.event(1));
    }

    static void add_event_with_bus_event_object() {
        BusTrace trace(MAX_EVENTS);

        // WHEN we add an event
        trace.add_event(BusEvent(456, BusEventFlags::SDA_LINE_STATE));

        // THEN we can retrieve the new event
        auto actual = trace.event(0);
        TEST_ASSERT_TRUE(*actual == BusEvent(456, BusEventFlags::SDA_LINE_STATE));
    }

    static void add_event_override() {
        BusTrace trace(MAX_EVENTS);

        // WHEN we add an event with the override
        trace.add_event(123, BusEventFlags::SDA_LINE_STATE);

        // THEN we can retrieve the new event
        auto actual = trace.event(0);
        TEST_ASSERT_TRUE(*actual == BusEvent(123, BusEventFlags::SDA_LINE_STATE));
    }

    static void add_event_override_calculates_delta() {
        BusTrace trace(MAX_EVENTS);

        // WHEN we add an event with the override
        trace.add_event(123, BusEventFlags::SDA_LINE_STATE);
        trace.add_event(130, BusEventFlags::SDA_LINE_STATE);

        // THEN we can retrieve the new event
        TEST_ASSERT_TRUE(*trace.event(0) == BusEvent(123, BusEventFlags::SDA_LINE_STATE));
        TEST_ASSERT_TRUE(*trace.event(1) == BusEvent(7, BusEventFlags::SDA_LINE_STATE));
    }

    static void add_event_gets_system_tick_directly_on_teensy4() {
#ifdef ARDUINO_TEENSY40
        BusTrace trace(MAX_EVENTS);

        // WHEN we add a couple of events and BusTrace get the system tick count
        trace.add_event(BusEventFlags::SCL_LINE_CHANGED);
        trace.add_event(BusEventFlags::SDA_LINE_CHANGED);

        // THEN we can retrieve the new event
        auto actual = trace.event(1);
        TEST_ASSERT_EQUAL(actual->flags, BusEventFlags::SDA_LINE_CHANGED);
        TEST_ASSERT_UINT32_WITHIN(2, 4, actual->delta_t_in_ticks);
#endif
    }

    static void add_event_is_fast_enough_on_a_teensy4() {
#ifdef ARDUINO_TEENSY40
        BusTrace trace(MAX_EVENTS);

        // WHEN we add events
        trace.add_event(BusEventFlags::SDA_LINE_CHANGED);
        trace.add_event(BusEventFlags::SDA_LINE_CHANGED);
        trace.add_event(BusEventFlags::SDA_LINE_CHANGED);
        trace.add_event(BusEventFlags::SDA_LINE_CHANGED);
        trace.add_event(BusEventFlags::SDA_LINE_CHANGED);

        // THEN the average time to add an event is a few ticks
        for (size_t i = 1; i < trace.event_count(); ++i) {
            auto actual = trace.event(i);
//            Serial.printf("Index %d: delta_t %d\n", i, actual->delta_t_in_ticks);
            TEST_ASSERT_UINT32_WITHIN(2, 6, actual->delta_t_in_ticks);
        }
#endif
    }

    static void add_event_gets_system_tick_from_clock() {
#if !defined(ARDUINO_TEENSY40) && !defined(ARDUINO_TEENSY41)
        common::hal::FakeClock clock;
        clock.system_tick = 12345;
        BusTrace trace(&clock, MAX_EVENTS);

        // WHEN we add an event
        trace.add_event(BusEventFlags::SDA_LINE_STATE);

        // THEN we can retrieve the new event
        auto actual = trace.event(0);
        TEST_ASSERT_TRUE(*actual == BusEvent(12345u, BusEventFlags::SDA_LINE_STATE));
#endif
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
        TEST_ASSERT_NULL(trace.event(1));
        TEST_ASSERT_TRUE(event == *trace.event(0));
    }

    static void reset() {
        // GIVEN a trace that contains some events
        BusTrace trace(MAX_EVENTS);
        trace.add_event(BusEventFlags::SCL_LINE_CHANGED);
        trace.add_event(BusEventFlags::SDA_LINE_CHANGED);
        delayNanoseconds(1000);
        TEST_ASSERT_EQUAL(2, trace.event_count());

        // WHEN we call reset()
        trace.reset();
        trace.add_event(BusEventFlags::SCL_LINE_CHANGED);

        // THEN the event count was reset
        TEST_ASSERT_EQUAL(1, trace.event_count());
        // AND the delta to the next event starts from 0 again
        TEST_ASSERT_UINT32_WITHIN(2, 6, trace.event(0)->delta_t_in_ticks);
    }

    static void destructor_does_not_deletes_supplied_array_of_events() {
        BusEvent events[MAX_EVENTS];
        auto trace = new BusTrace(events, MAX_EVENTS);
        delete(trace);
    }

    static void destructor_deletes_internal_array_of_events_if_it_owns_them() {
        auto trace = new BusTrace(MAX_EVENTS);
        delete(trace);
    }

    static void empty_traces_are_identical() {
        // GIVEN 2 traces with different capacities
        BusTrace trace1(MAX_EVENTS);
        BusTrace trace2(0);

        // WHEN we compare them
        auto first_difference = trace1.is_identical_to(trace2);

        // THEN they are equivalent
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, first_difference);
    }

    static void traces_are_identical_if_all_details_match() {
        // GIVEN 2 traces with
        //   - the same level edges and states
        //   - the same timings
        BusEvent events1[MAX_EVENTS];
        BusTrace trace1(events1, MAX_EVENTS);
        add_start_condition(trace1);
        BusEvent events2[MAX_EVENTS];
        BusTrace trace2(events2, MAX_EVENTS);
        trace2.add_event(BusEvent(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE));
        trace2.add_event(BusEvent(50, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED));
        trace2.add_event(BusEvent(50, BusEventFlags::SCL_LINE_CHANGED));

        // WHEN we compare the traces
        auto first_difference = trace1.compare_edges(trace2);

        // THEN they are equivalent if the levels and edges occur
        // in the same order. The deltas and any pin states are ignored.
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, first_difference);
    }

    static void traces_are_not_identical_if_one_is_longer_than_the_other() {
        // GIVEN 2 traces where one trace has an extra edge
        BusEvent first_event(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE);
        BusTrace trace1(MAX_EVENTS);
        trace1.add_event(first_event);
        BusTrace trace2(MAX_EVENTS);
        trace2.add_event(first_event);
        trace2.add_event(BusEvent(100, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED));

        // WHEN we compare the traces
        auto first_difference = trace1.is_identical_to(trace2);

        // THEN they are not edge equivalent
        TEST_ASSERT_EQUAL_UINT32(1, first_difference);
    }

    static void traces_are_not_identical_if_deltas_are_different() {
        // GIVEN 2 traces with identical edges but different timings
        BusEvent first_event(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE);
        BusTrace trace1(MAX_EVENTS);
        trace1.add_event(first_event);
        trace1.add_event(BusEvent(10, BusEventFlags::SDA_LINE_CHANGED));
        BusTrace trace2(MAX_EVENTS);
        trace2.add_event(first_event);
        trace2.add_event(BusEvent(133, BusEventFlags::SDA_LINE_CHANGED));

        // WHEN we compare the traces
        auto first_difference = trace1.is_identical_to(trace2);

        // THEN compare_to returns the index of the first difference
        TEST_ASSERT_EQUAL_UINT32(1, first_difference);
    }

    static void traces_are_not_identical_if_edges_are_different() {
        // GIVEN 2 traces with different edges but identical timings
        BusEvent first_event(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE);
        BusTrace trace1(MAX_EVENTS);
        trace1.add_event(first_event);
        trace1.add_event(BusEvent(10, BusEventFlags::SCL_LINE_CHANGED));
        BusTrace trace2(MAX_EVENTS);
        trace2.add_event(first_event);
        trace2.add_event(BusEvent(10, BusEventFlags::SDA_LINE_CHANGED));

        // WHEN we compare the traces
        auto first_difference = trace1.is_identical_to(trace2);

        // THEN compare_to returns the index of the first difference
        TEST_ASSERT_EQUAL_UINT32(1, first_difference);
    }

    static void empty_traces_are_edge_comparable() {
        // GIVEN 2 traces with different capacities
        BusTrace trace1(MAX_EVENTS);
        BusTrace trace2(0);

        // WHEN we compare them
        auto first_difference = trace1.compare_edges(trace2);

        // THEN they are equivalent
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, first_difference);
    }

    static void traces_are_edge_comparable_if_lines_and_edges_match() {
        // GIVEN 2 traces with
        //   - the same level edges and states
        //   - different timings
        BusTrace trace1(MAX_EVENTS);
        add_start_condition(trace1);
        BusTrace trace2(MAX_EVENTS);
        trace2.add_event(BusEvent(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE));
        trace2.add_event(BusEvent(50, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED));
        trace2.add_event(BusEvent(50, BusEventFlags::SCL_LINE_CHANGED));

        // WHEN we compare the traces
        auto equivalent = trace1.compare_edges(trace2);

        // THEN they are equivalent if the levels and edges occur
        // in the same order. The deltas and any pin states are ignored.
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, equivalent);
    }

    static void traces_are_not_edge_comparable_if_one_is_longer_than_the_other() {
        // GIVEN 2 traces where one trace has an extra edge
        BusTrace trace1(MAX_EVENTS);
        BusEvent first_event(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE);
        trace1.add_event(first_event);
        BusTrace trace2(MAX_EVENTS);
        trace2.add_event(first_event);
        trace2.add_event(BusEvent(100, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED));

        // WHEN we compare the traces
        auto equivalent = trace1.compare_edges(trace2);

        // THEN they are not edge equivalent
        TEST_ASSERT_EQUAL_UINT32(1, equivalent);
    }

    static void compare_edges_returns_index_of_first_difference() {
        // GIVEN 2 traces with different level edges
        BusEvent events1[MAX_EVENTS * 2];
        BusTrace trace1(events1, MAX_EVENTS * 2);
        add_start_condition(trace1);
        BusEvent events2[MAX_EVENTS];
        BusTrace trace2(events2, MAX_EVENTS * 2);
        trace2.add_event(BusEvent(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE));
        trace2.add_event(BusEvent(50, BusEventFlags::SDA_LINE_CHANGED));
        trace2.add_event(BusEvent(50, BusEventFlags::SCL_LINE_CHANGED));

        // WHEN we compare the traces
        auto first_difference = trace1.compare_edges(trace2);

        // THEN compare_to returns the index of the first difference
        TEST_ASSERT_EQUAL_UINT32(1, first_difference);
    }

    static void to_message_converts_empty_trace_to_empty_message() {
        // GIVEN an empty trace
        BusTrace trace(MAX_EVENTS);

        // WHEN we convert it to a message
        BusTrace message = trace.to_message();

        // THEN we get an empty message
        TEST_ASSERT_EQUAL_UINT32(0, message.event_count());
    }

    static void to_message_strips_out_spurious_SDA_changes() {
        // GIVEN a trace containing spurious pulses
        BusTrace trace(MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder.start_bit()
                .address_byte(0xF0, false);
        // Add a spurious SDA pulse in the ACK cycle
        trace.add_event(50, SDA_LINE_CHANGED | SDA_LINE_STATE);
        trace.add_event(50, SDA_LINE_CHANGED);
        builder.ack()
                .data_byte(0xFF)
                .ack()
                .stop_bit();

        // WHEN we convert it to a message
        BusTrace message = trace.to_message();

        // THEN the spurious edges are removed
        BusTrace expected(MAX_EVENTS);
        add_simple_message(expected);
        auto edge_comparable = expected.compare_edges(message);
        TEST_ASSERT_EQUAL(SIZE_MAX, edge_comparable);
    }

    static void to_message_does_not_strip_out_spurious_SDA_changes_when_instructed() {
        // GIVEN a trace containing spurious pulses
        BusTrace trace(MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder.start_bit()
                .address_byte(0xF0, false);
        // Add a spurious SDA pulse in the ACK cycle
        trace.add_event(50, SDA_LINE_CHANGED | SDA_LINE_STATE);
        trace.add_event(50, SDA_LINE_CHANGED);
        builder.ack()
                .data_byte(0xFF)
                .ack()
                .stop_bit();

        // WHEN we convert it to a message and request that the spurious pulses are retained
        BusTrace message = trace.to_message(false);

        // THEN the spurious edges are NOT removed
        auto edge_comparable = trace.compare_edges(message);
        TEST_ASSERT_EQUAL(SIZE_MAX, edge_comparable);
    }

    static void given_a_trace_with_merged_events(BusTrace& trace) {
        // GIVEN a trace containing a data bit
        // WHERE the edges for SDA and SCL have been merged in every case
        // Add a data 1 bit where we clear SDA at the end of the bit
        trace.add_event(10, SCL_LINE_CHANGED | SDA_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        trace.add_event(20, SCL_LINE_CHANGED | SDA_LINE_CHANGED);
    }

    // The BusRecorder often records traces where the falling edge of SCL
    // and the change to SDA after SCL are recorded in a single event.
    // This only happens on the trailing edge of data bits, ACKs and NACKs
    // It could happen with START and STOP bits if the timing is very broken
    static void to_message_splits_events_with_2_edges() {
        // GIVEN a trace containing a START, a data bit and a STOP bit
        // WHERE the edges for SDA and SCL have been merged in every case
        BusTrace trace(MAX_EVENTS);
        given_a_trace_with_merged_events(trace);

        // WHEN we convert it to a message
        BusTrace message = trace.to_message();

        BusTrace expected(MAX_EVENTS);
        // THEN the start event for the data bit is split so SDA rises when SCL is LOW
        expected.add_event(10, SDA_LINE_CHANGED | SDA_LINE_STATE);
        expected.add_event(0, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        // AND the end event for the data bit is split so SDA falls when SCL is LOW
        expected.add_event(20, SCL_LINE_CHANGED | SDA_LINE_STATE);
        expected.add_event(0, SDA_LINE_CHANGED);
//        Serial.println("Expected:");
//        Serial.println(expected);
//        Serial.println("Got:");
//        Serial.println(message);
        auto identical = expected.compare_edges(message);
        TEST_ASSERT_EQUAL(SIZE_MAX, identical);
    }

    static void to_message_does_not_split_events_when_instructed() {
        // GIVEN a trace containing a START, a data bit and a STOP bit
        // WHERE the edges for SDA and SCL have been merged in every case
        BusTrace trace(MAX_EVENTS);
        given_a_trace_with_merged_events(trace);

        // WHEN we convert it to a message
        BusTrace message = trace.to_message(true, false);

        // THEN the message should be identical to the original trace
        auto edge_comparable = trace.compare_edges(message);
        TEST_ASSERT_EQUAL(SIZE_MAX, edge_comparable);
    }

    static void to_message_can_remove_last_event() {
        // GIVEN a trace containing spurious pulses
        BusTrace trace(MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder.start_bit()
                .address_byte(0xF0, false);
        // Add a spurious SDA pulse in the ACK cycle
        trace.add_event(50, SDA_LINE_CHANGED | SDA_LINE_STATE);
        trace.add_event(50, SDA_LINE_CHANGED);

        // WHEN we convert it to a message
        BusTrace message = trace.to_message();

        // THEN the spurious edges are removed
        BusTrace expected(MAX_EVENTS);
        BusTraceBuilder builder2(expected, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder2.start_bit()
                .address_byte(0xF0, false);
        auto edge_comparable = expected.compare_edges(message);
        TEST_ASSERT_EQUAL(SIZE_MAX, edge_comparable);
    }

    static void to_message_copies_clock() {
        // GIVEN a trace with some events and a clock
        common::hal::FakeClock clock;
        BusTrace trace(&clock, MAX_EVENTS);
        trace.add_event(BusEvent(100, BusEventFlags::SDA_LINE_CHANGED));
        trace.add_event(BusEvent(200, BusEventFlags::SCL_LINE_CHANGED));

        // WHEN we convert the trace into a message
        BusTrace message = trace.to_message();

        // THEN the message can calculate times correctly
        uint32_t expected = trace.nanos_to_previous(1);
        uint32_t actual = message.nanos_to_previous(1);
        TEST_ASSERT_EQUAL_UINT32(expected, actual);
    }

    static void empty_traces_are_message_comparable() {
        // GIVEN 2 traces
        BusEvent events1[MAX_EVENTS];
        BusTrace trace1(events1, MAX_EVENTS);
        BusEvent events2[0];
        BusTrace trace2(events2, 0);

        // WHEN we compare them
        auto equivalent = trace1.compare_messages(trace2);

        // THEN they are equivalent
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, equivalent);
    }

    static void traces_are_message_comparable_if_they_are_edge_comparable() {
        // GIVEN 2 traces which are edge comparable.
        BusEvent events1[MAX_EVENTS];
        BusTrace trace1(events1, MAX_EVENTS);
        add_simple_message(trace1);
        BusEvent events2[MAX_EVENTS];
        BusTrace trace2(events2, MAX_EVENTS);
        add_simple_message(trace2);

        // WHEN we compare the traces
        auto edge_comparable = trace1.compare_edges(trace2);
        auto message_comparable = trace1.compare_messages(trace2);

        // THEN they are both edge and message comparable.
        TEST_ASSERT_EQUAL_UINT32(edge_comparable, message_comparable);
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, message_comparable);
    }

    static void traces_are_message_comparable_even_if_spurious_SDA_changes_are_different() {
        // GIVEN 2 traces which are edge comparable.
        BusEvent events1[MAX_EVENTS];
        BusTrace trace(events1, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder.start_bit()
                .address_byte(0xF0, false);
        // Add a spurious SDA pulse in the ACK cycle
        trace.add_event(50, SDA_LINE_CHANGED | SDA_LINE_STATE);
        trace.add_event(50, SDA_LINE_CHANGED);
        builder.ack()
                .data_byte(0xFF)
                .ack()
                .stop_bit();

        // WHEN we compare the traces
        BusEvent events2[MAX_EVENTS];
        BusTrace expected(events2, MAX_EVENTS);
        add_simple_message(expected);
        auto edge_comparable = trace.compare_edges(expected);
        auto message_comparable = trace.compare_messages(expected);

        // THEN they are message comparable but not edge comparable
        TEST_ASSERT_NOT_EQUAL(SIZE_MAX, edge_comparable);
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, message_comparable);
    }

    static void compare_messages_returns_index_of_first_difference() {
        // GIVEN 2 traces where one trace has an extra edge
        BusEvent events1[MAX_EVENTS];
        BusTrace trace(events1, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder.start_bit()
                .address_byte(0xF0, false);
        trace.add_event(BusEvent(0, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE));
        trace.add_event(BusEvent(0, BusEventFlags::SDA_LINE_CHANGED));
        builder.ack()
                .data_byte(0xFF)
                .nack() // NACK instead of ACK
                .stop_bit();
        BusEvent events2[MAX_EVENTS];

        // WHEN we compare the traces
        BusTrace expected(events2, MAX_EVENTS);
        add_simple_message(expected);
        auto equivalent = trace.compare_messages(expected);

        // THEN they are not message equivalent
        TEST_ASSERT_EQUAL_UINT32(39, equivalent);
    }

    static void message_comparable_does_not_ignore_SDA_changes_while_SCL_is_HIGH() {
        // GIVEN 2 traces which are edge comparable.
        BusEvent events1[MAX_EVENTS];
        BusTrace trace1(events1, MAX_EVENTS);
        BusTraceBuilder builder(trace1, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        // Add spurious SDA changes while SCL is HIGH
        builder.start_bit()
                .address_byte(0xF0, false);
        trace1.add_event(50, SCL_LINE_CHANGED | SCL_LINE_STATE);
        trace1.add_event(50, SDA_LINE_CHANGED | SDA_LINE_STATE | SCL_LINE_STATE);
        trace1.add_event(50, SDA_LINE_CHANGED | SCL_LINE_STATE);
        trace1.add_event(50, SCL_LINE_CHANGED);
        builder.data_byte(0xFF)
                .ack()
                .stop_bit();
        BusEvent events2[MAX_EVENTS];
        BusTrace trace2(events2, MAX_EVENTS);
        add_simple_message(trace2);

        // WHEN we compare the traces
        auto edge_comparable = trace1.compare_edges(trace2);
        auto message_comparable = trace1.compare_messages(trace2);

        // THEN both comparisons return the same result
        TEST_ASSERT_EQUAL_UINT32(message_comparable, edge_comparable);
        TEST_ASSERT_EQUAL_UINT32(21, edge_comparable);
        TEST_ASSERT_EQUAL_UINT32(21, message_comparable);
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

    static void add_simple_message(BusTrace& trace) {
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder.start_bit()
            .address_byte(0xF0, false)
            .ack()
            .data_byte(0xFF)
            .ack()
            .stop_bit();
    }

    static void add_start_condition(BusTrace& trace) {
        trace.add_event(BusEvent(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE));   // Both HIGH
        trace.add_event(BusEvent(100, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED)); // SDA falls
        trace.add_event(BusEvent(100, BusEventFlags::SCL_LINE_CHANGED)); // SCL falls
    }

    static void nanos_to_previous_when_index_is_out_of_range() {
        // GIVEN a trace with a clock but no events
        common::hal::FakeClock clock;
        BusTrace trace(&clock, MAX_EVENTS);

        // WHEN we get the duration since the previous event
        uint32_t actual = trace.nanos_to_previous(0);

        // THEN the result is UINT32_MAX if the index is out of range
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, actual);
    }

    static void nanos_to_previous_without_a_clock() {
        // GIVEN a trace with some events but no clock
        BusTrace trace(MAX_EVENTS);
        trace.add_event(100, BusEventFlags::SDA_LINE_CHANGED);
        trace.add_event(200, BusEventFlags::SCL_LINE_CHANGED);

        // WHEN we get the duration between 2 existing events
        uint32_t actual = trace.nanos_to_previous(1);

        // THEN the result is UINT32_MAX as there is no clock
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, actual);
    }

    static void nanos_to_previous_for_first_event() {
        // GIVEN a trace with an event and a clock
        common::hal::FakeClock clock;
        BusTrace trace(&clock, MAX_EVENTS);
        trace.add_event(100, BusEventFlags::SDA_LINE_CHANGED);

        // THEN nanos_to_previous is always 0 for the first event
        uint32_t actual = trace.nanos_to_previous(0);
        TEST_ASSERT_EQUAL_UINT32(0, actual);
    }

    static void nanos_to_previous() {
        // GIVEN a trace with some events and a clock
        common::hal::FakeClock clock;
        BusTrace trace(&clock, MAX_EVENTS);
        trace.add_event(BusEvent(100, BusEventFlags::SDA_LINE_CHANGED));
        trace.add_event(BusEvent(200, BusEventFlags::SCL_LINE_CHANGED));

        // WHEN we get the duration between 2 existing events
        uint32_t actual = trace.nanos_to_previous(1);

        // THEN we get the correct duration in nanoseconds
        TEST_ASSERT_EQUAL_UINT32(400, actual);
    }

    // Returns UINT32_MAX if either index is out of range or
    // first > last, or this trace doesn't have a clock.
    static void nanos_between_when_from_is_out_of_range() {
        // GIVEN a trace with a clock
        common::hal::FakeClock clock;
        BusTrace trace(&clock, MAX_EVENTS);
        trace.add_event(100, BusEventFlags::SDA_LINE_CHANGED);

        // WHEN we try to get the duration
        uint32_t actual = trace.nanos_between(0, 1);

        // THEN the result is UINT32_MAX
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, actual);
    }

    static void nanos_between_when_to_is_out_of_range() {
        // GIVEN a trace with a clock
        common::hal::FakeClock clock;
        BusTrace trace(&clock, MAX_EVENTS);
        trace.add_event(100, BusEventFlags::SDA_LINE_CHANGED);

        // WHEN we try to get the duration
        uint32_t actual = trace.nanos_between(1, 0);

        // THEN the result is UINT32_MAX
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, actual);
    }

    static void nanos_between_when_from_is_greater_than_last() {
        // GIVEN a trace with a clock
        common::hal::FakeClock clock;
        BusTrace trace(&clock, MAX_EVENTS);
        trace.add_event(100, BusEventFlags::SDA_LINE_CHANGED);
        trace.add_event(100, BusEventFlags::SDA_LINE_CHANGED);

        // WHEN we try to get the duration
        uint32_t actual = trace.nanos_between(0, 1);

        // THEN the result is UINT32_MAX
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, actual);
    }

    static void nanos_between_without_a_clock() {
        // GIVEN a trace without a clock
        BusTrace trace(MAX_EVENTS);
        trace.add_event(100, BusEventFlags::SDA_LINE_CHANGED);
        trace.add_event(100, BusEventFlags::SDA_LINE_CHANGED);

        // WHEN we try to get the duration
        uint32_t actual = trace.nanos_between(1, 0);

        // THEN the result is UINT32_MAX
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, actual);
    }

    static void nanos_between_all_events() {
        // GIVEN a trace with some events
        common::hal::FakeClock clock;
        BusTrace trace(&clock, MAX_EVENTS);
        trace.add_event(BusEvent(100, BusEventFlags::SDA_LINE_CHANGED));
        trace.add_event(BusEvent(200, BusEventFlags::SDA_LINE_CHANGED));
        trace.add_event(BusEvent(300, BusEventFlags::SDA_LINE_CHANGED));

        // WHEN we try to get the duration between all events
        uint32_t actual = trace.nanos_between(2, 0);

        // THEN the result is correct
        TEST_ASSERT_EQUAL_UINT32(500*clock.nanos_per_tick, actual);
    }

    static void nanos_between() {
        // GIVEN a trace with some events
        common::hal::FakeClock clock;
        BusTrace trace(&clock, MAX_EVENTS);
        trace.add_event(BusEvent(100, BusEventFlags::SDA_LINE_CHANGED));
        trace.add_event(BusEvent(200, BusEventFlags::SDA_LINE_CHANGED));
        trace.add_event(BusEvent(50, BusEventFlags::SDA_LINE_CHANGED));
        trace.add_event(BusEvent(300, BusEventFlags::SDA_LINE_CHANGED));

        // WHEN we try to get the duration between events which
        // are neither at the beginning nor the end of the trace
        uint32_t actual = trace.nanos_between(2, 1);

        // THEN the result is correct
        TEST_ASSERT_EQUAL_UINT32(50*clock.nanos_per_tick, actual);
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(max_events_required_without_pin_events);
        RUN_TEST(max_events_required_with_pin_events);

        RUN_TEST(new_trace_is_empty);
        RUN_TEST(cannot_get_event_that_has_not_been_added);
        RUN_TEST(add_event_with_bus_event_object);
        RUN_TEST(add_event_override);
        RUN_TEST(add_event_override_calculates_delta);
        RUN_TEST(add_event_gets_system_tick_directly_on_teensy4);
        RUN_TEST(add_event_is_fast_enough_on_a_teensy4);
        RUN_TEST(add_event_gets_system_tick_from_clock);
        RUN_TEST(add_event_drops_excess_events);
        RUN_TEST(reset);
        RUN_TEST(destructor_does_not_deletes_supplied_array_of_events);
        RUN_TEST(destructor_deletes_internal_array_of_events_if_it_owns_them);

        // is_identical_to
        RUN_TEST(empty_traces_are_identical);
        RUN_TEST(traces_are_identical_if_all_details_match);
        RUN_TEST(traces_are_not_identical_if_one_is_longer_than_the_other);
        RUN_TEST(traces_are_not_identical_if_deltas_are_different);
        RUN_TEST(traces_are_not_identical_if_edges_are_different);

        // compare_edges
        RUN_TEST(empty_traces_are_edge_comparable);
        RUN_TEST(traces_are_edge_comparable_if_lines_and_edges_match);
        RUN_TEST(traces_are_not_edge_comparable_if_one_is_longer_than_the_other);
        RUN_TEST(compare_edges_returns_index_of_first_difference);

        // to_message and compare_messages
        RUN_TEST(to_message_converts_empty_trace_to_empty_message);
        RUN_TEST(to_message_can_remove_last_event);
        RUN_TEST(to_message_strips_out_spurious_SDA_changes);
        RUN_TEST(to_message_does_not_strip_out_spurious_SDA_changes_when_instructed);
        RUN_TEST(to_message_splits_events_with_2_edges);
        RUN_TEST(to_message_does_not_split_events_when_instructed);
        RUN_TEST(empty_traces_are_message_comparable);
        RUN_TEST(to_message_copies_clock);

        RUN_TEST(traces_are_message_comparable_if_they_are_edge_comparable);
        RUN_TEST(traces_are_message_comparable_even_if_spurious_SDA_changes_are_different);
        RUN_TEST(compare_messages_returns_index_of_first_difference);
        RUN_TEST(message_comparable_does_not_ignore_SDA_changes_while_SCL_is_HIGH);

        // duration between events
        RUN_TEST(nanos_to_previous_when_index_is_out_of_range);
        RUN_TEST(nanos_to_previous_without_a_clock);
        RUN_TEST(nanos_to_previous_for_first_event);
        RUN_TEST(nanos_to_previous);
        RUN_TEST(nanos_between_when_from_is_out_of_range);
        RUN_TEST(nanos_between_when_to_is_out_of_range);
        RUN_TEST(nanos_between_when_from_is_greater_than_last);
        RUN_TEST(nanos_between_without_a_clock);
        RUN_TEST(nanos_between_all_events);
        RUN_TEST(nanos_between);

        RUN_TEST(print_trace);
    }

    BusTraceTest() : TestSuite(__FILE__) {};
};

// Define statics

}
#endif //I2C_UNDERNEATH_BUS_TRACE_TEST_H
