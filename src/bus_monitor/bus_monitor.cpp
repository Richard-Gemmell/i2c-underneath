// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)
#include "bus_monitor.h"

#include <utility>

bus_monitor::BusMonitor::BusMonitor(common::hardware::Pin& sda,
                                    common::hardware::Pin& scl,
                                    common::hardware::Timer& timer,
                                    uint32_t bus_busy_timeout_ns,
                                    uint32_t bus_stuck_timeout_micros) :
    sda(sda),
    scl(scl),
    timer(timer),
    bus_busy_timeout_ns(bus_busy_timeout_ns),
    bus_stuck_timeout_micros(bus_stuck_timeout_micros) {
}

bus_monitor::BusMonitor::~BusMonitor() {
    // TODO: call end()
}

void bus_monitor::BusMonitor::begin() {
    // TODO: register callbacks on pins
    // TODO: start timer
}

void bus_monitor::BusMonitor::end() {
    // TODO: Stop timer
}

bus_monitor::BusState bus_monitor::BusMonitor::get_state() {
    return bus_monitor::BusState::idle;
}

void bus_monitor::BusMonitor::on_stuck(const std::function<void(bool sda_stuck, bool scl_stuck)>& callback) {
//    this->callback = callback;
}
