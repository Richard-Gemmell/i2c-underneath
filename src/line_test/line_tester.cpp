// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)


#include "line_tester.h"
#include <Arduino.h>
#include <cmath>
#include <core_pins.h>
#include <common/hal/teensy/teensy_timestamp.h>
#include <common/hal/teensy/super_fast_io.h>

namespace line_test {

LineTestReport::LineTestReport(
        analysis::DurationStatistics measured_fall_time,
        analysis::DurationStatistics measured_rise_time,
        analysis::DurationStatistics estimated_fall_time,
        analysis::DurationStatistics estimated_rise_time)
        : measured_fall_time(measured_fall_time),
          measured_rise_time(measured_rise_time),
          estimated_fall_time(estimated_fall_time),
          estimated_rise_time(estimated_rise_time) {
}

// This implementation sets the pin and then polls it until
// the level changes. The poll takes about 40 nanos. Any
// edge that takes less than this is reported as 5ns.
// Edges that take 2 polls get reported as 45 nanos etc.
// This quantisation effect fades away above 100 nanos as
// individual times vary a little and the average gets closer
// to the truth.
//
// In practice, the measurement are always a bit too high.
// Actual rise times for the Teensy are nearly always 15 to 20 nanos
// less than the estimate. This is handy for estimating I2C rise times
// as it's better to aim low than high.
//
// For a 1 MHz bus, the measurement accuracy is only good enough
// to split rise times into Fast, Medium, Slow and too slow.
uint32_t LineTester::measure_line_change(uint8_t pin, bool high, uint32_t timeout_nanos) {
    volatile uint32_t const* read_register = getDigitalReadPort(pin);
    const uint32_t bitmask = digitalPinToBitMask(pin);
    const uint32_t expected = high ? bitmask : 0;
    volatile uint32_t* write_register = getDigitalWritePort(pin, high);
    const uint32_t timeout = common::hal::TeensyTimestamp::nanos_to_ticks(timeout_nanos);

    uint32_t stop;
    noInterrupts()
    const uint32_t start = ARM_DWT_CYCCNT;
    *write_register = bitmask;
    do {
        stop = ARM_DWT_CYCCNT;
    } while(expected != (bitmask & *read_register) && ((stop - start) < timeout));
    interrupts()

    if(expected == (bitmask & *read_register)) {
        return common::hal::TeensyTimestamp::nanos_between(start, stop);
    } else {
        return UINT32_MAX;
    }
}

LineTestReport LineTester::TestLine(uint8_t pin, uint32_t timeout_in_nanos) {
    analysis::DurationStatistics measured_fall_time;
    analysis::DurationStatistics estimated_fall_time;
    analysis::DurationStatistics measured_rise_time;
    analysis::DurationStatistics estimated_rise_time;

    pinMode(pin, OUTPUT_OPENDRAIN);
    const uint32_t delay_nanos = 5000;  // Gives a 100kHz signal which helps to avoid bus problems
    const int repeats = 9;  // Should do soft reset of the bus if the line is SCL
    for (int i = 0; i < repeats; ++i) {
        delayNanoseconds(delay_nanos);
        uint32_t rise = LineTester::measure_line_change(pin, true, timeout_in_nanos);
        measured_rise_time.include(rise);
        estimated_rise_time.include(std::llround(ScaleMeasuredToEstimate * rise));

        delayNanoseconds(delay_nanos);
        uint32_t fall = LineTester::measure_line_change(pin, false, timeout_in_nanos);
        measured_fall_time.include(fall);
        estimated_fall_time.include(std::llround(ScaleMeasuredToEstimate * fall));
    }
    pinMode(pin, INPUT_DISABLE);    // Release the line

    return {measured_fall_time, measured_rise_time, estimated_fall_time, estimated_rise_time};
}
} // line_test