// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#pragma once

#include <cstdint>
#include <cstddef>

namespace bus_trace {

// Events are recorded when either a bus line level changes or a bus
// control pin changes from floating to pull-down. The time delay
// between a pin change and a line change gives the bus rise and fall time.
//
// Pin values and changes are not recorded if we're just spying on the
// bus.

enum BusEventFlagBits : uint8_t {
    SDA_PIN_CHANGED_BIT = 7,
    SCL_PIN_CHANGED_BIT = 6,
    SDA_PIN_STATE_BIT = 5,
    SCL_PIN_STATE_BIT = 4,
    SDA_LINE_CHANGED_BIT = 3,
    SCL_LINE_CHANGED_BIT = 2,
    SDA_LINE_STATE_BIT = 1,
    SCL_LINE_STATE_BIT = 0,
};

// Note that the PIN_CHANGED and PIN_STATE flags are rarely captured.
enum BusEventFlags : uint8_t {
    SDA_PIN_CHANGED = 1 << (uint8_t)BusEventFlagBits::SDA_PIN_CHANGED_BIT,
    SCL_PIN_CHANGED = 1 << (uint8_t)BusEventFlagBits::SCL_PIN_CHANGED_BIT,
    SDA_PIN_STATE = 1 << (uint8_t)BusEventFlagBits::SDA_PIN_STATE_BIT,
    SCL_PIN_STATE = 1 << (uint8_t)BusEventFlagBits::SCL_PIN_STATE_BIT,
    SDA_LINE_CHANGED = 1 << (uint8_t)BusEventFlagBits::SDA_LINE_CHANGED_BIT,
    SCL_LINE_CHANGED = 1 << (uint8_t)BusEventFlagBits::SCL_LINE_CHANGED_BIT,
    SDA_LINE_STATE = 1 << (uint8_t)BusEventFlagBits::SDA_LINE_STATE_BIT,
    SCL_LINE_STATE = 1 << (uint8_t)BusEventFlagBits::SCL_LINE_STATE_BIT,
    BOTH_LOW_AND_UNCHANGED = 0
};

// Sets bit 'bit' of 'flags' with the new value
inline BusEventFlags update_from_bool(BusEventFlags flags, bool value, BusEventFlagBits bit) {
    return BusEventFlags((flags & ~(1 << bit)) | (value << bit));
}

// Bitwise OR
inline BusEventFlags operator|(BusEventFlags lhs, BusEventFlags rhs) {
    return static_cast<BusEventFlags>(
            static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)
    );
}

inline BusEventFlags& operator|=(BusEventFlags& lhs, BusEventFlags rhs) {
    return lhs = lhs | rhs;
}

// Bitwise AND
inline BusEventFlags operator&(BusEventFlags lhs, BusEventFlags rhs) {
    return static_cast<BusEventFlags>(
            static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)
    );
}

// Bitwise XOR
inline BusEventFlags operator^(BusEventFlags lhs, BusEventFlags rhs) {
    return static_cast<BusEventFlags>(
            static_cast<uint8_t>(lhs) ^ static_cast<uint8_t>(rhs)
    );
}

// Bitwise NOT
inline BusEventFlags operator~(BusEventFlags flags) {
    return static_cast<BusEventFlags>(
            ~static_cast<uint8_t>(flags)
    );
}
}
