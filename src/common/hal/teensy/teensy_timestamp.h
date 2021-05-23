// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_HAL_TEENSY_TIMESTAMP_H
#define I2C_UNDERNEATH_HAL_TEENSY_TIMESTAMP_H

#include <Arduino.h>
#include "common/hal/timestamp.h"

namespace common {
namespace hal {

class TeensyTimestamp : public Timestamp {
public:
    TeensyTimestamp();

    ~TeensyTimestamp() override = default;

    void reset() override;

    bool timed_out_nanos(uint32_t timeout_in_nanos) override;

    // Converts ticks to nanoseconds. Caps the result to UINT32_MAX if
    // ticks is larger than UINT32_MAX * 0.6.
    static uint32_t ticks_to_nanos(uint32_t ticks);

    // Returns the time between two tick counts in nanoseconds.
    // Handles the case when the tick count wraps round to 0.
    // Uses 'ticks_to_nanos' to convert to nanos.
    static uint32_t nanos_between(uint32_t ticks_start, uint32_t ticks_end);

private:
    uint32_t millis_;
    uint32_t tick_count_;
};

}
}

#endif //I2C_UNDERNEATH_HAL_TEENSY_TIMESTAMP_H
