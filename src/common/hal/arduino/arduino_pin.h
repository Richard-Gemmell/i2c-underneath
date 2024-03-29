// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_HAL_ARDUINO_ARDUINO_PIN_H
#define I2C_UNDERNEATH_COMMON_HAL_ARDUINO_ARDUINO_PIN_H

#include <cstdint>
#include "common/hal/pin.h"

namespace common {
namespace hal {

class ArduinoPin : public common::hal::Pin {
private:
    uint8_t pin_;
    bool first_interrupt = true;
    bool high = false;
    bool on_edge_callback_registered_ = false;
    void(*on_edge_isr_)() = nullptr;
    std::function<void(bool rising)> on_edge_callback_;
    void remove_callback();

public:
    // Assumes 'pin' is configured as INPUT_PULLUP or equivalent
    explicit ArduinoPin(uint8_t pin);

    ~ArduinoPin() override;

    void write_pin(bool float_high) override;

    bool read_line() override;

    // You must call 'set_on_edge_isr' before calling this
    // otherwise this call will be ignored.
    void on_edge(const std::function<void(bool rising)>& callback) override;

    // 'on_edge_isr' must call 'raise_on_edge'. This is necessary
    // because Interrupt Service Routines must be static
    void set_on_edge_isr(void(*on_edge_isr)());

    // Fires the callback registered with 'on_edge'.
    inline void raise_on_edge() {
        // Avoid calling digitalReadFast() for every interrupt as it's 20 nanos slower.
        if(first_interrupt) {
            high = digitalReadFast(pin_);
            first_interrupt = false;
        } else {
            high = !high;
        }
        on_edge_callback_(high);
    }
};

}
}

#endif //I2C_UNDERNEATH_COMMON_HAL_ARDUINO_ARDUINO_PIN_H
