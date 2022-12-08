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
    // The next edge must be SCL going LOW
    DurationStatistics start_hold_time_stats;
    DurationStatistics scl_low_time_stats;
    DurationStatistics scl_high_time_stats;
    DurationStatistics start_setup_time_stats;
    DurationStatistics clock_frequency_stats;
    DurationStatistics data_hold_time_stats;
    DurationStatistics data_setup_time_stats;
    DurationStatistics stop_setup_time_stats;
    DurationStatistics bus_free_time_stats;

    // Edge zero should be both lines high. Ignore it.
    size_t current_edge = 0;
    size_t previous_scl_rise_event = 0;
    size_t previous_scl_fall_event = current_edge;
    size_t latest_clock_low = 0;
    for (++current_edge; current_edge < trace.event_count(); ++current_edge) {
        auto previous_event = trace.event(current_edge - 1);
        auto current_event = trace.event(current_edge);
        auto flags = current_event->flags;
        if(flags & bus_trace::BusEventFlags::SCL_LINE_CHANGED) {
            // SCL changed
            if(current_event->scl_rose()) {
                // SCL LOW -> HIGH
                previous_scl_rise_event = current_edge;
                latest_clock_low = trace.nanos_between(current_edge, previous_scl_fall_event);
//                Serial.printf("Index %d LOW time %d\n", current_edge, latest_clock_low);
                scl_low_time_stats.include(adjust_clock_low_time(latest_clock_low, scl_fall_time, scl_rise_time));
            } else {
                // SCL HIGH -> LOW
                previous_scl_fall_event = current_edge;
                if (previous_event->scl_rose()) {
                    // This is a data bit or a NACK/ACK
                    auto clock_high = trace.nanos_between(current_edge, previous_scl_rise_event);
//                    Serial.printf("Index %d HIGH time %d\n", current_edge, clock_high);
                    scl_high_time_stats.include(adjust_clock_high_time(clock_high, scl_rise_time, scl_fall_time));

                    // Calculate frequency for the last clock cycle
                    auto period = clock_high + latest_clock_low;
                    auto frequency = (uint32_t)((1e9 * 1.0) / period);
//                    Serial.printf("Index %d period %d frequency %d\n", current_edge, period, frequency);
                    clock_frequency_stats.include(frequency);
                }
                else if(previous_event->sda_fell()) {
                    // SCL HIGH -> LOW after SDA fell. This is a START condition.
                    uint32_t start_hold_time = adjust_start_hold_time(trace.nanos_to_previous(current_edge), sda_fall_time, scl_fall_time);
                    start_hold_time_stats.include(start_hold_time);
                }
                else {
                    Serial.println("Invalid Trace: Found SCL falling edge but not a data bit or STOP condition.");
                }
            }
        } else {
            // SDA changed
            if(flags & bus_trace::BusEventFlags::SCL_LINE_STATE) {
                // SDA LOW -> HIGH while SCL is HIGH. This is a STOP or START condition.
                if (current_event->sda_rose()) {
                    // SDA LOW -> HIGH while SCL is HIGH. This is a STOP condition.
                    uint32_t setup_stop_time = adjust_setup_stop_time(trace.nanos_to_previous(current_edge), sda_rise_time, scl_rise_time);
                    stop_setup_time_stats.include(setup_stop_time);
                } else {
                    // SDA HIGH -> LOW while SCL is HIGH. This is a START condition.
                    if (previous_event->scl_rose()) {
                        // This is a repeated START condition
                        uint32_t setup_start_time = adjust_setup_start_time(trace.nanos_to_previous(current_edge), sda_fall_time, scl_rise_time);
                        start_setup_time_stats.include(setup_start_time);
                    } else if (previous_event->sda_rose()) {
                        // This is START following a STOP
                        uint32_t bus_free_time = adjust_bus_free_time(trace.nanos_to_previous(current_edge), sda_rise_time, sda_fall_time);
                        bus_free_time_stats.include(bus_free_time);
                    } else if (previous_event->flags == (bus_trace::BusEventFlags::SDA_LINE_STATE | bus_trace::BusEventFlags::SCL_LINE_STATE)) {
                        // This is START at the beginning of a trace
                        // There are no I2C requirements for the interval so ignore it.
                    } else {
                        // The previous event must have been SDA falling as well.
                        // This doesn't make sense.
                        Serial.println("Invalid Trace: Found 2 falling edges on SDA in a row.");
                    }
                }
            }
        }
    }

    return {
//        .well_formed = well_formed,
        .clock_frequency = clock_frequency_stats,
        .start_hold_time = start_hold_time_stats,
        .scl_low_time = scl_low_time_stats,
        .scl_high_time = scl_high_time_stats,
        .start_setup_time = start_setup_time_stats,
        .data_hold_time = data_hold_time_stats,
        .data_setup_time = data_setup_time_stats,
        .stop_setup_time = stop_setup_time_stats,
        .bus_free_time = bus_free_time_stats,
    };
}

// The Teensy triggers too soon when SCL rises and too late when SDA rises.
uint32_t I2CTimingAnalyser::adjust_setup_stop_time(uint32_t raw_time, uint16_t sda_rise_time, uint16_t scl_rise_time) {
    auto adjusted =  (uint32_t)(raw_time - (scl_rise_time * RISE_TRIGGER_to_V0_7) - (sda_rise_time * RISE_V0_3_to_TRIGGER));
//    Serial.printf("Setup stop time (tSU;STO) raw: %d adjusted %d\n", raw_time, adjusted);
    return adjusted;
}

// The Teensy triggers too soon when SCL rises and too late when SDA falls.
uint32_t I2CTimingAnalyser::adjust_setup_start_time(uint32_t raw_time, uint16_t sda_fall_time, uint16_t scl_rise_time) {
    auto adjusted = (uint32_t)(raw_time - (scl_rise_time * RISE_TRIGGER_to_V0_7) - (sda_fall_time * FALL_V0_7_to_TRIGGER));
//    Serial.printf("Setup time for repeated start (tSU;STA) raw: %d adjusted %d\n", raw_time, adjusted);
    return adjusted;
}

// Adjusts for the fact that the Teensy triggers too soon when SDA falls and too late when SCL falls.
uint32_t I2CTimingAnalyser::adjust_start_hold_time(uint32_t raw_time, uint16_t sda_fall_time, uint16_t scl_fall_time) {
    auto adjusted = (uint32_t)(raw_time - (sda_fall_time * FALL_TRIGGER_to_V0_3) - (scl_fall_time * FALL_V0_7_to_TRIGGER));
//    Serial.printf("Start hold (tHD;STA) raw: %d adjusted %d\n", raw_time, adjusted);
    return adjusted;
}

// Adjusts for the fact that the Teensy triggers at 0.5 Vdd
uint32_t I2CTimingAnalyser::adjust_clock_low_time(uint32_t raw_time, uint16_t scl_fall_time, uint16_t scl_rise_time) {
    auto adjusted = (uint32_t)(raw_time - (scl_fall_time * FALL_TRIGGER_to_V0_3) - (scl_rise_time * RISE_V0_3_to_TRIGGER));
//    Serial.printf("Clock low (tLOW) raw: %d adjusted %d\n", raw_time, adjusted);
    return adjusted;
}

// Adjusts for the fact that the Teensy triggers at 0.5 Vdd
uint32_t I2CTimingAnalyser::adjust_clock_high_time(uint32_t raw_time, uint16_t scl_rise_time, uint16_t scl_fall_time) {
    auto adjusted = (uint32_t)(raw_time - (scl_rise_time * RISE_TRIGGER_to_V0_7) - (scl_fall_time * FALL_V0_7_to_TRIGGER));
//    Serial.printf("Clock high (tHIGH) raw: %d adjusted %d\n", raw_time, adjusted);
    return adjusted;
}

uint32_t I2CTimingAnalyser::adjust_bus_free_time(uint32_t raw_time, uint16_t sda_rise_time, uint16_t sda_fall_time) {
    auto adjusted = (uint32_t)(raw_time - (sda_rise_time * RISE_TRIGGER_to_V0_7) - (sda_fall_time * FALL_V0_7_to_TRIGGER));
//    Serial.printf("Bus Free Time (tBUF) raw: %d adjusted %d\n", raw_time, adjusted);
    return adjusted;
}

} // analysis
