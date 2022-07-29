// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_TRACE_BUILDER_TEST_H
#define I2C_UNDERNEATH_BUS_TRACE_BUILDER_TEST_H
#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include "bus_trace/bus_trace_builder.h"

namespace bus_trace {

class BusTraceBuilderTest : public TestSuite {
    static const size_t MAX_EVENTS = 256;
    static BusEvent events[MAX_EVENTS];

public:
    static void bus_initially_idle() {
        // GIVEN a builder
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Max, common::i2c_specification::StandardMode);

        // WHEN we say the bus is idle
        builder.bus_initially_idle();

        // THEN the trace contains the correct events
        const int numExpected = 1;
        BusEvent expected_trace[numExpected] = {
            BusEvent(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE),
        };
        TEST_ASSERT_EQUAL(numExpected, trace.event_count());
        TEST_ASSERT_TRUE(expected_trace[0] == *trace.event(0))
    }

    static void start_bit() {
        // GIVEN a builder
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);

        // WHEN we request a start bit
        builder.start_bit();

        // THEN the trace contains the correct events
        const int numExpected = 2;
        BusEvent expected_trace[numExpected] = {
            BusEvent(0, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE),
            BusEvent(4'000, BusEventFlags::SCL_LINE_CHANGED),
        };
        TEST_ASSERT_EQUAL(numExpected, trace.event_count());
        TEST_ASSERT_TRUE(expected_trace[0] == *trace.event(0))
        TEST_ASSERT_TRUE(expected_trace[1] == *trace.event(1))
    }

    static void ack_after_zero() {
        // GIVEN our last bit was a zero
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        trace.add_event(BusEvent(0, BusEventFlags::SCL_LINE_CHANGED));
        size_t events_so_far = trace.event_count();

        // WHEN we add an ack
        builder.ack();

        // THEN the trace contains the correct events
        BusEvent expected_trace[2] = {
            BusEvent(4'700, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE),
            BusEvent(4'000, BusEventFlags::SCL_LINE_CHANGED),
        };
        TEST_ASSERT_EQUAL(events_so_far+2, trace.event_count());
        TEST_ASSERT_TRUE(expected_trace[0] == *trace.event(events_so_far + 0))
        TEST_ASSERT_TRUE(expected_trace[1] == *trace.event(events_so_far + 1))
    }

    static void ack_after_one() {
        // GIVEN our last bit was a one
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        trace.add_event(BusEvent(0, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE));
        size_t events_so_far = trace.event_count();

        // WHEN we add an ack
        builder.ack();

        // THEN the trace contains the correct events
        BusEvent expected_trace[3] = {
            BusEvent(300, BusEventFlags::SDA_LINE_CHANGED),
            BusEvent(4'400, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE),
            BusEvent(4'000, BusEventFlags::SCL_LINE_CHANGED),
        };
        TEST_ASSERT_EQUAL(events_so_far+3, trace.event_count());
        TEST_ASSERT_TRUE(expected_trace[0] == *trace.event(events_so_far + 0))
        TEST_ASSERT_TRUE(expected_trace[1] == *trace.event(events_so_far + 1))
        TEST_ASSERT_TRUE(expected_trace[2] == *trace.event(events_so_far + 2))
    }

    static void nack_after_zero() {
        // GIVEN our last bit was a zero
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        trace.add_event(BusEvent(0, BusEventFlags::SCL_LINE_CHANGED));
        size_t events_so_far = trace.event_count();

        // WHEN we add a nack
        builder.nack();

        // THEN the trace contains the correct events
        BusEvent expected_trace[3] = {
                BusEvent(300, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE),
                BusEvent(4'400, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE),
                BusEvent(4'000, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE),
        };
        TEST_ASSERT_EQUAL(events_so_far+3, trace.event_count());
        TEST_ASSERT_TRUE(expected_trace[0] == *trace.event(events_so_far + 0))
        TEST_ASSERT_TRUE(expected_trace[1] == *trace.event(events_so_far + 1))
        TEST_ASSERT_TRUE(expected_trace[2] == *trace.event(events_so_far + 2))
    }

    static void nack_after_one() {
        // GIVEN our last bit was a one
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        trace.add_event(BusEvent(0, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE));
        size_t events_so_far = trace.event_count();

        // WHEN we add a nack
        builder.nack();

        // THEN the trace contains the correct events
        BusEvent expected_trace[2] = {
            BusEvent(4'700, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE),
            BusEvent(4'000, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE),
        };
        TEST_ASSERT_EQUAL(events_so_far+2, trace.event_count());
        TEST_ASSERT_TRUE(expected_trace[0] == *trace.event(events_so_far + 0))
        TEST_ASSERT_TRUE(expected_trace[1] == *trace.event(events_so_far + 1))
    }

    static void stop_bit_after_ack() {
        // GIVEN the last thing was an ACK
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        trace.add_event(BusEvent(0, BusEventFlags::SCL_LINE_CHANGED));
        size_t events_so_far = trace.event_count();

        // WHEN we extend the trace
        builder.stop_bit();

        // THEN the trace contains the correct events
        const int numExpected = 2;
        BusEvent expected_trace[numExpected] = {
            BusEvent(4'700, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE),
            BusEvent(4'000, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE),
        };
        TEST_ASSERT_EQUAL(events_so_far+2, trace.event_count());
        TEST_ASSERT_TRUE(expected_trace[0] == *trace.event(events_so_far + 0))
        TEST_ASSERT_TRUE(expected_trace[1] == *trace.event(events_so_far + 1))
    }

    static void stop_bit_after_nack() {
        // GIVEN the last thing was a NACK
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        trace.add_event(BusEvent(0, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE));
        size_t events_so_far = trace.event_count();

        // WHEN we extend the trace
        builder.stop_bit();

        // THEN the trace contains the correct events
        BusEvent expected_trace[3] = {
            BusEvent(300, BusEventFlags::SDA_LINE_CHANGED),
            BusEvent(4'400, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE),
            BusEvent(4'000, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE),
        };
        TEST_ASSERT_EQUAL(events_so_far+3, trace.event_count());
        TEST_ASSERT_TRUE(expected_trace[0] == *trace.event(events_so_far + 0))
        TEST_ASSERT_TRUE(expected_trace[1] == *trace.event(events_so_far + 1))
        TEST_ASSERT_TRUE(expected_trace[2] == *trace.event(events_so_far + 2))
    }

    static void data_bits() {
        // GIVEN a builder
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder.bus_initially_idle();
        builder.start_bit();
        size_t events_so_far = trace.event_count();

        // Test all 4 bit transitions 0->0, 0->1, 1->1, 1->0
        // WHEN we send a 0 bit when SDA is already LOW
        builder.data_bit(false);

        // THEN a 0->0 bit transition is defined correctly
        BusEvent bit_0_expected_trace[2] = {
                BusEvent(4'700, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE),
                BusEvent(4'000, BusEventFlags::SCL_LINE_CHANGED),
        };
        TEST_ASSERT_EQUAL(events_so_far+2, trace.event_count());
        TEST_ASSERT_TRUE(bit_0_expected_trace[0] == *trace.event(events_so_far+0))
        TEST_ASSERT_TRUE(bit_0_expected_trace[1] == *trace.event(events_so_far+1))
        events_so_far = trace.event_count();

        // WHEN we send a 1 bit after a 0 bit
        builder.data_bit(true);

        // THEN the trace contains the correct events
        BusEvent bit_1_expected_trace[3] = {
            BusEvent(300, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE),
            BusEvent(4'400, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE),
            BusEvent(4'000, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE),
        };
        TEST_ASSERT_EQUAL(events_so_far+3, trace.event_count());
        TEST_ASSERT_TRUE(bit_1_expected_trace[0] == *trace.event(events_so_far+0))
        TEST_ASSERT_TRUE(bit_1_expected_trace[1] == *trace.event(events_so_far+1))
        TEST_ASSERT_TRUE(bit_1_expected_trace[2] == *trace.event(events_so_far+2))
        events_so_far = trace.event_count();

        // WHEN we send a 1 bit after a 1 bit
        builder.data_bit(true);

        // THEN the trace contains the correct events
        BusEvent bit_2_expected_trace[2] = {
            BusEvent(4'700, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE),
            BusEvent(4'000, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE),
        };
        TEST_ASSERT_EQUAL(events_so_far+2, trace.event_count());
        TEST_ASSERT_TRUE(bit_2_expected_trace[0] == *trace.event(events_so_far+0))
        TEST_ASSERT_TRUE(bit_2_expected_trace[1] == *trace.event(events_so_far+1))
        events_so_far = trace.event_count();

        // WHEN we send a 0 bit after a 1 bit
        builder.data_bit(false);

        // THEN the trace contains the correct events
        BusEvent bit_3_expected_trace[3] = {
                BusEvent(300, BusEventFlags::SDA_LINE_CHANGED),
                BusEvent(4'400, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE),
                BusEvent(4'000, BusEventFlags::SCL_LINE_CHANGED),
        };
        TEST_ASSERT_EQUAL(events_so_far+3, trace.event_count());
        TEST_ASSERT_TRUE(bit_3_expected_trace[0] == *trace.event(events_so_far+0))
        TEST_ASSERT_TRUE(bit_3_expected_trace[1] == *trace.event(events_so_far+1))
        TEST_ASSERT_TRUE(bit_3_expected_trace[2] == *trace.event(events_so_far+2))
    }

    static void data_byte() {
        // GIVEN a builder with a start bit
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder.bus_initially_idle();
        builder.start_bit();

        // WHEN we set a data byte
        builder.data_byte(0x53);

        // THEN we get the expected trace
        BusEvent expected_events[MAX_EVENTS];
        BusTrace expected_trace(expected_events, MAX_EVENTS);
        BusTraceBuilder builder2(expected_trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder2.bus_initially_idle()
            .start_bit()
            // value 0x53 == 0101 0011
            .data_bit(false)
            .data_bit(true)
            .data_bit(false)
            .data_bit(true)
            .data_bit(false)
            .data_bit(false)
            .data_bit(true)
            .data_bit(true);
        TEST_ASSERT_EQUAL_UINT32(expected_trace.event_count(), trace.event_count());
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, expected_trace.compare_to(events, trace.event_count()));
    }

    static void address_byte_read() {
        // GIVEN a builder with a start bit
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder.bus_initially_idle();
        builder.start_bit();

        // WHEN we set the address byte
        builder.address_byte(0x53, true);

        // THEN we get the expected trace
        BusEvent expected_events[MAX_EVENTS];
        BusTrace expected_trace(expected_events, MAX_EVENTS);
        BusTraceBuilder builder2(expected_trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder2.bus_initially_idle()
            .start_bit()
            // 0x53 left shifted + 1
            .data_byte(0xA7);
        TEST_ASSERT_EQUAL_UINT32(expected_trace.event_count(), trace.event_count());
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, expected_trace.compare_to(events, trace.event_count()));
    }

    static void address_byte_write() {
        // GIVEN a builder with a start bit
        BusTrace trace(events, MAX_EVENTS);
        BusTraceBuilder builder(trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder.bus_initially_idle();
        builder.start_bit();

        // WHEN we set the address byte
        builder.address_byte(0x53, false);

        // THEN we get the expected trace
        BusEvent expected_events[MAX_EVENTS];
        BusTrace expected_trace(expected_events, MAX_EVENTS);
        BusTraceBuilder builder2(expected_trace, BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder2.bus_initially_idle();
        builder2.start_bit();
        // 0x53 left shifted + 0
        builder2.data_byte(0xA6);
        TEST_ASSERT_EQUAL_UINT32(expected_trace.event_count(), trace.event_count());
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, expected_trace.compare_to(events, trace.event_count()));
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(bus_initially_idle);
        RUN_TEST(start_bit);
        RUN_TEST(ack_after_zero);
        RUN_TEST(ack_after_one);
        RUN_TEST(nack_after_zero);
        RUN_TEST(nack_after_one);
        RUN_TEST(stop_bit_after_ack);
        RUN_TEST(stop_bit_after_nack);
        RUN_TEST(data_bits);
        RUN_TEST(data_byte);
        RUN_TEST(address_byte_read);
        RUN_TEST(address_byte_write);
    }

    BusTraceBuilderTest() : TestSuite(__FILE__) {};
};

// Define statics
BusEvent bus_trace::BusTraceBuilderTest::events[MAX_EVENTS];

}
#endif //I2C_UNDERNEATH_BUS_TRACE_BUILDER_TEST_H
