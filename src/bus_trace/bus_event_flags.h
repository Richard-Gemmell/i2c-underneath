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

enum class BusEventFlagBits : uint8_t {
// PIN states are not supported as they're not likely to be useful.
    SDA_LINE_CHANGED_BIT = 3,
    SCL_LINE_CHANGED_BIT = 2,
    SDA_LINE_STATE_BIT = 1,
    SCL_LINE_STATE_BIT = 0,
};

enum class BusEventFlags : uint8_t {
// PIN states are not supported as they're not likely to be useful.
//    SDA_PIN_CHANGED = 1 << 7,
//    SCL_PIN_CHANGED = 1 << 6,
//    SDA_PIN_STATE = 1 << 5,
//    SCL_PIN_STATE = 1 << 4,
    SDA_LINE_CHANGED = 1 << (uint8_t)BusEventFlagBits::SDA_LINE_CHANGED_BIT,
    SCL_LINE_CHANGED = 1 << (uint8_t)BusEventFlagBits::SCL_LINE_CHANGED_BIT,
    SDA_LINE_STATE = 1 << (uint8_t)BusEventFlagBits::SDA_LINE_STATE_BIT,
    SCL_LINE_STATE = 1 << (uint8_t)BusEventFlagBits::SCL_LINE_STATE_BIT,
    BOTH_LOW_AND_UNCHANGED = 0
};

BusEventFlags operator|(BusEventFlags lhs, BusEventFlags rhs);

BusEventFlags operator&(BusEventFlags lhs, BusEventFlags rhs);
}

#endif //I2C_UNDERNEATH_BUS_EVENT_FLAGS_H
