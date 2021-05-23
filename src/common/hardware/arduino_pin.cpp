// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include <Arduino.h>
#include "arduino_pin.h"

common::hardware::ArduinoPin::ArduinoPin(uint8_t pin)
    : pin_(pin) {
}

common::hardware::ArduinoPin::~ArduinoPin() {
    remove_callback();
}

void common::hardware::ArduinoPin::write_pin(bool float_high) {
    digitalWrite(pin_, float_high);
}

bool common::hardware::ArduinoPin::read_line() {
    return digitalReadFast(pin_);
}

void common::hardware::ArduinoPin::on_edge(const std::function<void(bool)>& callback) {
    if(on_edge_isr_) {
        if (callback) {
            on_edge_callback_ = callback;
            attachInterrupt(digitalPinToInterrupt(pin_), on_edge_isr_, CHANGE);
            on_edge_callback_registered_ = true;
        } else {
            remove_callback();
        }
    } else {
        Serial.println("ArduinoPin: You must call set_on_edge_isr() before calling on_edge()");
    }
}

void common::hardware::ArduinoPin::set_on_edge_isr(void (* on_edge_isr)()) {
    on_edge_isr_ = on_edge_isr;
}

void common::hardware::ArduinoPin::raise_on_edge() {
    on_edge_callback_(read_line());
}

void common::hardware::ArduinoPin::remove_callback() {
    if (on_edge_callback_registered_) {
        detachInterrupt(digitalPinToInterrupt(pin_));
        on_edge_callback_ = nullptr;
        on_edge_callback_registered_ = false;
    }
}
