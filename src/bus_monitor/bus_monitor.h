// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_MONITOR_H
#define I2C_UNDERNEATH_BUS_MONITOR_H

#include <functional>
#include <common/hardware/pin.h>
#include "bus_state.h"

namespace bus_monitor {

// Watches a bus and reports problems.
class BusMonitor {
    BusMonitor(common::hardware::Pin& sda, common::hardware::Pin& scl);

    // The current state of the bus
    bus_monitor::BusState get_state();

    // Register a callback to be called if the bus gets stuck.
    // 'callback': the function that will be called when the bus is stuck
    // 'sda_stuck': true if SDA is stuck LOW
    // 'scl_stuck': true if SCL is stuck LOW
    void on_stuck(std::function<void(bool sda_stuck, bool scl_stuck)> callback);
};

}

#endif //I2C_UNDERNEATH_BUS_MONITOR_H
