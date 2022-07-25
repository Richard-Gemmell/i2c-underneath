// Copyright © 2021-2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_TRACE_H
#define I2C_UNDERNEATH_BUS_TRACE_H

#include <cstdint>
#include <cstddef>
#include <array>
#include <bus_trace/bus_event.h>
#include <bus_trace/bus_event_flags.h>
#include <common/hal/clock.h>
#include "common/hal/teensy/teensy_clock.h"

namespace bus_trace {

// A list of BusEvents. Used to record all activity on an I2C bus.
// See also BusEvent
class BusTrace : public Printable {
public:
    // Calculates the maximum number of 'BusEvent' objects required
    // to describe an I2C message.
    // Set 'include_pin_events' to true if the system is driving the bus
    // pins. Set it to false if the system is watching a bus without
    // driving it.
    // Multiply by sizeof(BusEvent) to get RAM required
    static size_t max_events_required(uint32_t bytes_per_message, bool include_pin_events);

    // events: an array of BusEvents that will be populated by the trace
    // max_event_count: maximum number of bus events that can be stored in this trace.
    // clock: provides system time. Can be faked for unit tests.
    // Additional events are dropped. Must be less than SIZE_MAX.
    BusTrace(BusEvent* events, size_t max_event_count, const common::hal::Clock& clock);

    // The number of events we've recorded.
    size_t event_count() const;

    // Returns a recorded event or nullptr if index is out of range.
    const BusEvent* event(size_t index) const;

    // Adds an event to the trace as long as there is space for it.
    // Discards the event if there's no more space
    void record_event(const BusEventFlags& event);

    // Returns the index of the first BusEvent that doesn't match
    // or SIZE_MAX if the traces are equivalent.
    // Only line states and transitions matter. Pin changes and timings
    // are ignored.
    size_t compare_to(const BusEvent* other, size_t other_event_count);

    // Prints the trace
    size_t printTo(Print& p) const override;

private:
    const common::hal::Clock& clock;    // Provides system time
    uint32_t ticks_at_latest_event = 0;

    BusEvent* events;               // Array of events
    size_t max_event_count;         // Maximum number of items in 'events'
    size_t current_event_count = 0; // Current event count

    static void append_event_symbol(String& string, bool sda, BusEventFlags flags);
};

}
#endif //I2C_UNDERNEATH_BUS_TRACE_H