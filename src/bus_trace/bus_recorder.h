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
// It takes about 125 nanoseconds to handle the interrupt.
// Let us call this time T.
// The BusRecorder will record edges reliably as long as gap
// between them is greater than T.
//
// If both lines change state with T nanos then they may be
// recorded as if they happened simultaneously.
// At most 2 edges will be recorded for a single line in this
// time. If a line toggles repeatedly and returns to its initial
// state then there might be no record of the change.
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
    void set_callback(void (*on_change)());

    // Stops any recording that's in progress and then starts a new recording.
    // Bus events are added to 'trace'.
    // The recording stops automatically when the trace is full.
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
        const uint32_t pin_states = fastGpio->PSR;

        // Clear the interrupt flags
        gpio->ISR = masks;

        // We don't want to record this event.
        if(!current_trace) return;

        // If both pins have changed then report them in a single event.
        // We don't know which one happened first anyway.
        BusEventFlags previous_line_states = line_states;
        setLineStates(pin_states);
        auto changed_flags = (BusEventFlags)((line_states ^ previous_line_states) << 2);
        current_trace->add_event(timestamp, changed_flags | line_states);

        // WARNING: If the ISR takes less than about 125 nanos then it'll fire twice.
    }

private:
    const uint32_t sda_mask;
    const uint32_t scl_mask;
    const uint32_t masks;
    IMXRT_GPIO_t* const gpio;
    IMXRT_GPIO_t* const fastGpio;
    const IRQ_NUMBER_t irq;
    const IRQ_NUMBER_t irq_scl;
    void (*isr)() = nullptr;
    BusTrace* current_trace = nullptr;
    BusEventFlags line_states = BusEventFlags::BOTH_LOW_AND_UNCHANGED;

    void attach_gpio_interrupt();
    void detach_gpio_interrupt();

    inline void setLineStates(uint32_t pin_states) {
        // Ensure the code takes the same time to execute irrespective of which line is set
        // This reduces jitter in the bus timings.
        auto sda = (pin_states & sda_mask) ? BusEventFlags::SDA_LINE_STATE : BusEventFlags::BOTH_LOW_AND_UNCHANGED;
        auto scl = (pin_states & scl_mask) ? BusEventFlags::SCL_LINE_STATE : BusEventFlags::BOTH_LOW_AND_UNCHANGED;
        line_states = sda | scl;
    }
};

} // bus_trace

#endif //I2C_UNDERNEATH_BUS_RECORDER_H
