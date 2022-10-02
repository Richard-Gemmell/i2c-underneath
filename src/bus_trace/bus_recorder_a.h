// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_RECORDER_A_H
#define I2C_UNDERNEATH_BUS_RECORDER_A_H

#include "bus_trace.h"
#include <common/hal/pin.h>
#include <common/hal/clock.h>

namespace bus_trace {
#define NUM_I2C_PORTS 4

// Records the electrical activity on an I2C bus.
// The recorder is driven by interrupts. It will continue
// recording while the processor performs other work.
//
// Other interrupts can also affect the results. It's best to avoid
// having any when recording.
//
// This implementation should work with other Arduino compatible
// devices. It was tested on a Teensy 4.
//
// WARNING
// The BusRecorder can't measure the time between 2 edges that happen
// in quick succession. This happens when the 2nd edge occurs whilst
// the 1st edge is being recorded. On the Teensy 4 this means that
// the smallest recorded interval is about 64 nanoseconds.
//
// WARNING 2
// On the Teensy 4, edges that are less than 40 nanoseconds apart may
// be recorded in the wrong order. i.e. the trace says that SCL changed
// before SDA when the opposite happened. This is a side effect of the
// way the Teensy 4 fires interrupt service routines. The Teensy only
// seems to check for interrupts every so often. If both lines have changed
// state in this time it always fires the ISR for one pin before the other.
// Swapping the pin assignments round helps because it's usually SCL that
// changes before SDA not the other way round.
class BusRecorderA {
public:
    BusRecorderA(uint8_t pin_sda, uint8_t pin_scl)
        :pin_sda(pin_sda), pin_scl(pin_scl) {
    }

    // Sets up the interrupt service routines (ISR) on the pins.
    // The easiest way to do this is to create a static BusRecorder called
    // 'bus_recorder'. This line of code will then setup the callbacks correctly.
    // bus_recorder.set_callbacks([](){bus_recorder.add_event(false);}, [](){bus_recorder.add_event(true);});
    void set_callbacks(void (*on_sda_changed)(), void (*on_scl_changed)());

    // Stops any recording that's in progress and then
    // starts recording. Bus events are added to 'trace'.
    // The recording stops automatically when the trace is full.
    void start(BusTrace& trace);

    // Stops recording
    void stop();

    // Returns true if we're recording
    bool is_recording() const;

    // Adds an event to the trace. DON'T call this method directly.
    // Use set_callbacks() to fire it automatically when the pins
    // detect a rising or falling edge.
    inline void add_event(bool scl) {
        if(!current_trace) return;
        bus_trace::BusEventFlags flags;
        if(scl) {
            line_states = (line_states ^ bus_trace::BusEventFlags::SCL_LINE_STATE);
            flags = line_states | bus_trace::BusEventFlags::SCL_LINE_CHANGED;
        } else {
            line_states = (line_states ^ bus_trace::BusEventFlags::SDA_LINE_STATE);
            flags = line_states | bus_trace::BusEventFlags::SDA_LINE_CHANGED;
        }
        current_trace->add_event(flags);
    }

private:
    uint8_t pin_sda;
    uint8_t pin_scl;
    void (*sda_isr)() = nullptr;
    void (*scl_isr)() = nullptr;
    BusTrace* current_trace = nullptr;
    BusEventFlags line_states = BusEventFlags::BOTH_LOW_AND_UNCHANGED;

    // We need to reduce the priority of the I2C interrupts to record an accurate trace.
    // ANY other interrupts can cause problems but I2C is the obvious one.
    uint8_t original_i2c_irq_priorities[NUM_I2C_PORTS] = {};

    // Drops the priority of all I2C interrupts, so they're lower than the GPIO pins.
    void reduce_i2c_irq_priorities();

    // Reset the I2C IRQ priorities to whatever they were initially
    void reset_i2c_irq_priorities();
};

} // bus_trace

#endif //I2C_UNDERNEATH_BUS_RECORDER_A_H
