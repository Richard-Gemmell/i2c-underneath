// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_I2C_DESIGN_PARAMETERS_H
#define I2C_UNDERNEATH_I2C_DESIGN_PARAMETERS_H

#include <cstdint>

namespace analysis {

using TimeRange = common::i2c_specification::TimeRange;

struct SlaveDesignParameters {
    SlaveDesignParameters(TimeRange data_hold_time, TimeRange data_setup_time)
        : data_hold_time(data_hold_time), data_setup_time(data_setup_time) {
    };

    // Hold time is equal to valid time - tf or tr depending on whether SDA is falling or rising.
    // We can't actually measure this directly.
    TimeRange data_hold_time;   // tHD;DAT - time during which SDA holds its value after a clock pulse
    TimeRange data_setup_time;  // tSU;DAT - data setup time

    // Probably don't need rise & fall times. Leave them out for now.
    // uint32_t rise_time;            // tr - rise time of both SDA and SCL signals
    // uint32_t fall_time;            // tr - fall time of both SDA and SCL signals

    // Valid time is equal to hold time + tf or tr depending on whether SDA is falling or rising.
    // This is what we actually measure with BusTrace.
//     tVD;DAT - time before SDA changes state after a clock pulse.
//     tVD;ACK - data valid acknowledge time. Wrapped into data_hold_time
};

struct MasterDesignParameters : public SlaveDesignParameters {
    MasterDesignParameters(TimeRange clock_frequency,
                           TimeRange start_hold_time,
                           TimeRange scl_low_time,
                           TimeRange scl_high_time,
                           TimeRange data_hold_time,
                           TimeRange data_setup_time,
                           TimeRange stop_setup_time,
                           TimeRange bus_free_time)
        : SlaveDesignParameters(data_hold_time, data_setup_time),
        clock_frequency{clock_frequency}, start_hold_time(start_hold_time),
        scl_low_time(scl_low_time), scl_high_time(scl_high_time),
        stop_setup_time(stop_setup_time), bus_free_time(bus_free_time) {
    }

    TimeRange clock_frequency;      // fSCL - maximum SCL clock frequency
    TimeRange start_hold_time;     // tHD;STA - hold time for a START or repeated START condition
    TimeRange scl_low_time;         // tLOW - LOW period of the SCL clock
    TimeRange scl_high_time;        // tHIGH - HIGH period of the SCL clock
//    uint32_t start_setup_time;     // tSU;STA - set-up time for a repeated START condition
    TimeRange stop_setup_time;      // tSU;STO - setup time for STOP condition
    TimeRange bus_free_time;        // tBUF - minimum bus free time between a STOP and START condition
};
} // analysis

#endif //I2C_UNDERNEATH_I2C_DESIGN_PARAMETERS_H
