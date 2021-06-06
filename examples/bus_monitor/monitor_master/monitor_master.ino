// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)
//
// This example shows how to monitor an I2C bus and detect
// a "stuck bus". The bus is said to be stuck if either
// of the bus lines (SDA or SCL) stays LOW. Devices cannot
// use a stuck bus.
//
// If the bus is stuck because SDA is LOW then it will attempt
// to clear the fault automatically. (Most I2C drivers will
// do this automatically.)
//
// If SCL is stuck LOW then you must reset all I2C devices
// in hardware. Many I2C slaves have a reset pin that can
// be used to reset them.
//
// This example is written for a Teensy 4 with the
// https://github.com/Richard-Gemmell/teensy4_i2c/ driver but
// system for clearing the bus is the same for any device.

#ifndef UNIT_TEST
#include <Arduino.h>
#include <i2c_driver.h>
#include <imx_rt1060/imx_rt1060_i2c_driver.h>
#include "bus_monitor.h"
#include "common/hal/arduino/arduino_pin.h"
#include "common/hal/teensy/teensy_timestamp.h"

I2CMaster& master = Master;
void finish(bool report_timeout);
void trace(const char* message);
void trace(const char* message, uint8_t address);

// Configure pins to monitor the bus
uint8_t sda_monitor_pin = 17;
uint8_t scl_monitor_pin = 16;
common::hal::ArduinoPin sda(sda_monitor_pin);
static void sda_on_edge_isr() {
    sda.raise_on_edge();
}
common::hal::ArduinoPin scl(scl_monitor_pin);
static void scl_on_edge_isr() {
    scl.raise_on_edge();
}

common::hal::TeensyTimestamp timestamp;
bus_monitor::BusState previous_state = bus_monitor::BusState::unknown;

// Create a BusMonitor with the default timeouts
bus_monitor::BusMonitor monitor(sda, scl, timestamp);

void setup() {
    // Initialise the I2C master
    master.begin(100'000U);

    // Set the interrupt service routines for the pins
    // we'll use to monitor the bus
    pinMode(sda_monitor_pin, INPUT_PULLUP);
    pinMode(scl_monitor_pin, INPUT_PULLUP);
    sda.set_on_edge_isr(sda_on_edge_isr);
    scl.set_on_edge_isr(scl_on_edge_isr);

    // Start monitoring the bus
    monitor.begin();

    // The LED is on when the bus is stuck
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, monitor.get_state() == bus_monitor::BusState::stuck);

    Serial.begin(9600);
    Serial.println("Started Stuck Bus With Recovery");
    delay(100);
}

// Sends 9 clock pulses on SCL.
// This will often clear a stuck slave.
void clear_bus() {
    pinMode(scl_monitor_pin, OUTPUT_OPENDRAIN);
    delayMicroseconds(10);
    const int reset_cycles = 9;
    for(int i=0; i<reset_cycles; i++) {
        scl.write_pin(false);
        delayMicroseconds(5);
        scl.write_pin(true);
        delayMicroseconds(5);
    }
    pinMode(scl_monitor_pin, INPUT_PULLUP);
    delayMicroseconds(10);
}

void monitor_loop() {
    bus_monitor::BusState bus_state = monitor.get_state();
    if (bus_state != previous_state) {
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
                // Data line (SDA) is stuck.
                Serial.println("I2C data line is stuck LOW. Clearing fault.");
                clear_bus();
                if (monitor.get_state() == bus_monitor::BusState::stuck) {
                    Serial.println("Failed to clear I2C bus automatically. Reset set all devices to clear the fault.");
                } else {
                    Serial.println("I2C bus is free again.");
                }
            }
        }
        digitalWrite(LED_BUILTIN, monitor.get_state() == bus_monitor::BusState::stuck);
    }
    previous_state = bus_state;
}

// This loop checks for slave devices on the bus.
// Replace it with your own I2C code
bool enable_trace = false; // Set to true for more logging
void loop() {
    Serial.println("Searching for slave devices...");
    // See if the bus is stuck and fix it if necessary.
    monitor_loop();

    uint8_t num_found = 0;
    uint8_t buffer[] = {0};
    for (uint8_t address = 1; address < 127; address++) {
        trace("Checking address ", address);
        master.read_async(address, buffer, 1, true);
        finish(false);
        master.write_async(address, buffer, 0, true);
        finish(true);

        I2CError status = master.error();
        if (status == I2CError::ok) {
            Serial.print("Slave device found at address ");
            Serial.println(address);
            num_found++;
        } else if (status == I2CError::address_nak) {
            // This is the code we expect if there's nobody listening on this address.
            trace("Address not in use.");
        } else {
            Serial.print("Unexpected error at address ");
            Serial.println(address);
        }
        trace("");

        // See if the bus is stuck and fix it if necessary.
        monitor_loop();
    }
    if (num_found == 0) {
        Serial.println("No I2C slave devices found.");
    } else {
        Serial.print("Found ");
        Serial.print(num_found);
        Serial.println(" slave devices.");
    }
    Serial.println();

    delay(1000);
}

void finish(bool report_timeout) {
    elapsedMillis timeout;
    while (timeout < 200) {
        if (master.finished()){
            return;
        }
    }
    if (report_timeout) {
        Serial.println("Master: ERROR timed out waiting for transfer to finish.");
    }
}

void trace(const char* message) {
    if (enable_trace) {
        Serial.println(message);
    }
}

void trace(const char* message, uint8_t address) {
    if (enable_trace) {
        Serial.print(message);
        Serial.println(address);
    }
}

#endif