#include <unity.h>
#include <Arduino.h>

void setUpExample() {
    // set stuff up here
    Serial.println("Setup Example");
}

void tearDownExample() {
    // clean stuff up here
}

void test_example_one() {
    TEST_ASSERT_EQUAL(5, 5);
}

void test_example_two() {
    TEST_ASSERT_EQUAL(10, 1);
}

// Include all the tests here
void test_example() {
    UNITY_BEGIN();

    RUN_TEST(test_example_one);
    RUN_TEST(test_example_two);

    UNITY_END();
}
