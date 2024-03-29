// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#pragma once

#include <bus_trace/bus_trace.h>
#include <analysis/i2c_timing_analysis.h>

namespace analysis {

class I2CTimingAnalyser {
public:
    static const uint16_t DEFAULT_FALL_TIME = 8;

    // Analyses the timings in an I2C transaction. The resulting times can be
    // compared to the I2C Specification
    //
    // Gives misleading results unless the rise and fall times are set correctly.
    // (e.g. reporting that the setup stop time, tSU;STO, is valid when it's not.
    // You can measure the rise and fall times with LineTester::TestLine()
    // or an oscilloscope.
    //
    // Will not record all times if 'trace' contains any events in which SDA and
    // SCL changed at the same time. You can use BusTrace::to_message() to split
    // merged events.
    static I2CTimingAnalysis analyse(const bus_trace::BusTrace& trace,
                                     uint16_t sda_rise_time,
                                     uint16_t scl_rise_time,
                                     uint16_t sda_fall_time = DEFAULT_FALL_TIME,
                                     uint16_t scl_fall_time = DEFAULT_FALL_TIME);

private:
    class Adjuster {
    public:
        Adjuster(uint16_t sda_rise_time, uint16_t sda_fall_time, uint16_t scl_rise_time, uint16_t scl_fall_time);

        uint32_t setup_stop_time(uint32_t raw_time) const;
        uint32_t setup_start_time(uint32_t raw_time) const;
        uint32_t start_hold_time(uint32_t raw_time) const;
        uint32_t clock_low_time(uint32_t raw_time) const;
        uint32_t clock_high_time(uint32_t raw_time) const;
        uint32_t bus_free_time(uint32_t raw_time) const;
        uint32_t data_setup_time(uint32_t raw_time, bool sda_rose) const;
        uint32_t data_hold_time(uint32_t raw_time, bool sda_rose) const;

    private:
        // Time to rise from GND to 0.3 Vdd as a proportion of the rise time
        constexpr static double RISE_V0_to_V0_3 = 0.421;
        // Time to rise from 0.3 Vdd to the Teensy's trigger voltage (0.5 Vdd), as a proportion of the rise time
        constexpr static double RISE_V0_3_to_TRIGGER = 0.397;
        // Time to rise from the Teensy's trigger voltage (0.5 Vdd) to 0.7 Vdd, as a proportion of the rise time
        constexpr static double RISE_TRIGGER_to_V0_7 = 0.603;

        // Time to fall from Vdd to 0.7 Vdd as a proportion of the rise time
        constexpr static double FALL_V1_to_V0_7 = RISE_V0_to_V0_3;
        // Time to fall from 0.7 Vdd to the Teensy's trigger voltage (0.5 Vdd), as a proportion of the fall time
        constexpr static double FALL_V0_7_to_TRIGGER = RISE_V0_3_to_TRIGGER;
        // Time to fall from the Teensy's trigger voltage (0.5 Vdd) to 0.3 Vdd, as a proportion of the fall time
        constexpr static double FALL_TRIGGER_to_V0_3 = RISE_TRIGGER_to_V0_7;

        double scl_rise_trigger_to_V0_7() const {
            return scl_rise_time * RISE_TRIGGER_to_V0_7;
        }

        double scl_rise_V0_3_to_trigger() const {
            return scl_rise_time * RISE_V0_3_to_TRIGGER;
        }

        double scl_fall_V0_7_to_trigger() const {
            return scl_fall_time * FALL_V0_7_to_TRIGGER;
        }

        double scl_fall_trigger_to_V0_3() const {
            return scl_fall_time * FALL_TRIGGER_to_V0_3;
        }

        double sda_rise_trigger_to_V0_7() const {
            return sda_rise_time * RISE_TRIGGER_to_V0_7;
        }

        double sda_rise_V0_3_to_trigger() const {
            return sda_rise_time * RISE_V0_3_to_TRIGGER;
        }

        double sda_fall_V0_7_to_trigger() const {
            return sda_fall_time * FALL_V0_7_to_TRIGGER;
        }

        double sda_fall_trigger_to_V0_3() const {
            return sda_fall_time * FALL_TRIGGER_to_V0_3;
        }

        uint16_t sda_rise_time;
        uint16_t sda_fall_time;
        uint16_t scl_rise_time;
        uint16_t scl_fall_time;
    };
};

} // analysis
