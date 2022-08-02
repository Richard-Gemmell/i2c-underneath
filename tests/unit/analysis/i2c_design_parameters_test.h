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
        SlaveDesignParameters params(123, 555);

        TEST_ASSERT_EQUAL(123, params.data_hold_time);
        TEST_ASSERT_EQUAL(555, params.data_setup_time);
    }

    static void slave_static_initialiser() {
        SlaveDesignParameters params = {
            123, // data_hold_time
            555, // data_setup_time
        };

        TEST_ASSERT_EQUAL(123, params.data_hold_time);
        TEST_ASSERT_EQUAL(555, params.data_setup_time);
    }

    static void master_design_parameters_ctor() {
        MasterDesignParameters params(1, 2, 3, 4, 5, 6, 7, 8);

        TEST_ASSERT_EQUAL(1, params.clock_frequency);
        TEST_ASSERT_EQUAL(2, params.start_hold_time);
        TEST_ASSERT_EQUAL(3, params.scl_low_time);
        TEST_ASSERT_EQUAL(4, params.scl_high_time);
        TEST_ASSERT_EQUAL(5, params.data_hold_time);
        TEST_ASSERT_EQUAL(6, params.data_setup_time);
        TEST_ASSERT_EQUAL(7, params.stop_setup_time);
        TEST_ASSERT_EQUAL(8, params.bus_free_time);
    }

    static void master_static_initialiser() {
        MasterDesignParameters params = {
                1, // clock_frequency
                2, // start_hold_time
                3, // scl_low_time
                4, // scl_high_time
                5, // data_hold_time
                6, // data_setup_time
                7, // stop_setup_time
                8, // bus_free_time
        };

        TEST_ASSERT_EQUAL(1, params.clock_frequency);
        TEST_ASSERT_EQUAL(2, params.start_hold_time);
        TEST_ASSERT_EQUAL(3, params.scl_low_time);
        TEST_ASSERT_EQUAL(4, params.scl_high_time);
        TEST_ASSERT_EQUAL(5, params.data_hold_time);
        TEST_ASSERT_EQUAL(6, params.data_setup_time);
        TEST_ASSERT_EQUAL(7, params.stop_setup_time);
        TEST_ASSERT_EQUAL(8, params.bus_free_time);
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
