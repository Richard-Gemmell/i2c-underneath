// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_RECORDER_TEST_H
#define I2C_UNDERNEATH_BUS_RECORDER_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include "bus_trace/bus_recorder.h"
#include "fakes/common/hal/fake_clock.h"

namespace bus_trace {

class BusRecorderTest : public TestSuite {
    static const size_t MAX_EVENTS = 10;
    static common::hal::FakeClock clock;
    static bus_trace::BusEvent events[MAX_EVENTS];
    static common::hal::FakePin* sda;
    static common::hal::FakePin* scl;

public:
    void setUp() final {
        sda = new common::hal::FakePin();
        scl = new common::hal::FakePin();
    }

    void tearDown() final {
        delete sda;
        sda = nullptr;
        delete scl;
        scl = nullptr;
    }

    static void fake_pins_start_high() {
        TEST_ASSERT_TRUE(sda->read_line())
        TEST_ASSERT_TRUE(scl->read_line())
    }

    static void is_recording() {
        // WHEN we are recording
        BusRecorder recorder(*sda, *scl, clock);
        BusTrace trace(events, MAX_EVENTS);
        bool before_start = recorder.is_recording();
        recorder.start(trace);

        // THEN is_recording is set
        bool after_start = recorder.is_recording();
        recorder.stop();
        bool after_stop = recorder.is_recording();

        TEST_ASSERT_FALSE(before_start)
        TEST_ASSERT_TRUE(after_start)
        TEST_ASSERT_FALSE(after_stop)
    }

    static void records_initial_line_states() {
        BusRecorder recorder(*sda, *scl, clock);
        BusTrace trace(events, MAX_EVENTS);

        // WHEN there are no line edges during the recording
        recorder.start(trace);
        recorder.stop();

        // THEN the trace contains just the initial line states
        TEST_ASSERT_EQUAL(1, trace.event_count());
        auto both_lines_high = BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_STATE;
        TEST_ASSERT_EQUAL(both_lines_high, trace.event(0)->flags);
        TEST_ASSERT_EQUAL(0, trace.event(0)->delta_t_in_ticks);
    }

    static void ignores_edges_when_not_recording() {
        BusRecorder recorder(*sda, *scl, clock);
        BusTrace trace(events, MAX_EVENTS);

        // GIVEN edges before the recording
        clock.system_tick += 1;
        sda->write_pin(LOW);
        clock.system_tick += 1;
        scl->write_pin(LOW);

        // WHEN we make a recording
        recorder.start(trace);
        recorder.stop();

        // AND there are edges after we've stopped recording
        clock.system_tick += 1;
        sda->write_pin(HIGH);
        clock.system_tick += 1;
        scl->write_pin(HIGH);

        // THEN the trace contains just the initial line states
        TEST_ASSERT_EQUAL(1, trace.event_count());
    }

    static void records_edges() {
        BusRecorder recorder(*sda, *scl, clock);
        BusTrace trace(events, MAX_EVENTS);

        // WHEN both pins transition from HIGH->LOW->HIGH
        recorder.start(trace);
        clock.system_tick += 10;
        sda->write_pin(LOW);
        clock.system_tick += 11;
        scl->write_pin(LOW);
        clock.system_tick += 20;
        sda->write_pin(HIGH);
        clock.system_tick += 22;
        scl->write_pin(HIGH);
        recorder.stop();

        // THEN the trace contains events for all 4 edges
        TEST_ASSERT_EQUAL(5, trace.event_count());
        BusEvent expected_trace[5] = {
                BusEvent(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE),
                BusEvent(10, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED),
                BusEvent(11, BusEventFlags::SCL_LINE_CHANGED),
                BusEvent(20, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED),
                BusEvent(22, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE | BusEventFlags::SCL_LINE_CHANGED),
        };
        size_t equivalent = trace.compare_to(expected_trace, 5);
        TEST_ASSERT_EQUAL(SIZE_MAX, equivalent);
        // AND the deltas are correct
        TEST_ASSERT_EQUAL(0, trace.event(0)->delta_t_in_ticks);
        TEST_ASSERT_EQUAL(10, trace.event(1)->delta_t_in_ticks);
        TEST_ASSERT_EQUAL(11, trace.event(2)->delta_t_in_ticks);
        TEST_ASSERT_EQUAL(20, trace.event(3)->delta_t_in_ticks);
        TEST_ASSERT_EQUAL(22, trace.event(4)->delta_t_in_ticks);
    }

    static void delta_is_calculated_correctly_when_tick_count_wraps() {
        // GIVEN we've recorded an event just before the system tick count hits its maximum value
        BusRecorder recorder(*sda, *scl, clock);
        BusTrace trace(events, MAX_EVENTS);
        clock.system_tick = UINT32_MAX - 100;   // A very large tick count
        recorder.start(trace);

        // WHEN we record another event after the clock wraps
        clock.system_tick = 150;    // A very small tick count
        sda->write_pin(LOW);
        recorder.stop();

        // THEN the delta is calculated correctly
        TEST_ASSERT_EQUAL(2, trace.event_count());
        auto actualEvent = trace.event(1);
        TEST_ASSERT_NOT_NULL(actualEvent)
        TEST_ASSERT_EQUAL_UINT32(251, actualEvent->delta_t_in_ticks);
    }

    static void creates_another_recording_correctly() {
        // GIVEN we've already recorded a trace
        BusRecorder recorder(*sda, *scl, clock);
        BusTrace trace1(events, MAX_EVENTS);
        recorder.start(trace1);
        sda->write_pin(LOW);
        recorder.stop();
        clock.system_tick += 12345;

        // WHEN we create a second trace
        BusTrace trace2(events, MAX_EVENTS);
        recorder.start(trace2);
        clock.system_tick += 9;
        sda->write_pin(HIGH);
        recorder.stop();

        // THEN the events and deltas are recorded correctly
        TEST_ASSERT_EQUAL(2, trace2.event_count());
        BusEvent expected_trace[5] = {
                BusEvent(0, BusEventFlags::SCL_LINE_STATE),
                BusEvent(9, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED),
        };
        size_t equivalent = trace2.compare_to(expected_trace, 2);
        TEST_ASSERT_EQUAL(SIZE_MAX, equivalent);
        // AND the deltas are correct
        TEST_ASSERT_EQUAL(0, trace2.event(0)->delta_t_in_ticks);
        TEST_ASSERT_EQUAL(9, trace2.event(1)->delta_t_in_ticks);
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(fake_pins_start_high);
        RUN_TEST(is_recording);
        RUN_TEST(records_initial_line_states);
        RUN_TEST(ignores_edges_when_not_recording);
        RUN_TEST(records_edges);
        RUN_TEST(delta_is_calculated_correctly_when_tick_count_wraps);
        RUN_TEST(creates_another_recording_correctly);
    }

    BusRecorderTest() : TestSuite(__FILE__) {};
};

// Define statics
bus_trace::BusEvent BusRecorderTest::events[MAX_EVENTS];
common::hal::FakeClock BusRecorderTest::clock = common::hal::FakeClock();
common::hal::FakePin* BusRecorderTest::sda;
common::hal::FakePin* BusRecorderTest::scl;

}
#endif //I2C_UNDERNEATH_BUS_RECORDER_TEST_H
