// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_CLOCK_H
#define I2C_UNDERNEATH_CLOCK_H

#include <cstdint>

namespace common {
namespace hal {

// A source of time.
// Can be faked for unit testing.
// Not a real time clock.
class Clock {
public:
    virtual ~Clock() = default;

    virtual uint32_t get_system_tick() const = 0;

    virtual uint32_t get_system_mills() const = 0;

    virtual uint32_t nanos_between(uint32_t ticks_start, uint32_t ticks_end) const = 0;
};

}
}
#endif //I2C_UNDERNEATH_CLOCK_H
