// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_FAKES_COMMON_HARDWARE_FAKE_PIN_H
#define I2C_UNDERNEATH_FAKES_COMMON_HARDWARE_FAKE_PIN_H

#include <common/hardware/pin.h>

namespace common {
namespace hardware {

class FakePin : public common::hardware::Pin {
public:
    ~FakePin() override {
        on_edge_callback_ = nullptr;
    };

    void write_pin(bool float_high) override {
        line_high_ = float_high;
        if (on_edge_callback_) {
            on_edge_callback_(line_high_);
        }
    }

    bool read_line() override {
        return line_high_;
    }

    void on_edge(const std::function<void(bool)>& callback) override {
        on_edge_callback_ = callback;
    }

private:
    bool line_high_ = true;
    std::function<void(bool rising)> on_edge_callback_ = nullptr;
};

}
}
#endif //I2C_UNDERNEATH_FAKES_COMMON_HARDWARE_FAKE_PIN_H
