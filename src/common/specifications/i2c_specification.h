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
    TimeRange start_hold_time;      // tHD;STA - hold time for a START or repeated START condition
    TimeRange scl_low_time;         // tLOW - LOW period of the SCL clock
    TimeRange scl_high_time;        // tHIGH - HIGH period of the SCL clock
//    TimeRange repeated_start_setup_time; // tSU;STA - setup time for a repeated START condition
    TimeRange data_hold_time;       // tHD;DAT - time during which SDA holds its value after a clock pulse
    TimeRange data_setup_time;      // tSU;DAT - data setup time
    TimeRange rise_time;            // tr - rise time of both SDA and SCL signals
    TimeRange fall_time;            // tr - fall time of both SDA and SCL signals
    TimeRange stop_setup_time;      // tSU;STO - setup time for STOP condition
    TimeRange bus_free_time;        // tBUF - minimum bus free time between a STOP and START condition
    // tVD;DAT - time before SDA changes state after a clock pulse.
    //           Valid time is equal to hold time + tf or tr depending on whether SDA is falling or rising.
    //           This is the value that we can actually measure with BusRecorder.
    //           As tHD:DAT has no max value and tVD;DAT has no min value I've merged them
    //           together into data_hold_time.
    // tVD;ACK - data valid time for an ACK
};

struct I2CParameters {
    Times times;
};

const I2CParameters StandardMode = {
        .times = {
                .output_fall_time = {.min = 0, .max = 250},
                .spike_width = {.min = 0, .max = 0},    // Not applicable for standard mode
                .start_hold_time = {.min = 4'000, .max = UINT32_MAX},
                .scl_low_time = {.min = 4'700, .max = UINT32_MAX},
                .scl_high_time = {.min = 4'000, .max = UINT32_MAX},
//                .repeated_start_setup_time = {.min = 4'700, .max = UINT32_MAX},
                .data_hold_time = {.min = 300, .max = 3'450},
                .data_setup_time = {.min = 250, .max = UINT32_MAX},
                .rise_time = {.min = 0, .max = 1'000},
                .fall_time = {.min = 0, .max = 300},
                .stop_setup_time = {.min = 4'000, .max = UINT32_MAX},
                .bus_free_time = {.min = 4'700, .max = UINT32_MAX},
        }
};

const I2CParameters FastMode = {
        .times = {
                .output_fall_time = {.min = 12, .max = 250},
                .spike_width = {.min = 0, .max = 50},
                .start_hold_time = {.min = 600, .max = UINT32_MAX},
                .scl_low_time = {.min = 1'300, .max = UINT32_MAX},
                .scl_high_time = {.min = 600, .max = UINT32_MAX},
//                .repeated_start_setup_time = {.min = 600, .max = UINT32_MAX},
                .data_hold_time = {.min = 300, .max = 950},
                .data_setup_time = {.min = 100, .max = UINT32_MAX},
                .rise_time = {.min = 0, .max = 300},
                .fall_time = {.min = 12, .max = 300},
                .stop_setup_time = {.min = 600, .max = UINT32_MAX},
                .bus_free_time = {.min = 1'300, .max = UINT32_MAX},
        }
};

const I2CParameters FastModePlus = {
        .times = {
                .output_fall_time = {.min = 12, .max = 120},
                .spike_width = {.min = 0, .max = 50},
                .start_hold_time = {.min = 260, .max = UINT32_MAX},
                .scl_low_time = {.min = 500, .max = UINT32_MAX},
                .scl_high_time = {.min = 260, .max = UINT32_MAX},
//                .repeated_start_setup_time = {.min = 260, .max = UINT32_MAX},
                .data_hold_time = {.min = 0, .max = 450},
                .data_setup_time = {.min = 50, .max = UINT32_MAX},
                .rise_time = {.min = 0, .max = 120},
                .fall_time = {.min = 12, .max = 120},
                .stop_setup_time = {.min = 260, .max = UINT32_MAX},
                .bus_free_time = {.min = 500, .max = UINT32_MAX},
        }
};

}
}
#endif //I2C_UNDERNEATH_COMMON_SPECIFICATIONS_I2C_SPECIFICATION_H
