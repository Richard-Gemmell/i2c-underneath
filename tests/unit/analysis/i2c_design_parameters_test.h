// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_I2C_DESIGN_PARAMETERS_TEST_H
#define I2C_UNDERNEATH_I2C_DESIGN_PARAMETERS_TEST_H
#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include <analysis/i2c_design_parameters.h>

namespace analysis {
class I2CDesignParametersTest : public TestSuite {
public:
    static void slave_design_parameters_ctor() {
        SlaveDesignParameters params({123, 456}, {555, 666});

        TEST_ASSERT_EQUAL(123, params.data_hold_time.min);
        TEST_ASSERT_EQUAL(456, params.data_hold_time.max);
        TEST_ASSERT_EQUAL(555, params.data_setup_time.min);
        TEST_ASSERT_EQUAL(666, params.data_setup_time.max);
    }

    static void slave_static_initialiser() {
        SlaveDesignParameters params = {
                {123, 456}, // data_hold_time
                {555, 666}, // data_setup_time
        };

        TEST_ASSERT_EQUAL(123, params.data_hold_time.min);
        TEST_ASSERT_EQUAL(456, params.data_hold_time.max);
        TEST_ASSERT_EQUAL(555, params.data_setup_time.min);
        TEST_ASSERT_EQUAL(666, params.data_setup_time.max);
    }

    static void master_design_parameters_ctor() {
        MasterDesignParameters params(
                {1, 11},
                {2, 22},
                {3, 33},
                {4, 44},
                {5, 55},
                {6, 66},
                {7, 77},
                {8, 88});

        TEST_ASSERT_EQUAL(1, params.clock_frequency.min);
        TEST_ASSERT_EQUAL(11, params.clock_frequency.max);
        TEST_ASSERT_EQUAL(2, params.start_hold_time.min);
        TEST_ASSERT_EQUAL(22, params.start_hold_time.max);
        TEST_ASSERT_EQUAL(3, params.scl_low_time.min);
        TEST_ASSERT_EQUAL(33, params.scl_low_time.max);
        TEST_ASSERT_EQUAL(4, params.scl_high_time.min);
        TEST_ASSERT_EQUAL(44, params.scl_high_time.max);
        TEST_ASSERT_EQUAL(5, params.data_hold_time.min);
        TEST_ASSERT_EQUAL(55, params.data_hold_time.max);
        TEST_ASSERT_EQUAL(6, params.data_setup_time.min);
        TEST_ASSERT_EQUAL(66, params.data_setup_time.max);
        TEST_ASSERT_EQUAL(7, params.stop_setup_time.min);
        TEST_ASSERT_EQUAL(77, params.stop_setup_time.max);
        TEST_ASSERT_EQUAL(8, params.bus_free_time.min);
        TEST_ASSERT_EQUAL(88, params.bus_free_time.max);
    }

    static void master_static_initialiser() {
        MasterDesignParameters params = {
                {1, 11}, // clock_frequency
                {2, 22}, // start_hold_time
                {3, 33}, // scl_low_time
                {4, 44}, // scl_high_time
                {5, 55}, // data_hold_time
                {6, 66}, // data_setup_time
                {7, 77}, // stop_setup_time
                {8, 88}, // bus_free_time
        };

        TEST_ASSERT_EQUAL(1, params.clock_frequency.min);
        TEST_ASSERT_EQUAL(11, params.clock_frequency.max);
        TEST_ASSERT_EQUAL(2, params.start_hold_time.min);
        TEST_ASSERT_EQUAL(22, params.start_hold_time.max);
        TEST_ASSERT_EQUAL(3, params.scl_low_time.min);
        TEST_ASSERT_EQUAL(33, params.scl_low_time.max);
        TEST_ASSERT_EQUAL(4, params.scl_high_time.min);
        TEST_ASSERT_EQUAL(44, params.scl_high_time.max);
        TEST_ASSERT_EQUAL(5, params.data_hold_time.min);
        TEST_ASSERT_EQUAL(55, params.data_hold_time.max);
        TEST_ASSERT_EQUAL(6, params.data_setup_time.min);
        TEST_ASSERT_EQUAL(66, params.data_setup_time.max);
        TEST_ASSERT_EQUAL(7, params.stop_setup_time.min);
        TEST_ASSERT_EQUAL(77, params.stop_setup_time.max);
        TEST_ASSERT_EQUAL(8, params.bus_free_time.min);
        TEST_ASSERT_EQUAL(88, params.bus_free_time.max);
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(slave_design_parameters_ctor);
        RUN_TEST(slave_static_initialiser);
        RUN_TEST(master_design_parameters_ctor);
        RUN_TEST(master_static_initialiser);
    }

    I2CDesignParametersTest() : TestSuite(__FILE__) {};
};
} // analysis
#endif //I2C_UNDERNEATH_I2C_DESIGN_PARAMETERS_TEST_H
