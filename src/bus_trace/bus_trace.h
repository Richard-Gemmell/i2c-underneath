// Copyright © 2021-2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_TRACE_H
#define I2C_UNDERNEATH_BUS_TRACE_H

#include <Arduino.h>
#include <cstdint>
#include <cstddef>
#include <array>
#include <bus_trace/bus_event.h>
#include <bus_trace/bus_event_flags.h>

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

    // Creates a trace.
    // max_event_count: maximum number of bus events that can be stored in this trace.
    // Additional events are dropped. Must be less than SIZE_MAX.
    explicit BusTrace(size_t max_event_count);

    virtual ~BusTrace();

    // Allows you to define the array of events wherever you want. This may
    // be important for large traces.
    // events: an array of BusEvents that will be populated by the trace
    // max_event_count: maximum number of bus events that can be stored in this trace.
    // Additional events are dropped. Must be less than SIZE_MAX.
    BusTrace(BusEvent* events, size_t max_event_count);

    // The number of events we've recorded.
    size_t event_count() const;

    // Returns a recorded event or nullptr if index is out of range.
    const BusEvent* event(size_t index) const;

    // Adds an event to the trace as long as there is space for it.
    // Discards the event if there's no more space
    void add_event(const BusEvent& event);
    void add_event(uint32_t delta_t_nanos, BusEventFlags flags);

    // Creates a copy of this message without any spurious changes in SDA
    // that occur while SCL is LOW. These don't affect the meaning of the
    // message.
    BusTrace to_message() const;

    // Traces are equivalent if they have the same meaning in I2C
    // terms. Ignores the timing between each event and pin values.
    // Ignores spurious changes in SDA that can occur while SCL is LOW.
    // For example, it's quite common to see SDA pulse from 0->1->0
    // before an ACK. Whether this happens or not is an implementation
    // detail.
    //
    // Returns the index of the first BusEvent that doesn't match
    // or SIZE_MAX if the traces are equivalent.
    size_t compare_messages(const BusTrace& other) const;

    // Traces are equivalent if their line states and edges match exactly.
    // Pin changes and timings are ignored.
    //
    // Returns the index of the first BusEvent that doesn't match
    // or SIZE_MAX if the traces are equivalent.
    size_t compare_edges(const BusTrace& other) const;

    // Prints the trace
    size_t printTo(Print& p) const override;

private:
    BusEvent* events;               // Array of events
    bool created_events;            // True if we own events. False if it was passed to the constructor.
    size_t max_event_count;         // Maximum number of items in 'events'
    size_t current_event_count = 0; // Current event count

    static void append_event_symbol(String& string, bool sda, BusEventFlags flags);
};

}
#endif //I2C_UNDERNEATH_BUS_TRACE_H