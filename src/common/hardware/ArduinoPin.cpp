// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "ArduinoPin.h"
#include <Arduino.h>

common::hardware::ArduinoPin::ArduinoPin(uint8_t pin)
    : pin_(pin) {
}

common::hardware::ArduinoPin::~ArduinoPin() {
    if (on_edge_callback_registered) {
        detachInterrupt(digitalPinToInterrupt(pin_));
    }
}

void common::hardware::ArduinoPin::write_pin(bool float_high) {
    digitalWrite(pin_, float_high);
}

bool common::hardware::ArduinoPin::read_line() {
    return digitalReadFast(pin_);
}

void common::hardware::ArduinoPin::on_edge(void (*callback)()) {
    attachInterrupt(digitalPinToInterrupt(pin_), callback, CHANGE);
    on_edge_callback_registered = true;
}
