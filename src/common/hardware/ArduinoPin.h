// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_HARDWARE_ARDUINO_PIN_H
#define I2C_UNDERNEATH_HARDWARE_ARDUINO_PIN_H

#include <cstdint>
#include "pin.h"
namespace common {
namespace hardware {

class ArduinoPin : public common::hardware::Pin {
    uint8_t pin_;
    bool on_edge_callback_registered = false;

public:
    // Assumes pin is configured as INPUT_PULLUP or equivalent
    explicit ArduinoPin(uint8_t pin);

    ~ArduinoPin() override;

    void write_pin(bool float_high) override;

    bool read_line() override;

    void on_edge(void (*callback)()) override;
};

}
}

#endif //I2C_UNDERNEATH_HARDWARE_ARDUINO_PIN_H
