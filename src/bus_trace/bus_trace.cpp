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

BusTrace::BusTrace(BusEvent* events, size_t max_event_count)
    : events(events), max_event_count(max_event_count) {
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

void BusTrace::add_event(const BusEvent& event) {
    if(current_event_count == max_event_count) {
        // We can't take another event. Discard it.
        return;
    }
    events[current_event_count] = BusEvent(event.delta_t_in_ticks, event.flags);
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

size_t BusTrace::printTo(Print& p) const {
    String sda("SDA ");
    String scl("SCL ");
    for (size_t i = 0; i < event_count(); ++i) {
        const BusEvent* e = event(i);
        append_event_symbol(sda, true, e->flags);
        append_event_symbol(scl, false, e->flags);
    }
    sda.append("\r\n");
    scl.append("\r\n");
    size_t count = p.print(sda);
    count += p.print(scl);
    return count;
}

void BusTrace::append_event_symbol(String& string, bool sda, BusEventFlags flags) {
    bool level;
    if(sda) {
        level = (flags & BusEventFlags::SDA_LINE_STATE) != BusEventFlags::BOTH_LOW_AND_UNCHANGED;
    } else {
        level = (flags & BusEventFlags::SCL_LINE_STATE) != BusEventFlags::BOTH_LOW_AND_UNCHANGED;
    }
    bool edge;
    if(sda) {
        edge = (flags & BusEventFlags::SDA_LINE_CHANGED) != BusEventFlags::BOTH_LOW_AND_UNCHANGED;
    } else {
        edge = (flags & BusEventFlags::SCL_LINE_CHANGED) != BusEventFlags::BOTH_LOW_AND_UNCHANGED;
    }
    // It would be nice to use the UTF-8 characters "↑↓_‾" but they're not supported by serial monitor.
    // Stick to ASCII instead.
    if(level) {
        edge ? string.append('/') : string.append("'");
    } else {
        edge ? string.append('\\') : string.append('_');
    }
}

}