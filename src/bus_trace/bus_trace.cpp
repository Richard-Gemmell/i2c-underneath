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

BusTrace::BusTrace(BusEvent* events, size_t max_event_count, const common::hal::Clock& clock)
    : clock(clock), events(events), max_event_count(max_event_count) {
}

size_t BusTrace::event_count() const {
    return current_event_count;
}

const BusEvent* BusTrace::event(size_t index) const {
    if(index+1 <= current_event_count) {
        return &events[index];
    }
    return nullptr;
}

void BusTrace::record_event(const BusEventFlags& event) {
    if(current_event_count == max_event_count) {
        // We can't take another event. Discard it.
        return;
    }
    uint32_t system_ticks = clock.GetSystemTick();
    uint32_t delta_t_in_ticks = 0;
    if(current_event_count > 0) {
        delta_t_in_ticks = system_ticks - ticks_at_latest_event;
    }
    events[current_event_count] = BusEvent(delta_t_in_ticks, event);
    ticks_at_latest_event = system_ticks;
    current_event_count++;
}

size_t BusTrace::compare_to(const BusEvent* other, size_t other_event_count) {
    size_t min_count = min(current_event_count, other_event_count);
    for (size_t i = 0; i < min_count; ++i) {
        if(event(i)->flags != other[i].flags) {
            return i;
        }
    }
    if(min_count < max(current_event_count, other_event_count)) {
        return min_count;
    }
    return SIZE_MAX;
}

}