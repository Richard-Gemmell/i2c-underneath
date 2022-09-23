// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_I2C_TIMING_ANALYSIS_H
#define I2C_UNDERNEATH_I2C_TIMING_ANALYSIS_H

#include <analysis/duration_statistics.h>

namespace analysis {

struct I2CTimingAnalysis {
//    bool well_formed;                       // true if the trace has the correct structure for an I2C message
    DurationStatistics clock_frequency;     // fSCL - SCL clock frequency
    DurationStatistics start_hold_time;     // tHD;STA - hold time for a START or repeated START condition
    DurationStatistics scl_low_time;        // tLOW - LOW period of the SCL clock
    DurationStatistics scl_high_time;       // tHIGH - HIGH period of the SCL clock
    DurationStatistics start_setup_time;    // tSU;STA - set-up time for a repeated START condition
    DurationStatistics data_hold_time;      // tHD;DAT - time during which SDA holds its value after a clock pulse
    DurationStatistics data_setup_time;     // tSU;DAT - data setup time
    DurationStatistics stop_setup_time;     // tSU;STO - setup time for STOP condition
    DurationStatistics bus_free_time;       // tBUF - minimum bus free time between a STOP and START condition
    // tVD;DAT - time before SDA changes state after a clock pulse. Wrapped into data_hold_time
    // tVD;ACK - data valid acknowledge time. Wrapped into data_hold_time
};

} // analysis

#endif //I2C_UNDERNEATH_I2C_TIMING_ANALYSIS_H
