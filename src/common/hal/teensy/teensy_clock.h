// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_TEENSY_CLOCK_H
#define I2C_UNDERNEATH_TEENSY_CLOCK_H

#include <Arduino.h>
#include <cstdint>
#include <imxrt.h>
#include <common/hal/clock.h>

namespace common {
namespace hal {

// This implementation is as fast as calling the hardware
// specific functions directly.
class TeensyClock : public Clock {
public:
    inline uint32_t get_system_tick() const override {
        return ARM_DWT_CYCCNT;
    }

    inline uint32_t get_system_mills() const override {
        return millis();
    }

    uint32_t nanos_between(uint32_t ticks_start, uint32_t ticks_end) const override {
        return TeensyTimestamp::nanos_between(ticks_start, ticks_end);
    }
};

} // common
} // hal

#endif //I2C_UNDERNEATH_TEENSY_CLOCK_H
