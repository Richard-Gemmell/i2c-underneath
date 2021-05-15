#ifndef I2C_UNDERNEATH_TEST_EXAMPLE_TEST_SUITE_TWO_H
#define I2C_UNDERNEATH_TEST_EXAMPLE_TEST_SUITE_TWO_H

#include <unity.h>
#include <Arduino.h>
#include "../utils/test_suite.h"

class TestSuiteTwo : public TestSuite {
public:
    // We can't access the 'this' pointer in the test methods
    // so all test data has to be static.
    //static int some_test_data;

    static void test_two_a() {
        TEST_ASSERT_EQUAL(2, 1);
    }

    static void test_two_b() {
        TEST_ASSERT_EQUAL(1, 1);
    }

    // Include all the tests here
    void test() final {
        UNITY_BEGIN();

        RUN_TEST(test_two_a);
        RUN_TEST(test_two_b);

        UNITY_END();
    }
};

#endif //I2C_UNDERNEATH_TEST_EXAMPLE_TEST_SUITE_TWO_H