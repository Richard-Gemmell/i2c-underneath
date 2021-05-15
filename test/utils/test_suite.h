// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_TEST_TEST_SUITE_H
#define I2C_UNDERNEATH_TEST_TEST_SUITE_H

class TestSuite {
public:
    virtual ~TestSuite() = default;

    // Called before each test
    virtual void setUp() {};

    // Called after each test
    virtual void tearDown() {};

    // Executes the tests in the suite
    virtual void test() = 0;
};

#endif //I2C_UNDERNEATH_TEST_TEST_SUITE_H
