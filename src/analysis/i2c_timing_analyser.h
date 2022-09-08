// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#pragma once
#include <bus_trace/bus_trace.h>
#include <analysis/i2c_timing_analysis.h>

namespace analysis {

class I2CTimingAnalyser {
public:
    static I2CTimingAnalysis analyse(const bus_trace::BusTrace& trace);

private:
//    static bool valid_if_edge_is(const bus_trace::BusTrace& trace, bus_trace::BusEventFlags flags);
//    static size_t find_start_bit(const bus_trace::BusTrace& trace);
};

} // analysis
