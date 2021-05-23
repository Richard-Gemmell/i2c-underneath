// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_HAL_TIMER_H
#define I2C_UNDERNEATH_COMMON_HAL_TIMER_H

#include <cstdint>
#include <functional>

namespace common {
namespace hal {

class Timer {
public:
    // Frees any hardware resources controlled by the timer
    virtual ~Timer() = default;

    // Triggers the callback at regular intervals
    virtual void begin_micros(const std::function<void()>& callback, uint32_t period) = 0;

    // Resets the timer. Equivalent to calling 'end()' and 'begin()' with the same
    // period as before.
    virtual void restart() = 0;

    // Stops the timer and frees any hardware resources it controls
    virtual void end() = 0;
};

}
}
#endif //I2C_UNDERNEATH_COMMON_HAL_TIMER_H
