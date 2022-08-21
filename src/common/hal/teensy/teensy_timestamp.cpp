// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include <Arduino.h>
#include "teensy_timestamp.h"

common::hal::TeensyTimestamp::TeensyTimestamp()
    : millis_(millis()), tick_count_(ARM_DWT_CYCCNT) {
}

void common::hal::TeensyTimestamp::reset() {
    millis_ = millis();
    tick_count_ = ARM_DWT_CYCCNT;
}

bool common::hal::TeensyTimestamp::timed_out_nanos(uint32_t timeout_in_nanos) {
    uint32_t timeout_millis = timeout_in_nanos / 1'000'000;
    uint32_t delta_millis = millis() - millis_;
    if (delta_millis > timeout_millis) {
        return true;
    } else if (delta_millis < timeout_millis) {
        return false;
    } else {
        // We're on the same millisecond
        // It's down to the nanosecond difference
        return nanos_between(tick_count_, ARM_DWT_CYCCNT) >= timeout_in_nanos;
    }
}

uint32_t common::hal::TeensyTimestamp::nanos_between(uint32_t ticks_start, uint32_t ticks_end) {
    return ticks_to_nanos(ticks_end - ticks_start);
}

const double nanos_per_tick = 1'000'000'000.0 / F_CPU_ACTUAL;

uint32_t common::hal::TeensyTimestamp::ticks_to_nanos(uint32_t ticks) {
    // It's about 15% faster to cast the result to a uint32_t than to return a double
    // This is very similar for the time take to do it in uin32_t arithmetic but
    // without the rounding problems
    // About  35 nanos per call for this version.
    // About 137 nanos per call for a version that uses uin64_t arithmetic
    return (uint32_t)(nanos_per_tick * ticks);
}

const double ticks_per_nano = F_CPU_ACTUAL / 1'000'000'000.0;

uint32_t common::hal::TeensyTimestamp::nanos_to_ticks(uint32_t nanos) {
    return (uint32_t)(ticks_per_nano * nanos);
}
