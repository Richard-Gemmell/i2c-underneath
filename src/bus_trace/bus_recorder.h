// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_RECORDER_H
#define I2C_UNDERNEATH_BUS_RECORDER_H

#include <cstdint>
#include "bus_trace.h"
#include "common/hal/teensy/teensy_pin.h"

namespace bus_trace {

// Records the electrical activity on an I2C bus.
// The recorder is driven by interrupts. It will continue
// recording while the processor performs other work.
// It may affect other tasks if they're time sensitive.
//
// CONFIGURATION
// This version of BusRecorder requires that you pass a matched
// pair of pins to the constructor. The pins are valid if:
//   getSlowIRQ(pin_sda) == getSlowIRQ(pin_scl)
// start() will return an error code if the combination is not valid.
// You can take any pair of pins from one of these lists:
//   (0, 1, 24, 25)
//   (14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 26, 27, 38, 39, 40, 41)
//   (6, 9, 10, 11, 12, 13, 32)
//   (7, 8, 34, 35, 36, 37)
//   (42, 43, 44, 45)
//   (28, 30, 31, 46, 47)
//   (2, 3, 4, 5, 33)
//   (29, 48, 49, 50, 51, 52, 53, 54)
//
// WARNING
// The BusRecorder records edges when a GPIO interrupt fires.
// It takes about 130 nanoseconds to handle the interrupt.
// Let us call this time T.
// The BusRecorder will record edges reliably as long as gap
// between them is greater than T.
//
// If both lines change state within T nanos then they may be
// recorded as if they happened simultaneously.
// At most 2 edges will be recorded for a single line in this
// time. If a line toggles repeatedly before settling on a new
// state then the extra edges are lost.
//
// The recorder can record glitches: short pulses taking less than
// T nanos. It can record simultaneous glitches on both lines.
// It won't detect a glitch on one line if it's recording a single
// transition on the other line. This is for performance reasons.
// Recording glitches takes 200 nanoseconds. 70 nanos longer than it
// take to record a transition.
//
// Recording a 1 MHz I2C transaction will take roughly 1/2 of
// the Teensy's clock cycles.
class BusRecorder {
public:
    BusRecorder(uint8_t pin_sda, uint8_t pin_scl)
        : sda_mask(getPortBitmask(pin_sda)), scl_mask(getPortBitmask(pin_scl)),
          masks(sda_mask | scl_mask),
          gpio(getSlowGPIO(pin_sda)), fastGpio(getGPIO(pin_sda)),
          irq(getSlowIRQ(pin_sda)), irq_scl(getSlowIRQ(pin_scl)) {
    }

    // Sets up the interrupt service routine (ISR).
    // The easiest way to do this is to create a static BusRecorder called
    // 'recorder'. This line of code will then set up the callbacks correctly.
    // recorder.set_callback([]() { recorder.add_event(); });
    void set_callback(void (* on_change)());

    // Stops any recording that's in progress and then starts a new recording.
    // Bus events are added to 'trace' as long as there's room. Events are
    // dropped silently when 'trace' is full.
    //
    // Returns false if the recorder can't start. This only happens if the
    // BusRecorder has been configured incorrectly. The failure reason is
    // printed to Serial.
    bool start(BusTrace& trace);

    // Stops recording
    void stop();

    // Returns true if we're recording
    bool is_recording() const;

    // Adds an event to the trace. DON'T call this method directly.
    // Use set_callbacks() to fire it automatically when the pins
    // detect a rising or falling edge.
    inline void add_event() {
        // Get the timestamp as soon as possible
        // The edge that triggered this interrupt happened about 44 nanos before this
        // unless it occurred during the previous interrupt.
        uint32_t timestamp = ARM_DWT_CYCCNT;

        // It's much faster to use the fast GPIO port to read the pins.
        const uint32_t pin_states = fastGpio->PSR & masks;

        // Clear the interrupt flags
        if (pin_states != previous_pin_states) {
            // Assume there wasn't a glitch on either line

            // Clear the interrupt
            gpio->ISR = masks;

            // We don't want to record this event.
            if (!current_trace) return;

            // If both pins have changed then report them in a single event.
            // We don't know which one happened first anyway.
            BusEventFlags previous_line_states = line_states;
            setLineStates(pin_states);
            auto changed_flags = (BusEventFlags)((line_states ^ previous_line_states) << 2);
            current_trace->add_event(timestamp, changed_flags | line_states);
        } else {
            // A line has glitched. i.e. changed state and then change back
            // Can be caused by noise or by the master handing control to the slave
            // ISR tells us which line triggered the interrupt
            uint32_t interrupt_pins = gpio->ISR;

            // Clear the interrupt
            gpio->ISR = masks;

            // We don't want to record this event.
            if (!current_trace) return;

            const BusEventFlags glitch_lines = pin_states_to_line_states(interrupt_pins);
            const BusEventFlags glitch_line_states = glitch_lines ^ line_states;
            auto changed_flags = (BusEventFlags)(glitch_lines << 2);
            current_trace->add_event(timestamp, changed_flags | glitch_line_states);
            current_trace->add_event(timestamp, changed_flags | line_states);
        }
        previous_pin_states = pin_states;
        // WARNING: If the ISR exits too soon after clearing gpio->ISR then it'll fire again immediately
    }

private:
    const uint32_t sda_mask;
    const uint32_t scl_mask;
    const uint32_t masks;
    IMXRT_GPIO_t* const gpio;
    IMXRT_GPIO_t* const fastGpio;
    const IRQ_NUMBER_t irq;
    const IRQ_NUMBER_t irq_scl;

    void (* isr)() = nullptr;

    BusTrace* current_trace = nullptr;
    BusEventFlags line_states = BOTH_LOW_AND_UNCHANGED;
    uint32_t previous_pin_states = 0;

    void attach_gpio_interrupt();

    void detach_gpio_interrupt();

    inline void setLineStates(uint32_t pin_states) {
        line_states = pin_states_to_line_states(pin_states);
    }

    inline BusEventFlags pin_states_to_line_states(uint32_t pin_states) const {
        // Ensure the code takes the same time to execute irrespective of which line is set
        // This reduces jitter in the bus timings.
        BusEventFlags sda = (pin_states & sda_mask) ? SDA_LINE_STATE : BOTH_LOW_AND_UNCHANGED;
        BusEventFlags scl = (pin_states & scl_mask) ? SCL_LINE_STATE : BOTH_LOW_AND_UNCHANGED;
        return sda | scl;
    }
};

} // bus_trace

#endif //I2C_UNDERNEATH_BUS_RECORDER_H
