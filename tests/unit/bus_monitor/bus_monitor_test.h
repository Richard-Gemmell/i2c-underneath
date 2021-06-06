// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_UNIT_TEST_BUS_MONITOR_TEST_H
#define I2C_UNDERNEATH_UNIT_TEST_BUS_MONITOR_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "../../utils/test_suite.h"
#include <common/hal/pin.h>
#include <bus_monitor.h>
#include <bus_monitor/bus_state.h>
#include "../../fakes/common/hal/fake_pin.h"
#include "../../fakes/common/hal/fake_timestamp.h"

namespace bus_monitor {

class BusMonitorTest : public TestSuite {
public:
    static common::hal::FakePin* scl;
    static common::hal::FakePin* sda;
    static common::hal::FakeTimestamp* timestamp;

    static void bus_state_is_unknown_on_construction() {
        // WHEN we create a monitor
        // THEN the bus state is unknown
        BusMonitor monitor(*sda, *scl, *timestamp);
        TEST_ASSERT_EQUAL(BusState::unknown, monitor.get_state());
    }

    static void get_state_does_not_set_state_unless_running() {
        // GIVEN one pin is low and the other is high
        sda->write_pin(false);
        BusMonitor monitor(*sda, *scl, *timestamp);

        // WHEN we call read the state
        BusState state = monitor.get_state();

        // THEN the monitor does not query the lines or update the sate
        TEST_ASSERT_EQUAL(state, BusState::unknown);
        TEST_ASSERT_FALSE(sda->read_line_called);
        TEST_ASSERT_FALSE(scl->read_line_called);
    }

    static void call_to_begin_is_ignored_if_already_running() {
        // GIVEN monitor is running
        BusMonitor monitor(*sda, *scl, *timestamp);
        monitor.begin();
        scl->on_edge_called = false;
        sda->on_edge_called = false;

        // WHEN we call begin() without calling end()
        monitor.begin();

        // THEN the call to begin() is ignore
        TEST_ASSERT_FALSE(scl->on_edge_called);
        TEST_ASSERT_FALSE(sda->on_edge_called);
    }

    static void call_to_end_is_ignored_if_not_running() {
        // GIVEN we're not monitoring the bus
        BusMonitor monitor(*sda, *scl, *timestamp);

        // WHEN we call end()
        monitor.end();

        // THEN the call to end is ignored
        TEST_ASSERT_FALSE(scl->on_edge_called);
        TEST_ASSERT_FALSE(sda->on_edge_called);
    }

    static void call_to_end_is_ignored_if_was_running_but_now_stopped() {
        // GIVEN we're not monitoring the bus
        BusMonitor monitor(*sda, *scl, *timestamp);
        monitor.begin();
        monitor.end();
        scl->on_edge_called = false;
        sda->on_edge_called = false;

        // WHEN we call end()
        monitor.end();

        // THEN the call to end is ignored
        TEST_ASSERT_FALSE(scl->on_edge_called);
        TEST_ASSERT_FALSE(sda->on_edge_called);
    }

    static void bus_state_idle_if_pins_high_on_begin() {
        // GIVEN both pins are high
        BusMonitor monitor(*sda, *scl, *timestamp);

        // WHEN we call begin()
        monitor.begin();

        // THEN the bus state is idle
        TEST_ASSERT_EQUAL(BusState::idle, monitor.get_state());
    }

    static void bus_state_busy_if_one_pin_is_low_on_begin() {
        // GIVEN one pin is low and the other is high
        sda->write_pin(false);
        BusMonitor monitor(*sda, *scl, *timestamp);

        // WHEN we call begin()
        monitor.begin();

        // THEN the bus is busy to start with
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_state_busy_if_both_pins_low_on_begin() {
        // GIVEN both pins are low
        sda->write_pin(false);
        scl->write_pin(false);
        BusMonitor monitor(*sda, *scl, *timestamp);

        // WHEN we call begin()
        monitor.begin();

        // THEN the bus is busy to start with
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_state_is_unknown_after_calling_end() {
        // GIVEN we're monitoring a bus
        BusMonitor monitor(*sda, *scl, *timestamp);
        monitor.begin();

        // WHEN we call end()
        monitor.end();

        // THEN the bus state returns to unknown
        TEST_ASSERT_EQUAL(BusState::unknown, monitor.get_state());
    }

    static void stops_listening_to_pin_events_after_end() {
        // GIVEN we're monitoring a bus
        BusMonitor monitor(*sda, *scl, *timestamp);
        monitor.begin();

        // WHEN we call end()
        monitor.end();

        // THEN the monitor is no longer listening for pin events
        TEST_ASSERT_NULL(sda->on_edge_callback_);
        TEST_ASSERT_NULL(scl->on_edge_callback_);
    }

    static void destructor_calls_end() {
        // GIVEN we're monitoring a bus
        auto monitor = new BusMonitor(*sda, *scl, *timestamp);
        monitor->begin();

        // WHEN we call the destructor
        delete(monitor);

        // THEN the monitor is no longer monitoring the bus
        TEST_ASSERT_NULL(sda->on_edge_callback_);
        TEST_ASSERT_NULL(scl->on_edge_callback_);
    }

    static void bus_state_becomes_busy_if_sda_changes() {
        // GIVEN we're monitoring a bus
        uint32_t bus_busy_timeout_micros = 5;
        BusMonitor monitor(*sda, *scl, *timestamp, bus_busy_timeout_micros * 1'000);
        monitor.begin();

        // WHEN SDA changes state
        sda->write_pin(false);

        // THEN the bus state changes to busy
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_state_becomes_busy_if_scl_changes() {
        // GIVEN we're monitoring a bus
        BusMonitor monitor(*sda, *scl, *timestamp);
        monitor.begin();

        // WHEN SCL changes state
        scl->write_pin(false);

        // THEN the bus state changes to busy
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_state_remains_stuck_if_sda_goes_high() {
        // GIVEN the bus is stuck
        uint32_t bus_stuck_timeout = 5'000'000;
        BusMonitor monitor(*sda, *scl, *timestamp, 1'000, bus_stuck_timeout);
        monitor.begin();
        sda->write_pin(false);
        scl->write_pin(false);
        timestamp->set_time_passed(bus_stuck_timeout + 1);
        TEST_ASSERT_EQUAL(BusState::stuck, monitor.get_state());

        // WHEN SDA rises
        sda->write_pin(true);

        // THEN the bus state remains stuck
        TEST_ASSERT_EQUAL(BusState::stuck, monitor.get_state());
    }

    static void bus_state_remains_stuck_if_scl_goes_high() {
        // GIVEN the bus is stuck
        uint32_t bus_stuck_timeout = 5'000'000;
        BusMonitor monitor(*sda, *scl, *timestamp, 1'000, bus_stuck_timeout);
        monitor.begin();
        sda->write_pin(false);
        scl->write_pin(false);
        timestamp->set_time_passed(bus_stuck_timeout + 1);
        TEST_ASSERT_EQUAL(BusState::stuck, monitor.get_state());

        // WHEN SDA rises
        scl->write_pin(true);

        // THEN the bus state remains stuck
        TEST_ASSERT_EQUAL(BusState::stuck, monitor.get_state());
    }

    static void bus_no_longer_stuck_when_both_lines_go_high() {
        // GIVEN the bus is stuck with both pins low
        uint32_t bus_stuck_timeout = 5'000'000;
        BusMonitor monitor(*sda, *scl, *timestamp, 1'000, bus_stuck_timeout);
        monitor.begin();
        sda->write_pin(false);
        scl->write_pin(false);
        timestamp->set_time_passed(bus_stuck_timeout + 1);
        TEST_ASSERT_EQUAL(BusState::stuck, monitor.get_state());

        // WHEN both pin rises
        scl->write_pin(true);
        sda->write_pin(true);

        // THEN the bus state changes to busy
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_remains_busy_if_sda_stays_low() {
        // GIVEN the bus has just become busy
        uint32_t bus_busy_timeout = 5'000;
        BusMonitor monitor(*sda, *scl, *timestamp, bus_busy_timeout);
        monitor.begin();
        sda->write_pin(false);

        // WHEN the bus busy timeout expires
        timestamp->set_time_passed(bus_busy_timeout + 1);

        // THEN the bus is still busy
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_remains_busy_if_scl_stays_low() {
        // GIVEN the bus has just become busy
        uint32_t bus_busy_timeout = 5'000;
        BusMonitor monitor(*sda, *scl, *timestamp, bus_busy_timeout);
        monitor.begin();
        scl->write_pin(false);

        // WHEN the bus busy timeout expires
        timestamp->set_time_passed(bus_busy_timeout + 1);

        // THEN the bus is still busy
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_does_not_transition_to_idle_before_timeout() {
        // GIVEN one of the bus lines has just been pulsed
        uint32_t bus_busy_timeout = 5'000;
        BusMonitor monitor(*sda, *scl, *timestamp, bus_busy_timeout);
        monitor.begin();
        scl->write_pin(false);
        scl->write_pin(true);

        // WHEN the timeout has not expired
        timestamp->set_time_passed(bus_busy_timeout - 1);

        // THEN the bus ist still busy
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_transitions_from_busy_to_idle() {
        // GIVEN one of the bus lines has just been pulsed
        uint32_t bus_busy_timeout = 5'000;
        BusMonitor monitor(*sda, *scl, *timestamp, bus_busy_timeout);
        monitor.begin();
        scl->write_pin(false);
        scl->write_pin(true);

        // WHEN the timeout expires
        timestamp->set_time_passed(bus_busy_timeout + 1);

        // THEN the bus becomes idle
        TEST_ASSERT_EQUAL(BusState::idle, monitor.get_state());
    }

    static void bus_does_not_transition_to_stuck_before_timeout() {
        // GIVEN the bus is already busy
        uint32_t bus_stuck_timeout = 10'000'000;
        BusMonitor monitor(*sda, *scl, *timestamp, 1'000, bus_stuck_timeout);
        monitor.begin();
        sda->write_pin(false);

        // WHEN the stuck timeout passes
        timestamp->set_time_passed(bus_stuck_timeout - 1);
        BusState state = monitor.get_state();

        // THEN the bus state remains busy
        TEST_ASSERT_EQUAL(BusState::busy, state);
    }

    static void bus_state_becomes_stuck_if_sda_stays_low() {
        // GIVEN the bus is already busy
        uint32_t bus_stuck_timeout = 10'000'000;
        BusMonitor monitor(*sda, *scl, *timestamp, 1'000, bus_stuck_timeout);
        monitor.begin();
        sda->write_pin(false);

        // WHEN the stuck timeout passes
        timestamp->set_time_passed(bus_stuck_timeout + 1);
        BusState state = monitor.get_state();

        // THEN the bus state changes to stuck
        TEST_ASSERT_EQUAL(BusState::stuck, state);
    }

    static void bus_state_becomes_stuck_if_scl_stays_low() {
        // GIVEN the bus is already busy
        uint32_t bus_stuck_timeout = 10'000'000;
        BusMonitor monitor(*sda, *scl, *timestamp, 1'000, bus_stuck_timeout);
        monitor.begin();
        scl->write_pin(false);

        // WHEN the stuck timeout passes
        timestamp->set_time_passed(bus_stuck_timeout + 1);
        BusState state = monitor.get_state();

        // THEN the bus state changes to stuck
        TEST_ASSERT_EQUAL(BusState::stuck, state);
    }

    static void bus_timeout_is_reset_if_sda_changes_level() {
        // GIVEN the bus is busy
        uint32_t bus_busy_timeout = 5'000;
        BusMonitor monitor(*sda, *scl, *timestamp, bus_busy_timeout);
        monitor.begin();
        sda->write_pin(false);

        // WHEN the line changes before the bus busy timeout expires
        timestamp->set_time_passed(bus_busy_timeout - 1);
        sda->write_pin(true);

        // THEN the timeout is reset and the bus remains busy
        TEST_ASSERT_EQUAL(0, timestamp->get_time_passed());
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_timeout_is_reset_if_scl_changes_level() {
        // GIVEN the bus is busy
        uint32_t bus_busy_timeout = 5'000;
        BusMonitor monitor(*sda, *scl, *timestamp, bus_busy_timeout);
        monitor.begin();
        scl->write_pin(false);

        // WHEN the line changes before the bus busy timeout expires
        timestamp->set_time_passed(bus_busy_timeout - 1);
        scl->write_pin(true);

        // THEN the timeout is reset and the bus remains busy
        TEST_ASSERT_EQUAL(0, timestamp->get_time_passed());
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(bus_state_is_unknown_on_construction);

        // Monitor not active unless running
        RUN_TEST(get_state_does_not_set_state_unless_running);
        RUN_TEST(call_to_begin_is_ignored_if_already_running);
        RUN_TEST(call_to_end_is_ignored_if_not_running);
        RUN_TEST(call_to_end_is_ignored_if_was_running_but_now_stopped);

        // begin() sets initial state
        RUN_TEST(bus_state_idle_if_pins_high_on_begin);
        RUN_TEST(bus_state_busy_if_one_pin_is_low_on_begin);
        RUN_TEST(bus_state_busy_if_both_pins_low_on_begin);

        // end() resets the monitor
        RUN_TEST(bus_state_is_unknown_after_calling_end);
        RUN_TEST(stops_listening_to_pin_events_after_end);
        RUN_TEST(destructor_calls_end);

        // Edge events on pins can trigger following state changes:
        //   idle => busy
        //   stuck => busy
        RUN_TEST(bus_state_becomes_busy_if_sda_changes);
        RUN_TEST(bus_state_becomes_busy_if_scl_changes);
        RUN_TEST(bus_state_remains_stuck_if_sda_goes_high);
        RUN_TEST(bus_state_remains_stuck_if_scl_goes_high);
        RUN_TEST(bus_no_longer_stuck_when_both_lines_go_high);

        // The follow state changes are driven by timeouts:
        //   busy => idle
        //   busy => stuck
        RUN_TEST(bus_remains_busy_if_sda_stays_low);
        RUN_TEST(bus_remains_busy_if_scl_stays_low);
        RUN_TEST(bus_does_not_transition_to_idle_before_timeout);
        RUN_TEST(bus_transitions_from_busy_to_idle);
        RUN_TEST(bus_does_not_transition_to_stuck_before_timeout);
        RUN_TEST(bus_state_becomes_stuck_if_sda_stays_low);
        RUN_TEST(bus_state_becomes_stuck_if_scl_stays_low);

        // Bus activity resets timeouts
        RUN_TEST(bus_timeout_is_reset_if_sda_changes_level);
        RUN_TEST(bus_timeout_is_reset_if_scl_changes_level);
    }

    void setUp() override {
        scl = new common::hal::FakePin();
        sda = new common::hal::FakePin();
        timestamp = new common::hal::FakeTimestamp();
    }

    void tearDown() override {
        delete(scl);
        scl = nullptr;
        delete(sda);
        sda = nullptr;
        delete(timestamp);
        timestamp = nullptr;
    }

    BusMonitorTest() : TestSuite(__FILE__) {};
};

// Define statics
common::hal::FakePin* BusMonitorTest::scl;
common::hal::FakePin* BusMonitorTest::sda;
common::hal::FakeTimestamp* BusMonitorTest::timestamp;

}

#endif //I2C_UNDERNEATH_UNIT_TEST_BUS_MONITOR_TEST_H
