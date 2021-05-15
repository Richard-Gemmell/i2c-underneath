// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_HARDWARE_PIN_H
#define I2C_UNDERNEATH_COMMON_HARDWARE_PIN_H
namespace common {
namespace hardware {

// Controls a single Pin
// Different concrete implementations of this class support different platforms.
class Pin {
public:
    virtual ~Pin() = 0;

    // write() isn't a good word because it's open-drain
    // We can only pull the line down or release it
    // pulldown/release
    virtual void write(bool float_high) = 0;

    // True if the line is HIGH and false if the line is LOW.
    // Note that this returns the state of the line, not the pin.
    // It's common for the line to be LOW when the pin is HIGH.
    virtual bool read_line() = 0;

    // True if the pin is floating. The line will be HIGH if all
    // connected pins are floating. False if this pin is shorting
    // the line to GND.
    virtual bool floating() = 0;

    // Registers a callback that will be called when the line changes
    // value.
    virtual void on_edge(std::function<void(bool rising)> callback) = 0;
};

}
}
#endif //I2C_UNDERNEATH_COMMON_HARDWARE_PIN_H
