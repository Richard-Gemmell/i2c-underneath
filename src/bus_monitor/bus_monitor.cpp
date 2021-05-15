// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)
#include "bus_monitor.h"

bus_monitor::BusMonitor::BusMonitor(common::hardware::Pin& sda, common::hardware::Pin& scl) {

}

bus_monitor::BusState bus_monitor::BusMonitor::get_state() {
    return BusState::stuck;
}

void bus_monitor::BusMonitor::on_stuck(std::function<void(bool, bool)> callback) {

}

