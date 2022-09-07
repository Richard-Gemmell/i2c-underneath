// Copyright © 2021-2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#pragma once

#include <Arduino.h>
#include <cstdint>
#include <cstddef>
#include <array>
#include <bus_trace/bus_event.h>
#include <bus_trace/bus_event_flags.h>
#include <common/hal/clock.h>

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

    // Creates a trace.
    // This overload is provided to optimise BusRecorder. See add_event(BusEventFlags).
    // max_event_count: maximum number of bus events that can be stored in this trace.
    // clock: system clock for use by add_event(BusEventFlags)
    // Additional events are dropped. Must be less than SIZE_MAX.
    BusTrace(const common::hal::Clock* clock, size_t max_event_count);

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

    // Returns the time since the previous event in nanoseconds.
    // Returns UINT32_MAX if index is out of range or this trace
    // doesn't have a clock.
    // Returns 0 for the first event.
    uint32_t nanos_to_previous(size_t index) const;

    // Removes any existing events and resets the clock
    void reset();

    // Adds an event to the trace as long as there is space for it.
    // Discards the event if there's no more space
    inline void add_event(const BusEvent& event) {
        if(current_event_count == max_event_count) {
            // We can't take another event. Discard it.
            return;
        }
        events[current_event_count] = event;
        current_event_count++;
    }

    inline void add_event(uint32_t delta_t_in_ticks, BusEventFlags flags) {
        // This looks crazy but it makes BusRecorder very slightly faster
        add_event(BusEvent(delta_t_in_ticks, flags));
    }

    inline void add_event(BusEventFlags flags) {
        uint32_t past = ticks_start;
        set_ticks_start();
        add_event(BusEvent(ticks_start - past, flags));
    }

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
    const common::hal::Clock* clock;
    uint32_t ticks_start = 0;
    BusEvent* events;               // Array of events
    bool created_events;            // True if we own events. False if it was passed to the constructor.
    size_t max_event_count;         // Maximum number of items in 'events'
    size_t current_event_count = 0; // Current event count

    static void append_event_symbol(String& string, bool sda, BusEventFlags flags);

    inline void set_ticks_start() {
#ifdef ARDUINO_TEENSY40
        // It's about 13 nanoseconds (8 ticks) faster to get the tick count directly.
        ticks_start = ARM_DWT_CYCCNT;
#else
        if (clock) {
            ticks_start = clock->get_system_tick();
        }
#endif
    }
};

}
