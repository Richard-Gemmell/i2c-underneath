// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#pragma once

#include <cstdint>
#include <Printable.h>
#include <analysis/duration_statistics.h>

namespace line_test {

class LineTestReport /*: public Printable*/ {
public:
    LineTestReport(
            analysis::DurationStatistics measured_fall_time,
            analysis::DurationStatistics measured_rise_time,
            analysis::DurationStatistics estimated_fall_time,
            analysis::DurationStatistics estimated_rise_time
    );

    // Returns an estimate of the "rise time" as defined by the
    // I2C spec. (The time for a signal to rise from 0.3 to 0.7 Vdd)
    // The estimate assumes the signal follows an RC curve and will
    // be incorrect if it doesn't.
    //
    // The measurement process tends to return values in steps of 40
    // nanoseconds starting at 5. Rise times less than 40 nanos get
    // reported as 45. Times between 45 and 85 nanos are reported as
    // 85 etc.
    //
    // The time will be UINT32_MAX if the measurement timed out
    // before the pin changed state. This happens if there aren't
    // any pullup resistors on the line or if the rise time is
    // hopelessly slow.
    analysis::DurationStatistics get_estimated_rise_time() {
        return estimated_rise_time;
    }

    // Returns an estimate of the "fall time" as defined by the
    // I2C spec. (The time for a signal to fall from 0.7 to 0.3 Vdd)
    // The estimate assumes the signal follows an RC curve and will
    // be incorrect if it doesn't.
    //
    // The measurement process tends to return values in steps of 40
    // nanoseconds. Falls times less than 5 get reported as 5 nanos.
    // Fall times between 5 and 45 nanos get reported as 45 etc.
    //
    // The time will be UINT32_MAX if the measurement timed out
    // before the pin changed state.
    analysis::DurationStatistics get_estimated_fall_time() {
        return estimated_fall_time;
    }

    // The actual time from when the pin was set to 1 and the
    // pin reporting a value of 1. On the Teensy 4 this is the
    // time for the line to rise from Vss (GND) to 0.5Vdd (1.65V).
    //
    // This is NOT the "rise time" as defined in the I2C spec.
    //
    // The time will be UINT32_MAX if the measurement timed out
    // before the pin changed state.
    analysis::DurationStatistics get_0_to_1_time() {
        return measured_rise_time;
    }

    // The actual time between the pin being set to 0 and the
    // pin reporting a value of 0. On the Teensy 4 this is the
    // time for the line to fall from Vdd (3.3V) to 0.5Vdd (1.65V).
    //
    // This is NOT the "fall time" as defined in the I2C spec.
    //
    // The time will be UINT32_MAX if the measurement timed out
    // before the pin changed state.
    analysis::DurationStatistics get_1_to_0_time() {
        return measured_fall_time;
    }

//    size_t printTo(Print& p) const override;
private:
    analysis::DurationStatistics measured_fall_time;
    analysis::DurationStatistics measured_rise_time;
    analysis::DurationStatistics estimated_fall_time;
    analysis::DurationStatistics estimated_rise_time;
};

// Runs electrical and timing tests on an I2C bus.
class LineTester {
public:
    // Runs the following tests on whichever I2C line you connect 'pin'
    // to:
    //   - estimate rise time
    //   - estimate fall time
    // 'pin' - connect this pin to whichever bus line you want to
    // test. (SDA or SCL) Don't use a pin that's already being used
    // as an I2C master or slave.
    // 'timeout' - the maximum time the test will wait for the pin
    // to respond. The test cannot measure a rise or fall time that
    // takes longer than this.
    //
    // WARNINGS:
    // You can run this test on any line which is being driven by
    // open-drain or open-collector pins. These are lines that
    // require pull-up resistors. You may damage one of your devices
    // if you run this test on a line controlled by a push-pull pin.
    //
    // This test will not give correct results unless all I2C devices
    // are connected to the bus and active. The slaves must be listening
    // and the master must have started the bus. This is because active
    // devices affect the electrical behaviour of the line.
    //
    // Connecting an oscilloscope to the line will increase the rise time.
    // Detaching the scope to get a more accurate result. Connecting unused
    // pins can also affect the result.
    //
    // This test configures the pin and drives the bus line it's
    // connected to. This may cause one or more devices to get
    // confused and cause the bus to become stuck. This is less
    // likely to happen if you test SDA first and then SCL.
    static LineTestReport TestLine(uint8_t pin, uint32_t timeout_in_nanos = 3000);

private:
    static uint32_t measure_line_change(uint8_t pin, bool high, uint32_t timeout_nanos);

    // The conversion factor used to estimate rise/fall times
    // from measured values.
    // It's the ratio of the time taken for an RC curve to rise
    // from (0.3 Vdd to 0.7 Vdd) versus (0 to 0.5 Vdd).
    //
    // It's the same value for both rise and fall times.
    //
    // Assumes both the rise and fall signals are perfect RC curves.
    constexpr const static double ScaleMeasuredToEstimate = 1.222;

    static void include_edge_times(uint32_t measured_time, analysis::DurationStatistics& measured, analysis::DurationStatistics& estimated);
};

} // line_test
