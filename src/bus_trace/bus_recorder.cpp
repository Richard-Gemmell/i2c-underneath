// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "bus_recorder.h"

namespace bus_trace {

void bus_trace::BusRecorder::set_callback(void (*on_change)()) {
    this->isr = on_change;
}

bool BusRecorder::start(BusTrace& trace) {
    if(irq_scl != irq) {
        Serial.println("ERROR: Cannot start BusRecorder. SDA and SCL pins have different interrupt blocks.");
        return false;
    }

    if (!(isr)) {
        Serial.println("ERROR: Cannot start BusRecorder. You must call set_callback() before start()");
        return false;
    }

    stop(); // Stop the current recording if there is one.

    // Start a new trace
    current_trace = &trace;

    noInterrupts()
    attach_gpio_interrupt();
    previous_pin_states = fastGpio->PSR;
    line_states = update_from_bool(line_states, previous_pin_states & sda_mask, BusEventFlagBits::SDA_LINE_STATE_BIT);
    line_states = update_from_bool(line_states, previous_pin_states & scl_mask, BusEventFlagBits::SCL_LINE_STATE_BIT);
    current_trace->reset();
    trace.add_event(line_states);
    interrupts()

    return true;
}

void BusRecorder::stop() {
    noInterrupts()
    detach_gpio_interrupt();
    interrupts()
    current_trace = nullptr;
}

bool BusRecorder::is_recording() const {
    return current_trace != nullptr;
}

void BusRecorder::attach_gpio_interrupt() {
    uint32_t mask = sda_mask | scl_mask;
    gpio->IMR &= ~(mask);	// disable interrupts while we fiddle with them

    // Wire up the interrupt service routine
    attachInterruptVector(irq, isr);
    NVIC_ENABLE_IRQ(irq);

    // Configure interrupts for these pins
    gpio->EDGE_SEL |= mask; // Either edge will trigger the interrupts
    gpio->ISR = mask;       // Clear the pending interrupts if there are any
    gpio->IMR |= mask;      // Enable the interrupts
}

void BusRecorder::detach_gpio_interrupt() {
    uint32_t mask = sda_mask | scl_mask;
    gpio->IMR &= ~mask;
    NVIC_DISABLE_IRQ(irq);
}

} // bus_trace