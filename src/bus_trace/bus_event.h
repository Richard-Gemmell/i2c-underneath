// Copyright © 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#pragma once
#include <cstdint>
#include <cstddef>
#include "bus_event_flags.h"

namespace bus_trace {

// Records a change to the state of the bus.
class BusEvent {
public:
    BusEvent() = default;

    BusEvent(uint16_t delta_t_in_ticks, BusEventFlags flags)
        : delta_t_in_ticks(delta_t_in_ticks), flags(flags) {
    }

    // System ticks since previous event.
    //
    // The conversion from system ticks to nanoseconds depends on the device
    // and may be affected by overclocking.
    //
    // The data type must be large enough to record the largest expected period
    // between events. The SMBus time out is 35 ms which is 35,000,000 nanos.
    // This would require a 4 byte type. 2 bytes between events gives 109,000
    // nanos on a Teensy 4. Enough for a 9 KHz baud rate. SMBus is minimum of 10 kHz
    uint16_t delta_t_in_ticks;

    // Describes the event that happened and the state of the bus lines.
    BusEventFlags flags;

//    uint8_t unused; // Pack byte.

    bool scl_fell() const {
        return includes_flags(BusEventFlags::SCL_LINE_CHANGED) && excludes_flags(BusEventFlags::SCL_LINE_STATE);
    }

    bool scl_rose() const {
        return includes_flags(BusEventFlags::SCL_LINE_CHANGED | BusEventFlags::SCL_LINE_STATE);
    }

    bool sda_fell() const {
        return includes_flags(BusEventFlags::SDA_LINE_CHANGED) && excludes_flags(BusEventFlags::SDA_LINE_STATE);
    }

    bool sda_rose() const {
        return includes_flags(BusEventFlags::SDA_LINE_CHANGED | BusEventFlags::SDA_LINE_STATE);
    }

private:
    bool includes_flags(const BusEventFlags& requiredFlags) const {
        return (flags & requiredFlags) == requiredFlags;
    }

    bool excludes_flags(const BusEventFlags& requiredFlags) const {
        return !(flags & requiredFlags);
    }
};

inline bool operator==(const BusEvent& lhs, const BusEvent& rhs) {
    return lhs.flags == rhs.flags &&
           lhs.delta_t_in_ticks == rhs.delta_t_in_ticks;
}

inline bool operator!=(const BusEvent& lhs, const BusEvent& rhs) {
    return !(lhs == rhs);
}

}
