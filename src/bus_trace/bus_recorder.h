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
