// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_RECORDER_TEST_H
#define I2C_UNDERNEATH_BUS_RECORDER_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include "bus_trace/bus_recorder.h"
#include "common/hal/teensy/teensy_clock.h"

namespace bus_trace {

class BusRecorderTest : public TestSuite {
    static const size_t MAX_EVENTS = 10;
    static bus_trace::BusEvent events[MAX_EVENTS];
    const static uint32_t PIN_SNIFF_SDA = 2;
    const static uint32_t PIN_SNIFF_SCL = 3;
    static BusRecorder recorder;
    static void (*sda_trigger)();
    static void (*scl_trigger)();

public:
    void setUp() final {
        pinMode(PIN_SNIFF_SDA, INPUT_PULLUP);
        digitalWriteFast(PIN_SNIFF_SDA, HIGH);
        pinMode(PIN_SNIFF_SCL, INPUT_PULLUP);
        digitalWriteFast(PIN_SNIFF_SCL, HIGH);
        delayMicroseconds(10);
        recorder.set_callbacks(sda_trigger, scl_trigger);
    }

    void tearDown() final {
        recorder.set_callbacks(nullptr, nullptr);
        pinMode(PIN_SNIFF_SDA, INPUT_DISABLE);
        pinMode(PIN_SNIFF_SCL, INPUT_DISABLE);
        pinMode(SDA, INPUT_DISABLE);
        pinMode(SCL, INPUT_DISABLE);
    }

    static void pins_start_high() {
        TEST_ASSERT_TRUE(digitalReadFast(PIN_SNIFF_SDA));
        TEST_ASSERT_TRUE(digitalReadFast(PIN_SNIFF_SCL));
    }

    static void cannot_record_unless_callbacks_have_been_set() {
        // GIVEN the callbacks are undefined
        recorder.set_callbacks(nullptr, nullptr);

        // WHEN we attempt to record without calling set_callbacks
        BusTrace trace(events, MAX_EVENTS);
        recorder.start(trace);

        // THEN we do not start recording.
        TEST_ASSERT_EQUAL(0, trace.event_count());
    }

    static void is_recording() {
        // WHEN we are recording
        BusTrace trace(events, MAX_EVENTS);
        bool before_start = recorder.is_recording();
        recorder.start(trace);

        // THEN is_recording is set
        bool after_start = recorder.is_recording();
        recorder.stop();
        bool after_stop = recorder.is_recording();

        TEST_ASSERT_FALSE(before_start);
        TEST_ASSERT_TRUE(after_start);
        TEST_ASSERT_FALSE(after_stop);
    }

    static void records_initial_line_states() {
        BusTrace trace(events, MAX_EVENTS);

        // WHEN there are no line edges during the recording
        recorder.start(trace);
        recorder.stop();

        // THEN the trace contains just the initial line states
        TEST_ASSERT_EQUAL(1, trace.event_count());
        auto both_lines_high = BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_STATE;
        TEST_ASSERT_EQUAL(both_lines_high, trace.event(0)->flags);
        TEST_ASSERT_UINT32_WITHIN(4, 8, trace.event(0)->delta_t_in_ticks);
    }

    static void ignores_edges_when_not_recording() {
        BusTrace trace(events, MAX_EVENTS);

        // GIVEN edges before the recording
        sda_trigger();
        scl_trigger();

        // WHEN we make a recording
        recorder.start(trace);
        recorder.stop();

        // AND there are edges after we've stopped recording
        sda_trigger();
        scl_trigger();

        // THEN the trace contains just the initial line states
        TEST_ASSERT_EQUAL(1, trace.event_count());
    }

    static void records_edges() {
        BusTrace trace(MAX_EVENTS);

        // WHEN both pins transition from HIGH->LOW->HIGH
        recorder.start(trace);
        delayNanoseconds(20);
        sda_trigger();
        delayNanoseconds(50);
        scl_trigger();
        delayNanoseconds(100);
        sda_trigger();
        delayNanoseconds(200);
        scl_trigger();
        recorder.stop();

        // THEN the trace contains events for all 4 edges
        TEST_ASSERT_EQUAL(5, trace.event_count());
        BusEvent expected_trace[5] = {
                BusEvent(8, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE),
                BusEvent(58, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED),
                BusEvent(82, BusEventFlags::SCL_LINE_CHANGED),
                BusEvent(88, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED),
                BusEvent(166, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE | BusEventFlags::SCL_LINE_CHANGED),
        };
        TEST_ASSERT_EQUAL(expected_trace[0].flags, trace.event(0)->flags);
        TEST_ASSERT_EQUAL(expected_trace[1].flags, trace.event(1)->flags);
        TEST_ASSERT_EQUAL(expected_trace[2].flags, trace.event(2)->flags);
        TEST_ASSERT_EQUAL(expected_trace[3].flags, trace.event(3)->flags);
        TEST_ASSERT_EQUAL(expected_trace[4].flags, trace.event(4)->flags);

        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[0].delta_t_in_ticks, trace.event(0)->delta_t_in_ticks);
        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[1].delta_t_in_ticks, trace.event(1)->delta_t_in_ticks);
        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[2].delta_t_in_ticks, trace.event(2)->delta_t_in_ticks);
        TEST_ASSERT_UINT32_WITHIN(110, expected_trace[3].delta_t_in_ticks, trace.event(3)->delta_t_in_ticks);
        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[4].delta_t_in_ticks, trace.event(4)->delta_t_in_ticks);
    }

//    static void delta_is_calculated_correctly_when_tick_count_wraps() {
//        // GIVEN we've recorded an event just before the system tick count hits its maximum value
//        BusRecorder recorder(PIN_SNIFF_SDA, PIN_SNIFF_SCL);
//        BusTrace trace(events, MAX_EVENTS);
//        clock.system_tick = UINT32_MAX - 100;   // A very large tick count
//        recorder.start(trace);
//
//        // WHEN we record another event after the clock wraps
//        clock.system_tick = 150;    // A very small tick count
//        sda->write_pin(LOW);
//        recorder.stop();
//
//        // THEN the delta is calculated correctly
//        TEST_ASSERT_EQUAL(2, trace.event_count());
//        auto actualEvent = trace.event(1);
//        TEST_ASSERT_NOT_NULL(actualEvent)
//        TEST_ASSERT_EQUAL_UINT32(502, actualEvent->delta_t_in_ticks);
//    }

    static void creates_another_recording_correctly() {
        // GIVEN we've already recorded a trace
        BusTrace trace1(events, MAX_EVENTS);
        recorder.start(trace1);
        sda_trigger();
        recorder.stop();
        scl_trigger();

        // WHEN we create a second trace
        BusTrace trace2(events, MAX_EVENTS);
        recorder.start(trace2);
        scl_trigger();
        recorder.stop();

        // THEN the events and deltas are recorded correctly
        BusEvent expected_trace[2] = {
                BusEvent(8, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_STATE),
                BusEvent(34, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE),
        };
        TEST_ASSERT_EQUAL(2, trace2.event_count());
        TEST_ASSERT_EQUAL(expected_trace[0].flags, trace2.event(0)->flags);
        TEST_ASSERT_EQUAL(expected_trace[1].flags, trace2.event(1)->flags);

        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[0].delta_t_in_ticks, trace2.event(0)->delta_t_in_ticks);
        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[1].delta_t_in_ticks, trace2.event(1)->delta_t_in_ticks);
    }

    static void interrupt_is_fast_enough() {
        // Local callbacks get inlined
        auto sda_trigger_local = [](){
            recorder.add_event(false);
        };
        auto scl_trigger_local = [](){
            recorder.add_event(true);
        };
        recorder.set_callbacks(sda_trigger_local, scl_trigger_local);
        common::hal::TeensyClock real_clock;

        // This test measures the time to record an event
        // It's critically important that the event handlers don't get any slower
        // as this can stop the BusRecorder working with 1 MHz buses.
        BusTrace trace(events, MAX_EVENTS);

        // WHEN we toggle both pins
        recorder.start(trace);
        uint32_t start = real_clock.get_system_tick();
        sda_trigger_local();
        scl_trigger_local();
        sda_trigger_local();
        scl_trigger_local();
        uint32_t stop = real_clock.get_system_tick();
        recorder.stop();

        // THEN the time to record all 4 transitions is reasonable
        uint32_t average_duration = real_clock.nanos_between(start, stop) / (trace.event_count()-1);
//        Serial.printf("Average: %d\n", average_duration);
//        for (int i = 0; i < trace.event_count(); ++i) {
//            Serial.printf("Index %d: delta %d\n", i, trace.event(i)->delta_t_in_ticks);
//        }
        TEST_ASSERT_EQUAL(5, trace.event_count());
        TEST_ASSERT_LESS_OR_EQUAL_UINT32(18, average_duration);
    }

    static void deprioritises_i2c_irqs() {
        // GIVEN the I2C priorities have their default values
        const uint8_t default_priority = 128;
        int original_gpio_priority = NVIC_GET_PRIORITY(IRQ_GPIO6789);
        int original_i2c_priorities[] = {
                NVIC_GET_PRIORITY(IRQ_LPI2C1),
                NVIC_GET_PRIORITY(IRQ_LPI2C2),
                NVIC_GET_PRIORITY(IRQ_LPI2C3),
                NVIC_GET_PRIORITY(IRQ_LPI2C4)
        };
        BusTrace trace(events, MAX_EVENTS);
        TEST_ASSERT_EQUAL(default_priority, original_i2c_priorities[0]);
        TEST_ASSERT_EQUAL(default_priority, original_i2c_priorities[1]);
        TEST_ASSERT_EQUAL(default_priority, original_i2c_priorities[2]);
        TEST_ASSERT_EQUAL(default_priority, original_i2c_priorities[3]);

        // WHEN we start recording
        recorder.start(trace);
        int recording_gpio_priority = NVIC_GET_PRIORITY(IRQ_GPIO6789);
        int recording_i2c_priorities[] = {
                NVIC_GET_PRIORITY(IRQ_LPI2C1),
                NVIC_GET_PRIORITY(IRQ_LPI2C2),
                NVIC_GET_PRIORITY(IRQ_LPI2C3),
                NVIC_GET_PRIORITY(IRQ_LPI2C4)
        };
        recorder.stop();

        // THEN the I2C interrupts were de-prioritised
//        for (int i = 0; i < 4; ++i) {
//            Serial.printf("%d was %d then %d now %d\n", i, original_i2c_priorities[i], recording_i2c_priorities[i], NVIC_GET_PRIORITY(IRQ_LPI2C1+i));
//        }
        TEST_ASSERT_EQUAL(original_gpio_priority + 16, recording_i2c_priorities[0]);
        TEST_ASSERT_EQUAL(original_gpio_priority + 16, recording_i2c_priorities[1]);
        TEST_ASSERT_EQUAL(original_gpio_priority + 16, recording_i2c_priorities[2]);
        TEST_ASSERT_EQUAL(original_gpio_priority + 16, recording_i2c_priorities[3]);

        // AND the I2C priorities were returned to their initial values
        TEST_ASSERT_EQUAL(original_i2c_priorities[0], NVIC_GET_PRIORITY(IRQ_LPI2C1));
        TEST_ASSERT_EQUAL(original_i2c_priorities[1], NVIC_GET_PRIORITY(IRQ_LPI2C2));
        TEST_ASSERT_EQUAL(original_i2c_priorities[2], NVIC_GET_PRIORITY(IRQ_LPI2C3));
        TEST_ASSERT_EQUAL(original_i2c_priorities[3], NVIC_GET_PRIORITY(IRQ_LPI2C4));

        // AND the GPIO priorities never changed
        TEST_ASSERT_EQUAL(original_gpio_priority, recording_gpio_priority);
        TEST_ASSERT_EQUAL(original_gpio_priority, NVIC_GET_PRIORITY(IRQ_GPIO6789));
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(pins_start_high);
        RUN_TEST(cannot_record_unless_callbacks_have_been_set);
        RUN_TEST(is_recording);
        RUN_TEST(records_initial_line_states);
        RUN_TEST(ignores_edges_when_not_recording);
        RUN_TEST(records_edges);
//        RUN_TEST(delta_is_calculated_correctly_when_tick_count_wraps);
        RUN_TEST(creates_another_recording_correctly);
        RUN_TEST(interrupt_is_fast_enough);
        RUN_TEST(deprioritises_i2c_irqs);
    }

    BusRecorderTest() : TestSuite(__FILE__) {};
};

// Define statics
bus_trace::BusEvent BusRecorderTest::events[MAX_EVENTS];
BusRecorder BusRecorderTest::recorder(PIN_SNIFF_SDA, PIN_SNIFF_SCL); // NOLINT(cppcoreguidelines-interfaces-global-init)
void (*BusRecorderTest::sda_trigger)() = [](){
    recorder.add_event(false);
};
void (*BusRecorderTest::scl_trigger)() = [](){
    recorder.add_event(true);
};

}
#endif //I2C_UNDERNEATH_BUS_RECORDER_TEST_H
