// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "bus_recorder.h"
#include "bus_event.h"

namespace bus_trace {
BusRecorder::BusRecorder(common::hal::Pin& sda, common::hal::Pin& scl, const common::hal::Clock& clock)
    : sda(sda), scl(scl), clock(clock) {
}

void BusRecorder::start(BusTrace& trace) {
    current_trace = &trace;
    sda_high = sda.read_line();
    scl_high = scl.read_line();
    ticks_at_latest_event = clock.get_system_tick();
    // Record the initial line states
    on_change(BusEventFlags::BOTH_LOW_AND_UNCHANGED);
    // Start listening for line changes
    sda.on_edge([this](bool rising) {
        sda_high = rising;
        on_change(BusEventFlags::SDA_LINE_CHANGED);
    });
    scl.on_edge([this](bool rising) {
        scl_high = rising;
        on_change(BusEventFlags::SCL_LINE_CHANGED);
    });
}

void BusRecorder::stop() {
    sda.on_edge(nullptr);
    scl.on_edge(nullptr);
    current_trace = nullptr;
}

bool BusRecorder::is_recording() const {
    return current_trace != nullptr;
}

void BusRecorder::on_change(BusEventFlags edges) {
    // Calculate time since last event
    // Do this first to minimise the delay before getting the system tick
    uint32_t delta_t_in_nanos = clock.nanos_since(ticks_at_latest_event);

    BusEventFlags flags = edges;
    if(sda_high) {
        flags = flags | BusEventFlags::SDA_LINE_STATE;
    }
    if(scl_high) {
        flags = flags | BusEventFlags::SCL_LINE_STATE;
    }

    current_trace->add_event(delta_t_in_nanos, flags);
}
} // bus_trace