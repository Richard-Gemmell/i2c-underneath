#include <unity.h>
#include <unity_internals.h>
#include "example/example.h"
#include "test_two/test_two.h"

enum TestSuite {
    Example,
    Two
};
TestSuite test_suite;

// Called before each test.
void setUp(void) {
    if(test_suite == TestSuite::Example) {
        setUpExample();
    }
    if(test_suite == TestSuite::Two) {
        setUpTwo();
    }
}

// Called after each test.
void tearDown(void) {
    if(test_suite == TestSuite::Example) {
        tearDownExample();
    }
    if(test_suite == TestSuite::Two) {
        tearDownTwo();
    }
}

// Blink the LED to make sure the Teensy hasn't hung
IntervalTimer blink_timer;
volatile bool led_high = false;
void blink_isr();

void process() {
    // Run each test suite in succession.
    test_suite = TestSuite::Example;
    test_example();

    test_suite = TestSuite::Two;
    test_two();
}

#include <Arduino.h>
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
