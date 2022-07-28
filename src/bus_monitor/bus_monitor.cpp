// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)
#include "bus_monitor.h"

bus_monitor::BusMonitor::BusMonitor(common::hal::Pin& sda,
                                    common::hal::Pin& scl,
                                    common::hal::Timestamp& timestamp,
                                    uint32_t bus_busy_timeout_ns,
                                    uint32_t bus_stuck_timeout_micros) :
        sda_(sda),
        scl_(scl),
        bus_busy_timeout_ns_(bus_busy_timeout_ns),
        bus_stuck_timeout_ns_(bus_stuck_timeout_micros),
        last_edge_(timestamp),
        running_(false) {
}

bus_monitor::BusMonitor::~BusMonitor() {
    end();
}

void bus_monitor::BusMonitor::begin() {
    if (running_) return;

    if(scl_.read_line() && sda_.read_line()) {
        bus_state_ = BusState::idle;
    } else {
        bus_state_ = BusState::busy;
    }

    auto on_edge_callback = [this](bool rising) {
        this->on_line_changed(rising);
    };
    sda_.on_edge(on_edge_callback);
    scl_.on_edge(on_edge_callback);
    running_ = true;
}

void bus_monitor::BusMonitor::end() {
    if (!running_) return;

    sda_.on_edge(nullptr);
    scl_.on_edge(nullptr);
    bus_state_ = BusState::unknown;

    running_ = false;
}

bus_monitor::BusState bus_monitor::BusMonitor::get_state() {
    if (running_) {
        if (bus_state_ == BusState::busy) {
            if (scl_.read_line() && sda_.read_line()) {
                if (last_edge_.timed_out_nanos(bus_busy_timeout_ns_)) {
                    bus_state_ = BusState::idle;
                }
            } else {
                if (last_edge_.timed_out_nanos(bus_stuck_timeout_ns_)) {
                    bus_state_ = BusState::stuck;
                }
            }
        }
    }
    return bus_state_;
}

void bus_monitor::BusMonitor::on_line_changed(bool line_level) {
    last_edge_.reset();
    if (bus_state_ == BusState::idle) {
        bus_state_ = BusState::busy;
    } else if (bus_state_ == BusState::stuck) {
        // Bus remains stuck until both lines are high
        if (sda_.read_line() && scl_.read_line()) {
            bus_state_ = BusState::busy;
        }
    }
}
