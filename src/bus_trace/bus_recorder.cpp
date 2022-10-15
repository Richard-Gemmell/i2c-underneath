// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "bus_recorder.h"

namespace bus_trace {

#define DEFAULT_IRQ_PRIORITY 128

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
    setLineStates(fastGpio->PSR);
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
    gpio->IMR &= ~(masks);	// disable interrupts while we fiddle with them

    // Wire up the interrupt service routine
    attachInterruptVector(irq, isr);
    NVIC_ENABLE_IRQ(irq);
    // Make this IRQ 1 step higher priority than I2C.
    // Assumes the I2C priorities are still at the default value.
    NVIC_SET_PRIORITY(irq, DEFAULT_IRQ_PRIORITY - 16);

    // Configure interrupts for these pins
    gpio->ISR = masks;      // Clear pending interrupts
    gpio->EDGE_SEL |= masks;// Either edge will trigger the interrupts
    gpio->IMR = masks;      // Enable the interrupts
}

void BusRecorder::detach_gpio_interrupt() {
    uint32_t mask = sda_mask | scl_mask;
    gpio->IMR &= ~mask;
    NVIC_DISABLE_IRQ(irq);
    NVIC_SET_PRIORITY(irq, DEFAULT_IRQ_PRIORITY);
}

} // bus_trace