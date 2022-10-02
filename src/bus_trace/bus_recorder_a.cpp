// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "bus_recorder_a.h"

uint8_t I2C_IRQS[NUM_I2C_PORTS] = {IRQ_LPI2C1, IRQ_LPI2C2, IRQ_LPI2C3, IRQ_LPI2C4};

void bus_trace::BusRecorderA::set_callbacks(void (*on_sda_changed)(), void (*on_scl_changed)()) {
    this->sda_isr = on_sda_changed;
    this->scl_isr = on_scl_changed;
}

void bus_trace::BusRecorderA::start(BusTrace& trace) {
    if (!(sda_isr && scl_isr)) {
        Serial.println("You must call set_callbacks() before start()");
        return;
    }
    current_trace = &trace;
    reduce_i2c_irq_priorities();
    noInterrupts()
    attachInterrupt(digitalPinToInterrupt(pin_sda), sda_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pin_scl), scl_isr, CHANGE);
    line_states = update_from_bool(line_states, digitalRead(pin_sda), BusEventFlagBits::SDA_LINE_STATE_BIT);
    line_states = update_from_bool(line_states, digitalRead(pin_scl), BusEventFlagBits::SCL_LINE_STATE_BIT);
    current_trace->reset();
    trace.add_event(line_states);
    interrupts()
}

void bus_trace::BusRecorderA::stop() {
    noInterrupts()
    detachInterrupt(digitalPinToInterrupt(pin_sda));
    detachInterrupt(digitalPinToInterrupt(pin_scl));
    interrupts()
    current_trace = nullptr;
    reset_i2c_irq_priorities();
}

bool bus_trace::BusRecorderA::is_recording() const {
    return current_trace != nullptr;
}

// Drops the priority of all I2C interrupts, so they're lower than the GPIO pins.
void bus_trace::BusRecorderA::reduce_i2c_irq_priorities() {
    uint8_t gpio_priority = NVIC_GET_PRIORITY(IRQ_GPIO6789);
    for (int i = 0; i < NUM_I2C_PORTS; ++i) {
        original_i2c_irq_priorities[i] = NVIC_GET_PRIORITY(I2C_IRQS[i]);
        // Make I2C priority one step lower than the priority for GPIO
        NVIC_SET_PRIORITY(I2C_IRQS[i], gpio_priority + 16);
    }
}

// Reset the I2C IRQ priorities to whatever they were initially
void bus_trace::BusRecorderA::reset_i2c_irq_priorities() {
    for (int i = 0; i < NUM_I2C_PORTS; ++i) {
        NVIC_SET_PRIORITY(I2C_IRQS[i], original_i2c_irq_priorities[i]);
    }
}
