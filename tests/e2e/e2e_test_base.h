// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_E2E_TEST_BASE_H
#define I2C_UNDERNEATH_E2E_TEST_BASE_H

#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"

namespace e2e {

class E2ETestBase : public TestSuite {
public:
    // WARNING: Changing pins can affect the order of edges.
    // SDA on pin 23 and SCL on pin 22 works Ok.
    // SDA on pin 21 and SCL on pin 20 works Ok.
    constexpr static uint32_t PIN_SNIFF_SDA = 21;   // GPIO1 and GPIO6 bit 27
    constexpr static uint32_t PIN_SNIFF_SCL = 20;   // GPIO1 and GPIO6 bit 26

    // Use these pins to drive the SNIFF pins
    constexpr static uint32_t PIN_DRIVE_SDA = 18;   // I2C Port 0
    constexpr static uint32_t PIN_DRIVE_SCL = 19;   // I2C Port 0

    explicit E2ETestBase(const char* test_file_name)
            : TestSuite(test_file_name) {
    };

    void setUp() override {
        pinMode(PIN_SNIFF_SDA, INPUT);
        pinMode(PIN_SNIFF_SCL, INPUT);
        pinMode(PIN_DRIVE_SDA, OUTPUT);
        pinMode(PIN_DRIVE_SCL, OUTPUT);
    }

    void tearDown() override {
        pinMode(PIN_SNIFF_SDA, INPUT_DISABLE);
        pinMode(PIN_SNIFF_SCL, INPUT_DISABLE);
        pinMode(PIN_DRIVE_SDA, INPUT_DISABLE);
        pinMode(PIN_DRIVE_SCL, INPUT_DISABLE);
    }

    static void print_trace(const bus_trace::BusTrace& busTrace) {
        // Remove comments to see trace timings.
        for (size_t i = 0; i < busTrace.event_count(); ++i) {
            Serial.printf("Index %d: delta %d ns\n", i, common::hal::TeensyTimestamp::ticks_to_nanos(busTrace.event(i)->delta_t_in_ticks));
        }
    }
};
}
// Define statics
#endif //I2C_UNDERNEATH_E2E_TEST_BASE_H
