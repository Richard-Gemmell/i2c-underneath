#include <unity.h>
#include <Arduino.h>

void setUpTwo() {
    // set stuff up here
    Serial.println("Setup Two");
}

void tearDownTwo() {
    // clean stuff up here
}

void test_two_a() {
    TEST_ASSERT_EQUAL(2, 1);
}

void test_two_b() {
    TEST_ASSERT_EQUAL(1, 1);
}

// Include all the tests here
void test_two() {
    UNITY_BEGIN();

    RUN_TEST(test_two_a);
    RUN_TEST(test_two_b);

    UNITY_END();
}
