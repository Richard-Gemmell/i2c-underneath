#ifndef I2C_UNDERNEATH_TEENSY_PIN_E2E_TEST_H
#define I2C_UNDERNEATH_TEENSY_PIN_E2E_TEST_H

#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include "e2e/e2e_test_base.h"
#include "common/hal/teensy/teensy_pin.h"

namespace common {
namespace hal {

class TeensyPinTest : public e2e::E2ETestBase {
public:
    static void test_set() {
        common::hal::TeensyPin pin(PIN_DRIVE_SCL, OUTPUT);

        // GIVEN the pin is LOW to start with
        TEST_ASSERT_FALSE(digitalRead(PIN_SNIFF_SCL));

        // WHEN we set the pin
        pin.set();

        // THEN the line goes high
        TEST_ASSERT_TRUE(digitalRead(PIN_SNIFF_SCL));
    }

    static void test_clear() {
        common::hal::TeensyPin pin(PIN_DRIVE_SCL, OUTPUT);
        digitalWriteFast(PIN_DRIVE_SCL, HIGH);

        // GIVEN the pin is HIGH to start with
        TEST_ASSERT_TRUE(digitalRead(PIN_SNIFF_SCL));

        // WHEN we clear the pin
        pin.clear();

        // THEN the line goes high
        TEST_ASSERT_FALSE(digitalRead(PIN_SNIFF_SCL));
    }

    static void test_toggle() {
        common::hal::TeensyPin pin(PIN_DRIVE_SCL, OUTPUT);

        // GIVEN the pin is LOW to start with
        TEST_ASSERT_FALSE(digitalRead(PIN_SNIFF_SCL));

        // WHEN we toggle the pin the first time
        pin.toggle();

        // THEN the line goes high
        TEST_ASSERT_TRUE(digitalRead(PIN_SNIFF_SCL));

        // WHEN we toggle the pin the second time
        pin.toggle();

        // THEN the line goes low
        TEST_ASSERT_FALSE(digitalRead(PIN_SNIFF_SCL));
    }

    static void test_read_input() {
        // GIVEN the pin is configured as an input
        common::hal::TeensyPin pin(PIN_SNIFF_SCL, INPUT);
        // AND the pin is LOW to start with
        pinMode(PIN_DRIVE_SCL, OUTPUT);
        TEST_ASSERT_FALSE(digitalRead(PIN_DRIVE_SCL));

        // WHEN we set the pin HIGH
        digitalWrite(PIN_DRIVE_SCL, HIGH);
        // THEN the line reads high
        TEST_ASSERT_TRUE(pin.read());

        // WHEN we set the pin LOW
        digitalWrite(PIN_DRIVE_SCL, LOW);
        // THEN the line reads low
        TEST_ASSERT_FALSE(pin.read());
    }

    static void test_read_output() {
        // GIVEN the pin is configured as an output
        common::hal::TeensyPin pin(PIN_SNIFF_SCL, OUTPUT);
        // AND the pin is LOW to start with
        pinMode(PIN_DRIVE_SCL, OUTPUT);
        TEST_ASSERT_FALSE(digitalRead(PIN_DRIVE_SCL));

        // WHEN we set the pin HIGH
        pin.set();  // It's actually reading the pad status not the line
        // THEN the line reads high
        TEST_ASSERT_TRUE(pin.read());

        // WHEN we set the pin LOW
        pin.clear();  // It's actually reading the pad status not the line
        // THEN the line reads low
        TEST_ASSERT_FALSE(pin.read());
    }

    static void compare_execution_times() {
        common::hal::TeensyPin pin(PIN_SNIFF_SCL, OUTPUT);
        pin.clear();
        delayNanoseconds(100);

        int iterations = 10;
        uint32_t start = ARM_DWT_CYCCNT;
        for (int i = 0; i < iterations; ++i) {
            pin.set();
            pin.clear();
        }
        double teensyPinTime = (double)TeensyTimestamp::nanos_between(start, ARM_DWT_CYCCNT) / iterations;

//        uint32_t mask = getPortBitmask(PIN_SNIFF_SCL);
//        volatile uint32_t* set_port = getDigitalWritePort(PIN_SNIFF_SCL, true);
//        volatile uint32_t* clear_port = getDigitalWritePort(PIN_SNIFF_SCL, false);
//        start = ARM_DWT_CYCCNT;
//        for (int i = 0; i < iterations; ++i) {
//            *set_port = mask;
//            *clear_port = mask;
//        }
//        double superFastTime = (double)TeensyTimestamp::nanos_between(start, ARM_DWT_CYCCNT) / iterations;
//
//        start = ARM_DWT_CYCCNT;
//        for (int i = 0; i < iterations; ++i) {
//            digitalWriteFast(PIN_SNIFF_SCL, HIGH);
//            digitalWriteFast(PIN_SNIFF_SCL, LOW);
//        }
//        double digitalFastWriteTime = (double)TeensyTimestamp::nanos_between(start, ARM_DWT_CYCCNT) / iterations;

//        Serial.printf("TeensyPin %.1f. SuperFast: %.1f WriteFast %.1f\n", teensyPinTime, superFastTime, digitalFastWriteTime);
        TEST_ASSERT_LESS_OR_EQUAL(9, teensyPinTime);
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(test_set);
        RUN_TEST(test_clear);
        RUN_TEST(test_toggle);
        RUN_TEST(test_read_input);
        RUN_TEST(test_read_output);
        RUN_TEST(compare_execution_times);
    }

    TeensyPinTest() : E2ETestBase(__FILE__) {};
};

}
}

#endif  //I2C_UNDERNEATH_TEENSY_PIN_E2E_TEST_H