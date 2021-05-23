// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_HAL_TIMESTAMP_H
#define I2C_UNDERNEATH_COMMON_HAL_TIMESTAMP_H

#include <cstdint>

namespace common {
namespace hal {

// Records a point in time. The timestamp is relative to the time when
// the device started or when the timestamp clock wrapped around whichever
// is most recent.
//
// Timestamps wrap around after a certain amount of time. The wraparound
// time is platform dependent but will should be more than 40 days.
//
// Methods that compare timestamps assume that they are within half of
// the wraparound period.
//
// The precision is 1 clock tick or 1 nanosecond; whichever is worse.
// For example, a 100 MHz processor will have a precision of 1 clock tick
// which is 10 nanos. A 10 GHz processor will have a precision of 1 nano.
class Timestamp {
public:
    // Creates a new timestamp
    Timestamp() = default;

    virtual ~Timestamp() = default;

    // Updates this timestamp. Equivalent to swapping this timestamp for a new one.
    virtual void reset() = 0;

    // true if 'timeout_in_nanos' have passed since this timestamp was created
    virtual bool timed_out_nanos(uint32_t timeout_in_nanos) = 0;
};

}
}

#endif //I2C_UNDERNEATH_COMMON_HAL_TIMESTAMP_H
