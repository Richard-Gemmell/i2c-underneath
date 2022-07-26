// Copyright © 2021-2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_SPECIFICATIONS_I2C_SPECIFICATION_H
#define I2C_UNDERNEATH_COMMON_SPECIFICATIONS_I2C_SPECIFICATION_H

// All data is taken from the I2C Specification, Rev 6, Section 6

#include <cstdint>

namespace common {
namespace i2c_specification {

// The SMBus specification requires devices to reset themselves
// if either SCL or SDA is held down for this amount of time.
#define SMBUS_TIMEOUT_MILLIS 35

// Times are in nanoseconds
struct TimeRange {
    uint32_t min;
    uint32_t max;
};

// Times are in nanoseconds
struct Times {
    TimeRange output_fall_time;     // tof - output fall time from V_IHmin to V_ILmax
    TimeRange spike_width;          // tSP - pulse width of spikes that must be suppressed by the input filter
    TimeRange bus_free_time;        // tBUF - minimum bus free time between a STOP and START condition
    TimeRange hold_time;            // tHD;STA - hold time for a START or repeated START condition
    TimeRange scl_low_time;         // tLOW - LOW period of the SCL clock
    TimeRange scl_high_time;        // tHIGH - HIGH period of the SCL clock
    // tSA;STA
    TimeRange data_hold_time;       // tHD;DAT - time during which SDA holds its value after a clock pulse
    // tSU;DAT - data setup time
    TimeRange rise_time;            // tr - rise time of both SDA and SCL signals
    TimeRange fall_time;            // tr - fall time of both SDA and SCL signals
    TimeRange stop_setup_time;      // tSU;STO - setup time for STOP condition
    // tBUF
    // tVD;DAT - time before SDA changes state after a clock pulse. Wrapped into data_hold_time
    // tVD;ACK
};

struct I2CParameters {
    Times times;
};

const I2CParameters StandardMode = {
        .times = {
                .output_fall_time = {.min = 0, .max = 250},
                .spike_width = {.min = 0, .max = 0},    // Not applicable for standard mode
                .bus_free_time = {.min = 4'700, .max = UINT32_MAX},
                .hold_time = {.min = 4'000, .max = UINT32_MAX},
                .scl_low_time = {.min = 4'700, .max = UINT32_MAX},
                .scl_high_time = {.min = 4'000, .max = UINT32_MAX},
//                . = {.min = , .max = },
                // Could be wrong about hold time. Hard to interpret from the spec
                .data_hold_time = {.min = 300, .max = 3'450},
                .rise_time = {.min = 0, .max = 1'000},
                .fall_time = {.min = 0, .max = 300},
                .stop_setup_time = {.min = 4'000, .max = UINT32_MAX},
//                . = {.min = , .max = },
//                . = {.min = , .max = },
        }
};

}
}
#endif //I2C_UNDERNEATH_COMMON_SPECIFICATIONS_I2C_SPECIFICATION_H
