// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_EVENTS_H
#define I2C_UNDERNEATH_BUS_EVENTS_H

#include <cstdint>
#include <cstddef>
#include "bus_event_flags.h"

namespace bus_trace {

// Records a change to the state of the bus.
class BusEvent {
public:
    BusEvent() = default;

    BusEvent(uint16_t delta_t_in_ticks, BusEventFlags flags)
        : delta_t_nanos(delta_t_in_ticks),
          flags(flags) {
    }

    // Nanoseconds since previous event.
    // Data type must be large enough to record the largest expected period between events.
    // SMBus time out is 35 ms which is 35,000,000 nanos. Would require a 4 byte type.
    // 2 bytes between events gives 65536 nanos. Enough for a 9 KHz baud rate. SMBus is minimum of 10 kHz
    uint16_t delta_t_nanos;

    // Describes the event that happened and the state of the bus lines.
    BusEventFlags flags;

//    uint8_t unused; // Pack byte.
};

inline bool operator==(const BusEvent& lhs, const BusEvent& rhs) {
    return lhs.flags == rhs.flags &&
           lhs.delta_t_nanos == rhs.delta_t_nanos;
}
inline bool operator!=(const BusEvent& lhs, const BusEvent& rhs) { return !(lhs == rhs); }

}
#endif //I2C_UNDERNEATH_BUS_EVENTS_H