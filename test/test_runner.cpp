#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"

// Unit Tests
#include "example/example.h"
#include "test_two/test_two.h"

// Full Stack Tests
#include "common/hardware/ArduinoPinTest.h"

TestSuite* test_suite;
void test(TestSuite* suite);

void process() {
    // Run each test suite in succession.

//    Serial.println("Run Unit Tests");
//    Serial.println("--------------");
//    test(new ExampleTestSuite());
//    test(new TestSuiteTwo());

    // Full Stack Tests
    // These tests require working hardware
    Serial.println("Run Full Stack Tests");
    Serial.println("--------------------");
    test(new common::hardware::ArduinoPinTest());
}

void test(TestSuite* suite) {
    test_suite = suite;
    test_suite->test();
    delete(test_suite);
}

// Called before each test.
void setUp(void) {
    test_suite->setUp();
}

// Called after each test.
void tearDown(void) {
    test_suite->tearDown();
}

// Blink the LED to make sure the Teensy hasn't hung
IntervalTimer blink_timer;
volatile bool led_high = false;
void blink_isr();

void setup() {
    // Blink the LED to show we're still alive
    blink_timer.begin(blink_isr, 300'000);

    // You must connect the Serial Monitor before this delay times out
    // otherwise the test output gets truncated in a weird way.
    delay(2000);
    process();
}

int max_pokes = 10;
void loop() {
    delay(1000);
    if (max_pokes-- > 0) {
        // Print a character from time to time to force the serial monitor
        // to refresh in case the user didn't connect the monitor in time.
        Serial.print(" ");
    }
}

void blink_isr() {
    led_high = !led_high;
    digitalWrite(LED_BUILTIN, led_high);
}
