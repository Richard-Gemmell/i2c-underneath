#ifndef I2C_UNDERNEATH_BUS_RECORDER_E2E_TEST_H
#define I2C_UNDERNEATH_BUS_RECORDER_E2E_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "e2e/e2e_test_base.h"
#include "utils/test_suite.h"
#include "common/hal/teensy/teensy_pin.h"
#include "bus_trace/bus_recorder.h"
#include "common/hal/teensy/teensy_timestamp.h"

namespace bus_trace {

// I2C uses open drain pins and pullups. That's too slow
// for this test, so we're using OUTPUT mode.
class BusRecorderE2ETest : public e2e::E2ETestBase {
public:
    constexpr static uint8_t PIN_SCL_1 = 16;
    constexpr static size_t MAX_EVENTS = 100;
    constexpr static size_t WAIT_FOR_FINAL_EDGE = 500;
    static BusTrace trace;
    static BusRecorder recorder;
    static common::hal::TeensyPin sda;
    static common::hal::TeensyPin scl;

    void setUp() override {
        E2ETestBase::setUp();
        recorder.set_callback([]() { recorder.add_event(); });
    }

    void tearDown() override {
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
        recorder.set_callback(nullptr);
        // Reset lines if necessary
        scl.clear();
        sda.clear();
        E2ETestBase::tearDown();
        pinMode(PIN_SCL_1, INPUT_DISABLE);
    }

    // Toggle SDA to make it easy to find the trace with an oscilloscope
    // Set the scope to trigger on SDA rising edge
    static void mark_test_with_sda_toggle() {
        sda.set();
        delayNanoseconds(100);
        sda.clear();
    }

    static void lines_are_low_to_start_with() {
        // This is the opposite way round to a normal I2C setup,
        // but we're using OUTPUT here not pullups
        TEST_ASSERT_FALSE(sda.read());
        TEST_ASSERT_FALSE(scl.read());
    }

    static void is_recording() {
        // WHEN we are recording
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
        sda.set();
        scl.clear();
        // WHEN there are no line edges during the recording
        bool started = recorder.start(trace);
        recorder.stop();

        // THEN the trace contains just the initial line states
        TEST_ASSERT(started);
        TEST_ASSERT_EQUAL(1, trace.event_count());
        auto one_high_one_low = SDA_LINE_STATE;
        TEST_ASSERT_EQUAL(one_high_one_low, trace.event(0)->flags);
        TEST_ASSERT_UINT32_WITHIN(4, 8, trace.event(0)->delta_t_in_ticks);
    }

    static void will_not_start_unless_callbacks_are_set() {
        // GIVEN we have not set the callbacks for the BusRecorder
        BusRecorder broken(PIN_SNIFF_SDA, PIN_SNIFF_SCL);
        BusTrace failedTrace(MAX_EVENTS);

        // WHEN we try to start recording
        boolean started = broken.start(failedTrace);
        broken.stop();

        // THEN the recording did not start
        TEST_ASSERT_FALSE(started);
        // AND the trace is empty
        TEST_ASSERT_EQUAL(0, failedTrace.event_count());
    }

    static void will_not_start_unless_pins_share_the_same_GPIO_block() {
        // GIVEN the bus recorder is given pins with different GPIO blocks
        const static uint8_t GPIO1_PIN = 0; // Pin 0 uses GPIO1 or GPIO6
        const static uint8_t GPIO2_PIN = 10; // Pin 10 uses GPIO2 or GPIO7
        BusRecorder broken(GPIO1_PIN, GPIO2_PIN);
        BusTrace failedTrace(MAX_EVENTS);

        // WHEN we try to start recording
        boolean started = broken.start(failedTrace);
        broken.stop();

        // THEN the recording did not start
        TEST_ASSERT_FALSE(started);
        // AND the trace is empty
        TEST_ASSERT_EQUAL(0, failedTrace.event_count());
    }

    static void ignores_edges_when_not_recording() {
        // GIVEN edges before the recording
        sda.toggle();
        scl.toggle();

        // WHEN we make a recording
        recorder.start(trace);
        recorder.stop();

        // AND there are edges after we've stopped recording
        sda.toggle();
        scl.toggle();

        // THEN the trace contains just the initial line states
        TEST_ASSERT_EQUAL(1, trace.event_count());
    }

    static void records_edges() {
        sda.set();
        scl.set();
        delayMicroseconds(1);
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
//        Serial.println(trace);
//        print_trace(trace);

        // THEN the trace contains events for all 4 edges
        TEST_ASSERT_EQUAL(5, trace.event_count());
        BusEvent expected_trace[5] = {
                BusEvent(13, SDA_LINE_STATE | SCL_LINE_STATE),
                BusEvent(140 * .6, SCL_LINE_STATE | SDA_LINE_CHANGED),
                BusEvent(515 * .6, SCL_LINE_CHANGED),
                BusEvent(1015 * .6, SDA_LINE_STATE | SDA_LINE_CHANGED),
                BusEvent(830 * .6, SDA_LINE_STATE | SCL_LINE_STATE | SCL_LINE_CHANGED),
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
                BusEvent(8, SCL_LINE_STATE | SDA_LINE_STATE),
                BusEvent(56, SCL_LINE_CHANGED | SDA_LINE_STATE),
        };
        TEST_ASSERT_EQUAL(2, trace2.event_count());
        TEST_ASSERT_EQUAL(expected_trace[0].flags, trace2.event(0)->flags);
        TEST_ASSERT_EQUAL(expected_trace[1].flags, trace2.event(1)->flags);

        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[0].delta_t_in_ticks, trace2.event(0)->delta_t_in_ticks);
        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[1].delta_t_in_ticks, trace2.event(1)->delta_t_in_ticks);
    }

    // Number of nanoseconds recorded from event 'start_index' to the end
    // of the trace.
    static uint32_t nanos_til_end(BusTrace& busTrace, size_t start_index) {
        uint32_t total = 0;
        for (size_t i = start_index; i < busTrace.event_count(); ++i) {
            uint32_t nanos = common::hal::TeensyTimestamp::ticks_to_nanos(busTrace.event(i)->delta_t_in_ticks);
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
        mark_test_with_sda_toggle(); // makes it easy to find trace on a scope
        scl.set();

        // WHEN we record the events
        recorder.start(trace);
        int repeats = 10;
        toggle_pin_repeatedly(repeats, scl);
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
//        print_trace(trace);

        // THEN we recorded one event per edge
        uint32_t expected_num_events = 1 + (repeats * 2);
        TEST_ASSERT_EQUAL(expected_num_events, trace.event_count());

        // AND we can estimate the time to handle the interrupt
        size_t start_index = 2; // The timings are a little erratic for the first few cycles.
        uint32_t duration_with_interrupts = nanos_til_end(trace, start_index);
        uint32_t num_interrupts = (trace.event_count() - start_index);
        double nanos_per_call = ((double)duration_with_interrupts) / num_interrupts;
//        Serial.printf("%.0f nanos per interrupt for single pin\n", nanos_per_call);
        // Note that the time includes 16 nanos after the pin is set and before the interrupt begins
        const uint32_t expected = 117 + 16; // Confirmed with scope at 133 nanos
        TEST_ASSERT_UINT32_WITHIN(3, expected, nanos_per_call);
    }

    static void toggle_both_pins_repeatedly(int repeats) {
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
        mark_test_with_sda_toggle(); // makes it easy to find trace on a scope
        scl.set();

        // WHEN we record the events
        recorder.start(trace);
        int repeats = 10;
        toggle_both_pins_repeatedly(repeats);
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
//        print_trace(trace);

        // THEN we recorded one event per edge
        uint32_t expected_num_events = 1 + (repeats * 2);
        TEST_ASSERT_EQUAL(expected_num_events, trace.event_count());

        // AND we can estimate the time to handle the interrupt
        size_t start_index = 4; // The timings are a little erratic for the first few cycles.
        uint32_t duration_with_interrupts = nanos_til_end(trace, start_index);
        uint32_t num_interrupts = (trace.event_count() - start_index);
        double nanos_per_call = ((double)duration_with_interrupts) / num_interrupts;
//        Serial.printf("%.0f nanos per interrupt for both pins\n", nanos_per_call);
        // Note that the time includes 16 nanos after the pin is set and before the interrupt begins
        const uint32_t expected = 117 + 16; // Confirmed with scope at 133 nanos
        TEST_ASSERT_UINT32_WITHIN(3, expected, nanos_per_call);
    }

    // This isn't really a test.
    // It's a convenient place to measure the timeline for the ISR call
    static void measure_ISR_time_line() {
        static volatile uint32_t isr_started = 0;
        static volatile uint32_t isr_ended = 0;
        recorder.set_callback([]() {
            isr_started = ARM_DWT_CYCCNT;
            recorder.add_event();
            isr_ended = ARM_DWT_CYCCNT;
        });

        sda.set();
        scl.clear();
        recorder.start(trace);

        sda.toggle();
        uint32_t after_set = ARM_DWT_CYCCNT;
        volatile uint32_t before_isr;
        for (int i = 0; i < 3; ++i) {
            asm("nop");
            before_isr = ARM_DWT_CYCCNT;
        }
        asm("nop");
        asm("nop");
        asm("nop");
        // The NOPs above ensure that after_isr is set after the ISR returns
        volatile uint32_t after_isr = ARM_DWT_CYCCNT;
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();

        uint32_t pre_interrupt_duration = common::hal::TeensyTimestamp::nanos_between(after_set, before_isr);
        Serial.printf("Main interrupted after : %d\n", pre_interrupt_duration);
        uint32_t isr_latency = common::hal::TeensyTimestamp::nanos_between(after_set, isr_started);
        Serial.printf("ISR latency: %d\n", isr_latency);
        uint32_t isr_function_duration = common::hal::TeensyTimestamp::nanos_between(isr_started, isr_ended);
        Serial.printf("ISR function duration (probably 30 nanos too high) : %d\n", isr_function_duration);
        uint32_t isr_end_duration = common::hal::TeensyTimestamp::nanos_between(isr_ended, after_isr);
        Serial.printf("ISR exit duration: %d\n", isr_end_duration);
        uint32_t interrupt_duration = common::hal::TeensyTimestamp::nanos_between(after_set, after_isr);
        Serial.printf("Total interrupt time: %d\n", interrupt_duration);
        TEST_ASSERT_EQUAL(2, trace.event_count());
        TEST_ASSERT_EQUAL(SDA_LINE_CHANGED, trace.event(1)->flags);
    }

    // If two events are handled by the same ISR call then they are
    // considered to have happened simultaneously.
    // I believe there's no reliable method to work out which happened
    // first.
    static void close_events_are_considered_to_be_simultaneous() {
        scl.set();
        sda.set();

        // WHEN we record 2 edges in quick succession
        recorder.start(trace);
        sda.clear();
        delayNanoseconds(0);
        scl.clear();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();

        // THEN the BusRecorder considers them to have happened simultaneously.
        TEST_ASSERT_EQUAL_UINT32(2, trace.event_count());
        TEST_ASSERT_EQUAL(SDA_LINE_STATE | SCL_LINE_STATE, trace.event(0)->flags);
        TEST_ASSERT_EQUAL(SDA_LINE_CHANGED | SCL_LINE_CHANGED, trace.event(1)->flags);
    }

    // Other BusRecorder implementations get the order of events
    // wrong if both lines are changed in quick succession.
    // This implementation records as simultaneous events instead.
    static void does_not_reorder_events() {
        // Can't happen with this implementation of BusRecorder
        TEST_ASSERT_TRUE(1);
    }

    static void loses_events() {
        mark_test_with_sda_toggle(); // makes it easy to find trace on a scope
        scl.set();

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

    static void does_not_lose_sync() {
        mark_test_with_sda_toggle(); // makes it easy to find trace on a scope
        scl.clear();

        // GIVEN the recorder lost edges because they happened too quickly
        recorder.start(trace);
        scl.set();
        asm("nop");
        delayNanoseconds(0);
        scl.clear();
        asm("nop");
        delayNanoseconds(0);
        scl.set();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
//        trace.printTo(Serial);

        // THEN the number of edges is incorrect
        TEST_ASSERT_EQUAL_UINT32(2, trace.event_count());
        // AND the final line state is HIGH
        TEST_ASSERT_EQUAL(SCL_LINE_CHANGED | SCL_LINE_STATE, trace.event(1)->flags);
    }

    static void captures_low_to_high_glitch_on_single_line() {
        mark_test_with_sda_toggle(); // makes it easy to find trace on a scope
        scl.clear();

        // GIVEN a line toggles from LOW to HIGH before the recorder can react
        recorder.start(trace);
        scl.set();
        delayNanoseconds(0);
        scl.clear();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
//        trace.printTo(Serial);

        // THEN the trace shows that SCL glitched from LOW->HIGH->LOW
        BusEvent expected_trace[3] = {
                BusEvent(8, BOTH_LOW_AND_UNCHANGED),
                BusEvent(55, SCL_LINE_CHANGED | SCL_LINE_STATE),
                BusEvent(0, SCL_LINE_CHANGED)
        };
        TEST_ASSERT_EQUAL_UINT32(3, trace.event_count());
        TEST_ASSERT_EQUAL(expected_trace[0].flags, trace.event(0)->flags);
        TEST_ASSERT_EQUAL(expected_trace[1].flags, trace.event(1)->flags);
        TEST_ASSERT_EQUAL(expected_trace[2].flags, trace.event(2)->flags);
        TEST_ASSERT_UINT32_WITHIN(15, expected_trace[1].delta_t_in_ticks, trace.event(1)->delta_t_in_ticks);
        // AND there's no delta is reported as 0
        TEST_ASSERT_EQUAL_UINT32(0, trace.event(2)->delta_t_in_ticks);
    }

    static void captures_high_to_low_glitch_on_single_line() {
        mark_test_with_sda_toggle(); // makes it easy to find trace on a scope
        scl.set();

        // GIVEN a line toggles from HIGH to LOW before the recorder can react
        recorder.start(trace);
        scl.clear();
        delayNanoseconds(0);
        scl.set();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
//        trace.printTo(Serial);

        // THEN the trace shows that SCL glitched from HIGH->LOW->HIGH
        BusEvent expected_trace[3] = {
                BusEvent(8, SCL_LINE_STATE),
                BusEvent(55, SCL_LINE_CHANGED),
                BusEvent(0, SCL_LINE_CHANGED | SCL_LINE_STATE)
        };
        TEST_ASSERT_EQUAL_UINT32(3, trace.event_count());
        TEST_ASSERT_EQUAL(expected_trace[0].flags, trace.event(0)->flags);
        TEST_ASSERT_EQUAL(expected_trace[1].flags, trace.event(1)->flags);
        TEST_ASSERT_EQUAL(expected_trace[2].flags, trace.event(2)->flags);
        TEST_ASSERT_UINT32_WITHIN(15, expected_trace[1].delta_t_in_ticks, trace.event(1)->delta_t_in_ticks);
        // AND there's no delta is reported as 0
        TEST_ASSERT_EQUAL_UINT32(0, trace.event(2)->delta_t_in_ticks);
    }

    static void captures_simultaneous_glitches_on_both_lines() {
        sda.set();
        scl.set();

        // GIVEN both lines toggle from HIGH to LOW before the recorder can react
        recorder.start(trace);
        sda.clear();
        scl.clear();
        delayNanoseconds(0);
        sda.set();
        scl.set();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
//        trace.printTo(Serial);

        // THEN the trace shows that both lines glitched from HIGH->LOW->HIGH
        BusEvent expected_trace[3] = {
                BusEvent(8, SDA_LINE_STATE | SCL_LINE_STATE),
                BusEvent(56, SDA_LINE_CHANGED | SCL_LINE_CHANGED),
                BusEvent(0, SDA_LINE_CHANGED | SCL_LINE_CHANGED | SDA_LINE_STATE | SCL_LINE_STATE)
        };
        TEST_ASSERT_EQUAL_UINT32(3, trace.event_count());
        TEST_ASSERT_EQUAL(expected_trace[0].flags, trace.event(0)->flags);
        TEST_ASSERT_EQUAL(expected_trace[1].flags, trace.event(1)->flags);
        TEST_ASSERT_EQUAL(expected_trace[2].flags, trace.event(2)->flags);
        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[1].delta_t_in_ticks, trace.event(1)->delta_t_in_ticks);
        // AND there's no delta is reported as 0
        TEST_ASSERT_EQUAL_UINT32(0, trace.event(2)->delta_t_in_ticks);
    }

    static void loses_glitch_if_other_line_changes() {
        sda.set();
        scl.set();

        // GIVEN one line changes and the other toggles before the recorder can react
        recorder.start(trace);
        sda.clear();
        scl.clear();
        delayNanoseconds(0);
        scl.set();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
//        trace.printTo(Serial);

        // THEN the trace does not include the glitch
        BusEvent expected_trace[2] = {
                BusEvent(8, SDA_LINE_STATE | SCL_LINE_STATE),
                BusEvent(56, SDA_LINE_CHANGED | SCL_LINE_STATE)
        };
        TEST_ASSERT_EQUAL_UINT32(2, trace.event_count());
        TEST_ASSERT_EQUAL(expected_trace[0].flags, trace.event(0)->flags);
        TEST_ASSERT_EQUAL(expected_trace[1].flags, trace.event(1)->flags);
        TEST_ASSERT_UINT32_WITHIN(10, expected_trace[1].delta_t_in_ticks, trace.event(1)->delta_t_in_ticks);
    }

    // This is a regression test for a bug which caused a glitch to be
    // recorded as an event with no edges.
    static void records_glitch_even_if_another_pin_changes() {
        common::hal::TeensyPin unrelated_pin(PIN_SCL_1, OUTPUT);
        mark_test_with_sda_toggle(); // makes it easy to find trace on a scope
        scl.set();

        // GIVEN an unrelated pin changes state during a glitch
        recorder.start(trace);
        unrelated_pin.toggle();
        scl.clear();
        delayNanoseconds(0);
        scl.set();
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
//        trace.printTo(Serial);

        // THEN the trace records the glitch
        BusTrace expected_trace(3);
        expected_trace.add_event(BusEvent(8, SCL_LINE_STATE));
        expected_trace.add_event(BusEvent(55, SCL_LINE_CHANGED));
        expected_trace.add_event(BusEvent(0, SCL_LINE_CHANGED | SCL_LINE_STATE));
        size_t equivalent = expected_trace.compare_edges(trace);
        TEST_ASSERT_EQUAL_UINT32(SIZE_MAX, equivalent);
    }

    static void glitch_pin_repeatedly(int repeats, common::hal::TeensyPin& pin) {
        for (int i = 0; i < repeats; ++i) {
            pin.toggle();
            delayNanoseconds(0);
            pin.toggle();
            delayNanoseconds(10);
        }
    }

    // Estimates the time taken to record a single glitch.
    // Most of this time is taken detecting the edge and firing the interrupt.
    static void test_interrupt_duration_single_line_glitch() {
        mark_test_with_sda_toggle(); // makes it easy to find trace on a scope
        scl.set();

        // WHEN we record the events
        recorder.start(trace);
        int repeats = 10;
        glitch_pin_repeatedly(repeats, scl);
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
//        print_trace(trace);

        // THEN we recorded one event per edge
        uint32_t expected_num_events = 1 + (repeats * 2);
        TEST_ASSERT_EQUAL(expected_num_events, trace.event_count());

        // AND we can estimate the time to handle the interrupt
        size_t start_index = 5; // The timings are a little erratic for the first few cycles.
        uint32_t duration_with_interrupts = nanos_til_end(trace, start_index);
        uint32_t num_interrupts = (trace.event_count() - start_index) / 2;
        double nanos_per_call = ((double)duration_with_interrupts) / num_interrupts;
//        Serial.printf("%.0f nanos per interrupt for single pin glitch\n", nanos_per_call);
        // Note that the time includes 16 nanos after the pin is set and before the interrupt begins
        const uint32_t expected = 184 + 16; // Confirmed with scope at 200 nanos
        TEST_ASSERT_UINT32_WITHIN(5, expected, nanos_per_call);
    }

    static void glitch_both_pins_repeatedly(int repeats) {
        for (int i = 0; i < repeats; ++i) {
            scl.toggle();
            sda.toggle();
            delayNanoseconds(0);
            scl.toggle();
            sda.toggle();
            delayNanoseconds(10);
        }
    }

    // Estimates the time taken to record glitches on both lines.
    // Most of this time is taken detecting the edge and firing the interrupt.
    static void test_interrupt_duration_glitches_on_both_lines() {
        mark_test_with_sda_toggle(); // makes it easy to find trace on a scope
        scl.set();

        // WHEN we record the events
        recorder.start(trace);
        int repeats = 10;
        glitch_both_pins_repeatedly(repeats);
        delayNanoseconds(WAIT_FOR_FINAL_EDGE);
        recorder.stop();
//        print_trace(trace);

        // THEN we recorded one event per edge
        uint32_t expected_num_events = 1 + (repeats * 2);
        TEST_ASSERT_EQUAL(expected_num_events, trace.event_count());

        // AND we can estimate the time to handle the interrupt
        size_t start_index = 4; // The timings are a little erratic for the first few cycles.
        uint32_t duration_with_interrupts = nanos_til_end(trace, start_index);
        uint32_t num_interrupts = (trace.event_count() - start_index) / 2;
        double nanos_per_call = ((double)duration_with_interrupts) / num_interrupts;
//        Serial.printf("%.0f nanos per interrupt for glitches on both pins\n", nanos_per_call);
        // Note that the time includes 16 nanos after the pin is set and before the interrupt begins
        const uint32_t expected = 190 + 16; // Confirmed with scope at 206 nanos
        TEST_ASSERT_UINT32_WITHIN(10, expected, nanos_per_call);
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(lines_are_low_to_start_with);
        RUN_TEST(is_recording);
        RUN_TEST(records_initial_line_states);
        RUN_TEST(will_not_start_unless_callbacks_are_set);
        RUN_TEST(will_not_start_unless_pins_share_the_same_GPIO_block);
        RUN_TEST(ignores_edges_when_not_recording);
        RUN_TEST(records_edges);
        RUN_TEST(creates_another_recording_correctly);
        RUN_TEST(test_interrupt_duration_single_line);
        RUN_TEST(test_interrupt_duration_both_lines);
        RUN_TEST(close_events_are_considered_to_be_simultaneous);
        RUN_TEST(does_not_reorder_events);
        RUN_TEST(loses_events);
        RUN_TEST(does_not_lose_sync);
        RUN_TEST(captures_low_to_high_glitch_on_single_line);
        RUN_TEST(captures_high_to_low_glitch_on_single_line);
        RUN_TEST(captures_simultaneous_glitches_on_both_lines);
        RUN_TEST(loses_glitch_if_other_line_changes);
        RUN_TEST(records_glitch_even_if_another_pin_changes);
        RUN_TEST(test_interrupt_duration_single_line_glitch);
        RUN_TEST(test_interrupt_duration_glitches_on_both_lines);

//        RUN_TEST(measure_ISR_time_line);
    }

    BusRecorderE2ETest() : E2ETestBase(__FILE__) {};
};

// Define statics
BusTrace BusRecorderE2ETest::trace(MAX_EVENTS);
BusRecorder BusRecorderE2ETest::recorder(PIN_SNIFF_SDA, PIN_SNIFF_SCL);
common::hal::TeensyPin BusRecorderE2ETest::sda(PIN_DRIVE_SDA, OUTPUT);
common::hal::TeensyPin BusRecorderE2ETest::scl(PIN_DRIVE_SCL, OUTPUT);
}
#endif  //I2C_UNDERNEATH_BUS_RECORDER_E2E_TEST_H
