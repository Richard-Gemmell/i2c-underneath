// Copyright © 2021-2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include <Arduino.h>
#include "bus_trace.h"

namespace bus_trace {

size_t BusTrace::max_events_required(uint32_t bytes_per_message, bool include_pin_events) {
    const size_t clock_events_per_byte = 9 * 2; // SCL pulses once per bit
    const size_t data_events_per_byte = 10;     // SDA only creates edges if the current bit is different to the previous one.
                                          // The worst case is 10101010 followed by a NACK.
    size_t events_per_byte = clock_events_per_byte + data_events_per_byte;
    if (include_pin_events) {
        events_per_byte *= 2;    // If we're driving the pins then there's an extra event per edge.
    }
    return 4 +  // Allow for START and STOP bits
           (events_per_byte * (bytes_per_message + 1)); // Allow for address byte
}

}