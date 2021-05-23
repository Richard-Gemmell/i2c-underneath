// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)
#include "bus_monitor.h"

bus_monitor::BusMonitor::BusMonitor(common::hal::Pin& sda,
                                    common::hal::Pin& scl,
//                                    common::hal::Timestamp& timestamp,
                                    uint32_t bus_busy_timeout_ns,
                                    uint32_t bus_stuck_timeout_micros) :
        sda_(sda),
        scl_(scl),
        bus_busy_timeout_ns_(bus_busy_timeout_ns),
        bus_stuck_timeout_ns_(bus_stuck_timeout_micros)
        //,
//        last_edge_(timestamp)
        {
}

bus_monitor::BusMonitor::~BusMonitor() {
    end();
}

void bus_monitor::BusMonitor::begin() {
    if(scl_.read_line() && sda_.read_line()) {
        bus_state_ = BusState::idle;
    } else {
        bus_state_ = BusState::busy;
    }
    // TODO: register callbacks on pins
    auto on_edge_callback = std::bind(&bus_monitor::BusMonitor::on_line_changed, this, std::placeholders::_1);
    sda_.on_edge(on_edge_callback);
    scl_.on_edge(on_edge_callback);
    // TODO: start timer
}

void bus_monitor::BusMonitor::end() {
    // TODO: release pin callbacks
    // TODO: Stop timer
    bus_state_ = BusState::unknown;
}

bus_monitor::BusState bus_monitor::BusMonitor::get_state() {
    bool scl = scl_.read_line();
    bool sda = sda_.read_line();
    if (bus_state_ == BusState::busy) {
        if (scl && sda) {
            if (last_edge_.timed_out_nanos(bus_busy_timeout_ns_)) {
                bus_state_ = BusState::idle;
            }
        } else {
            if (last_edge_.timed_out_nanos(bus_stuck_timeout_ns_)) {
                bus_state_ = BusState::stuck;
            }
        }
    }
    return bus_state_;
}

void bus_monitor::BusMonitor::on_line_changed(bool line_level) {
//    last_edge_.mark();
    bus_state_ = BusState::busy;
}
