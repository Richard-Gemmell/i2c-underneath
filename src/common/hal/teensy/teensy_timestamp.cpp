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

const double ticks_per_nano = F_CPU_ACTUAL / 1'000'000'000.0;

uint32_t common::hal::TeensyTimestamp::nanos_to_ticks(uint32_t nanos) {
    return (uint32_t)(ticks_per_nano * nanos);
}
