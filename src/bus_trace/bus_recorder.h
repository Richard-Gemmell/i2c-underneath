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
// The BusRecorder records edges when a GPIO interrupts fires.
// The interrupt service routine takes about 150 nanoseconds
// to record each edge.
//
// The main consequence of this is that if a line changes value more
// than once in the time it takes the ISR to finish then only one or
// two edges will be recorded. There's no way for the system to detect
// 5 rapid line changes in a row for example.
//
// WARNING 2
// If both lines change in the same ISR cycle then the recorder can't tell
// which one changed first. It just assumes that SCL changed before SDA
// as this common in a working I2C.
class BusRecorder {
public:
    BusRecorder(uint8_t pin_sda, uint8_t pin_scl)
        : sda_mask(getPortBitmask(pin_sda)), scl_mask(getPortBitmask(pin_scl)),
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
        if(!current_trace) return;

        // It's much faster to use the fast GPIO port to read the pins.
        const uint32_t pin_states = fastGpio->PSR;

        // Clear the interrupt flags
        gpio->ISR = sda_mask | scl_mask;

        // If both pins have changed then we don't know what order they happened.
        // In I2C it's common for SDA to fall immediately after SCL, so assume SCL
        // changed first.
        // TODO: If we don't know the order then raise 1 event instead of 2

        // Handle SCL interrupt
        const uint32_t changed_pins = pin_states ^ previous_pin_states;
        if(changed_pins & scl_mask) {
            if(pin_states & scl_mask) {
                line_states = line_states | bus_trace::BusEventFlags::SCL_LINE_STATE;
            } else {
                line_states = line_states & bus_trace::BusEventFlags::SDA_LINE_STATE;
            }
            current_trace->add_event(line_states | bus_trace::BusEventFlags::SCL_LINE_CHANGED);
        }

        // Handle SDA interrupt
        if(changed_pins & sda_mask) {
            if(pin_states & sda_mask) {
                line_states = line_states | bus_trace::BusEventFlags::SDA_LINE_STATE;
            } else {
                line_states = line_states & bus_trace::BusEventFlags::SCL_LINE_STATE;
            }
            current_trace->add_event(line_states | bus_trace::BusEventFlags::SDA_LINE_CHANGED);
        }
        previous_pin_states = pin_states;
    }

private:
    const uint32_t sda_mask;
    const uint32_t scl_mask;
    IMXRT_GPIO_t* const gpio;
    IMXRT_GPIO_t* const fastGpio;
    const IRQ_NUMBER_t irq;
    const IRQ_NUMBER_t irq_scl;
    void (*isr)() = nullptr;
    BusTrace* current_trace = nullptr;
    uint32_t previous_pin_states = 0;
    BusEventFlags line_states = BusEventFlags::BOTH_LOW_AND_UNCHANGED;

    void attach_gpio_interrupt();
    void detach_gpio_interrupt();
};

} // bus_trace

#endif //I2C_UNDERNEATH_BUS_RECORDER_H
