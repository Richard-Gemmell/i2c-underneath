#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"

// Unit Tests
//#include "example/example.h"
#include "unit/bus_monitor/bus_monitor_test.h"

// End to End Tests
#include "e2e/common/hal/arduino/arduino_pin_test.h"
#include "e2e/common/hal/teensy/teensy_timer_test.h"
#include "e2e/common/hal/teensy/teensy_timestamp_test.h"

TestSuite* test_suite;
void test(TestSuite* suite);
void report_test_results();
void report_overall_results();
UNITY_COUNTER_TYPE tests_run = 0;
UNITY_COUNTER_TYPE tests_ignored = 0;
UNITY_COUNTER_TYPE tests_failed = 0;

void run_tests() {
    // Run each test suite in succession.

    // Unit tests run an any board layout
    Serial.println("Run Unit Tests");
    Serial.println("--------------");
//    test(new ExampleTestSuite());
    test(new bus_monitor::BusMonitorTest());

    // Full Stack Tests
    // These tests require working hardware
    Serial.println("Run Full Stack Tests");
    Serial.println("--------------------");
    test(new common::hal::ArduinoPinTest());
    test(new common::hal::TeensyTimerTest());
    test(new common::hal::TeensyTimestampTest());
}

void test(TestSuite* suite) {
    test_suite = suite;
    UnitySetTestFile(test_suite->get_file_name());
    test_suite->test();
    delete(test_suite);
    tests_run = Unity.NumberOfTests - tests_run;
    tests_failed = Unity.TestFailures - tests_failed;
    tests_ignored = Unity.TestIgnores - tests_ignored;
    report_test_results();
    Serial.println("");
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
    delay(1000);
    UNITY_BEGIN();
    run_tests();
    UNITY_END();
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

// Equivalent to UNITY_END() except it doesn't halt the test runner.
void report_test_results() {
    Serial.println("----------------------");
    Serial.print(tests_run);
    Serial.print(" Executed "); // DON'T say "Tests" here. It'll stop the test suite running.
    Serial.print(tests_failed);
    Serial.print(" Failures ");
    Serial.print(tests_ignored);
    Serial.println(" Ignored ");
    if (tests_failed == 0U) {
        Serial.println("OK");
    } else {
        Serial.println("FAIL");
    }
    Serial.println(".");
}

void blink_isr() {
    led_high = !led_high;
    digitalWrite(LED_BUILTIN, led_high);
}
