#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"

// Unit Tests
//#include "example/example.h"
#include "unit_tests/bus_monitor/bus_monitor_test.h"

// End to End Tests
#include "end_to_end/common/hal/arduino/arduino_pin_test.h"
#include "end_to_end/common/hal/teensy/teensy_timer_test.h"

TestSuite* test_suite;
void test(TestSuite* suite);
void report_overall_results();
UNITY_COUNTER_TYPE total_tests = 0;
UNITY_COUNTER_TYPE total_ignores = 0;
UNITY_COUNTER_TYPE total_failures = 0;

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
}

void test(TestSuite* suite) {
    UNITY_BEGIN();
    test_suite = suite;
    UnitySetTestFile(test_suite->get_file_name());
    test_suite->test();
    delete(test_suite);
    total_tests += Unity.NumberOfTests;
    total_failures += Unity.TestFailures;
    total_ignores += Unity.TestIgnores;
    UNITY_END();
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
    delay(2000);
    run_tests();
    report_overall_results();
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

void report_overall_results()
{
    Serial.println("================");
    Serial.println("Combined Results");
    Serial.println("----------------");
    UnityPrintNumber((UNITY_INT)(total_tests));
    Serial.print(" Tests ");
    UnityPrintNumber((UNITY_INT)(total_failures));
    Serial.print(" Failures ");
    UnityPrintNumber((UNITY_INT)(total_ignores));
    Serial.println(" Ignored ");
    if (total_failures == 0U)
    {
        Serial.println("OK");
    }
    else
    {
        Serial.println("FAIL");
    }
    Serial.println();
}

void blink_isr() {
    led_high = !led_high;
    digitalWrite(LED_BUILTIN, led_high);
}
