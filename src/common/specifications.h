// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_SPECIFICATIONS_H
#define I2C_UNDERNEATH_COMMON_SPECIFICATIONS_H

// All data is taken from the I2C Specification, Rev 6, Section 6

#include <cstdint>

namespace common {

// The SMBus specification requires devices to reset themselves
// if either SCL or SDA is held down for this amount of time.
#define SMBUS_TIMEOUT_MILLIS 35

// Times are in nanoseconds unless stated otherwise
struct Times {
    uint32_t min_output_fall_time;  // t_of - output fall time from V_IHmin to V_ILmax
    uint32_t max_output_fall_time;  //
    uint32_t min_spike_width;       // t_SP - pulse width of spikes that must be suppressed by the input filter
    uint32_t max_spike_width;       //
    uint32_t min_bus_free_time;     // t_BUF - minimum bus free time between a STOP and START condition
};

struct I2CParameters {
    Times times;
};

const I2CParameters StandardMode = {
    .times = {
        .min_output_fall_time = 0,
        .max_output_fall_time = 250'000,
        .min_spike_width = 0,
        .max_spike_width = 0,
        .min_bus_free_time = 4'700
    }
};

}
#endif //I2C_UNDERNEATH_COMMON_SPECIFICATIONS_H
