// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "bus_recorder.h"

namespace bus_trace {

void bus_trace::BusRecorder::set_callbacks(void (*on_sda_changed)(), void (*on_scl_changed)()) {
    // TODO: We need only one callback
    this->sda_isr = on_sda_changed;
    this->scl_isr = on_scl_changed;
}

bool BusRecorder::start(BusTrace& trace) {
    if(irq_scl != irq) {
        Serial.println("ERROR: Cannot start BusRecorder. SDA and SCL pins have different interrupt blocks.");
        return false;
    }

    if (!(sda_isr && scl_isr)) {
        Serial.println("ERROR: Cannot start BusRecorder. You must call set_callbacks() before start()");
        return false;
    }

    current_trace = &trace;
    noInterrupts()
    attach_gpio_interrupt(this->scl_isr);
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

void BusRecorder::attach_gpio_interrupt(void (*isr)()) {
    uint32_t mask = sda_mask | scl_mask;
    gpio->IMR &= ~(mask);	// disable interrupt while we fiddle with it

    // Wire up the interrupt service routine
    attachInterruptVector(irq, isr);
    NVIC_ENABLE_IRQ(irq);

    // Configure the interrupt for this pin
    gpio->EDGE_SEL |= mask; // Either edge will trigger the interrupt
    gpio->ISR = mask;       // Clear the pending interrupt if there is one
    gpio->IMR |= mask;      // Enable the interrupt
}

void BusRecorder::detach_gpio_interrupt() {
    uint32_t mask = sda_mask | scl_mask;
    gpio->IMR &= ~mask;
    NVIC_DISABLE_IRQ(irq);
}

} // bus_trace