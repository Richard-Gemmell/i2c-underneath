// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_MONITOR_H
#define I2C_UNDERNEATH_BUS_MONITOR_H

#include <functional>
#include <common/hal/pin.h>
#include <common/hal/timestamp.h>
#include <common/specifications.h>
#include "bus_state.h"

namespace bus_monitor {

// Watches a bus and reports problems.
class BusMonitor {
public:
    // 'sda' and 'scl' must be connected to the data and clock lines respectively.
    // If this device is also a bus Master or Slave then you can use the actual
    // I2C pins unless your I2C driver needs to register callbacks on these pins.
    // In this case, let the I2C driver configure the pins. If you're using other
    // pins then they must be configured as open drain inputs.
    // 'timestamp' tracks the time of the most recent edge on SDA or SCL
    // 'bus_busy_timeout_ns' is a time in nanosecond. The bus is deemed to be busy
    // for this amount of time after the most recent I2C transfers.
    // 'bus_stuck_timeout_ns' is a time in nanoseconds. The bus state changes
    // to 'stuck' if either 'sda' or 'scl' remains low for this amount of time.
    BusMonitor(common::hal::Pin& sda,
               common::hal::Pin& scl,
//               common::hal::Timestamp& timestamp,
               uint32_t bus_busy_timeout_ns = uint32_t(common::StandardMode.times.min_bus_free_time * 1.1),
               uint32_t bus_stuck_timeout_ns = SMBUS_TIMEOUT_MILLIS * 1'000'000UL);

    ~BusMonitor();

    // Start monitoring the bus
    void begin();

    // Stop monitoring the bus
    void end();

    // The current state of the bus. Idle, busy or stuck.
    bus_monitor::BusState get_state();

    // Register a callback to be called if the bus gets stuck.
    // 'callback': the function that will be called when the bus is stuck
    // 'sda_stuck': true if SDA is stuck LOW
    // 'scl_stuck': true if SCL is stuck LOW
//    void on_stuck(const std::function<void(bool sda_stuck, bool scl_stuck)>& callback);

private:
    common::hal::Pin& sda_;
    common::hal::Pin& scl_;
    uint32_t bus_busy_timeout_ns_;
    uint32_t bus_stuck_timeout_ns_;
    common::hal::Timestamp& last_edge_;
    BusState bus_state_ = BusState::unknown;
    void on_line_changed(bool line_level);
};

}

#endif //I2C_UNDERNEATH_BUS_MONITOR_H
