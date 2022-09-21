// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "i2c_timing_analyser.h"

namespace analysis {
I2CTimingAnalysis I2CTimingAnalyser::analyse(const bus_trace::BusTrace& trace,
                                             uint16_t sda_rise_time, uint16_t scl_rise_time,
                                             uint16_t sda_fall_time, uint16_t scl_fall_time) {
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
    DurationStatistics start_hold_time_statistics;
    uint32_t start_hold_time = adjust_start_hold_time(trace.nanos_to_previous(current_edge), sda_fall_time,
                                                      scl_fall_time);
    start_hold_time_statistics.include(start_hold_time);

    DurationStatistics scl_low_time;
    DurationStatistics scl_high_time;
    DurationStatistics clock_frequency;
    DurationStatistics data_hold_time;
    DurationStatistics data_setup_time;
    DurationStatistics stop_setup_time_stats;
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
                uint32_t setup_stop_time = adjust_setup_stop_time(trace.nanos_to_previous(current_edge), sda_rise_time, scl_rise_time);
                stop_setup_time_stats.include(setup_stop_time);
            }
        }
    }

    return {
//        .well_formed = well_formed,
        .clock_frequency = clock_frequency,
        .start_hold_time = start_hold_time_statistics,
        .scl_low_time = scl_low_time,
        .scl_high_time = scl_high_time,
        .data_hold_time = data_hold_time,
        .data_setup_time = data_setup_time,
        .stop_setup_time = stop_setup_time_stats,
        .bus_free_time = bus_free_time,
    };
}

// The Teensy triggers too soon when SCL rises line and too late when SDA rises.
uint32_t I2CTimingAnalyser::adjust_setup_stop_time(uint32_t raw_time, uint16_t sda_rise_time, uint16_t scl_rise_time) {
        uint32_t adjusted =  raw_time - (scl_rise_time * RISE_TRIGGER_to_V0_7) - (sda_rise_time * RISE_V0_3_to_TRIGGER);
//        Serial.printf("Setup stop time (tSU;STO) raw: %d adjusted %d\n", raw_time, adjusted);
        return adjusted;
}

// Adjusts for the fact that the Teensy triggers too soon when SDA falls and too late when SCL falls.
uint32_t I2CTimingAnalyser::adjust_start_hold_time(uint32_t raw_time, uint16_t sda_fall_time, uint16_t scl_fall_time) {
        uint32_t adjusted = raw_time - (sda_fall_time * FALL_TRIGGER_to_V0_3) - (scl_fall_time * FALL_V0_7_to_TRIGGER);
//        Serial.printf("Start hold (tHD;STA) raw: %d adjusted %d\n", raw_time, adjusted);
        return adjusted;
    }

} // analysis