// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_FAKES_COMMON_HAL_FAKE_PIN_H
#define I2C_UNDERNEATH_FAKES_COMMON_HAL_FAKE_PIN_H

#include <common/hal/pin.h>

namespace common {
namespace hal {

class FakePin : public common::hal::Pin {
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
        read_line_called = true;
        return line_high_;
    }

    void on_edge(const std::function<void(bool)>& callback) override {
        on_edge_called = true;
        on_edge_callback_ = callback;
    }

    bool line_high_ = true;
    bool on_edge_called = false;
    bool read_line_called = false;
    std::function<void(bool rising)> on_edge_callback_ = nullptr;
};

}
}
#endif //I2C_UNDERNEATH_FAKES_COMMON_HAL_FAKE_PIN_H
