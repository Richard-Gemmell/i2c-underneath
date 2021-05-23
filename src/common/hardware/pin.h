// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_HARDWARE_PIN_H
#define I2C_UNDERNEATH_COMMON_HARDWARE_PIN_H

#include <functional>

namespace common {
namespace hardware {

// Controls a single pin. The pin must be configured to be open-drain.
// This means the pin can pull the line LOW but it cannot force it HIGH.
//
// Different concrete implementations of this class support different platforms.
class Pin {
public:
    // Cancels the callback registered with 'on_edge()'.
    virtual ~Pin() = default;

    // Pulls the pin down if 'float_high' is LOW.
    // Sets the pin to float if 'float_high' is HIGH.
    // The line will remain LOW if another pin is pulling it down.
    virtual void write_pin(bool float_high) = 0;

    // True if the line is HIGH and false if the line is LOW.
    // Note that this returns the state of the line, not the pin.
    // It's common for the line to be LOW when the pin is HIGH.
    virtual bool read_line() = 0;

    // True if the pin is floating. The line will be HIGH if all connected
    // pins are floating. False if this pin is pulling the line down to GND.
//    virtual bool read_pin() = 0;

    // Registers a callback that will be called when the line changes value.
    // 'rising' is true if the line moved from LOW to HIGH and false otherwise
    // Call with 'nullptr' to remove the previously registered callback
    virtual void on_edge(const std::function<void(bool rising)>& callback) = 0;
};

}
}
#endif //I2C_UNDERNEATH_COMMON_HARDWARE_PIN_H
