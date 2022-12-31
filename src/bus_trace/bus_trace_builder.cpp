// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "bus_trace_builder.h"

namespace bus_trace {
BusTraceBuilder::BusTraceBuilder(BusTrace& trace, TimingStrategy timing_strategy, const common::i2c_specification::I2CParameters& params)
    : trace(trace), timing_strategy(timing_strategy), params(params) {
}

BusTraceBuilder& BusTraceBuilder::bus_initially_idle() {
    trace.add_event(BusEvent(0, BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE));
    return *this;
}

BusTraceBuilder& BusTraceBuilder::start_bit() {
    uint32_t tf = get_time(params.times.fall_time);
    trace.add_event(BusEvent(tf, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE));
    uint32_t tHD_STA = get_time(params.times.start_hold_time);
    trace.add_event(BusEvent(tHD_STA + tf, BusEventFlags::SCL_LINE_CHANGED));
    return *this;
}

BusTraceBuilder& BusTraceBuilder::stop_bit() {
    // Assumes the previous event was an ACK or NACK
    uint32_t tr = get_time(params.times.rise_time);
    uint32_t tf = get_time(params.times.fall_time);
    uint32_t tLOW = get_time(params.times.scl_low_time);
    uint32_t dt = 0;
    if(sda_was_high()) {
        uint32_t tHD_DAT = get_time(params.times.data_hold_time) + data_hold_offset;
        dt = tHD_DAT + tf;
        trace.add_event(BusEvent(dt, BusEventFlags::SDA_LINE_CHANGED));
    }
    trace.add_event(BusEvent(tLOW + tr - dt, BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE));
    uint32_t tSU_STO = get_time(params.times.stop_setup_time);
    trace.add_event(BusEvent(tSU_STO + tr, BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE | BusEventFlags::SCL_LINE_STATE));
    return *this;
}

BusTraceBuilder& BusTraceBuilder::data_bit(bool one) {
    // NB: We consider each bit to end when SCL goes LOW again
    // Set SDA if it's changing
    uint32_t tr = get_time(params.times.rise_time);
    uint32_t tf = get_time(params.times.fall_time);
    uint32_t dt = 0;    // Time for first event if it's required
    BusEventFlags sda_state = one ? BusEventFlags::SDA_LINE_STATE : BusEventFlags::BOTH_LOW_AND_UNCHANGED;
    if(sda_was_high() != one) {
        uint32_t tHD_DAT = get_time(params.times.data_hold_time) + data_hold_offset;
        dt = tHD_DAT + (one ? tr : tf);
        trace.add_event(BusEvent(dt, BusEventFlags::SDA_LINE_CHANGED | sda_state));
    }
    // Pulse SCL
    uint32_t tLOW = get_time(params.times.scl_low_time);
    trace.add_event(BusEvent(tLOW + tr - dt, BusEventFlags::SCL_LINE_CHANGED | sda_state | BusEventFlags::SCL_LINE_STATE));
    uint32_t tHIGH = get_time(params.times.scl_high_time);
    trace.add_event(BusEvent(tHIGH + tf, BusEventFlags::SCL_LINE_CHANGED | sda_state));
    return *this;
}

BusTraceBuilder& BusTraceBuilder::ack() {
    data_bit(false);
    return *this;
}

BusTraceBuilder& BusTraceBuilder::nack() {
    data_bit(true);
    return *this;
}

BusTraceBuilder& BusTraceBuilder::address_byte(uint8_t address, bool read) {
    uint8_t value = address << 1;
    if(read) {
        value += 1;
    }
    data_byte(value);
    return *this;
}

BusTraceBuilder& BusTraceBuilder::data_byte(uint8_t value) {
    data_bit(value & 0x80);
    data_bit(value & 0x40);
    data_bit(value & 0x20);
    data_bit(value & 0x10);
    data_bit(value & 0x08);
    data_bit(value & 0x04);
    data_bit(value & 0x02);
    data_bit(value & 0x01);
    return *this;
}

bool BusTraceBuilder::sda_was_high() const {
    if(trace.event_count() > 0) {
        auto previous_event = trace.event(trace.event_count() - 1);
        return (bool)(previous_event->flags & BusEventFlags::SDA_LINE_STATE);
    }
    return true;    // Assume the lines were HIGH because they're idle
}

uint32_t BusTraceBuilder::get_time(const common::i2c_specification::TimeRange& time_range) const {
    if(timing_strategy == TimingStrategy::Min) {
        return time_range.min;
    }
    return time_range.max;
}

} // bus_trace
