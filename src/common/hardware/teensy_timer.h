// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_COMMON_HARDWARE_TEENSYTIMER_H
#define I2C_UNDERNEATH_COMMON_HARDWARE_TEENSYTIMER_H

#include <Arduino.h>
#include "timer.h"

namespace common {
namespace hardware {

class TeensyTimer : public Timer {
public:
    ~TeensyTimer() override;

    void set_timer_isr(void(*isr)());

    void raise_timer_event();

    void begin_micros(const std::function<void()>& callback, uint32_t period) override;

    void restart() override;

    void end() override;

public:
private:
    IntervalTimer timer_;
    void(*timer_isr_)() = nullptr;
    std::function<void()> timer_callback_;
    uint32_t period_ = 0;
};

}
}

#endif //I2C_UNDERNEATH_COMMON_HARDWARE_TEENSYTIMER_H
