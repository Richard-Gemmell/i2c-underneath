#ifndef I2C_UNDERNEATH_TEST_EXAMPLE_TEST_SUITE_H
#define I2C_UNDERNEATH_TEST_EXAMPLE_TEST_SUITE_H

#include <unity.h>
#include <Arduino.h>
#include "e2e/e2e_test_base.h"
#include "utils/test_suite.h"
#include "common/hal/teensy/teensy_pin.h"

namespace bus_trace {

// I2C uses open drain pins and pullups. That's too slow
// for this test, so we're using OUTPUT mode.
class BusRecorderAE2ETest : public e2e::E2ETestBase {
public:
    static const size_t MAX_EVENTS = 100;
    static const size_t WAIT_FOR_FINAL_EDGE = 500;
    static BusRecorderA recorder;

    static void print_trace(const BusTrace& trace) {
        // Remove comments to see trace timings.
//        for (size_t i = 0; i < trace.event_count(); ++i) {
//            Serial.printf("Index %d: delta %d ns\n", i, common::hal::TeensyTimestamp::ticks_to_nanos(trace.event(i)->delta_t_in_ticks));
//        }
    }

    void setUp() override {
        E2ETestBase::setUp();
        recorder.set_callbacks(
                []() FASTRUN __attribute__((always_inline)) {
                    recorder.add_event(false);
                },
                []() FASTRUN __attribute__((always_inline)) {
                    recorder.add_event(true);
                }
        );
    }

    void tearDown() override {
        recorder.stop();
        recorder.set_callbacks(nullptr, nullptr);
        E2ETestBase::tearDown();
    }

    static void records_edges() {
        common::hal::TeensyPin scl(PIN_DRIVE_SCL, OUTPUT);
        common::hal::TeensyPin sda(PIN_DRIVE_SDA, OUTPUT);
        sda.set();
        scl.set();
        BusTrace trace(MAX_EVENTS);

        // WHEN both pins transition from HIGH->LOW->HIGH
        recorder.start(trace);
        delayNanoseconds(20);
        sda.toggle();
        delayNanoseconds(500);
        scl.toggle();
        delayNanoseconds(1000);
        sda.toggle();
        delayNanoseconds(800);
        scl.toggle();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
//        print_trace(trace);

        // THEN the trace contains events for all 4 edges
        TEST_ASSERT_EQUAL(5, trace.event_count());
        BusEvent expected_trace[5] = {
                BusEvent(8, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE),
                BusEvent(200 * .6, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED),
                BusEvent(520 * .6, BusEventFlags::SCL_LINE_CHANGED),
                BusEvent(1020 * .6, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED),
                BusEvent(820 * .6, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE | BusEventFlags::SCL_LINE_CHANGED),
        };
        TEST_ASSERT_EQUAL(expected_trace[0].flags, trace.event(0)->flags);
        TEST_ASSERT_EQUAL(expected_trace[1].flags, trace.event(1)->flags);
        TEST_ASSERT_EQUAL(expected_trace[2].flags, trace.event(2)->flags);
        TEST_ASSERT_EQUAL(expected_trace[3].flags, trace.event(3)->flags);
        TEST_ASSERT_EQUAL(expected_trace[4].flags, trace.event(4)->flags);

        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[0].delta_t_in_ticks, trace.event(0)->delta_t_in_ticks);
        TEST_ASSERT_UINT32_WITHIN(20, expected_trace[1].delta_t_in_ticks, trace.event(1)->delta_t_in_ticks);
        TEST_ASSERT_UINT32_WITHIN(20, expected_trace[2].delta_t_in_ticks, trace.event(2)->delta_t_in_ticks);
        TEST_ASSERT_UINT32_WITHIN(20, expected_trace[3].delta_t_in_ticks, trace.event(3)->delta_t_in_ticks);
        TEST_ASSERT_UINT32_WITHIN(20, expected_trace[4].delta_t_in_ticks, trace.event(4)->delta_t_in_ticks);
    }

    static void creates_another_recording_correctly() {
        common::hal::TeensyPin scl(PIN_DRIVE_SCL, OUTPUT);
        common::hal::TeensyPin sda(PIN_DRIVE_SDA, OUTPUT);
        sda.set();
        scl.set();

        // GIVEN we've already recorded a trace
        BusTrace trace1(MAX_EVENTS);
        recorder.start(trace1);
        sda.toggle();
        delayNanoseconds(100);
        sda.toggle();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();

        // WHEN we create a second trace
        BusTrace trace2(MAX_EVENTS);
        recorder.start(trace2);
        scl.toggle();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();

        // THEN the events and deltas are recorded correctly
        BusEvent expected_trace[2] = {
                BusEvent(8, BusEventFlags::SCL_LINE_STATE | BusEventFlags::SDA_LINE_STATE),
                BusEvent(100, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE),
        };
        TEST_ASSERT_EQUAL(2, trace2.event_count());
        TEST_ASSERT_EQUAL(expected_trace[0].flags, trace2.event(0)->flags);
        TEST_ASSERT_EQUAL(expected_trace[1].flags, trace2.event(1)->flags);

        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[0].delta_t_in_ticks, trace2.event(0)->delta_t_in_ticks);
        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[1].delta_t_in_ticks, trace2.event(1)->delta_t_in_ticks);
    }

    // Number of nanoseconds recorded from event 'start_index' to the end
    // of the trace.
    static uint32_t nanos_til_end(BusTrace& trace, size_t start_index) {
        uint32_t total = 0;
        for (size_t i = start_index; i < trace.event_count(); ++i) {
            uint32_t nanos = common::hal::TeensyTimestamp::ticks_to_nanos(trace.event(i)->delta_t_in_ticks);
            total += nanos;
        }
        return total;
    }

    static void toggle_pin_repeatedly(int repeats, common::hal::TeensyPin& pin) {
        for (int i = 0; i < repeats; ++i) {
            pin.toggle();
            delayNanoseconds(10);
            pin.toggle();
            delayNanoseconds(10);
        }
    }

    // Estimates the time taken to record an edge.
    // Most of this time is taken detecting the edge and firing the interrupt.
    static void test_interrupt_duration_single_line() {
        common::hal::TeensyPin scl(PIN_DRIVE_SCL, OUTPUT);
        BusTrace trace(MAX_EVENTS);
        scl.set();

        // WHEN we record the events
        recorder.start(trace);
        int repeats = 10;
        toggle_pin_repeatedly(repeats, scl);
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
        print_trace(trace);

        // THEN we recorded one event per edge
        uint32_t expected_num_events = 1 + (repeats * 2);
        TEST_ASSERT_EQUAL(expected_num_events, trace.event_count());

        // AND we can estimate the time to handle the interrupt
        size_t start_index = 3; // The timings are a little erratic for the first few cycles.
        uint32_t duration_with_interrupts = nanos_til_end(trace, start_index);
        uint32_t num_interrupts = (trace.event_count() - start_index);
        double nanos_per_call = ((double)duration_with_interrupts) / num_interrupts;
//        Serial.printf("%.0f nanos per interrupt for single pin\n", nanos_per_call);
        const uint32_t expected = 267; // Confirmed with scope
        TEST_ASSERT_UINT32_WITHIN(20, expected, nanos_per_call);
    }

    static void toggle_both_pins_repeatedly(int repeats, common::hal::TeensyPin& sda, common::hal::TeensyPin& scl) {
        for (int i = 0; i < repeats; ++i) {
            scl.toggle();
            sda.toggle();
            delayNanoseconds(10);
            scl.toggle();
            sda.toggle();
            delayNanoseconds(10);
        }
    }

    // Estimates the time taken to record an edge.
    // Most of this time is taken detecting the edge and firing the interrupt.
    static void test_interrupt_duration_both_lines() {
        common::hal::TeensyPin sda(PIN_DRIVE_SDA, OUTPUT);
        common::hal::TeensyPin scl(PIN_DRIVE_SCL, OUTPUT);
        BusTrace trace(MAX_EVENTS);
        sda.set();
        scl.set();

        // WHEN we record the events
        recorder.start(trace);
        int repeats = 10;
        toggle_both_pins_repeatedly(repeats, sda, scl);
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
        print_trace(trace);

        // THEN we recorded one event per edge
        uint32_t expected_num_events = 1 + (2 * (repeats * 2));
        TEST_ASSERT_EQUAL(expected_num_events, trace.event_count());

        // AND we can estimate the time to handle the interrupt
        size_t start_index = 5; // The timings are a little erratic for the first few cycles.
        uint32_t duration_with_interrupts = nanos_til_end(trace, start_index);
        uint32_t num_interrupts = (trace.event_count() - start_index) / 2;
        double nanos_per_call = ((double)duration_with_interrupts) / num_interrupts;
//        Serial.printf("%.0f nanos per interrupt for both pins\n", nanos_per_call);
        const uint32_t expected = 361; // Confirmed with scope
        TEST_ASSERT_UINT32_WITHIN(20, expected, nanos_per_call);
    }

    // If both lines are changed in quick succession
    // then the BusRecorder gets the order of SDA and SCL wrong.
    // This is because they end up getting handled by the same
    // interrupt service routine call and the pins are handled
    // in a fixed order, not the order the events occurred.
    // Note: I wasn't able to make edges disappear entirely with this trick.
    static void test_reorder_events() {
        common::hal::TeensyPin scl(PIN_DRIVE_SCL, OUTPUT);
        common::hal::TeensyPin sda(PIN_DRIVE_SDA, OUTPUT);
        scl.set();
        sda.set();
        BusTrace trace(MAX_EVENTS);

        int interval = 0;  // Edges are <10 ns apart. BusRecorder flips the order of the pins.
//        int interval = 1;  // Edges are further apart. BusRecorder gets pin order correct.

        // WHEN we record 2 edges in quick succession
        recorder.start(trace);
        sda.clear();
        delayNanoseconds(interval);
        scl.clear();
        delayNanoseconds(100);
        recorder.stop();

        // THEN the BusRecorder gets the order of SDA and SCL wrong.
//        trace.printTo(Serial);
        TEST_ASSERT_EQUAL_UINT32(3, trace.event_count());
        TEST_ASSERT_EQUAL(BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE,
                          trace.event(0)->flags);
        TEST_ASSERT_NOT_EQUAL(BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE,
                              trace.event(1)->flags);
    }

    static void test_lost_events() {
        common::hal::TeensyPin scl(PIN_DRIVE_SCL, OUTPUT);
        scl.set();
        BusTrace trace(MAX_EVENTS);

        // WHEN we record a lot of edges in quick succession
        int interval = 0;
        recorder.start(trace);
        scl.toggle();
        delayNanoseconds(interval);
        scl.toggle();
        delayNanoseconds(interval);
        scl.toggle();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();

        // THEN the BusRecorder loses the last 2 edges
        TEST_ASSERT_EQUAL_UINT32(2, trace.event_count());
    }

    static void test_does_not_lose_sync() {
        common::hal::TeensyPin scl(PIN_DRIVE_SCL, OUTPUT);
        scl.set();
        BusTrace trace(MAX_EVENTS);

        // GIVEN the recorder lost edges because they happened too quickly
        recorder.start(trace);
        scl.clear();
        delayNanoseconds(1);
        scl.set();
        delayNanoseconds(0);
        scl.clear();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();

        // THEN the number of edges is incorrect
        TEST_ASSERT_EQUAL_UINT32(3, trace.event_count());
        // AND the final line state is LOW
        TEST_ASSERT_EQUAL(BusEventFlags::SCL_LINE_CHANGED, trace.event(2)->flags);
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(records_edges);
        RUN_TEST(creates_another_recording_correctly);
        RUN_TEST(test_interrupt_duration_single_line);
        RUN_TEST(test_interrupt_duration_both_lines);
        RUN_TEST(test_reorder_events);
        RUN_TEST(test_lost_events);
        RUN_TEST(test_does_not_lose_sync);
    }

    BusRecorderAE2ETest() : E2ETestBase(__FILE__) {};
};

// Define statics
BusRecorderA BusRecorderAE2ETest::recorder(PIN_SNIFF_SDA, PIN_SNIFF_SCL);

}
#endif  //I2C_UNDERNEATH_TEST_EXAMPLE_TEST_SUITE_H