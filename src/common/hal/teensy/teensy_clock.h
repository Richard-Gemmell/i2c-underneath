// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#pragma once
#include <Arduino.h>
#include <cstdint>
#include <imxrt.h>
#include <common/hal/clock.h>
#include <common/hal/teensy/teensy_timestamp.h>

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

    uint32_t ticks_to_nanos(uint32_t ticks) const override {
        return TeensyTimestamp::ticks_to_nanos(ticks);
    }

    inline uint32_t nanos_between(uint32_t ticks_start, uint32_t ticks_end) const override {
        return TeensyTimestamp::nanos_between(ticks_start, ticks_end);
    }

    inline uint32_t nanos_since(uint32_t& ticks_start) const override {
        uint32_t past = ticks_start;
        ticks_start = ARM_DWT_CYCCNT;
        return TeensyTimestamp::nanos_between(past, ticks_start);
    }
};

} // common
} // hal
