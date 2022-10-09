// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_SUPER_FAST_IO_TEST_H
#define I2C_UNDERNEATH_SUPER_FAST_IO_TEST_H
#include <unity.h>
#include <Arduino.h>
#include <common/hal/teensy/super_fast_io.h>
#include "utils/test_suite.h"

namespace common {
namespace hal {

class SuperFastIoTest : public TestSuite {
public:
    static const uint32_t FAST_TO_SLOW_GPIO_OFFSET = IMXRT_GPIO6_ADDRESS - IMXRT_GPIO1_ADDRESS;
    static void canGetFastGpioBlock_constPath() {
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(0), getGPIO(0));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(1), getGPIO(1));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(2), getGPIO(2));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(3), getGPIO(3));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(4), getGPIO(4));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(5), getGPIO(5));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(6), getGPIO(6));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(7), getGPIO(7));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(8), getGPIO(8));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(9), getGPIO(9));

        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(10), getGPIO(10));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(11), getGPIO(11));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(12), getGPIO(12));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(13), getGPIO(13));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(14), getGPIO(14));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(15), getGPIO(15));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(16), getGPIO(16));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(17), getGPIO(17));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(18), getGPIO(18));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(19), getGPIO(19));

        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(20), getGPIO(20));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(21), getGPIO(21));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(22), getGPIO(22));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(23), getGPIO(23));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(24), getGPIO(24));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(25), getGPIO(25));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(26), getGPIO(26));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(27), getGPIO(27));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(28), getGPIO(28));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(29), getGPIO(29));

        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(30), getGPIO(30));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(31), getGPIO(31));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(32), getGPIO(32));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(33), getGPIO(33));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(34), getGPIO(34));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(35), getGPIO(35));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(36), getGPIO(36));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(37), getGPIO(37));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(38), getGPIO(38));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(39), getGPIO(39));

#ifdef ARDUINO_TEENSY41
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(40), getGPIO(40));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(41), getGPIO(41));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(42), getGPIO(42));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(43), getGPIO(43));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(44), getGPIO(44));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(45), getGPIO(45));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(46), getGPIO(46));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(47), getGPIO(47));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(48), getGPIO(48));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(49), getGPIO(49));

        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(50), getGPIO(50));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(51), getGPIO(51));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(52), getGPIO(52));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(53), getGPIO(53));
        TEST_ASSERT_EQUAL((IMXRT_GPIO_t *)portOutputRegister(54), getGPIO(54));
#endif
    }

    static void canGetFastGpioBlock_nonConstPath() {
        for (int i = 0; i < CORE_NUM_DIGITAL; ++i) {
            auto expected = (IMXRT_GPIO_t *)portOutputRegister(i);
            TEST_ASSERT_EQUAL(expected, getGPIO(i));
        }
    }

    static void canGetSlowGpioBlock_constPath() {
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(0));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(1));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(2));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(3));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(4));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(5));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(6));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(7));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(8));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(9));

        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(10));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(11));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(12));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(13));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(14));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(15));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(16));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(17));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(18));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(19));

        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(20));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(21));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(22));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(23));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(24));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(25));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(26));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(27));
        TEST_ASSERT_EQUAL(IMXRT_GPIO3_ADDRESS, getSlowGPIO(28));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(29));

        TEST_ASSERT_EQUAL(IMXRT_GPIO3_ADDRESS, getSlowGPIO(30));
        TEST_ASSERT_EQUAL(IMXRT_GPIO3_ADDRESS, getSlowGPIO(31));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(32));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(33));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(34));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(35));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(36));
        TEST_ASSERT_EQUAL(IMXRT_GPIO2_ADDRESS, getSlowGPIO(37));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(38));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(39));
#ifdef ARDUINO_TEENSY41
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(40));
        TEST_ASSERT_EQUAL(IMXRT_GPIO1_ADDRESS, getSlowGPIO(41));
        TEST_ASSERT_EQUAL(IMXRT_GPIO3_ADDRESS, getSlowGPIO(42));
        TEST_ASSERT_EQUAL(IMXRT_GPIO3_ADDRESS, getSlowGPIO(43));
        TEST_ASSERT_EQUAL(IMXRT_GPIO3_ADDRESS, getSlowGPIO(44));
        TEST_ASSERT_EQUAL(IMXRT_GPIO3_ADDRESS, getSlowGPIO(45));
        TEST_ASSERT_EQUAL(IMXRT_GPIO3_ADDRESS, getSlowGPIO(46));
        TEST_ASSERT_EQUAL(IMXRT_GPIO3_ADDRESS, getSlowGPIO(47));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(48));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(49));

        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(50));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(51));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(52));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(53));
        TEST_ASSERT_EQUAL(IMXRT_GPIO4_ADDRESS, getSlowGPIO(54));
#endif
    }

    static void canGetSlowGpioBlock_nonConstPath() {
        for (int i = 0; i < CORE_NUM_DIGITAL; ++i) {
            auto expected = (IMXRT_GPIO_t *)((uint32_t)portOutputRegister(i) - FAST_TO_SLOW_GPIO_OFFSET);
            TEST_ASSERT_EQUAL(expected, getSlowGPIO(i));
        }
    }

    static void canGetSlowIRQ() {
        TEST_ASSERT_EQUAL(IRQ_GPIO1_0_15, getSlowIRQ(0));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_0_15, getSlowIRQ(1));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_0_15, getSlowIRQ(2));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_0_15, getSlowIRQ(3));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_0_15, getSlowIRQ(4));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_0_15, getSlowIRQ(5));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_0_15, getSlowIRQ(6));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_16_31, getSlowIRQ(7));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_16_31, getSlowIRQ(8));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_0_15, getSlowIRQ(9));

        TEST_ASSERT_EQUAL(IRQ_GPIO2_0_15, getSlowIRQ(10));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_0_15, getSlowIRQ(11));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_0_15, getSlowIRQ(12));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_0_15, getSlowIRQ(13));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(14));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(15));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(16));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(17));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(18));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(19));

        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(20));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(21));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(22));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(23));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_0_15, getSlowIRQ(24));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_0_15, getSlowIRQ(25));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(26));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(27));
        TEST_ASSERT_EQUAL(IRQ_GPIO3_16_31, getSlowIRQ(28));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_16_31, getSlowIRQ(29));

        TEST_ASSERT_EQUAL(IRQ_GPIO3_16_31, getSlowIRQ(30));
        TEST_ASSERT_EQUAL(IRQ_GPIO3_16_31, getSlowIRQ(31));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_0_15, getSlowIRQ(32));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_0_15, getSlowIRQ(33));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_16_31, getSlowIRQ(34));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_16_31, getSlowIRQ(35));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_16_31, getSlowIRQ(36));
        TEST_ASSERT_EQUAL(IRQ_GPIO2_16_31, getSlowIRQ(37));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(38));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(39));
#ifdef ARDUINO_TEENSY41
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(40));
        TEST_ASSERT_EQUAL(IRQ_GPIO1_16_31, getSlowIRQ(41));
        TEST_ASSERT_EQUAL(IRQ_GPIO3_0_15, getSlowIRQ(42));
        TEST_ASSERT_EQUAL(IRQ_GPIO3_0_15, getSlowIRQ(43));
        TEST_ASSERT_EQUAL(IRQ_GPIO3_0_15, getSlowIRQ(44));
        TEST_ASSERT_EQUAL(IRQ_GPIO3_0_15, getSlowIRQ(45));
        TEST_ASSERT_EQUAL(IRQ_GPIO3_16_31, getSlowIRQ(46));
        TEST_ASSERT_EQUAL(IRQ_GPIO3_16_31, getSlowIRQ(47));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_16_31, getSlowIRQ(48));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_16_31, getSlowIRQ(49));

        TEST_ASSERT_EQUAL(IRQ_GPIO4_16_31, getSlowIRQ(50));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_16_31, getSlowIRQ(51));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_16_31, getSlowIRQ(52));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_16_31, getSlowIRQ(53));
        TEST_ASSERT_EQUAL(IRQ_GPIO4_16_31, getSlowIRQ(54));
#endif
    }

    // Include all the tests here
    void test() final {
        RUN_TEST(canGetFastGpioBlock_constPath);
        RUN_TEST(canGetFastGpioBlock_nonConstPath);
        RUN_TEST(canGetSlowGpioBlock_constPath);
        RUN_TEST(canGetSlowGpioBlock_nonConstPath);
        RUN_TEST(canGetSlowIRQ);
    }

    SuperFastIoTest() : TestSuite(__FILE__) {};
};

}
}

#endif //I2C_UNDERNEATH_SUPER_FAST_IO_TEST_H
