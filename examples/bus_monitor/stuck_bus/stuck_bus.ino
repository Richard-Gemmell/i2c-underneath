// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)
//
// This example shows how to monitor an I2C bus and detect
// a "stuck bus". The bus is said to be stuck if either
// of the bus lines (SDA or SCL) stays LOW. Devices cannot
// use a stuck bus.
//
// If the bus is stuck because SDA is LOW than it may be
// possible to clear the fault in software. If SCL is stuck
// LOW then it can't be cleared without restarting the
// stuck device.


#ifndef UNIT_TEST
#include <Arduino.h>
#include "bus_monitor.h"
#include "common/hal/arduino/arduino_pin.h"
#include "common/hal/teensy/teensy_timestamp.h"


IntervalTimer blink_timer;
void blink();

IntervalTimer report_timer;
void report();

common::hal::ArduinoPin sda(SDA);
static void sda_on_edge_isr() {
    sda.raise_on_edge();
}
common::hal::ArduinoPin scl(SCL);
static void scl_on_edge_isr() {
    scl.raise_on_edge();
}

common::hal::TeensyTimestamp timestamp;
bus_monitor::BusState previous_state = bus_monitor::BusState::unknown;

// Create a BusMonitor with the default timeouts
bus_monitor::BusMonitor monitor(sda, scl, timestamp);

void setup() {
    // Blink the LED to show we're alive
    pinMode(LED_BUILTIN, OUTPUT);
    blink_timer.begin(blink, 500'000);

//    report_timer.begin(report, 500'000);

    // Set the interrupt service routines for our pins
    sda.set_on_edge_isr(sda_on_edge_isr);
    scl.set_on_edge_isr(scl_on_edge_isr);

    // Start monitoring the bus
    monitor.begin();

    Serial.begin(9600);
    Serial.println("Started Stuck Bus Example");
}

void loop() {
    bus_monitor::BusState bus_state = monitor.get_state();
    if (bus_state != previous_state) {
        report();
        if (bus_state == bus_monitor::BusState::stuck) {
            // One of the I2C devices on the bus is waiting for an event
            // that will never happen or has hung.
            //
            // The last slave device that was addressed is the one that's
            // most likely to be stuck. The master is a possibility as well.
            if (!scl.read_line()) {
                // SCL is stuck.
                //
                // If this device is acting as an I2C Master or Slave then
                // reset the driver here in case we're the stuck device.
                //
                // If one of the other devices is stuck then you'll need to
                // reset it by toggling a reset pin or by rebooting it.
                Serial.println("I2C clock is stuck LOW. Reset all devices to clear the fault.");
            }
            else if (!sda.read_line()) {
                // SDA is stuck.
                Serial.println("I2C data line is stuck LOW. Clearing fault.");
                // TODO: send 9 clock pulses on SCL to clear the fault
                if (monitor.get_state() == bus_monitor::BusState::stuck) {
                    Serial.println("Failed to clear I2C bus automatically. Reset set all devices to clear the fault.");
                } else {
                    Serial.println("I2C bus is free again.");
                }
            }
        }
    }
    previous_state = bus_state;

    delay(1);
}

void report() {
    Serial.print("SDA ");
    Serial.print(sda.read_line());
    Serial.print(" SCL ");
    Serial.println(scl.read_line());

    Serial.print("BusState is ");
    Serial.println((int)monitor.get_state());
}

void blink() {
    digitalToggle(LED_BUILTIN);
}

#endif