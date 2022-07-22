// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "bus_recorder.h"

namespace bus_trace {
BusRecorder::BusRecorder(common::hal::Pin& sda, common::hal::Pin& scl)
    : sda(sda), scl(scl) {
}

void BusRecorder::start(BusTrace& trace) {
    current_trace = &trace;
    sda_high = sda.read_line();
    scl_high = scl.read_line();
    // Record the initial line states
    on_change(false, false);
    // Start listening for line changes
    auto on_sda_edge = std::bind(&BusRecorder::on_sda_changed, this, std::placeholders::_1);
    sda.on_edge(on_sda_edge);
    auto on_scl_edge = std::bind(&BusRecorder::on_scl_changed, this, std::placeholders::_1);
    scl.on_edge(on_scl_edge);
}

void BusRecorder::stop() {
    sda.on_edge(nullptr);
    scl.on_edge(nullptr);
    current_trace = nullptr;
}

bool BusRecorder::is_recording() const {
    return current_trace != nullptr;
}

void BusRecorder::on_sda_changed(bool line_level) {
    sda_high = line_level;
    on_change(true, false);
}

void BusRecorder::on_scl_changed(bool line_level) {
    scl_high = line_level;
    on_change(false, true);
}

void BusRecorder::on_change(bool sda_changed, bool scl_changed) const {
    BusEventFlags flags = BusEventFlags::BOTH_LOW_AND_UNCHANGED;
    if(sda_changed) {
        flags = flags | BusEventFlags::SDA_LINE_CHANGED;
    }
    if(scl_changed) {
        flags = flags | BusEventFlags::SCL_LINE_CHANGED;
    }
    if(sda_high) {
        flags = flags | BusEventFlags::SDA_LINE_STATE;
    }
    if(scl_high) {
        flags = flags | BusEventFlags::SCL_LINE_STATE;
    }
    current_trace->record_event(flags);
}
} // bus_trace