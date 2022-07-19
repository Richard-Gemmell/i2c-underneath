// Copyright © 2021-2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_TRACE_H
#define I2C_UNDERNEATH_BUS_TRACE_H

#include <cstdint>
#include <cstddef>
#include <array>
#include <bus_trace/bus_event.h>
#include <bus_trace/bus_event_flags.h>

namespace bus_trace {

// A list of BusEvents. Used to record all activity on an I2C bus.
// See also BusEvent
class BusTrace {
public:
    // Calculates the maximum number of 'BusEvent' objects required
    // to describe an I2C message.
    // Set 'include_pin_events' to true if the system is driving the bus
    // pins. Set it to false if the system is watching a bus without
    // driving it.
    // RAM required is 30 to 60 bytes per byte of I2C message
    static size_t max_events_required(uint32_t bytes_per_message, bool include_pin_events);
//    static size_t max_events_required(uint32_t bytes_per_message, bool include_pin_events) {
//        size_t events_per_bit = 2 * 2; // 2 for SCL. 0-2 for SDA
//        if (include_pin_events) {
//            events_per_bit *= 2;    // If we're driving the pins then there's an extra event per edge.
//        }
//        const size_t events_per_byte = 9 * events_per_bit;  // 8 bits + the ACK
//        return (2 * events_per_bit) +  // Allow for START and STOP bits
//               (events_per_byte * (bytes_per_message + 1)); // Allow for address byte
//    }

    // max_events: maximum number of bus events that can be stored in this trace.
    // Additional events are dropped. Must be less than SIZE_MAX.
    BusTrace(size_t max_events);

    // Adds an event to the trace as long as there is space for it.
    // Discards the event if there's no more space
    void record_event(const BusEventFlags& event);

    // TODO: How to compare traces?
    // Do you compare times as well as edges?
    // Do we compare pin changes
    // Returns the index of the first BusEvent that doesn't match
    // or SIZE_MAX if the traces are equivalent.
    size_t compare_to(const BusTrace& other);

    // Overload to make it possible to compare a Trace to an expected Trace.
    size_t compare_to(const BusEvent* other);

private:
    uint32_t start_millis;  // Absolute time that the trace started
    uint32_t start_ticks;   // Tick count at start

    // The events. Max 4 events per I2C data bit
    // TODO: Maybe we should pass this array to the constructor so it can be a compile time object
    // Alternatively we may need to 'new' it.
    BusEvent events[];
};

// Records the electrical activity on I2C bus.
// A trace cannot last longer than 2^32 processor cycles.
// The trace is driven by interrupts so it will continue
// recording while the processor performs other work.
class BusRecorder {
    // Stops any recording that's in progress and then
    // starts recording. Bus events are added to 'trace'.
    // The recording stops automatically when the trace is full.
    void start(BusTrace& trace);

    // Stops recording
    void stop();

    // Returns true if we're recording
    bool is_recording();
};

}
#endif //I2C_UNDERNEATH_BUS_TRACE_H