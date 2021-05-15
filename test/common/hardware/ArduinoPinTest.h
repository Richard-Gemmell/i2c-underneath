// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_HARDWARE_ARDUINO_PINT_EST_H
#define I2C_UNDERNEATH_COMMON_HARDWARE_ARDUINO_PINT_EST_H

#include <Arduino.h>
#include "../../utils/test_suite.h"
#include "common/hardware/ArduinoPin.h"

namespace common {
namespace hardware {

#define TEST_PIN 14

class ArduinoPinTest : public TestSuite {
public:
    static bool called_back;
    static bool callback_value;
    static common::hardware::ArduinoPin* the_pin;

    void setUp() final {
        pinMode(TEST_PIN, INPUT_PULLUP);
        digitalWriteFast(TEST_PIN, HIGH);
        called_back = false;
        callback_value = false;
    }

    static void read_line() {
        common::hardware::ArduinoPin pin = ArduinoPin(TEST_PIN);

        digitalWrite(TEST_PIN, LOW);
        delayMicroseconds(10);
        TEST_ASSERT_FALSE(pin.read_line());

        digitalWrite(TEST_PIN, HIGH);
        delayMicroseconds(10);
        TEST_ASSERT_TRUE(pin.read_line());
    }

    static void write_pin() {
        common::hardware::ArduinoPin pin = common::hardware::ArduinoPin(TEST_PIN);

        pin.write_pin(false);
        delayMicroseconds(10);
        TEST_ASSERT_FALSE(digitalReadFast(TEST_PIN));

        pin.write_pin(true);
        delayMicroseconds(10);
        TEST_ASSERT_TRUE(digitalReadFast(TEST_PIN));
    }

    static void callback() {
        called_back = true;
        if (the_pin) {
            callback_value = the_pin->read_line();
        }
    }

    static void on_edge_registers_callback() {
        common::hardware::ArduinoPin pin = common::hardware::ArduinoPin(TEST_PIN);
        the_pin = &pin;
        pin.on_edge(callback);

        pin.write_pin(false);
        delayMicroseconds(10);
        TEST_ASSERT_TRUE(called_back);
        TEST_ASSERT_FALSE(callback_value);
        called_back = false;

        pin.write_pin(true);
        delayMicroseconds(10);
        TEST_ASSERT_TRUE(called_back);
        TEST_ASSERT_TRUE(callback_value);
    }

    static void destructor_releases_callback() {
        // GIVEN I've called on_edge on a pin
        auto* pin = new common::hardware::ArduinoPin(TEST_PIN);
        the_pin = pin;
        pin->on_edge(callback);

        // WHEN I destroy the pin
        delete(pin);

        // THEN the interrupt handler is removed
        digitalWrite(TEST_PIN, LOW);
        delayMicroseconds(10);
        TEST_ASSERT_FALSE(called_back);
    }

    static void destructor_does_not_detach_interrupt_if_it_was_never_attached() {
        // GIVEN something else has attached an interrupt handler to this pin
        attachInterrupt(digitalPinToInterrupt(TEST_PIN), callback, CHANGE);
        auto* pin = new common::hardware::ArduinoPin(TEST_PIN);

        // WHEN I destroy the pin
        delete(pin);

        // THEN the interrupt handler is still registered
        digitalWrite(TEST_PIN, LOW);
        delayMicroseconds(10);
        TEST_ASSERT_TRUE(called_back);
    }

    // Include all the tests here
    void test() final {
        UNITY_BEGIN();

        RUN_TEST(read_line);
        RUN_TEST(write_pin);
        RUN_TEST(on_edge_registers_callback);
        RUN_TEST(destructor_releases_callback);
        RUN_TEST(destructor_does_not_detach_interrupt_if_it_was_never_attached);

        UNITY_END();
    }
};

common::hardware::ArduinoPin* ArduinoPinTest::the_pin;
bool ArduinoPinTest::called_back;
bool ArduinoPinTest::callback_value;

}
}
#endif //I2C_UNDERNEATH_COMMON_HARDWARE_ARDUINO_PINT_EST_H
