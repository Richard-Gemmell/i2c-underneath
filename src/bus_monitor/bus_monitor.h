// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_MONITOR_H
#define I2C_UNDERNEATH_BUS_MONITOR_H

#include <functional>
#include <common/hardware/pin.h>
#include <common/hardware/timer.h>
#include <common/specifications.h>
#include "bus_state.h"

namespace bus_monitor {

// Watches a bus and reports problems.
class BusMonitor {
public:
    BusMonitor(common::hardware::Pin& sda,
               common::hardware::Pin& scl,
               common::hardware::Timer& timer,
               uint32_t bus_busy_timeout_ns = common::StandardMode.times.min_bus_free_time * 1.1,
               uint32_t bus_stuck_timeout_micros = (SMBUS_TIMEOUT_MILLIS + 1UL) * 1000UL);

    ~BusMonitor();

    // Start monitoring the bus
    void begin();

    // Stop monitoring the bus
    void end();

    // The current state of the bus
    bus_monitor::BusState get_state();

    // Register a callback to be called if the bus gets stuck.
    // 'callback': the function that will be called when the bus is stuck
    // 'sda_stuck': true if SDA is stuck LOW
    // 'scl_stuck': true if SCL is stuck LOW
    void on_stuck(const std::function<void(bool sda_stuck, bool scl_stuck)>& callback);

private:
    common::hardware::Pin& sda;
    common::hardware::Pin& scl;
    common::hardware::Timer& timer;
    uint32_t bus_busy_timeout_ns;
    uint32_t bus_stuck_timeout_micros;
    std::function<void(bool, bool)> callback;
};

}

#endif //I2C_UNDERNEATH_BUS_MONITOR_H
