// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_TRACE_BUILDER_H
#define I2C_UNDERNEATH_BUS_TRACE_BUILDER_H

#include "bus_event.h"
#include "common/specifications/i2c_specification.h"
#include "bus_trace.h"

namespace bus_trace {

class BusTraceBuilder {
public:
    static const bool WRITE = false;
    static const bool READ = true;

    enum class TimingStrategy {
        Min,    // Traces will have shortest valid deltas
        Max     // Traces will have largest valid deltas
    };

    BusTraceBuilder(BusTrace& trace, TimingStrategy timing_strategy, const common::i2c_specification::I2CParameters& params);

    BusTraceBuilder& bus_initially_idle();

    BusTraceBuilder& start_bit();

    BusTraceBuilder& stop_bit();

    BusTraceBuilder& data_bit(bool one);

    BusTraceBuilder& ack();

    BusTraceBuilder& nack();

    BusTraceBuilder& address_byte(uint8_t address, bool read);

    BusTraceBuilder& data_byte(uint8_t value);

private:
    BusTrace& trace;
    TimingStrategy timing_strategy;
    const common::i2c_specification::I2CParameters& params;

    bool sda_was_high() const;
    uint32_t get_time(const common::i2c_specification::TimeRange& time_range) const;
};

} // bus_trace

#endif //I2C_UNDERNEATH_BUS_TRACE_BUILDER_H
