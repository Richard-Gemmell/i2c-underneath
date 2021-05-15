#ifndef I2C_UNDERNEATH_TEST_EXAMPLE_TEST_SUITE_H
#define I2C_UNDERNEATH_TEST_EXAMPLE_TEST_SUITE_H

#include <unity.h>
#include <Arduino.h>
#include "../utils/test_suite.h"

class ExampleTestSuite : public TestSuite {
    // We can't access the 'this' pointer in the test methods
    // so all test data has to be static.
    static int value;

public:
    void setUp() final {
        Serial.println("Set Up Example");
        value = 5;
    }

    void tearDown() final {
        Serial.println();
        Serial.println("Tear Down Example");
        value = 0;
    }

    static void test_example_one() {
        TEST_ASSERT_EQUAL(5, value);
    }

    static void test_example_two() {
        TEST_ASSERT_EQUAL(10, 1);
    }

    // Include all the tests here
    void test() final {
        UNITY_BEGIN();

        RUN_TEST(test_example_one);
        RUN_TEST(test_example_two);

        UNITY_END();
    }
};

// Define statics
int ExampleTestSuite::value;

#endif  //I2C_UNDERNEATH_TEST_EXAMPLE_TEST_SUITE_H