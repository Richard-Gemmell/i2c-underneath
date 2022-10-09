// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_BUS_RECORDER_H
#define I2C_UNDERNEATH_BUS_RECORDER_H

#include <cstdint>
#include "bus_trace.h"
#include "common/hal/teensy/teensy_pin.h"

namespace bus_trace {

class BusRecorder {
public:
    BusRecorder(uint8_t pin_sda, uint8_t pin_scl)
        : sda_mask(getPortBitmask(pin_sda)), scl_mask(getPortBitmask(pin_scl)),
          gpio(getSlowGPIO(pin_sda)),
          irq(getSlowIRQ(pin_sda)), irq_scl(getSlowIRQ(pin_scl)) {
    }

    // Sets up the interrupt service routines (ISR) on the pins.
    // The easiest way to do this is to create a static BusRecorder called
    // 'bus_recorder'. This line of code will then setup the callbacks correctly.
    // bus_recorder.set_callbacks([](){bus_recorder.add_event(false);}, [](){bus_recorder.add_event(true);});
    void set_callbacks(void (*on_sda_changed)(), void (*on_scl_changed)());

    // Stops any recording that's in progress and then
    // starts recording. Bus events are added to 'trace'.
    // The recording stops automatically when the trace is full.
    // Returns false if the recorder won't start because the pins
    // belong to different GPIO blocks. Try a different pair of pins.
    bool start(BusTrace& trace);

    // Stops recording
    void stop();

    // Returns true if we're recording
    bool is_recording() const;

    // Adds an event to the trace. DON'T call this method directly.
    // Use set_callbacks() to fire it automatically when the pins
    // detect a rising or falling edge.
    inline void add_event(bool scl) {
        if(!current_trace) return;
        uint32_t isr = gpio->ISR;
        uint32_t pin_states = gpio->PSR;
        // Clear the interrupt flags
        gpio->ISR = sda_mask | scl_mask;

        // If both pins have changed then we don't know what order they happened.
        // In I2C it's common for SDA to fall immediately after SCL, so assume SCL
        // changed first.

        // Handle SCL interrupt
        if(isr & scl_mask) {
            if(pin_states & scl_mask) {
                line_states = line_states | bus_trace::BusEventFlags::SCL_LINE_STATE;
            } else {
                line_states = line_states & bus_trace::BusEventFlags::SDA_LINE_STATE;
            }
            current_trace->add_event(line_states | bus_trace::BusEventFlags::SCL_LINE_CHANGED);
        }

        // Handle SDA interrupt
        if(isr & sda_mask) {
            if(pin_states & sda_mask) {
                line_states = line_states | bus_trace::BusEventFlags::SDA_LINE_STATE;
            } else {
                line_states = line_states & bus_trace::BusEventFlags::SCL_LINE_STATE;
            }
            current_trace->add_event(line_states | bus_trace::BusEventFlags::SDA_LINE_CHANGED);
        }
    }

private:
    const uint32_t sda_mask;
    const uint32_t scl_mask;
    IMXRT_GPIO_t* const gpio;
    const IRQ_NUMBER_t irq;
    const IRQ_NUMBER_t irq_scl;
    void (*sda_isr)() = nullptr;
    void (*scl_isr)() = nullptr;
    BusTrace* current_trace = nullptr;
    BusEventFlags line_states = BusEventFlags::BOTH_LOW_AND_UNCHANGED;

    void attach_gpio_interrupt(void (*isr)());
    void detach_gpio_interrupt();
};

} // bus_trace

#endif //I2C_UNDERNEATH_BUS_RECORDER_H
