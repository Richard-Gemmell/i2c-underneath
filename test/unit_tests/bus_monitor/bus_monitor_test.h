// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_UNIT_TEST_BUS_MONITOR_TEST_H
#define I2C_UNDERNEATH_UNIT_TEST_BUS_MONITOR_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "../../utils/test_suite.h"
#include <common/hardware/pin.h>
#include <bus_monitor/bus_monitor.h>
#include <bus_monitor/bus_state.h>
#include "../../fakes/common/hardware/fake_pin.h"

namespace bus_monitor {

class BusMonitorTest : public TestSuite {
public:
    static common::hardware::FakePin* scl;
    static common::hardware::FakePin* sda;

    static void bus_state_is_unknown_on_construction() {
        // WHEN we create a monitor
        // THEN the bus state is unknown
        BusMonitor monitor(*sda, *scl);
        TEST_ASSERT_EQUAL(BusState::unknown, monitor.get_state());
    }

    static void bus_state_idle_if_pins_high_on_begin() {
        // GIVEN both pins are high
        BusMonitor monitor(*sda, *scl);

        // WHEN we call begin()
        monitor.begin();

        // THEN the bus state is idle
        TEST_ASSERT_EQUAL(BusState::idle, monitor.get_state());
    }

    static void bus_state_busy_if_one_pin_is_low_on_begin() {
        // GIVEN one pin is low and the other is high
        sda->write_pin(false);
        BusMonitor monitor(*sda, *scl);

        // WHEN we call begin()
        monitor.begin();

        // THEN the bus is busy to start with
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_state_busy_if_both_pins_low_on_begin() {
        // GIVEN both pins are low
        sda->write_pin(false);
        scl->write_pin(false);
        BusMonitor monitor(*sda, *scl);

        // WHEN we call begin()
        monitor.begin();

        // THEN the bus is busy to start with
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_state_is_unknown_after_calling_end() {
        // GIVEN we're monitoring a bus
        BusMonitor monitor(*sda, *scl);
        monitor.begin();

        // WHEN we call end()
        monitor.end();

        // THEN the bus state returns to unknown
        TEST_ASSERT_EQUAL(BusState::unknown, monitor.get_state());
    }

    static void destructor_calls_end() {
        // GIVEN we're monitoring a bus
        auto monitor = new BusMonitor(*sda, *scl);
        monitor->begin();

        // WHEN we call the destructor
        delete(monitor);

        // THEN the monitor is no longer monitoring the bus
        TEST_FAIL_MESSAGE("Not finished");
    }

    static void bust_state_becomes_busy_if_sda_changes() {
        // GIVEN we're monitoring a bus
        uint32_t bus_busy_timeout_micros = 5;
        BusMonitor monitor(*sda, *scl, bus_busy_timeout_micros * 1'000);
        monitor.begin();

        // WHEN SDA changes state
        sda->write_pin(false);

        // THEN the bus state changes to busy
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bust_state_becomes_busy_if_scl_changes() {
        // GIVEN we're monitoring a bus
        BusMonitor monitor(*sda, *scl);
        monitor.begin();

        // WHEN SCL changes state
        scl->write_pin(false);

        // THEN the bus state changes to busy
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_remains_busy_if_sda_stays_low() {
        // GIVEN the bus has just become busy
        uint32_t bus_busy_timeout_micros = 5;
        BusMonitor monitor(*sda, *scl, bus_busy_timeout_micros * 1'000);
        monitor.begin();
        sda->write_pin(false);

        // WHEN the bus busy timeout expires
        delayMicroseconds(bus_busy_timeout_micros * 2);

        // THEN the bus is still busy
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_remains_busy_if_scl_stays_low() {
        // GIVEN the bus has just become busy
        uint32_t bus_busy_timeout_micros = 5;
        BusMonitor monitor(*sda, *scl, bus_busy_timeout_micros * 1'000);
        monitor.begin();
        scl->write_pin(false);

        // WHEN the bus busy timeout expires
        delayMicroseconds(bus_busy_timeout_micros * 2);

        // THEN the bus is still busy
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_does_not_transition_to_idle_before_timeout() {
        // GIVEN one of the bus lines has just been pulsed
        uint32_t bus_busy_timeout = 5'000;
        BusMonitor monitor(*sda, *scl, bus_busy_timeout);
        monitor.begin();
        scl->write_pin(false);
        scl->write_pin(true);

        // WHEN the timeout has not expired
        delayNanoseconds(bus_busy_timeout - 100);

        // THEN the bus ist still busy
        TEST_ASSERT_EQUAL(BusState::busy, monitor.get_state());
    }

    static void bus_transitions_from_busy_to_idle() {
        // GIVEN one of the bus lines has just been pulsed
        uint32_t bus_busy_timeout = 5'000;
        BusMonitor monitor(*sda, *scl, bus_busy_timeout);
        monitor.begin();
        scl->write_pin(false);
        scl->write_pin(true);

        // WHEN the timeout expires
        delayNanoseconds(bus_busy_timeout + 100);

        // THEN the bus becomes idle
        TEST_ASSERT_EQUAL(BusState::idle, monitor.get_state());
    }

    // Include all the tests here
    void test() final {
//        RUN_TEST(bus_state_is_unknown_on_construction);
//        RUN_TEST(bus_state_idle_if_pins_high_on_begin);
//        RUN_TEST(bus_state_busy_if_one_pin_is_low_on_begin);
//        RUN_TEST(bus_state_busy_if_both_pins_low_on_begin);
//        RUN_TEST(bus_state_is_unknown_after_calling_end);
//        RUN_TEST(destructor_calls_end);
//        RUN_TEST(bust_state_becomes_busy_if_sda_changes);
//        RUN_TEST(bust_state_becomes_busy_if_scl_changes);
        RUN_TEST(bus_remains_busy_if_sda_stays_low);
        RUN_TEST(bus_remains_busy_if_scl_stays_low);
        RUN_TEST(bus_does_not_transition_to_idle_before_timeout);
        RUN_TEST(bus_transitions_from_busy_to_idle);
    }

    void setUp() override {
        scl = new common::hardware::FakePin();
        sda = new common::hardware::FakePin();
    }

    void tearDown() override {
        delete(scl);
        scl = nullptr;
        delete(sda);
        sda = nullptr;
    }

    BusMonitorTest() : TestSuite(__FILE__) {};
};

// Define statics
common::hardware::FakePin* BusMonitorTest::scl;
common::hardware::FakePin* BusMonitorTest::sda;

}

#endif //I2C_UNDERNEATH_UNIT_TEST_BUS_MONITOR_TEST_H
