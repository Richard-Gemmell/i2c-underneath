// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "ArduinoPin.h"
#include <Arduino.h>

common::hardware::ArduinoPin::ArduinoPin(uint8_t pin)
    : pin_(pin) {
}

common::hardware::ArduinoPin::~ArduinoPin() {
    if (on_edge_callback_registered_) {
        detachInterrupt(digitalPinToInterrupt(pin_));
    }
}

void common::hardware::ArduinoPin::write_pin(bool float_high) {
    digitalWrite(pin_, float_high);
}

bool common::hardware::ArduinoPin::read_line() {
    return digitalReadFast(pin_);
}

void common::hardware::ArduinoPin::on_edge(const std::function<void(bool rising)>& callback) {
    if(this->on_edge_isr_) {
        on_edge_callback = callback;
        attachInterrupt(digitalPinToInterrupt(pin_), this->on_edge_isr_, CHANGE);
        on_edge_callback_registered_ = true;
    }
}

void common::hardware::ArduinoPin::set_on_edge_isr(void (* on_edge_isr)()) {
    on_edge_isr_ = on_edge_isr;
}

void common::hardware::ArduinoPin::raise_on_edge() {
    on_edge_callback(read_line());
}
