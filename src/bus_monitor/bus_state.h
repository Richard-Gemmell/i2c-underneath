// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_MONITOR_BUS_STATE_H
#define I2C_UNDERNEATH_BUS_MONITOR_BUS_STATE_H

namespace bus_monitor {

// Indicates the state of an I2C bus.
enum class BusState {
    // There aren't any transactions in progress. A master is able to start
    // a new transaction.
    // Requires that both bus lines are HIGH and have been for longer than
    // the bus_busy_timeout.
    idle = 0,

    // Either a transaction's in progress or it's just finished.
    // Requires that one of the bus lines changed recently.
    busy,

    // The bus is inoperable because one or both bus lines is being
    // held LOW.
    // Requires that either SDA or SCL has been LOW for longer than
    // bus_stuck_timeout.
    stuck = 100
};

}
#endif //I2C_UNDERNEATH_BUS_MONITOR_BUS_STATE_H
