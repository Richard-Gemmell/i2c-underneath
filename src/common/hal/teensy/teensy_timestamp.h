// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#pragma once
#include <Arduino.h>
#include "common/hal/timestamp.h"

namespace common {
namespace hal {

const double nanos_per_tick = (1'000'000'000.0 / F_CPU_ACTUAL);

class TeensyTimestamp : public Timestamp {
public:
    TeensyTimestamp();

    ~TeensyTimestamp() override = default;

    void reset() override;

    bool timed_out_nanos(uint32_t timeout_in_nanos) override;

    // Converts ticks to nanoseconds. Caps the result to UINT32_MAX if
    // ticks is larger than UINT32_MAX * 0.6.
    inline static uint32_t ticks_to_nanos(uint32_t ticks) {
        // It's about 15% faster to cast the result to a uint32_t than to return a double
        // This is very similar for the time take to do it in uin32_t arithmetic but
        // without the rounding problems
        // About  35 nanos per call for this version.
        // About 137 nanos per call for a version that uses uin64_t arithmetic
        return (uint32_t)(nanos_per_tick * ticks);
    }

    // Converts nanoseconds to ticks.
    static uint32_t nanos_to_ticks(uint32_t nanos);

    // Returns the time between two tick counts in nanoseconds.
    // Handles the case when the tick count wraps round to 0.
    // Uses 'ticks_to_nanos' to convert to nanos.
    inline static uint32_t nanos_between(uint32_t ticks_start, uint32_t ticks_end) {
        return ticks_to_nanos(ticks_end - ticks_start);
    }

private:
    uint32_t millis_;
    uint32_t tick_count_;
};

}
}
