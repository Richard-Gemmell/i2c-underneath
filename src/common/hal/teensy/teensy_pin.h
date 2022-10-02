// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_TEENSY_PIN_H
#define I2C_UNDERNEATH_TEENSY_PIN_H

#include <cstdint>
#include "super_fast_io.h"

namespace common {
namespace hal {

// This is a very efficient way to handle pins.
// It's faster or as fast as digitalWriteFast().
class TeensyPin {
public:
    explicit TeensyPin(uint8_t pin, uint8_t mode) {
        pinMode(pin, mode);
        mask = getPortBitmask(pin);
        gpio = (IMXRT_GPIO_t*)(getDigitalReadPort(pin) - 2);
    }

    // Sets the pin HIGH
    void set() {
        gpio->DR_SET = mask;
    }

    // Sets the pin LOW
    void clear() {
        gpio->DR_CLEAR = mask;
    }

    // Changes the value of the pin. This is much faster
    // than reading the pin and then setting it to the opposite
    // of the current value
    void toggle() {
        gpio->DR_TOGGLE = mask;
    }

    // Returns 0 if the pin is LOW and a non-zero value if it's HIGH.
    // (Actually the pin's mask)
    // If the pin is configured as an output then this returns the last
    // value written to the pin.
    uint32_t read() {
        return gpio->DR & mask;
    }

private:
    uint32_t mask;
    IMXRT_GPIO_t* gpio;
};
}
}
#endif //I2C_UNDERNEATH_TEENSY_PIN_H
