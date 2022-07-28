// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_FAKE_CLOCK_H
#define I2C_UNDERNEATH_FAKE_CLOCK_H

#include <Arduino.h>
#include <cstdint>
#include <imxrt.h>
#include <common/hal/clock.h>

namespace common {
namespace hal {

class FakeClock : public Clock {
public:
    uint32_t system_tick = 1'000'000;
    uint32_t system_millis = 5'000;

    inline uint32_t get_system_tick() const override {
        return system_tick;
    }

    inline uint32_t get_system_mills() const override {
        return system_millis;
    }

    uint32_t nanos_between(uint32_t ticks_start, uint32_t ticks_end) const override {
        return (ticks_end - ticks_start)*2;
    }

    uint32_t nanos_since(uint32_t& ticks_start) const override {
        uint32_t past = ticks_start;
        ticks_start = system_tick;
        return nanos_between(past, ticks_start);
    }
};

}
}

#endif //I2C_UNDERNEATH_FAKE_CLOCK_H
