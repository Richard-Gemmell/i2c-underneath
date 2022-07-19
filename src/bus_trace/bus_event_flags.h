// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_EVENT_FLAGS_H
#define I2C_UNDERNEATH_BUS_EVENT_FLAGS_H

#include <cstdint>
#include <cstddef>

namespace bus_trace {

// Events are recorded when either a bus line level changes or a bus
// control pin changes from floating to pull-down. The time delay
// between a pin change and a line change gives the bus rise and fall time.
//
// Pin values and changes are not recorded if we're just spying on the
// bus.
enum class BusEventFlags : uint8_t {
    SDA_PIN_CHANGED = 1 << 7,
    SCL_PIN_CHANGED = 1 << 6,
    SDA_PIN_STATE = 1 << 5,
    SCL_PIN_STATE = 1 << 4,
    SDA_LINE_CHANGED = 1 << 3,
    SCL_LINE_CHANGED = 1 << 2,
    SDA_LINE_STATE = 1 << 1,
    SCL_LINE_STATE = 1 << 0,
};

BusEventFlags operator|(BusEventFlags lhs, BusEventFlags rhs);

BusEventFlags operator&(BusEventFlags lhs, BusEventFlags rhs);
}

#endif //I2C_UNDERNEATH_BUS_EVENT_FLAGS_H
