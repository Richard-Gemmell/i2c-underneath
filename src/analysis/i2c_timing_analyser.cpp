// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "i2c_timing_analyser.h"

namespace analysis {
I2CTimingAnalysis I2CTimingAnalyser::analyse(const bus_trace::BusTrace& trace) {
    // TODO: check that the trace is well formed
    // maybe get the trace to normalise itself first or maybe that's up to the caller
//    bool well_formed = true;
    // Edge zero should be both lines high
    size_t start_bit_index = 1;
    // The next edge must be SCL going LOW
    size_t current_edge = start_bit_index + 1;
//    const bus_trace::BusEvent* event = trace.event(current_edge);
//    if(event->flags != (bus_trace::SCL_LINE_CHANGED | bus_trace::BusEventFlags::SCL_LINE_STATE)) {
//        well_formed = false;
//        return {.well_formed = well_formed};
//    }
    DurationStatistics start_hold_time;
    start_hold_time.include(trace.nanos_to_previous(current_edge));

    DurationStatistics scl_low_time;
    DurationStatistics scl_high_time;
    DurationStatistics clock_frequency;
    DurationStatistics data_hold_time;
    DurationStatistics data_setup_time;
    DurationStatistics stop_setup_time;
    DurationStatistics bus_free_time;

    size_t previous_scl_rise_event = 0;
    size_t previous_scl_fall_event = current_edge;
    size_t latest_clock_low = 0;
    for (++current_edge; current_edge < trace.event_count(); ++current_edge) {
        auto flags = trace.event(current_edge)->flags;
        if(flags & bus_trace::BusEventFlags::SCL_LINE_CHANGED) {
            if(flags & bus_trace::BusEventFlags::SCL_LINE_STATE) {
                // SCL LOW -> HIGH
                previous_scl_rise_event = current_edge;
                latest_clock_low = trace.nanos_between(current_edge, previous_scl_fall_event);
//                Serial.printf("Index %d LOW time %d\n", current_edge, latest_clock_low);
                scl_low_time.include(latest_clock_low);
            } else {
                // SCL HIGH -> LOW
                previous_scl_fall_event = current_edge;
                auto clock_high = trace.nanos_between(current_edge, previous_scl_rise_event);
//                Serial.printf("Index %d HIGH time %d\n", current_edge, clock_high);
                scl_high_time.include(clock_high);

                // Calculate frequency for the last clock cycle
                auto period = clock_high + latest_clock_low;
                auto frequency = (uint32_t)((1e9 * 1.0) / period);
//                Serial.printf("Index %d period %d frequency %d\n", current_edge, period, frequency);
                clock_frequency.include(frequency);
            }
        }
        else {
            if(flags & bus_trace::BusEventFlags::SCL_LINE_STATE) {
                // This is a STOP condition
                uint32_t delta = trace.nanos_to_previous(current_edge);
                stop_setup_time.include(delta);
            }
        }
    }

    return {
//        .well_formed = well_formed,
        .clock_frequency = clock_frequency,
        .start_hold_time = start_hold_time,
        .scl_low_time = scl_low_time,
        .scl_high_time = scl_high_time,
        .data_hold_time = data_hold_time,
        .data_setup_time = data_setup_time,
        .stop_setup_time = stop_setup_time,
        .bus_free_time = bus_free_time,
    };
}

//size_t I2CTimingAnalyser::find_start_bit(const bus_trace::BusTrace& trace) {
//    for (int i = 0; i < trace.event_count(); ++i) {
//
//    }
//    return 0;
//}

} // analysis