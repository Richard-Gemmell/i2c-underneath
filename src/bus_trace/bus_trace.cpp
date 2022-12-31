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
    : clock(nullptr), events(events), created_events(false), max_event_count(max_event_count) {
}

BusTrace::BusTrace(size_t max_event_count)
    : clock(nullptr), events(new BusEvent[max_event_count]), created_events(true), max_event_count(max_event_count) {
}

BusTrace::BusTrace(const common::hal::Clock* clock, size_t max_event_count)
    : clock(clock), events(new BusEvent[max_event_count]), created_events(true), max_event_count(max_event_count) {
}

BusTrace::~BusTrace() {
    if (created_events && events) {
        delete[] events;
        events = nullptr;
    }
}

const BusEvent* BusTrace::event(size_t index) const {
    if (index + 1 <= current_event_count) {
        return &events[index];
    }
    return nullptr;
}

void BusTrace::reset() {
    current_event_count = 0;
    set_ticks_start();
}

class BusTraceIterator {
public:
    BusTraceIterator(const BusTrace& trace, bool split_events)
        : trace(trace), split_events(split_events) {
    }

    size_t event_count() const {
        if (split_events) {
            size_t count = 0;
            for (size_t i = 0; i < trace.event_count(); ++i) {
                if (is_merged_event(trace.event(i))) {
                    count += 2;
                } else {
                    count++;
                }
            }
            return count;
        }
        return trace.event_count();
    }

    bool has_next() const {
        return get_second_of_pair_next || next_index < trace.event_count();
    }

    BusEvent next() {
        if (!split_events) {
            return *trace.event(next_index++);
        }
        if (get_second_of_pair_next) {
            get_second_of_pair_next = false;
            return second_event_of_pair;
        }
        auto event = trace.event(next_index++);
        if (is_merged_event(event)) {
            split_event(event, event->flags & SCL_LINE_STATE);
            get_second_of_pair_next = true;
            return first_event_of_pair;
        }
        return *event;
    }

private:
    const BusEventFlags SDA_MASK = SDA_LINE_CHANGED | SDA_LINE_STATE;
    const BusEventFlags SCL_MASK = SCL_LINE_CHANGED | SCL_LINE_STATE;
    const BusTrace& trace;
    bool split_events;
    bool get_second_of_pair_next = false;
    BusEvent first_event_of_pair = BusEvent(0, BOTH_LOW_AND_UNCHANGED);
    BusEvent second_event_of_pair = BusEvent(0, BOTH_LOW_AND_UNCHANGED);
    size_t next_index = 0;

    static bool is_merged_event(const BusEvent* event) {
        const BusEventFlags both_changed = SDA_LINE_CHANGED | SCL_LINE_CHANGED;
        return (event->flags & both_changed) == both_changed;
    }

    void split_event(const BusEvent* event, bool sda_first) {
        if (sda_first) {
            BusEventFlags initial_sda_state = event->flags & SDA_MASK;
            BusEventFlags initial_scl_state = (~event->flags) & SCL_MASK;
            first_event_of_pair = BusEvent(event->delta_t_in_ticks, initial_sda_state | initial_scl_state);
            BusEventFlags final_sda_state = event->flags & SDA_LINE_STATE;
            BusEventFlags final_scl_state = event->flags & SCL_MASK;
            second_event_of_pair = BusEvent(0, final_sda_state | final_scl_state);
        } else {
            BusEventFlags initial_sda_state = (~event->flags) & SDA_MASK;
            BusEventFlags initial_scl_state = event->flags & SCL_MASK;
            first_event_of_pair = BusEvent(event->delta_t_in_ticks, initial_sda_state | initial_scl_state);
            BusEventFlags final_sda_state = event->flags & SDA_MASK;
            BusEventFlags final_scl_state = event->flags & SCL_LINE_STATE;
            second_event_of_pair = BusEvent(0, final_sda_state | final_scl_state);
        }
    }
};

BusTrace BusTrace::to_message(bool merge_sda_edges, bool split_events) const {
    auto sda_changed_while_scl_low = [](BusEventFlags flags) {
        bool scl_low = (flags & BusEventFlags::SCL_LINE_STATE) != BusEventFlags::SCL_LINE_STATE;
        bool sda_changed = (flags & BusEventFlags::SDA_LINE_CHANGED) == BusEventFlags::SDA_LINE_CHANGED;
        return scl_low && sda_changed;
    };
    auto only_sda_changed = [](BusEventFlags previous, BusEventFlags next) {
        BusEventFlags sda_flags = (BusEventFlags::SDA_LINE_STATE | BusEventFlags::SDA_LINE_CHANGED);
        return (previous | sda_flags) == (next | sda_flags);
    };
    BusTraceIterator iter(*this, split_events);
    BusTrace message(clock, iter.event_count());
    if (current_event_count > 0) {
        BusEvent previous = iter.next();
        bool has_previous = true;
        while (iter.has_next()) {
            const BusEvent next = iter.next();
            bool previous_may_be_spurious = sda_changed_while_scl_low(previous.flags);
            bool next_may_be_spurious = sda_changed_while_scl_low(next.flags);
            if (merge_sda_edges && previous_may_be_spurious && next_may_be_spurious && only_sda_changed(previous.flags, next.flags)) {
                // The two events cancel out. Throw them away.
                if (iter.has_next()) {
                    previous = iter.next();
                } else {
                    has_previous = false;
                }
            } else {
                message.add_event(previous);
                previous = next;
            }
        }
        if (has_previous) {
            message.add_event(previous);
        }
    }
    return message;
}

size_t BusTrace::compare_messages(const BusTrace& other) const {
    return to_message().compare_edges(other.to_message());
}

size_t BusTrace::compare_bus_events(const BusTrace& other, const std::function<bool(size_t)>& are_equal) const {
    size_t min_count = min(current_event_count, other.event_count());
    for (size_t i = 0; i < min_count; ++i) {
        if (!are_equal(i)) {
            return i;
        }
    }
    if (current_event_count != other.event_count()) {
        return min_count;
    }
    return SIZE_MAX;
}

size_t BusTrace::compare_edges(const BusTrace& other) const {
    return compare_bus_events(other, [this, &other](size_t index) {
        return this->event(index)->flags == other.event(index)->flags;
    });
}

size_t BusTrace::is_identical_to(const BusTrace& other) const {
    return compare_bus_events(other, [this, &other](size_t index) {
        return *event(index) == *other.event(index);
    });
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
    if (sda) {
        level = (flags & BusEventFlags::SDA_LINE_STATE) != BusEventFlags::BOTH_LOW_AND_UNCHANGED;
    } else {
        level = (flags & BusEventFlags::SCL_LINE_STATE) != BusEventFlags::BOTH_LOW_AND_UNCHANGED;
    }
    bool edge;
    if (sda) {
        edge = (flags & BusEventFlags::SDA_LINE_CHANGED) != BusEventFlags::BOTH_LOW_AND_UNCHANGED;
    } else {
        edge = (flags & BusEventFlags::SCL_LINE_CHANGED) != BusEventFlags::BOTH_LOW_AND_UNCHANGED;
    }
    // It would be nice to use the UTF-8 characters "↑↓_‾" but they're not supported by serial monitor.
    // Stick to ASCII instead.
    if (level) {
        edge ? string.append('/') : string.append("'");
    } else {
        edge ? string.append('\\') : string.append('_');
    }
}

uint32_t BusTrace::nanos_to_previous(size_t index) const {
    if (out_of_range(index) || !clock) {
        // Can't calculate a result. Return an error code.
        return UINT32_MAX;
    }
    if (index) {
        return clock->ticks_to_nanos(events[index].delta_t_in_ticks);
    }
    // It doesn't make sense to return a value for the first event so return 0.
    // This is mainly to allow us to change BusEvent to hold absolute
    // tick values in the future.
    return 0;
}

uint32_t BusTrace::nanos_between(size_t to, size_t from) const {
    if (out_of_range(from) || out_of_range(to)
        || from > to
        || !clock) {
        return UINT32_MAX;
    }
    uint32_t total_ticks = 0;
    for (size_t i = from + 1; i <= to; ++i) {
        total_ticks += events[i].delta_t_in_ticks;
    }
    return clock->ticks_to_nanos(total_ticks);
}

bool BusTrace::out_of_range(size_t index) const {
    return index >= event_count();
}

}
