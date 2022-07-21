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
        : delta_t_in_ticks(delta_t_in_ticks),
          flags(flags) {
    }

    // Ticks since previous event.
    // Data type must be large enough to record the largest expected period between events.
    // SMBus time out is 35 ms. Which is 35,000,000 nanos. A 3 byte uint gives 16,777,215 ticks which is 28,000,000 nanos.
    // 2 bytes between events gives 65536 ticks which is 100,000 nanos. Enough for a 5 KHz baud rate. SMBus is minimum of 10 kHz
    uint16_t delta_t_in_ticks;  // ticks since previous event

    // Describes the event that happened and the state of the bus lines.
    BusEventFlags flags;

//    uint8_t unused; // Pack byte.
};

}
#endif //I2C_UNDERNEATH_BUS_EVENTS_H