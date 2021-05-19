// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#include "teensy_timer.h"

common::hardware::TeensyTimer::~TeensyTimer() {
    timer_.end();
}

void common::hardware::TeensyTimer::set_timer_isr(void (* isr)()) {
    timer_isr_ = isr;
}

void common::hardware::TeensyTimer::raise_timer_event() {
    timer_callback_();
}

void common::hardware::TeensyTimer::begin_micros(const std::function<void()>& callback, uint32_t period) {
    if (timer_isr_) {
        period_ = period;
        timer_callback_ = callback;
        timer_.begin(timer_isr_, period);
    } else {
        Serial.println("TeensyTimer: You must call set_timer_isr() before calling begin_micros()");
    }
}

void common::hardware::TeensyTimer::restart() {
    timer_.begin(timer_isr_, period_);
}

void common::hardware::TeensyTimer::end() {
    timer_.end();
}

